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

#ifndef FRUIT_MODULE_TEMPLATES_H
#define FRUIT_MODULE_TEMPLATES_H

namespace fruit {
namespace impl {

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
  using type = ComponentImpl<add_to_list<C, typename Comp::Rs>, typename Comp::Ps, typename Comp::Deps>;
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
using RemoveRequirement = ComponentImpl<remove_from_list<C, typename Comp::Rs>, typename Comp::Ps, RemoveRequirementFromDeps<C, typename Comp::Deps>>;

template <typename Comp, typename C, typename ArgList>
struct AddProvideHelper {
  // Note: this should be before the static_assert so that we fail here in case of a loop.
  using newDeps = AddDep<ConstructDep<C, ArgList>, typename Comp::Deps>;
  static_assert(true || sizeof(newDeps), "");
  FruitDelegateCheck(CheckTypeAlreadyBound<!is_in_list<C, typename Comp::Ps>::value, C>);
  using Comp1 = ComponentImpl<typename Comp::Rs, add_to_list<C, typename Comp::Ps>, newDeps>;
  using type = RemoveRequirement<Comp1, C>;
};

// Adds C to the provides and removes it from the requirements (if it was there at all).
// Also checks that it wasn't already provided.
template <typename Comp, typename C, typename ArgList>
using AddProvide = typename AddProvideHelper<Comp, C, ArgList>::type;

// The types in TargetRequirements will not be auto-registered.
template <typename Comp, typename TargetRequirements, typename C>
struct AutoRegister;

template <typename Comp, typename TargetRequirements, bool is_already_provided_or_in_target_requirements, typename C>
struct EnsureProvidedType {}; // Not used.

// Already provided or in target requirements, ok.
template <typename Comp, typename TargetRequirements, typename C>
struct EnsureProvidedType<Comp, TargetRequirements, true, C> : public Identity<Comp> {};  

// Not yet provided nor in target requirements, try auto-registering.
template <typename Comp, typename TargetRequirements, typename C>
struct EnsureProvidedType<Comp, TargetRequirements, false, C> : public AutoRegister<Comp, TargetRequirements, C> {};

// General case, empty list.
template <typename Comp, typename TargetRequirements, typename L>
struct EnsureProvidedTypes : public Identity<Comp> {
  FruitStaticAssert(is_empty_list<L>::value, "Implementation error");
};

template <typename Comp, typename TargetRequirements, typename T, typename... Ts>
struct EnsureProvidedTypes<Comp, TargetRequirements, List<T, Ts...>> {
  using ProcessT = EnsureProvidedType<Comp,
    TargetRequirements,
    is_in_list<GetClassForType<T>, typename Comp::Ps>::value
    || is_in_list<GetClassForType<T>, TargetRequirements>::value,
    GetClassForType<T>>;
  using Comp1 = FunctorResult<ProcessT, Comp&&>;
  using ProcessTs = EnsureProvidedTypes<Comp1, TargetRequirements, List<Ts...>>;
  using Comp2 = FunctorResult<ProcessTs, Comp1&&>;
  Comp2 operator()(Comp&& m) {
    return ProcessTs()(ProcessT()(std::move(m)));
  }
};

template <typename Comp, typename TargetRequirements, bool has_inject_annotation, typename C>
struct AutoRegisterHelper {}; // Not used.

// C has an Inject typedef, use it.
template <typename Comp, typename TargetRequirements, typename C>
struct AutoRegisterHelper<Comp, TargetRequirements, true, C> {
  using RegisterC = RegisterConstructor<Comp, typename GetInjectAnnotation<C>::Signature>;
  using Comp1 = FunctorResult<RegisterC, Comp&&>;
  using RegisterArgs = EnsureProvidedTypes<Comp1, TargetRequirements, ExpandInjectorsInParams<typename GetInjectAnnotation<C>::Args>>;
  using Comp2 = FunctorResult<RegisterArgs, Comp1&&>;
  Comp2 operator()(Comp&& m) {
    return RegisterArgs()(RegisterC()(std::move(m)));
  }
};

template <typename Comp, typename TargetRequirements, typename C>
struct AutoRegisterHelper<Comp, TargetRequirements, false, C> {
  FruitDelegateCheck(NoBindingFoundError<C>);
};

template <typename Comp, typename TargetRequirements, bool has_inject_annotation, typename C, typename... Args>
struct AutoRegisterFactoryHelper {}; // Not used.

// C has an Inject typedef, use it.
// TODO: Doesn't work after renaming Argz->Args, consider minimizing the test case and filing a bug.
template <typename Comp, typename TargetRequirements, typename C, typename... Argz>
struct AutoRegisterFactoryHelper<Comp, TargetRequirements, true, C, Argz...> {
  using AnnotatedSignature = typename GetInjectAnnotation<C>::Signature;
  FruitDelegateCheck(CheckSameParametersInInjectionAnnotation<
    C,
    List<Argz...>,
    RemoveNonAssisted<SignatureArgs<AnnotatedSignature>>>);
  using NonAssistedArgs = RemoveAssisted<SignatureArgs<AnnotatedSignature>>;
  using RegisterC = RegisterConstructorAsFactory<Comp, AnnotatedSignature>;
  using Comp1 = FunctorResult<RegisterC, Comp&&>;
  using AutoRegisterArgs = EnsureProvidedTypes<Comp1, TargetRequirements, ExpandInjectorsInParams<NonAssistedArgs>>;
  using Comp2 = FunctorResult<AutoRegisterArgs, Comp1&&>;
  Comp2 operator()(Comp&& m) {
    return AutoRegisterArgs()(RegisterC()(std::move(m)));
  }
};

template <typename Comp, typename TargetRequirements, typename C, typename... Args>
struct AutoRegisterFactoryHelper<Comp, TargetRequirements, false, C, Args...> {
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
      HasInjectAnnotation<C>::value,
      C,
      Args...
>{};

template <typename Comp>
struct Identity {
  Comp operator()(Comp&& m) {
    return std::move(m);
  }
};

template <typename Comp, typename I, typename C>
struct Bind {
  using Comp1 = AddRequirement<Comp, C>;
  using Comp2 = AddProvide<Comp1, I, List<C>>;
  Comp2 operator()(Comp&& m) {
    FruitDelegateCheck(CheckClassType<I, GetClassForType<I>>);
    FruitDelegateCheck(CheckClassType<C, GetClassForType<C>>);
    FruitDelegateCheck(CheckBaseClass<I, C>);
    m.storage.template bind<I, C>();
    return Comp2(std::move(m.storage));
  };
};

// T can't be any injectable type, it must match the return type of the provider in one of
// the registerProvider() overloads in ComponentStorage.
template <typename Comp, typename Signature>
struct RegisterProvider {};

template <typename Comp, typename T, typename... Args>
struct RegisterProvider<Comp, T(Args...)> {
  using Signature = T(Args...);
  using SignatureRequirements = ExpandInjectorsInParams<List<GetClassForType<Args>...>>;
  using Comp1 = AddRequirements<Comp, SignatureRequirements>;
  using Comp2 = AddProvide<Comp1, GetClassForType<T>, SignatureRequirements>;
  Comp2 operator()(Comp&& m, Signature* provider, void (*deleter)(void*)) {
    m.storage.registerProvider(provider, deleter);
    return std::move(m.storage);
  }
};

template <typename Comp, typename AnnotatedSignature>
struct RegisterFactory {
  using InjectedFunctionType = InjectedFunctionTypeForAssistedFactory<AnnotatedSignature>;
  using RequiredSignature = RequiredSignatureForAssistedFactory<AnnotatedSignature>;
  using NewRequirements = ExpandInjectorsInParams<ExtractRequirementsFromAssistedParams<SignatureArgs<AnnotatedSignature>>>;
  using Comp1 = AddRequirements<Comp, NewRequirements>;
  using Comp2 = AddProvide<Comp1, std::function<InjectedFunctionType>, NewRequirements>;
  Comp2 operator()(Comp&& m, RequiredSignature* factory) {
    m.storage.template registerFactory<AnnotatedSignature>(factory);
    return std::move(m.storage);
  }
};

template <typename Comp, typename Signature>
struct RegisterConstructor {
  using Provider = decltype(ConstructorProvider<Signature>::f);
  using RegisterProviderOperation = RegisterProvider<Comp, Provider>;
  using Comp1 = FunctorResult<RegisterProviderOperation, Comp&&, Provider*, void(*)(void*)>;
  Comp1 operator()(Comp&& m) {
    return RegisterProviderOperation()(std::move(m), ConstructorProvider<Signature>::f, ConcreteClassDeleter<SignatureType<Signature>>::f);
  };
};

template <typename Comp, typename C>
struct RegisterInstance {
  using Comp1 = AddProvide<Comp, C, List<>>;
  Comp1 operator()(Comp&& m, C& instance) {
    m.storage.bindInstance(instance);
    return std::move(m.storage);
  };
};

template <typename Comp, typename AnnotatedSignature>
struct RegisterConstructorAsFactory {
  using RequiredSignature = RequiredSignatureForAssistedFactory<AnnotatedSignature>;
  using Provider = decltype(ConstructorFactoryProvider<RequiredSignature>::f);
  using RegisterFactoryOperation = RegisterFactory<Comp, AnnotatedSignature>;
  using Comp1 = FunctorResult<RegisterFactoryOperation, Comp&&, Provider*>;
  Comp1 operator()(Comp&& m) {
    return RegisterFactoryOperation()(std::move(m), ConstructorFactoryProvider<RequiredSignature>::f);
  };
};

template <typename Comp, typename OtherComp>
struct InstallComponent {
  using OtherComp_Rs = typename OtherComp::Rs;
  using OtherComp_Ps = typename OtherComp::Ps;
  using OtherComp_Deps = typename OtherComp::Deps;
  FruitDelegateCheck(DuplicatedTypesInComponentError<set_intersection<typename Comp::Ps, OtherComp_Ps>>);
  using new_Ps = concat_lists<typename Comp::Ps, OtherComp_Ps>;
  using new_Rs = set_difference<merge_sets<typename Comp::Rs, OtherComp_Rs>, new_Ps>;
  using new_Deps = AddDeps<typename Comp::Deps, OtherComp_Deps>;
  using Comp1 = ComponentImpl<new_Rs, new_Ps, new_Deps>;
  Comp1 operator()(Comp&& m, const OtherComp& otherComp) {
    m.storage.install(otherComp.storage);
    return std::move(m.storage);
  }
};

template <typename RsParam, typename PsParam, typename DepsParam>
ComponentImpl<RsParam, PsParam, DepsParam>::ComponentImpl(ComponentStorage&& storage) 
  : storage(storage) {
}

template <typename RsParam, typename PsParam, typename DepsParam>
template <typename Source_Rs, typename Source_Ps, typename Source_Deps>
ComponentImpl<RsParam, PsParam, DepsParam>::ComponentImpl(const ComponentImpl<Source_Rs, Source_Ps, Source_Deps>& sourceComponent) {
  // We need to register:
  // * All the types provided by the new component
  // * All the types required by the old component
  // except:
  // * The ones already provided by the old component.
  // * The ones required by the new one.
  using ToRegister = set_difference<merge_sets<Ps, Source_Rs>,
                                    merge_sets<Rs, Source_Ps>>;
  using SourceComponent = ComponentImpl<Source_Rs, Source_Ps, Source_Deps>;
  using Helper = EnsureProvidedTypes<SourceComponent, Rs, ToRegister>;
  SourceComponent sourceComponentCopy = sourceComponent;
  // Add the required bindings.
  auto extendedComponent = Helper()(std::move(sourceComponentCopy));
  
  FruitStaticAssert(true || sizeof(CheckComponentEntails<decltype(extendedComponent), This>), "");
  
  // Extract the bindings from the extended component.
  storage = std::move(extendedComponent.storage);
}

} // namespace impl
} // namespace fruit


#endif // FRUIT_MODULE_TEMPLATES_H
