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

#ifndef FRUIT_INJECTION_ERRORS
#define FRUIT_INJECTION_ERRORS

#include "component.utils.h"

namespace fruit {
namespace impl {

template <typename T>
struct NoBindingFoundError {
  static_assert(false && sizeof(T),
                "No explicit binding nor C::Inject definition was found for T.");
};

template <typename... Ts>
struct CheckNoRepeatedTypes {
  static_assert(list_size<list_to_set<List<Ts...>>>::value == list_size<List<Ts...>>::value, 
                "A type was specified more than once. Requirements and provided types should be unique.");
};

template <bool b, typename T>
struct CheckHasNoSelfLoop {
  static_assert(b, "Found a loop in the dependencies involving T.");
};

template <typename T, typename C>
struct CheckClassType {
  static_assert(std::is_same<T, C>::value,
                "A non-class type T was specified. Use C instead.");
};

template <bool b, typename C>
struct CheckTypeAlreadyBound {
  static_assert(b,
                "Trying to bind C but it is already bound.");
};

template <typename RequiredSignature, typename SignatureInInjectTypedef>
struct CheckSameSignatureInInjectionTypedef {
  static_assert(std::is_same<RequiredSignature, SignatureInInjectTypedef>::value,
                "The required C factory doesn't have the same signature as the Inject annotation in C.");
};

template <typename DuplicatedTypes>
struct DuplicatedTypesInComponentError {
  static_assert(is_empty_list<DuplicatedTypes>::value,
                "The installed component provides some types that are already provided by the current component.");
};

template <typename... Requirements>
struct CheckNoRequirementsInProvider {
  static_assert(is_empty_list<fruit::impl::List<Requirements...>>::value, 
                "A provider (including injectors) can't have requirements. If you want Fruit to try auto-resolving the requirements in the current scope, cast the component to a component with no requirements before constructing the injector with it.");
};

template <typename Rs>
struct CheckNoRequirementsInProviderHelper {};

template <typename... Requirements>
struct CheckNoRequirementsInProviderHelper<fruit::impl::List<Requirements...>> {
  FruitDelegateCheck(CheckNoRequirementsInProvider<Requirements...>);
};

template <typename C, typename CandidateSignature>
struct InjectTypedefNotASignature {
  static_assert(fruit::impl::IsValidSignature<CandidateSignature>::value,
                "C::Inject should be a typedef to a signature, e.g. C(int)");
};

template <typename C, typename SignatureReturnType>
struct InjectTypedefForWrongClass {
  static_assert(std::is_same<C, SignatureReturnType>::value,
                "C::Inject is a signature, but does not return a C. Maybe the class C has no Inject typedef and inherited the base class' one? If that's not the case, make sure it returns just C, not C* or other types.");
};

template <typename CandidateSignature>
struct ParameterIsNotASignature {
  static_assert(fruit::impl::IsValidSignature<CandidateSignature>::value,
                "CandidateSignature was specified as parameter, but it's not a signature. Signatures are of the form MyClass(int, float).");
};

template <typename Signature>
struct ConstructorDoesNotExist {}; // Not used.

template <typename C, typename... Args>
struct ConstructorDoesNotExist<C(Args...)> {
  static_assert(std::is_constructible<C, Args...>::value,
                "The specified constructor does not exist.");
};

template <typename I, typename C>
struct NotABaseClassOf {
  static_assert(std::is_base_of<I, C>::value,
                "I is not a base class of C.");
};

template <typename Signature, typename ProviderType>
struct FunctorUsedAsProvider {
  static_assert(std::is_constructible<Signature*, ProviderType>::value,
                "A stateful lambda or a non-lambda functor was used as provider. Only functions and stateless lambdas can be used as providers.");
};

template <typename... ComponentRequirements>
struct ComponentWithRequirementsInInjectorError {
  static_assert(fruit::impl::is_empty_list<fruit::impl::List<ComponentRequirements...>>::value,
                "When using the two-argument constructor of Injector, the component used as second parameter must not have requirements (while the normalized component can), but the specified component requires ComponentRequirements.");
};

template <typename ComponentRequirements>
struct ComponentWithRequirementsInInjectorErrorHelper {};

template <typename... ComponentRequirements>
struct ComponentWithRequirementsInInjectorErrorHelper<fruit::impl::List<ComponentRequirements...>> {
  FruitDelegateCheck(ComponentWithRequirementsInInjectorError<ComponentRequirements...>);
};

template <typename... UnsatisfiedRequirements>
struct UnsatisfiedRequirementsInNormalizedComponent {
  static_assert(fruit::impl::is_empty_list<fruit::impl::List<UnsatisfiedRequirements...>>::value,
                "The requirements in UnsatisfiedRequirements are required by the NormalizedComponent but are not provided by the Component (second parameter of the Injector constructor).");
};

template <typename UnsatisfiedRequirements>
struct UnsatisfiedRequirementsInNormalizedComponentHelper {};

template <typename... UnsatisfiedRequirements>
struct UnsatisfiedRequirementsInNormalizedComponentHelper<fruit::impl::List<UnsatisfiedRequirements...>> {
  FruitDelegateCheck(UnsatisfiedRequirementsInNormalizedComponent<UnsatisfiedRequirements...>);
};

template <typename... TypesNotProvided>
struct TypesInInjectorNotProvided {
  static_assert(fruit::impl::is_empty_list<fruit::impl::List<TypesNotProvided...>>::value,
                "The types in TypesNotProvided are declared as provided by the injector, but none of the two components passed to the Injector constructor provides them.");
};

template <typename TypesNotProvided>
struct TypesInInjectorNotProvidedHelper {};

template <typename... TypesNotProvided>
struct TypesInInjectorNotProvidedHelper<fruit::impl::List<TypesNotProvided...>> {
  FruitDelegateCheck(TypesInInjectorNotProvided<TypesNotProvided...>);
};

template <typename T, bool is_provided>
struct TypeNotProvidedError {
  static_assert(is_provided,
                "Trying to get an instance of T, but it is not provided by this Provider/Injector.");
};

template <typename C, typename InjectSignature>
struct NoConstructorMatchingInjectSignature {
  static_assert(is_constructible_with_list<C, UnlabelAssisted<SignatureArgs<InjectSignature>>>::value,
                "C contains an Inject typedef but it's not constructible with the specified types");
};

template <typename ExpectedSignature, typename FunctorSignature>
struct FunctorSignatureDoesNotMatch {
  static_assert(std::is_same<ExpectedSignature, FunctorSignature>::value,
                "Error: the specified functor doesn't have the expected signature (it should be the same as ExpectedSignature minus any Assisted types).");
};

template <bool returns_pointer, typename Signature>
struct FactoryReturningPointer {
  static_assert(!returns_pointer,
                "Error: the specified factory returns a pointer. This is not supported; return a value or a std::unique_ptr instead.");
};
  
#ifdef FRUIT_EXTRA_DEBUG
// NOTE: Internal-only error used for debugging, not user-visible.
template <typename AdditionalProvidedTypes>
struct CheckNoAdditionalProvidedTypes {
  static_assert(is_empty_list<AdditionalProvidedTypes>::value, 
                "The types in AdditionalProvidedTypes are provided by the new component but weren't provided before.");
};

// NOTE: Internal-only error used for debugging, not user-visible.
template <typename AdditionalBindings>
struct CheckNoAdditionalBindings {
  static_assert(is_empty_list<AdditionalBindings>::value, 
                "The types in AdditionalBindings are bindings in the new component but weren't bindings before.");
};

// NOTE: Internal-only error used for debugging, not user-visible.
template <typename NoLongerRequiredTypes>
struct CheckNoTypesNoLongerRequired {
  static_assert(is_empty_list<NoLongerRequiredTypes>::value, 
                "The types in NoLongerRequiredTypes were required before but are no longer required by the new component.");
};

// NOTE: Internal-only error used for debugging, not user-visible.
template <typename T, typename U>
struct CheckSame {
  static_assert(std::is_same<T, U>::value, "T and U should be the same type");
};

template <typename Deps, typename NormalizedDeps>
struct CheckDepsNormalized {
  static_assert(is_same_set<Deps, NormalizedDeps>::value,
                "Internal error: non-normalized deps");
};

#endif


} // namespace impl
} // namespace fruit


#endif // FRUIT_INJECTION_ERRORS
