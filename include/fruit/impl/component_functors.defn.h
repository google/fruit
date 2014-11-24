/*
 * Copyright 2014 Google Inc. All rights reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 * 
 *     http://www.apache.org/licenses/LICENSE-2.0
 * 
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef FRUIT_COMPONENT_FUNCTORS_DEFN_H
#define FRUIT_COMPONENT_FUNCTORS_DEFN_H

#include "../component.h"

#include "injection_errors.h"
#include "storage/component_storage.h"

#include <memory>

namespace fruit {

namespace impl {

template <typename Comp, typename TargetRequirements, typename L>
struct EnsureProvidedTypes;

template <typename Comp>
struct Identity {
  using Result = Comp;
  void operator()(ComponentStorage& storage) {
    (void)storage;
  }
};

// Doesn't actually bind in ComponentStorage. The binding is added later (if needed) using BindNonFactory.
template <typename Comp, typename I, typename C>
struct Bind {
  using Result = ConsComp<typename Comp::Rs,
                          typename Comp::Ps,
                          typename Comp::Deps,
                          Apply<AddToSet,
                                ConsBinding<I, C>,
                                typename Comp::Bindings>>;
  void operator()(ComponentStorage& storage) {
    (void)storage;
  };
};

template <typename Comp, typename I, typename C>
struct BindNonFactory {
  FruitDelegateCheck(NotABaseClassOf<I, C>);
  using Result = Apply<AddProvidedType, Comp, I, List<C>>;
  void operator()(ComponentStorage& storage) {
    storage.addBinding(InjectorStorage::createBindingDataForBind<I, C>());
  };
};

template <typename Comp, typename I, typename C>
struct AddMultibinding {
  FruitDelegateCheck(NotABaseClassOf<I, C>);
  using Result = Apply<AddRequirements, Comp, List<C>>;
  void operator()(ComponentStorage& storage) {
    storage.addMultibinding(InjectorStorage::createMultibindingDataForBinding<I, C>());
  };
};

template <typename Lambda, typename OptionalI>
struct RegisterProviderHelper {
  inline void operator()(ComponentStorage& component) {
    component.addBinding(InjectorStorage::createBindingDataForProvider<Lambda>());
    component.addCompressedBinding(InjectorStorage::createBindingDataForCompressedProvider<Lambda, OptionalI>());
  }
};

template <typename Lambda>
struct RegisterProviderHelper<Lambda, None> {
  inline void operator()(ComponentStorage& component) {
    component.addBinding(InjectorStorage::createBindingDataForProvider<Lambda>());
  }
};


// T can't be any injectable type, it must match the return type of the provider in one of
// the registerProvider() overloads in ComponentStorage.
template <typename Comp, typename Lambda>
struct RegisterProvider {
  using Signature = Apply<FunctionSignature, Lambda>;
  using C = Apply<GetClassForType, Apply<SignatureType, Signature>>;
  using Result = Apply<AddProvidedType,
                       Comp,
                       C,
                       Apply<SignatureArgs, Signature>>;
  void operator()(ComponentStorage& storage) {
    RegisterProviderHelper<Lambda, Apply<GetReverseBinding, C, typename Comp::Bindings>>()(storage);
  }
};

// T can't be any injectable type, it must match the return type of the provider in one of
// the registerMultibindingProvider() overloads in ComponentStorage.
template <typename Comp, typename Lambda>
struct RegisterMultibindingProvider {
  using Result = Apply<AddRequirements, Comp, Apply<SignatureArgs, Apply<FunctionSignature, Lambda>>>;
  void operator()(ComponentStorage& storage) {
    storage.addMultibinding(InjectorStorage::createMultibindingDataForProvider<Lambda>());
  }
};

// Non-assisted case.
template <int numAssistedBefore, int numNonAssistedBefore, typename Arg, typename InjectedArgsTuple, typename UserProvidedArgsTuple>
struct GetAssistedArgHelper {
  inline Arg operator()(InjectedArgsTuple& injected_args, UserProvidedArgsTuple&) {
    return std::get<numNonAssistedBefore>(injected_args);
  }
};

// Assisted case.
template <int numAssistedBefore, int numNonAssistedBefore, typename Arg, typename InjectedArgsTuple, typename UserProvidedArgsTuple>
struct GetAssistedArgHelper<numAssistedBefore, numNonAssistedBefore, Assisted<Arg>, InjectedArgsTuple, UserProvidedArgsTuple> {
  inline Arg operator()(InjectedArgsTuple&, UserProvidedArgsTuple& user_provided_args) {
    return std::get<numAssistedBefore>(user_provided_args);
  }
};

template <int index, typename AnnotatedArgs, typename InjectedArgsTuple, typename UserProvidedArgsTuple>
struct GetAssistedArg : public GetAssistedArgHelper<NumAssistedBefore<index, AnnotatedArgs>::value,
                                                    index - NumAssistedBefore<index, AnnotatedArgs>::value,
                                                    GetNthType<index, AnnotatedArgs>,
                                                    InjectedArgsTuple,
                                                    UserProvidedArgsTuple> {};

template <typename Comp,
          typename AnnotatedSignature,
          typename Lambda,
          typename InjectedSignature = Apply<InjectedSignatureForAssistedFactory, AnnotatedSignature>,
          typename RequiredSignature = Apply<RequiredSignatureForAssistedFactory, AnnotatedSignature>,
          typename InjectedArgs = Apply<RemoveAssisted, Apply<SignatureArgs, AnnotatedSignature>>,
          typename IndexSequence = GenerateIntSequence<
              ApplyC<ListSize,
                  Apply<RequiredArgsForAssistedFactory, AnnotatedSignature>
              >::value>
          >
struct RegisterFactory;

template <typename Comp, typename AnnotatedSignature, typename Lambda, typename C, typename... UserProvidedArgs, typename... AllArgs, typename... InjectedArgs, int... indexes>
struct RegisterFactory<Comp, AnnotatedSignature, Lambda, C(UserProvidedArgs...), C(AllArgs...), List<InjectedArgs...>, IntList<indexes...>> {
  using T = Apply<SignatureType, AnnotatedSignature>;
  using InjectedSignature = C(UserProvidedArgs...);
  using RequiredSignature = C(AllArgs...);
  FruitDelegateCheck(FunctorSignatureDoesNotMatch<RequiredSignature, Apply<FunctionSignature, Lambda>>);
  FruitDelegateCheck(FactoryReturningPointer<std::is_pointer<T>::value, AnnotatedSignature>);
  using fun_t = std::function<InjectedSignature>;
  using Result = Apply<AddProvidedType,
                       Comp,
                       fun_t,
                       Apply<SignatureArgs, AnnotatedSignature>>;
  void operator()(ComponentStorage& storage) {
    auto function_provider = [](InjectedArgs... args) {
      // TODO: Using auto and make_tuple here results in a GCC segfault with GCC 4.8.1.
      // Check this on later versions and consider filing a bug.
      std::tuple<InjectedArgs...> injected_args(args...);
      auto object_provider = [injected_args](UserProvidedArgs... params) mutable {
        auto user_provided_args = std::tie(params...);
        // These are unused if they are 0-arg tuples. Silence the unused-variable warnings anyway.
        (void) injected_args;
        (void) user_provided_args;
        
        return LambdaInvoker::invoke<Lambda, AllArgs...>(GetAssistedArg<indexes,
                                                            Apply<SignatureArgs, AnnotatedSignature>,
                                                            decltype(injected_args),
                                                            decltype(user_provided_args)
                                                        >()(injected_args, user_provided_args)
                                                        ...);
      };
      return fun_t(object_provider);
    };  
    storage.addBinding(InjectorStorage::createBindingDataForProvider<decltype(function_provider)>());
  }
};

template <typename Comp, typename Signature>
struct RegisterConstructor {
  // Something is wrong. We provide a Result and an operator() here to avoid backtracking with SFINAE, and to allow the function that
  // instantiated this class to return the appropriate error.
  // This method is not implemented.
  using Result = int;
  int operator()(Comp&& m);
};

template <typename Signature, typename OptionalI>
struct RegisterConstructorHelper {
  inline void operator()(ComponentStorage& component) {
    component.addBinding(InjectorStorage::createBindingDataForConstructor<Signature>());
    component.addCompressedBinding(InjectorStorage::createBindingDataForCompressedConstructor<Signature, OptionalI>());
  }
};

template <typename Signature>
struct RegisterConstructorHelper<Signature, None> {
  inline void operator()(ComponentStorage& component) {
    component.addBinding(InjectorStorage::createBindingDataForConstructor<Signature>());
  }
};

template <typename Comp, typename T, typename... Args>
struct RegisterConstructor<Comp, T(Args...)> {
  using C = Apply<GetClassForType, T>;
  using Result = Apply<AddProvidedType, Comp, C, List<Args...>>;
  void operator()(ComponentStorage& storage) {
    RegisterConstructorHelper<T(Args...), Apply<GetReverseBinding, C, typename Comp::Bindings>>()(storage);
  }
};

template <typename Comp, typename C>
struct RegisterInstance {
  using Result = Apply<AddProvidedType, Comp, C, List<>>;
  void operator()(ComponentStorage& storage, C& instance) {
    storage.addBinding(InjectorStorage::createBindingDataForBindInstance<C>(instance));
  };
};

template <typename Comp, typename C>
struct AddInstanceMultibinding {
  using Result = Comp;
  void operator()(ComponentStorage& storage, C& instance) {
    storage.addMultibinding(InjectorStorage::createMultibindingDataForInstance<C>(instance));
  };
};

template <typename Comp, typename AnnotatedSignature, typename RequiredSignature>
struct RegisterConstructorAsValueFactoryHelper {};

template <typename Comp, typename AnnotatedSignature, typename T, typename... Args>
struct RegisterConstructorAsValueFactoryHelper<Comp, AnnotatedSignature, T(Args...)> {
  void operator()(ComponentStorage& storage) {
    auto provider = [](Args... args) {
      return T(std::forward<Args>(args)...);
    };
    using RealRegisterFactoryOperation = RegisterFactory<Comp, AnnotatedSignature, decltype(provider)>;
    RealRegisterFactoryOperation()(storage);
  }
};

template <typename Comp, typename AnnotatedSignature>
struct RegisterConstructorAsValueFactory {
  using RequiredSignature = Apply<ConstructSignature,
                                  Apply<SignatureType, AnnotatedSignature>,
                                  Apply<RequiredArgsForAssistedFactory, AnnotatedSignature>>;
  using RegisterFactoryOperation = RegisterFactory<Comp, AnnotatedSignature, RequiredSignature>;
  using Result = typename RegisterFactoryOperation::Result;
  void operator()(ComponentStorage& storage) {
    RegisterConstructorAsValueFactoryHelper<Comp, AnnotatedSignature, RequiredSignature>()(storage);
  };
};

template <typename Comp, typename AnnotatedSignature, typename RequiredSignature>
struct RegisterConstructorAsPointerFactoryHelper {};

template <typename Comp, typename AnnotatedSignature, typename T, typename... Args>
struct RegisterConstructorAsPointerFactoryHelper<Comp, AnnotatedSignature, std::unique_ptr<T>(Args...)> {
  void operator()(ComponentStorage& storage) {
    auto provider = [](Args... args) {
      return std::unique_ptr<T>(new T(std::forward<Args>(args)...));
    };
    using RealRegisterFactoryOperation = RegisterFactory<Comp, AnnotatedSignature, decltype(provider)>;
    RealRegisterFactoryOperation()(storage);
  }
};

template <typename Comp, typename AnnotatedSignature>
struct RegisterConstructorAsPointerFactory {
  using RequiredSignature = Apply<ConstructSignature,
                                  std::unique_ptr<Apply<SignatureType, AnnotatedSignature>>,
                                  Apply<RequiredArgsForAssistedFactory, AnnotatedSignature>>;
  using RegisterFactoryOperation = RegisterFactory<Comp, AnnotatedSignature, RequiredSignature>;
  using Result = typename RegisterFactoryOperation::Result;
  void operator()(ComponentStorage& storage) {
    RegisterConstructorAsPointerFactoryHelper<Comp, AnnotatedSignature, RequiredSignature>()(storage);
  };
};

template <typename Comp, typename OtherComp>
struct InstallComponent {
  FruitDelegateCheck(DuplicatedTypesInComponentError<Apply<SetIntersection, typename OtherComp::Ps, typename Comp::Ps>>);
  using new_Ps = Apply<ConcatLists, typename OtherComp::Ps, typename Comp::Ps>;
  using new_Rs = Apply<SetDifference,
                       Apply<SetUnion,
                             typename OtherComp::Rs,
                             typename Comp::Rs>,
                       new_Ps>;
  using new_Deps = Apply<AddProofTreeListToForest, typename OtherComp::Deps, typename Comp::Deps, typename Comp::Ps>;
  using new_Bindings = Apply<SetUnion, typename OtherComp::Bindings, typename Comp::Bindings>;
  using Result = ConsComp<new_Rs, new_Ps, new_Deps, new_Bindings>;
  void operator()(ComponentStorage& storage, ComponentStorage&& other_storage) {
    storage.install(std::move(other_storage));
  }
};

// Used to limit the amount of metaprogramming in component.h, that might confuse users.
template <typename Comp, typename... OtherCompParams>
struct InstallComponentHelper : public InstallComponent<Comp, Apply<ConstructComponentImpl, OtherCompParams...>> {
};

template <typename DestComp, typename SourceComp>
struct ConvertComponent {
  // We need to register:
  // * All the types provided by the new component
  // * All the types required by the old component
  // except:
  // * The ones already provided by the old component.
  // * The ones required by the new one.
  using ToRegister = Apply<SetDifference,
                           Apply<SetUnion, typename DestComp::Ps, typename SourceComp::Rs>,
                           Apply<SetUnion, typename DestComp::Rs, typename SourceComp::Ps>>;
  using Helper = EnsureProvidedTypes<SourceComp, typename DestComp::Rs, ToRegister>;
  
  // Not needed, just double-checking.
  // Uses FruitStaticAssert instead of FruitDelegateCheck so that it's checked only in debug mode.
  FruitStaticAssert(true || sizeof(CheckComponentEntails<typename Helper::Result, DestComp>), "");
  
  void operator()(ComponentStorage& storage) {
    Helper()(storage);
  }
};

// The types in TargetRequirements will not be auto-registered.
template <typename Comp, typename TargetRequirements, typename C>
struct AutoRegister;

template <typename Comp, typename TargetRequirements, bool has_inject_annotation, typename C>
struct AutoRegisterHelper {}; // Not used.

// C has an Inject typedef, use it.
template <typename Comp, typename TargetRequirements, typename C>
struct AutoRegisterHelper<Comp, TargetRequirements, true, C> {
  using Inject = Apply<GetInjectAnnotation, C>;
  using RegisterC = RegisterConstructor<Comp, Inject>;
  using Comp1 = typename RegisterC::Result;
  using RegisterArgs = EnsureProvidedTypes<Comp1,
                                           TargetRequirements,
                                           Apply<ExpandProvidersInParams, Apply<SignatureArgs, Inject>>>;
  using Comp2 = typename RegisterArgs::Result;
  using Result = Comp2;
  void operator()(ComponentStorage& storage) {
    RegisterC()(storage);
    RegisterArgs()(storage);
  }
};

template <typename Comp, typename TargetRequirements, typename C>
struct AutoRegisterHelper<Comp, TargetRequirements, false, C> {
  FruitDelegateCheck(NoBindingFoundError<C>);
};

template <typename Comp, typename TargetRequirements, bool has_binding, bool has_inject_annotation, typename C, typename... Args>
struct AutoRegisterFactoryHelper {}; // Not used.

// I has a binding, use it and look for a factory that returns the type that I is bound to.
template <typename Comp, typename TargetRequirements, bool unused, typename I, typename... Argz>
struct AutoRegisterFactoryHelper<Comp, TargetRequirements, true, unused, std::unique_ptr<I>, Argz...> {
  using C = Apply<GetBinding, I, typename Comp::Bindings>;
  using original_function_t = std::function<std::unique_ptr<C>(Argz...)>;
  using function_t = std::function<std::unique_ptr<I>(Argz...)>;
  using AutoRegisterCFactory = EnsureProvidedTypes<Comp,
                                                   TargetRequirements,
                                                   List<original_function_t>>;
  using Comp1 = typename AutoRegisterCFactory::Result;
  using BindFactory = RegisterProvider<Comp1,
                                       function_t*(original_function_t*)>;
  using Comp2 = typename BindFactory::Result;
  using Result = Comp2;
  void operator()(ComponentStorage& storage) {
    AutoRegisterCFactory()(storage);
    auto provider = [](original_function_t* fun) {
      return new function_t([=](Argz... args) {
        C* c = (*fun)(args...).release();
        I* i = static_cast<I*>(c);
        return std::unique_ptr<I>(i);
      });
    };
    using RealBindFactory = RegisterProvider<Comp1, decltype(provider)>;
    static_assert(std::is_same<typename BindFactory::Result, typename RealBindFactory::Result>::value,
                  "Fruit bug, BindFactory and RealBindFactory out of sync.");
    RealBindFactory()(storage);
  }
};

// C doesn't have a binding as interface, nor an INJECT annotation.
// Bind std::function<unique_ptr<C>(Args...)> to std::function<C(Args...)>.
template <typename Comp, typename TargetRequirements, typename C, typename... Argz>
struct AutoRegisterFactoryHelper<Comp, TargetRequirements, false, false, std::unique_ptr<C>, Argz...> {
  using original_function_t = std::function<C(Argz...)>;
  using function_t = std::function<std::unique_ptr<C>(Argz...)>;
  using AutoRegisterCFactory = EnsureProvidedTypes<Comp, TargetRequirements, List<original_function_t>>;
  using Comp1 = typename AutoRegisterCFactory::Result;
  using BindFactory = RegisterProvider<Comp1,
                                       function_t*(original_function_t*)>;
  using Comp2 = typename BindFactory::Result;
  using Result = Comp2;
  void operator()(ComponentStorage& storage) {
    AutoRegisterCFactory()(storage);
    auto provider = [](original_function_t* fun) {
      return new function_t([=](Argz... args) {
        C* c = new C((*fun)(args...));
        return std::unique_ptr<C>(c);
      });
    };
    using RealBindFactory = RegisterProvider<Comp1, decltype(provider)>;
    static_assert(std::is_same<typename BindFactory::Result, typename RealBindFactory::Result>::value,
                  "Fruit bug, BindFactory and RealBindFactory out of sync.");
    RealBindFactory()(storage);
  }
};

// C has an Inject typedef, use it. unique_ptr case.
// TODO: Doesn't work after renaming Argz->Args, consider minimizing the test case and filing a bug.
template <typename Comp, typename TargetRequirements, typename C, typename... Argz>
struct AutoRegisterFactoryHelper<Comp, TargetRequirements, false, true, std::unique_ptr<C>, Argz...> {
  using AnnotatedSignature = Apply<GetInjectAnnotation, C>;
  using AnnotatedSignatureArgs = Apply<SignatureArgs, AnnotatedSignature>;
  FruitDelegateCheck(CheckSameSignatureInInjectionTypedef<
    Apply<ConstructSignature, C, List<Argz...>>,
    Apply<ConstructSignature, C, Apply<RemoveNonAssisted, AnnotatedSignatureArgs>>>);
  using NonAssistedArgs = Apply<RemoveAssisted, AnnotatedSignatureArgs>;
  using RegisterC = RegisterConstructorAsPointerFactory<Comp, AnnotatedSignature>;
  using Comp1 = typename RegisterC::Result;
  using AutoRegisterArgs = EnsureProvidedTypes<Comp1,
                                               TargetRequirements,
                                               Apply<ExpandProvidersInParams, NonAssistedArgs>>;
  using Comp2 = typename AutoRegisterArgs::Result;
  using Result = Comp2;
  void operator()(ComponentStorage& storage) {
    RegisterC()(storage);
    AutoRegisterArgs()(storage);
  }
};

// C has an Inject typedef, use it. Value (not unique_ptr) case.
// TODO: Doesn't work after renaming Argz->Args, consider minimizing the test case and filing a bug.
template <typename Comp, typename TargetRequirements, typename C, typename... Argz>
struct AutoRegisterFactoryHelper<Comp, TargetRequirements, false, true, C, Argz...> {
  using AnnotatedSignature = Apply<GetInjectAnnotation, C>;
  using AnnotatedSignatureArgs = Apply<SignatureArgs, AnnotatedSignature>;
  FruitDelegateCheck(CheckSameSignatureInInjectionTypedef<
    Apply<ConstructSignature, C, List<Argz...>>,
    Apply<ConstructSignature, C, Apply<RemoveNonAssisted, AnnotatedSignatureArgs>>>);
  using NonAssistedArgs = Apply<RemoveAssisted, AnnotatedSignatureArgs>;
  using RegisterC = RegisterConstructorAsValueFactory<Comp, AnnotatedSignature>;
  using Comp1 = typename RegisterC::Result;
  using AutoRegisterArgs = EnsureProvidedTypes<Comp1, TargetRequirements, Apply<ExpandProvidersInParams, NonAssistedArgs>>;
  using Comp2 = typename AutoRegisterArgs::Result;
  using Result = Comp2;
  void operator()(ComponentStorage& storage) {
    RegisterC()(storage);
    AutoRegisterArgs()(storage);
  }
};

template <typename Comp, typename TargetRequirements, typename C, typename... Args>
struct AutoRegisterFactoryHelper<Comp, TargetRequirements, false, false, C, Args...> {
  FruitDelegateCheck(NoBindingFoundError<std::function<C(Args...)>>);
};

// Tries to registers C by looking for a typedef called Inject inside C.
template <typename Comp, typename TargetRequirements, typename C>
struct AutoRegister : public AutoRegisterHelper<
      Comp,
      TargetRequirements,
      ApplyC<HasInjectAnnotation, C>::value,
      C
>{};

template <typename Comp, typename TargetRequirements, typename C, typename... Args>
struct AutoRegister<Comp, TargetRequirements, std::function<C(Args...)>> : public AutoRegisterFactoryHelper<
      Comp,
      TargetRequirements,
      ApplyC<HasBinding, C, typename Comp::Bindings>::value,
      ApplyC<HasInjectAnnotation, C>::value,
      C,
      Args...
>{};

template <typename Comp, typename TargetRequirements, typename C, typename... Args>
struct AutoRegister<Comp, TargetRequirements, std::function<std::unique_ptr<C>(Args...)>> : public AutoRegisterFactoryHelper<
      Comp,
      TargetRequirements,
      ApplyC<HasBinding, C, typename Comp::Bindings>::value,
      false,
      std::unique_ptr<C>,
      Args...
>{};

template <typename Comp, typename TargetRequirements, bool is_already_provided_or_in_target_requirements, bool has_binding, typename C>
struct EnsureProvidedType {}; // Not used.

// Already provided or in target requirements, ok.
template <typename Comp, typename TargetRequirements, bool unused, typename C>
struct EnsureProvidedType<Comp, TargetRequirements, true, unused, C> : public Identity<Comp> {};  

// Has a binding.
template <typename Comp, typename TargetRequirements, typename I>
struct EnsureProvidedType<Comp, TargetRequirements, false, true, I> {
  using C = Apply<GetBinding, I, typename Comp::Bindings>;
  using Binder = BindNonFactory<Comp, I, C>;
  using Comp1 = typename Binder::Result;
  using EnsureImplProvided = EnsureProvidedTypes<Comp1, TargetRequirements, List<C>>;
  using Comp2 = typename EnsureImplProvided::Result;
  using Result = Comp2;
  void operator()(ComponentStorage& storage) {
    Binder()(storage);
    EnsureImplProvided()(storage);
  }
};

// Not yet provided, nor in target requirements, nor in bindings. Try auto-registering.
template <typename Comp, typename TargetRequirements, typename C>
struct EnsureProvidedType<Comp, TargetRequirements, false, false, C> : public AutoRegister<Comp, TargetRequirements, C> {};

// General case, empty list.
template <typename Comp, typename TargetRequirements, typename L>
struct EnsureProvidedTypes : public Identity<Comp> {
  FruitStaticAssert(ApplyC<IsEmptyList, L>::value, "Implementation error");
};

template <typename Comp, typename TargetRequirements, typename... Ts>
struct EnsureProvidedTypes<Comp, TargetRequirements, List<None, Ts...>> 
  : public EnsureProvidedTypes<Comp, TargetRequirements, List<Ts...>> {
};

template <typename Comp, typename TargetRequirements, typename T, typename... Ts>
struct EnsureProvidedTypes<Comp, TargetRequirements, List<T, Ts...>> {
  using C = Apply<GetClassForType, T>;
  using ProcessT = EnsureProvidedType<Comp,
                                      TargetRequirements,
                                      ApplyC<IsInList, C, typename Comp::Ps>::value
                                   || ApplyC<IsInList, C, TargetRequirements>::value,
                                      ApplyC<HasBinding, C, typename Comp::Bindings>::value,
                                      C>;
  using Comp1 = typename ProcessT::Result;
  using ProcessTs = EnsureProvidedTypes<Comp1, TargetRequirements, List<Ts...>>;
  using Comp2 = typename ProcessTs::Result;
  using Result = Comp2;
  void operator()(ComponentStorage& storage) {
    ProcessT()(storage);
    ProcessTs()(storage);
  }
};

} // namespace impl
} // namespace fruit


#endif // FRUIT_COMPONENT_FUNCTORS_DEFN_H
