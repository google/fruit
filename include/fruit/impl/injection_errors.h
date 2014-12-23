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

#include "meta/set.h"
#include "meta/component.h"

namespace fruit {
namespace impl {

template <typename T>
struct NoBindingFoundError {
  static_assert(false && sizeof(T),
                "No explicit binding nor C::Inject definition was found for T.");
};

template <typename... Ts>
struct CheckNoRepeatedTypes {
  static_assert(meta::ApplyC<meta::ListSize, meta::Apply<meta::ListToSet, meta::List<Ts...>>>::value
             == meta::ApplyC<meta::ListSize, meta::List<Ts...>>::value, 
                "A type was specified more than once. Requirements and provided types should be unique.");
};

template <bool has_no_self_loop, typename T, typename... Requirements>
struct CheckHasNoSelfLoop {
  static_assert(has_no_self_loop,
                "Tried to add T in the component (depending on Requirements) but this introduces a loop in the dependencies.");
};

template <bool has_no_self_loop, typename T, typename Requirements>
struct CheckHasNoSelfLoopHelper;

template <bool has_no_self_loop, typename T, typename... Requirements>
struct CheckHasNoSelfLoopHelper<has_no_self_loop, T, meta::List<Requirements...>> 
: public CheckHasNoSelfLoop<has_no_self_loop, T, Requirements...> {
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
  static_assert(meta::ApplyC<meta::IsEmptyList, DuplicatedTypes>::value,
                "The installed component provides some types that are already provided by the current component.");
};

template <typename... Requirements>
struct CheckNoRequirementsInInjector {
  static_assert(meta::ApplyC<meta::IsEmptyList, meta::List<Requirements...>>::value, 
                "Injectors can't have requirements. If you want Fruit to try auto-resolving the requirements in the injector's scope, cast the component to a component with no requirements before constructing the injector with it.");
};

template <typename Rs>
struct CheckNoRequirementsInInjectorHelper {};

template <typename... Requirements>
struct CheckNoRequirementsInInjectorHelper<meta::List<Requirements...>> {
  FruitDelegateCheck(CheckNoRequirementsInInjector<Requirements...>);
};

template <typename C, typename CandidateSignature>
struct InjectTypedefNotASignature {
  static_assert(meta::ApplyC<meta::IsValidSignature, CandidateSignature>::value,
                "C::Inject should be a typedef to a signature, e.g. C(int)");
};

template <typename C, typename SignatureReturnType>
struct InjectTypedefForWrongClass {
  static_assert(std::is_same<C, SignatureReturnType>::value,
                "C::Inject is a signature, but does not return a C. Maybe the class C has no Inject typedef and inherited the base class' one? If that's not the case, make sure it returns just C, not C* or other types.");
};

template <typename CandidateSignature>
struct ParameterIsNotASignature {
  static_assert(meta::ApplyC<meta::IsValidSignature, CandidateSignature>::value,
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
  static_assert(meta::ApplyC<meta::IsEmptyList, meta::List<ComponentRequirements...>>::value,
                "When using the two-argument constructor of Injector, the component used as second parameter must not have requirements (while the normalized component can), but the specified component requires ComponentRequirements.");
};

template <typename ComponentRequirements>
struct ComponentWithRequirementsInInjectorErrorHelper {};

template <typename... ComponentRequirements>
struct ComponentWithRequirementsInInjectorErrorHelper<meta::List<ComponentRequirements...>> {
  FruitDelegateCheck(ComponentWithRequirementsInInjectorError<ComponentRequirements...>);
};

template <typename... UnsatisfiedRequirements>
struct UnsatisfiedRequirementsInNormalizedComponent {
  static_assert(meta::ApplyC<meta::IsEmptyList, meta::List<UnsatisfiedRequirements...>>::value,
                "The requirements in UnsatisfiedRequirements are required by the NormalizedComponent but are not provided by the Component (second parameter of the Injector constructor).");
};

template <typename UnsatisfiedRequirements>
struct UnsatisfiedRequirementsInNormalizedComponentHelper {};

template <typename... UnsatisfiedRequirements>
struct UnsatisfiedRequirementsInNormalizedComponentHelper<meta::List<UnsatisfiedRequirements...>> {
  FruitDelegateCheck(UnsatisfiedRequirementsInNormalizedComponent<UnsatisfiedRequirements...>);
};

template <typename... TypesNotProvided>
struct TypesInInjectorNotProvided {
  static_assert(meta::ApplyC<meta::IsEmptyList, meta::List<TypesNotProvided...>>::value,
                "The types in TypesNotProvided are declared as provided by the injector, but none of the two components passed to the Injector constructor provides them.");
};

template <typename TypesNotProvided>
struct TypesInInjectorNotProvidedHelper {};

template <typename... TypesNotProvided>
struct TypesInInjectorNotProvidedHelper<meta::List<TypesNotProvided...>> {
  FruitDelegateCheck(TypesInInjectorNotProvided<TypesNotProvided...>);
};

template <typename T, bool is_provided>
struct TypeNotProvidedError {
  static_assert(is_provided,
                "Trying to get an instance of T, but it is not provided by this Provider/Injector.");
};

template <typename C, typename InjectSignature>
struct NoConstructorMatchingInjectSignature {
  static_assert(meta::ApplyC<meta::IsConstructibleWithList, C, meta::Apply<meta::UnlabelAssisted, meta::Apply<meta::SignatureArgs, InjectSignature>>>::value,
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

template <typename Lambda>
struct CheckEmptyLambda {
  static_assert(std::is_empty<Lambda>::value,
                "Error: only lambdas with no captures are supported, and those should satisfy is_empty. If this error happens for a lambda with no captures, please file a bug at https://github.com/google/fruit/issues .");
};

} // namespace impl
} // namespace fruit


#endif // FRUIT_INJECTION_ERRORS
