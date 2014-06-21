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

template <typename M, typename L>
struct AddRequirementsHelper;

// Adds the types in L to the requirements (unless they are already provided/required).
template <typename M, typename L>
using AddRequirements = typename AddRequirementsHelper<M, L>::type;

template <typename M, bool is_already_present, typename C>
struct AddRequirementHelper; // Not used.

// Already present, nothing to check and nothing to add.
template <typename M, typename C>
struct AddRequirementHelper<M, true, C> {
  using type = M;
};

// Not present, add (general case).
template <typename M, typename C>
struct AddRequirementHelper<M, false, C> {
  using type = ModuleImpl<add_to_list<C, typename M::Rs>, typename M::Ps, typename M::Deps>;
};

// Adds C to the requirements (unless it's already provided/required).
template <typename M, typename C>
using AddRequirement = typename AddRequirementHelper<
    M,
    is_in_list<C, typename M::Ps>::value || is_in_list<C, typename M::Rs>::value,
    C>::type;    

// Empty list.
template <typename M, typename L>
struct AddRequirementsHelper {
  FruitStaticAssert(is_empty_list<L>::value, "Implementation error: L not a list");
  using type = M;
};

template <typename M, typename OtherR, typename... OtherRs>
struct AddRequirementsHelper<M, List<OtherR, OtherRs...>> {
  using M1 = typename AddRequirementsHelper<M, List<OtherRs...>>::type;
  using type = AddRequirement<M1, OtherR>;
};

// Removes the requirement, assumes that the type is now bound.
template <typename M, typename C>
using RemoveRequirement = ModuleImpl<remove_from_list<C, typename M::Rs>, typename M::Ps, RemoveRequirementFromDeps<C, typename M::Deps>>;

template <typename M, typename C, typename ArgList>
struct AddProvideHelper {
  // Note: this should be before the static_assert so that we fail here in case of a loop.
  using newDeps = AddDep<ConstructDep<C, ArgList>, typename M::Deps>;
  static_assert(true || sizeof(newDeps), "");
  FruitDelegateCheck(CheckTypeAlreadyBound<!is_in_list<C, typename M::Ps>::value, C>);
  using M1 = ModuleImpl<typename M::Rs, add_to_list<C, typename M::Ps>, newDeps>;
  using type = RemoveRequirement<M1, C>;
};

// Adds C to the provides and removes it from the requirements (if it was there at all).
// Also checks that it wasn't already provided.
template <typename M, typename C, typename ArgList>
using AddProvide = typename AddProvideHelper<M, C, ArgList>::type;

// The types in TargetRequirements will not be auto-registered.
template <typename M, typename TargetRequirements, typename C>
struct AutoRegister;

template <typename M, typename TargetRequirements, bool is_already_provided_or_in_target_requirements, typename C>
struct EnsureProvidedType {}; // Not used.

// Already provided or in target requirements, ok.
template <typename M, typename TargetRequirements, typename C>
struct EnsureProvidedType<M, TargetRequirements, true, C> : public Identity<M> {};  

// Not yet provided nor in target requirements, try auto-registering.
template <typename M, typename TargetRequirements, typename C>
struct EnsureProvidedType<M, TargetRequirements, false, C> : public AutoRegister<M, TargetRequirements, C> {};

// General case, empty list.
template <typename M, typename TargetRequirements, typename L>
struct EnsureProvidedTypes : public Identity<M> {
  FruitStaticAssert(is_empty_list<L>::value, "Implementation error");
};

template <typename M, typename TargetRequirements, typename T, typename... Ts>
struct EnsureProvidedTypes<M, TargetRequirements, List<T, Ts...>> {
  using ProcessT = EnsureProvidedType<M,
    TargetRequirements,
    is_in_list<GetClassForType<T>, typename M::Ps>::value
    || is_in_list<GetClassForType<T>, TargetRequirements>::value,
    GetClassForType<T>>;
  using M1 = FunctorResult<ProcessT, M&&>;
  using ProcessTs = EnsureProvidedTypes<M1, TargetRequirements, List<Ts...>>;
  using M2 = FunctorResult<ProcessTs, M1&&>;
  M2 operator()(M&& m) {
    return ProcessTs()(ProcessT()(std::move(m)));
  }
};

template <typename M, typename TargetRequirements, bool has_inject_annotation, typename C>
struct AutoRegisterHelper {}; // Not used.

