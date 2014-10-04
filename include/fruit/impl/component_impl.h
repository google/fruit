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

#ifndef FRUIT_COMPONENT_IMPL_H
#define FRUIT_COMPONENT_IMPL_H

#include "../fruit_forward_decls.h"
#include "component_storage.h"
#include "component.utils.h"
#include "injection_errors.h"
#include "fruit_assert.h"

namespace fruit {

namespace impl {

//********************************************************************************************************************************
// Part 1: Purely compile-time definitions.
//********************************************************************************************************************************

template <typename RsParam, typename PsParam, typename DepsParam, typename BindingsParam>
struct ConsComp {
  // Just for convenience.
  using Rs = RsParam;
  using Ps = PsParam;
  using Deps = DepsParam;
  using Bindings = BindingsParam;
  
  // Invariants:
  // * all types appearing as arguments of Deps are in Rs
  // * all types in Ps are at the head of one (and only one) Dep.
  //   (note that the types in Rs can appear in deps any number of times, 0 is also ok)
  // * Deps is of the form List<Dep...> with each Dep of the form T(Args...) and where List<Args...> is a set (no repetitions).
  // * Bindings is of the form List<ConsBinding<I1, C1>, ..., ConsBinding<In, Cn>> and is a set (no repetitions).
  
  FruitStaticAssert(true || sizeof(CheckDepsNormalized<AddDeps<Deps, List<>, List<>>, Deps>), "");
};

// Non-specialized case: no requirements.
template <typename... Ps>
struct ConstructComponentImplHelper {
  FruitDelegateCheck(fruit::impl::CheckNoRepeatedTypes<Ps...>);
  FruitDelegateChecks(fruit::impl::CheckClassType<Ps, fruit::impl::GetClassForType<Ps>>);
  using type = ConsComp<List<>,
                        List<Ps...>,
                        ConstructDeps<List<>, Ps...>,
                        List<>>;
};

// Non-specialized case: no requirements.
template <typename... Rs, typename... Ps>
struct ConstructComponentImplHelper<Required<Rs...>, Ps...> {
  FruitDelegateCheck(fruit::impl::CheckNoRepeatedTypes<Rs..., Ps...>);
  FruitDelegateChecks(fruit::impl::CheckClassType<Rs, fruit::impl::GetClassForType<Rs>>);
  FruitDelegateChecks(fruit::impl::CheckClassType<Ps, fruit::impl::GetClassForType<Ps>>);
  using type = ConsComp<List<Rs...>,
                        List<Ps...>,
                        ConstructDeps<List<Rs...>, Ps...>,
                        List<>>;
};

template <typename... Types>
using ConstructComponentImpl = typename ConstructComponentImplHelper<Types...>::type;

template <typename Comp, typename L>
struct AddRequirementsHelper;

// Adds the types in L to the requirements (unless they are already provided/required).
template <typename Comp, typename L>
using AddRequirements = typename AddRequirementsHelper<Comp, L>::type;

template <typename Comp, bool is_already_present, typename C>
struct AddRequirementHelper; // Not used.

// Already present, nothing to check and nothing to add.
template <typename Comp, typename C>
struct AddRequirementHelper<Comp, true, C> {
  using type = Comp;
};

// Not present, add (general case).
template <typename Comp, typename C>
struct AddRequirementHelper<Comp, false, C> {
  using type = ConsComp<add_to_list<C, typename Comp::Rs>, typename Comp::Ps, typename Comp::Deps, typename Comp::Bindings>;
};

// Adds C to the requirements (unless it's already provided/required).
template <typename Comp, typename C>
using AddRequirement = typename AddRequirementHelper<
    Comp,
    is_in_list<C, typename Comp::Ps>::value || is_in_list<C, typename Comp::Rs>::value,
    C>::type;    

// Empty list.
template <typename Comp, typename L>
struct AddRequirementsHelper {
  FruitStaticAssert(is_empty_list<L>::value, "Implementation error: L not a list");
  using type = Comp;
};

template <typename Comp, typename OtherR, typename... OtherRs>
struct AddRequirementsHelper<Comp, List<OtherR, OtherRs...>> {
  using Comp1 = typename AddRequirementsHelper<Comp, List<OtherRs...>>::type;
  using type = AddRequirement<Comp1, OtherR>;
};

// Removes the requirement, assumes that the type is now bound.
template <typename Comp, typename C>
using RemoveRequirement = ConsComp<remove_from_list<C, typename Comp::Rs>, typename Comp::Ps, RemoveRequirementFromDeps<C, typename Comp::Deps>, typename Comp::Bindings>;

template <typename Comp>
struct Identity {
  using Result = Comp;
  void operator()(ComponentStorage& storage) {
    (void)storage;
  }
};

template <typename Comp, typename C, typename ArgList>
struct AddProvideHelper {
  // Note: this should be before the FruitDelegateCheck so that we fail here in case of a loop.
  using newDeps = AddDep<ConstructDep<C, ArgList>, typename Comp::Deps, typename Comp::Ps>;
  FruitDelegateCheck(CheckTypeAlreadyBound<!is_in_list<C, typename Comp::Ps>::value, C>);
  using Comp1 = ConsComp<typename Comp::Rs, add_to_list<C, typename Comp::Ps>, newDeps, typename Comp::Bindings>;
  using type = RemoveRequirement<Comp1, C>;
};

// Adds C to the provides and removes it from the requirements (if it was there at all).
// Also checks that it wasn't already provided.
template <typename Comp, typename C, typename ArgList>
using AddProvide = typename AddProvideHelper<Comp, C, ArgList>::type;

//********************************************************************************************************************************
// Part 2: Functors to transform components.
//********************************************************************************************************************************

template <typename Comp, typename TargetRequirements, typename L>
struct EnsureProvidedTypes;

// Doesn't actually bind in ComponentStorage. The binding is added later (if needed) using BindNonFactory.
template <typename Comp, typename I, typename C>
struct Bind {
  using NewBindings = add_to_set<ConsBinding<I, C>, typename Comp::Bindings>;
  using Comp1 = ConsComp<typename Comp::Rs, typename Comp::Ps, typename Comp::Deps, NewBindings>;
  using Result = Comp1;
  void operator()(ComponentStorage& storage) {
    (void)storage;
  };
};

template <typename Comp, typename I, typename C>
struct BindNonFactory {
  FruitDelegateCheck(CheckClassType<I, GetClassForType<I>>);
  FruitDelegateCheck(CheckClassType<C, GetClassForType<C>>);
  FruitDelegateCheck(NotABaseClassOf<I, C>);
  using Comp1 = AddRequirement<Comp, C>;
  using Comp2 = AddProvide<Comp1, I, List<C>>;
  using Result = Comp2;
  void operator()(ComponentStorage& storage) {
    storage.template bind<I, C>();
  };
};

template <typename Comp, typename I, typename C>
struct AddMultibinding {
  FruitDelegateCheck(CheckClassType<I, GetClassForType<I>>);
  FruitDelegateCheck(CheckClassType<C, GetClassForType<C>>);
  FruitDelegateCheck(NotABaseClassOf<I, C>);
  using Comp1 = AddRequirement<Comp, C>;
  using Result = Comp1;
  void operator()(ComponentStorage& storage) {
    storage.template addMultibinding<I, C>();
  };
};

// T can't be any injectable type, it must match the return type of the provider in one of
// the registerProvider() overloads in ComponentStorage.
template <typename Comp, typename Function>
struct RegisterProvider {
  using Signature = FunctionSignature<Function>;
  using SignatureRequirements = ExpandProvidersInParams<GetClassForTypeList<SignatureArgs<Signature>>>;
  using Comp1 = AddRequirements<Comp, SignatureRequirements>;
  using Comp2 = AddProvide<Comp1, GetClassForType<SignatureType<Signature>>, SignatureRequirements>;
  using Result = Comp2;
  void operator()(ComponentStorage& storage, Function provider) {
    storage.registerProvider(provider);
  }
};

// T can't be any injectable type, it must match the return type of the provider in one of
// the registerMultibindingProvider() overloads in ComponentStorage.
template <typename Comp, typename Function>
struct RegisterMultibindingProvider {
  using SignatureRequirements = ExpandProvidersInParams<GetClassForTypeList<SignatureArgs<FunctionSignature<Function>>>>;
  using Comp1 = AddRequirements<Comp, SignatureRequirements>;
  using Result = Comp1;
  void operator()(ComponentStorage& storage, Function provider) {
    storage.registerMultibindingProvider(provider);
  }
};

template <typename Comp, typename AnnotatedSignature, typename Function>
struct RegisterFactory {
  using InjectedFunctionType = ConstructSignature<SignatureType<AnnotatedSignature>, InjectedFunctionArgsForAssistedFactory<AnnotatedSignature>>;
  using RequiredSignature = ConstructSignature<SignatureType<AnnotatedSignature>, RequiredArgsForAssistedFactory<AnnotatedSignature>>;
  FruitDelegateCheck(FunctorSignatureDoesNotMatch<RequiredSignature, FunctionSignature<Function>>);
  using NewRequirements = ExpandProvidersInParams<ExtractRequirementsFromAssistedParams<SignatureArgs<AnnotatedSignature>>>;
  using Comp1 = AddRequirements<Comp, NewRequirements>;
  using Comp2 = AddProvide<Comp1, std::function<InjectedFunctionType>, NewRequirements>;
  using Result = Comp2;
  void operator()(ComponentStorage& storage, Function factory) {
    storage.template registerFactory<AnnotatedSignature>(factory);
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

template <typename Comp, typename T, typename... Args>
struct RegisterConstructor<Comp, T(Args...)> {
  using Signature = T(Args...);
  using SignatureRequirements = ExpandProvidersInParams<List<GetClassForType<Args>...>>;
  using Comp1 = AddRequirements<Comp, SignatureRequirements>;
  using Comp2 = AddProvide<Comp1, GetClassForType<T>, SignatureRequirements>;
  using Result = Comp2;
  void operator()(ComponentStorage& storage) {
    storage.template registerConstructor<T, Args...>();
  }
};

template <typename Comp, typename C>
struct RegisterInstance {
  using Comp1 = AddProvide<Comp, C, List<>>;
  using Result = Comp1;
  void operator()(ComponentStorage& storage, C& instance) {
    storage.bindInstance(instance);
  };
};

template <typename Comp, typename C>
struct AddInstanceMultibinding {
  using Result = Comp;
  void operator()(ComponentStorage& storage, C& instance) {
    storage.addInstanceMultibinding(instance);
  };
};

template <typename Comp, typename AnnotatedSignature>
struct RegisterConstructorAsValueFactory {
  using RequiredSignature = ConstructSignature<SignatureType<AnnotatedSignature>, RequiredArgsForAssistedFactory<AnnotatedSignature>>;
  using Provider = decltype(ConstructorFactoryValueProvider<RequiredSignature>::f);
  using RegisterFactoryOperation = RegisterFactory<Comp, AnnotatedSignature, RequiredSignature>;
  using Comp1 = typename RegisterFactoryOperation::Result;
  using Result = Comp1;
  void operator()(ComponentStorage& storage) {
    RegisterFactoryOperation()(storage, ConstructorFactoryValueProvider<RequiredSignature>::f);
  };
};

template <typename Comp, typename AnnotatedSignature>
struct RegisterConstructorAsPointerFactory {
  using RequiredSignature = ConstructSignature<std::unique_ptr<SignatureType<AnnotatedSignature>>, RequiredArgsForAssistedFactory<AnnotatedSignature>>;
  using Provider = decltype(ConstructorFactoryPointerProvider<RequiredSignature>::f);
  using RegisterFactoryOperation = RegisterFactory<Comp, AnnotatedSignature, RequiredSignature>;
  using Comp1 = typename RegisterFactoryOperation::Result;
  using Result = Comp1;
  void operator()(ComponentStorage& storage) {
    RegisterFactoryOperation()(storage, ConstructorFactoryPointerProvider<RequiredSignature>::f);
  };
};

template <typename Comp, typename OtherComp>
struct InstallComponent {
  FruitDelegateCheck(DuplicatedTypesInComponentError<set_intersection<typename OtherComp::Ps, typename Comp::Ps>>);
  using new_Ps = concat_lists<typename OtherComp::Ps, typename Comp::Ps>;
  using new_Rs = set_difference<merge_sets<typename OtherComp::Rs, typename Comp::Rs>, new_Ps>;
  using new_Deps = AddDeps<typename OtherComp::Deps, typename Comp::Deps, typename Comp::Ps>;
  using new_Bindings = merge_sets<typename OtherComp::Bindings, typename Comp::Bindings>;
  using Comp1 = ConsComp<new_Rs, new_Ps, new_Deps, new_Bindings>;
  using Result = Comp1;
  void operator()(ComponentStorage& storage, ComponentStorage&& otherStorage) {
    storage.install(std::move(otherStorage));
  }
};

template <typename DestComp, typename SourceComp>
struct ConvertComponent {
  // We need to register:
  // * All the types provided by the new component
  // * All the types required by the old component
  // except:
  // * The ones already provided by the old component.
  // * The ones required by the new one.
  using ToRegister = set_difference<merge_sets<typename DestComp::Ps, typename SourceComp::Rs>,
                                    merge_sets<typename DestComp::Rs, typename SourceComp::Ps>>;
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
  using RegisterC = RegisterConstructor<Comp, typename GetInjectAnnotation<C>::Signature>;
  using Comp1 = typename RegisterC::Result;
  using RegisterArgs = EnsureProvidedTypes<Comp1, TargetRequirements, ExpandProvidersInParams<typename GetInjectAnnotation<C>::Args>>;
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

template <typename I, typename C, typename... Args>
struct BindFactoryFunction1 {
  static std::function<std::unique_ptr<I>(Args...)>* f(std::function<std::unique_ptr<C>(Args...)>* fun) {
    return new std::function<std::unique_ptr<I>(Args...)>([=](Args... args) {
      C* c = (*fun)(args...).release();
      I* i = static_cast<I*>(c);
      return std::unique_ptr<I>(i);
    });
  }
};
  
// I has a binding, use it and look for a factory that returns the type that I is bound to.
template <typename Comp, typename TargetRequirements, bool unused, typename I, typename... Argz>
struct AutoRegisterFactoryHelper<Comp, TargetRequirements, true, unused, std::unique_ptr<I>, Argz...> {
  using C = GetBinding<I, typename Comp::Bindings>;
  using AutoRegisterCFactory = EnsureProvidedTypes<Comp, TargetRequirements, List<std::function<std::unique_ptr<C>(Argz...)>>>;
  using Comp1 = typename AutoRegisterCFactory::Result;
  using BindFactory = RegisterProvider<Comp1, decltype(BindFactoryFunction1<I, C, Argz...>::f)>;
  using Comp2 = typename BindFactory::Result;
  using Result = Comp2;
  void operator()(ComponentStorage& storage) {
    AutoRegisterCFactory()(storage);
    BindFactory()(storage, BindFactoryFunction1<I, C, Argz...>::f);
  }
};

template <typename C, typename... Args>
struct BindFactoryFunction2 {
  static std::function<std::unique_ptr<C>(Args...)>* f(std::function<C(Args...)>* fun) {
    return new std::function<std::unique_ptr<C>(Args...)>([=](Args... args) {
      C* c = new C((*fun)(args...));
      return std::unique_ptr<C>(c);
    });
  }
};

// C doesn't have a binding as interface, nor an INJECT annotation.
// Bind std::function<unique_ptr<C>(Args...)> to std::function<C(Args...)>.
template <typename Comp, typename TargetRequirements, typename C, typename... Argz>
struct AutoRegisterFactoryHelper<Comp, TargetRequirements, false, false, std::unique_ptr<C>, Argz...> {
  using AutoRegisterCFactory = EnsureProvidedTypes<Comp, TargetRequirements, List<std::function<C(Argz...)>>>;
  using Comp1 = typename AutoRegisterCFactory::Result;
  using BindFactory = RegisterProvider<Comp1, decltype(BindFactoryFunction2<C, Argz...>::f)>;
  using Comp2 = typename BindFactory::Result;
  using Result = Comp2;
  void operator()(ComponentStorage& storage) {
    AutoRegisterCFactory()(storage);
    BindFactory()(storage, BindFactoryFunction2<C, Argz...>::f);
  }
};

// C has an Inject typedef, use it. unique_ptr case.
// TODO: Doesn't work after renaming Argz->Args, consider minimizing the test case and filing a bug.
template <typename Comp, typename TargetRequirements, typename C, typename... Argz>
struct AutoRegisterFactoryHelper<Comp, TargetRequirements, false, true, std::unique_ptr<C>, Argz...> {
  using AnnotatedSignature = typename GetInjectAnnotation<C>::Signature;
  FruitDelegateCheck(CheckSameSignatureInInjectionTypedef<
    ConstructSignature<C, List<Argz...>>,
    ConstructSignature<C, RemoveNonAssisted<SignatureArgs<AnnotatedSignature>>>>);
  using NonAssistedArgs = RemoveAssisted<SignatureArgs<AnnotatedSignature>>;
  using RegisterC = RegisterConstructorAsPointerFactory<Comp, AnnotatedSignature>;
  using Comp1 = typename RegisterC::Result;
  using AutoRegisterArgs = EnsureProvidedTypes<Comp1, TargetRequirements, ExpandProvidersInParams<NonAssistedArgs>>;
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
  using AnnotatedSignature = typename GetInjectAnnotation<C>::Signature;
  FruitDelegateCheck(CheckSameSignatureInInjectionTypedef<
    ConstructSignature<C, List<Argz...>>,
    ConstructSignature<C, RemoveNonAssisted<SignatureArgs<AnnotatedSignature>>>>);
  using NonAssistedArgs = RemoveAssisted<SignatureArgs<AnnotatedSignature>>;
  using RegisterC = RegisterConstructorAsValueFactory<Comp, AnnotatedSignature>;
  using Comp1 = typename RegisterC::Result;
  using AutoRegisterArgs = EnsureProvidedTypes<Comp1, TargetRequirements, ExpandProvidersInParams<NonAssistedArgs>>;
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
      HasInjectAnnotation<C>::value,
      C
>{};

template <typename Comp, typename TargetRequirements, typename C, typename... Args>
struct AutoRegister<Comp, TargetRequirements, std::function<C(Args...)>> : public AutoRegisterFactoryHelper<
      Comp,
      TargetRequirements,
      HasBinding<C, typename Comp::Bindings>::value,
      HasInjectAnnotation<C>::value,
      C,
      Args...
>{};

template <typename Comp, typename TargetRequirements, typename C, typename... Args>
struct AutoRegister<Comp, TargetRequirements, std::function<std::unique_ptr<C>(Args...)>> : public AutoRegisterFactoryHelper<
      Comp,
      TargetRequirements,
      HasBinding<C, typename Comp::Bindings>::value,
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
  using C = GetBinding<I, typename Comp::Bindings>;
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
  FruitStaticAssert(is_empty_list<L>::value, "Implementation error");
};

template <typename Comp, typename TargetRequirements, typename T, typename... Ts>
struct EnsureProvidedTypes<Comp, TargetRequirements, List<T, Ts...>> {
  using C = GetClassForType<T>;
  using ProcessT = EnsureProvidedType<Comp,
    TargetRequirements,
    is_in_list<C, typename Comp::Ps>::value
    || is_in_list<C, TargetRequirements>::value,
    HasBinding<C, typename Comp::Bindings>::value,
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

#endif // FRUIT_COMPONENT_IMPL_H