// C has an Inject typedef, use it.
template <typename M, typename TargetRequirements, typename C>
struct AutoRegisterHelper<M, TargetRequirements, true, C> {
  using RegisterC = RegisterConstructor<M, typename GetInjectAnnotation<C>::Signature>;
  using M1 = FunctorResult<RegisterC, M&&>;
  using RegisterArgs = EnsureProvidedTypes<M1, TargetRequirements, ExpandInjectorsInParams<typename GetInjectAnnotation<C>::Args>>;
  using M2 = FunctorResult<RegisterArgs, M1&&>;
  M2 operator()(M&& m) {
    return RegisterArgs()(RegisterC()(std::move(m)));
  }
};

template <typename M, typename TargetRequirements, typename C>
struct AutoRegisterHelper<M, TargetRequirements, false, C> {
  FruitDelegateCheck(NoBindingFoundError<C>);
};

template <typename M, typename TargetRequirements, bool has_inject_annotation, typename C, typename... Args>
struct AutoRegisterFactoryHelper {}; // Not used.

// C has an Inject typedef, use it.
// TODO: Doesn't work after renaming Argz->Args, consider minimizing the test case and filing a bug.
template <typename M, typename TargetRequirements, typename C, typename... Argz>
struct AutoRegisterFactoryHelper<M, TargetRequirements, true, C, Argz...> {
  using AnnotatedSignature = typename GetInjectAnnotation<C>::Signature;
  FruitDelegateCheck(CheckSameParametersInInjectionAnnotation<
    C,
    List<Argz...>,
    RemoveNonAssisted<SignatureArgs<AnnotatedSignature>>>);
  using NonAssistedArgs = RemoveAssisted<SignatureArgs<AnnotatedSignature>>;
  using RegisterC = RegisterConstructorAsFactory<M, AnnotatedSignature>;
  using M1 = FunctorResult<RegisterC, M&&>;
  using AutoRegisterArgs = EnsureProvidedTypes<M1, TargetRequirements, ExpandInjectorsInParams<NonAssistedArgs>>;
  using M2 = FunctorResult<AutoRegisterArgs, M1&&>;
  M2 operator()(M&& m) {
    return AutoRegisterArgs()(RegisterC()(std::move(m)));
  }
};

template <typename M, typename TargetRequirements, typename C, typename... Args>
struct AutoRegisterFactoryHelper<M, TargetRequirements, false, C, Args...> {
  FruitDelegateCheck(NoBindingFoundError<std::function<C(Args...)>>);
};

// Tries to registers C by looking for a typedef called Inject inside C.
template <typename M, typename TargetRequirements, typename C>
struct AutoRegister : public AutoRegisterHelper<
      M,
      TargetRequirements,
      HasInjectAnnotation<C>::value,
      C
>{};

template <typename M, typename TargetRequirements, typename C, typename... Args>
struct AutoRegister<M, TargetRequirements, std::function<C(Args...)>> : public AutoRegisterFactoryHelper<
      M,
      TargetRequirements,
      HasInjectAnnotation<C>::value,
      C,
      Args...
>{};

template <typename M>
struct Identity {
  M operator()(M&& m) {
    return std::move(m);
  }
};

template <typename M, typename I, typename C>
struct Bind {
  using M1 = AddRequirement<M, C>;
  using M2 = AddProvide<M1, I, List<C>>;
  M2 operator()(M&& m) {
    FruitDelegateCheck(CheckClassType<I, GetClassForType<I>>);
    FruitDelegateCheck(CheckClassType<C, GetClassForType<C>>);
    FruitDelegateCheck(CheckBaseClass<I, C>);
    m.unsafeModule.template bind<I, C>();
    return M2(std::move(m.unsafeModule));
  };
};

// T can't be any injectable type, it must match the return type of the provider in one of
// the registerProvider() overloads in UnsafeModule.
template <typename M, typename Signature>
struct RegisterProvider {};

template <typename M, typename T, typename... Args>
struct RegisterProvider<M, T(Args...)> {
  using Signature = T(Args...);
  using SignatureRequirements = ExpandInjectorsInParams<List<GetClassForType<Args>...>>;
  using M1 = AddRequirements<M, SignatureRequirements>;
  using M2 = AddProvide<M1, GetClassForType<T>, SignatureRequirements>;
  M2 operator()(M&& m, Signature* provider) {
    m.unsafeModule.registerProvider(provider);
    return std::move(m.unsafeModule);
  }
};

template <typename M, typename AnnotatedSignature>
struct RegisterFactory {
  using InjectedFunctionType = InjectedFunctionTypeForAssistedFactory<AnnotatedSignature>;
  using RequiredSignature = RequiredSignatureForAssistedFactory<AnnotatedSignature>;
  using NewRequirements = ExpandInjectorsInParams<ExtractRequirementsFromAssistedParams<SignatureArgs<AnnotatedSignature>>>;
  using M1 = AddRequirements<M, NewRequirements>;
  using M2 = AddProvide<M1, std::function<InjectedFunctionType>, NewRequirements>;
  M2 operator()(M&& m, RequiredSignature* factory) {
    m.unsafeModule.template registerFactory<AnnotatedSignature>(factory);
    return std::move(m.unsafeModule);
  }
};

template <typename M, typename Signature>
struct RegisterConstructor {
  using Provider = decltype(ConstructorProvider<Signature>::f);
  using RegisterProviderOperation = RegisterProvider<M, Provider>;
  using M1 = FunctorResult<RegisterProviderOperation, M&&, Provider*>;
  M1 operator()(M&& m) {
    return RegisterProviderOperation()(std::move(m), ConstructorProvider<Signature>::f);
  };
};

template <typename M, typename C>
struct RegisterInstance {
  using M1 = AddProvide<M, C, List<>>;
  M1 operator()(M&& m, C* instance) {
    m.unsafeModule.bindInstance(instance);
    return std::move(m.unsafeModule);
  };
};

template <typename M, typename AnnotatedSignature>
struct RegisterConstructorAsFactory {
  using RequiredSignature = RequiredSignatureForAssistedFactory<AnnotatedSignature>;
  using Provider = decltype(ConstructorFactoryProvider<RequiredSignature>::f);
  using RegisterFactoryOperation = RegisterFactory<M, AnnotatedSignature>;
  using M1 = FunctorResult<RegisterFactoryOperation, M&&, Provider*>;
  M1 operator()(M&& m) {
    return RegisterFactoryOperation()(std::move(m), ConstructorFactoryProvider<RequiredSignature>::f);
  };
};

template <typename M, typename OtherM>
struct InstallModule {
  using OtherM_Rs = typename OtherM::Rs;
  using OtherM_Ps = typename OtherM::Ps;
  using OtherM_Deps = typename OtherM::Deps;
  FruitDelegateCheck(DuplicatedTypesInModuleError<set_intersection<typename M::Ps, OtherM_Ps>>);
  using new_Ps = concat_lists<typename M::Ps, OtherM_Ps>;
  using new_Rs = set_difference<merge_sets<typename M::Rs, OtherM_Rs>, new_Ps>;
  using new_Deps = AddDeps<typename M::Deps, OtherM_Deps>;
  using M1 = ModuleImpl<new_Rs, new_Ps, new_Deps>;
  M1 operator()(M&& m, const OtherM& otherM) {
    m.unsafeModule.install(otherM.unsafeModule);
    return std::move(m.unsafeModule);
  }
};

template <typename RsParam, typename PsParam, typename DepsParam>
ModuleImpl<RsParam, PsParam, DepsParam>::ModuleImpl(UnsafeModule&& unsafeModule) 
  : unsafeModule(unsafeModule) {
}

template <typename RsParam, typename PsParam, typename DepsParam>
template <typename Source_Rs, typename Source_Ps, typename Source_Deps>
ModuleImpl<RsParam, PsParam, DepsParam>::ModuleImpl(const ModuleImpl<Source_Rs, Source_Ps, Source_Deps>& sourceModule) {
  // We need to register:
  // * All the types provided by the new module
  // * All the types required by the old module
  // except:
  // * The ones already provided by the old module.
  // * The ones required by the new one.
  using ToRegister = set_difference<merge_sets<Ps, Source_Rs>,
                                    merge_sets<Rs, Source_Ps>>;
  using SourceModule = ModuleImpl<Source_Rs, Source_Ps, Source_Deps>;
  using Helper = EnsureProvidedTypes<SourceModule, Rs, ToRegister>;
  SourceModule sourceModuleCopy = sourceModule;
  // Add the required bindings.
  auto extendedModule = Helper()(std::move(sourceModuleCopy));
  
  FruitStaticAssert(true || sizeof(CheckModuleEntails<decltype(extendedModule), M>), "");
  
  // Extract the bindings from the extended module.
  unsafeModule = std::move(extendedModule.unsafeModule);
}

} // namespace impl
} // namespace fruit


#endif // FRUIT_MODULE_TEMPLATES_H
