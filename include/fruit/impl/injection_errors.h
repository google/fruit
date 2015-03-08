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
#include "fruit_assert.h"

namespace fruit {
namespace impl {

template <typename... Ts>
struct AlwaysFalse {
  static constexpr bool value = false;
};

template <typename T>
struct NoBindingFoundError {
  static_assert(AlwaysFalse<T>::value,
                "No explicit binding nor C::Inject definition was found for T.");
};

template <typename... Ts>
struct RepeatedTypesError {
  static_assert(AlwaysFalse<Ts...>::value,
                "A type was specified more than once. Requirements and provided types should be "
                "unique.");
};

template <typename T, typename... Requirements>
struct SelfLoopError {
  static_assert(AlwaysFalse<T>::value,
                "Tried to add T in the component (depending on Requirements) but this introduces a "
                "loop in the dependencies.");
};

template <typename T, typename C>
struct NonClassTypeError {
  static_assert(AlwaysFalse<T>::value,
                "A non-class type T was specified. Use C instead.");
};

template <typename C>
struct TypeAlreadyBoundError {
  static_assert(AlwaysFalse<C>::value,
                "Trying to bind C but it is already bound.");
};

template <typename RequiredSignature, typename SignatureInInjectTypedef>
struct RequiredFactoryWithDifferentSignatureError {
  static_assert(AlwaysFalse<RequiredSignature>::value,
                "The required C factory doesn't have the same signature as the Inject annotation in "
                "C.");
};

template <typename... DuplicatedTypes>
struct DuplicateTypesInComponentError {
  static_assert(AlwaysFalse<DuplicatedTypes...>::value,
                "The installed component provides some types that are already provided by the "
                "current component.");
};

template <typename... Requirements>
struct InjectorWithRequirementsError {
  static_assert(AlwaysFalse<Requirements...>::value,
                "Injectors can't have requirements. If you want Fruit to try auto-resolving the "
                "requirements in the injector's scope, cast the component to a component with no "
                "requirements before constructing the injector with it.");
};

template <typename C, typename CandidateSignature>
struct InjectTypedefNotASignatureError {
  static_assert(AlwaysFalse<C>::value,
                "C::Inject should be a typedef to a signature, e.g. C(int)");
};

template <typename C, typename SignatureReturnType>
struct InjectTypedefForWrongClassError {
  static_assert(AlwaysFalse<C>::value,
                "C::Inject is a signature, but does not return a C. Maybe the class C has no "
                "Inject typedef and inherited the base class' one? If that's not the case, make "
                "sure it returns just C, not C* or other types.");
};

template <typename CandidateSignature>
struct NotASignatureError {
  static_assert(AlwaysFalse<CandidateSignature>::value,
                "CandidateSignature was specified as parameter, but it's not a signature. "
                "Signatures are of the form MyClass(int, float).");
};

template <typename Signature>
struct ConstructorDoesNotExistError {
  static_assert(AlwaysFalse<Signature>::value,
                "The specified constructor does not exist.");
};

template <typename I, typename C>
struct NotABaseClassOfError {
  static_assert(AlwaysFalse<I>::value,
                "I is not a base class of C.");
};

template <typename ProviderType>
struct FunctorUsedAsProviderError {
  static_assert(AlwaysFalse<ProviderType>::value,
                "A stateful lambda or a non-lambda functor was used as provider. Only functions "
                "and stateless lambdas can be used as providers.");
};

template <typename... ComponentRequirements>
struct ComponentWithRequirementsInInjectorError {
  static_assert(AlwaysFalse<ComponentRequirements...>::value,
                "When using the two-argument constructor of Injector, the component used as second "
                "parameter must not have requirements (while the normalized component can), but "
                "the specified component requires ComponentRequirements.");
};

template <typename... UnsatisfiedRequirements>
struct UnsatisfiedRequirementsInNormalizedComponentError {
  static_assert(AlwaysFalse<UnsatisfiedRequirements...>::value,
                "The requirements in UnsatisfiedRequirements are required by the "
                "NormalizedComponent but are not provided by the Component (second parameter of "
                "the Injector constructor).");
};

template <typename... TypesNotProvided>
struct TypesInInjectorNotProvidedError {
  static_assert(AlwaysFalse<TypesNotProvided...>::value,
                "The types in TypesNotProvided are declared as provided by the injector, but none "
                "of the two components passed to the Injector constructor provides them.");
};

template <typename T>
struct TypeNotProvidedError {
  static_assert(AlwaysFalse<T>::value,
                "Trying to get an instance of T, but it is not provided by this "
                "Provider/Injector.");
};

template <typename C, typename InjectSignature>
struct NoConstructorMatchingInjectSignatureError {
  static_assert(AlwaysFalse<C>::value,
                "C contains an Inject typedef but it's not constructible with the specified types");
};

template <typename ExpectedSignature, typename FunctorSignature>
struct FunctorSignatureDoesNotMatchError {
  static_assert(AlwaysFalse<ExpectedSignature>::value,
                "Unexpected functor signature (it should be the same as ExpectedSignature minus "
                "any Assisted types).");
};

template <typename Signature>
struct FactoryReturningPointerError {
  static_assert(AlwaysFalse<Signature>::value,
                "The specified factory returns a pointer. This is not supported; return a value or "
                "an std::unique_ptr instead.");
};

template <typename Lambda>
struct LambdaWithCapturesError {
  // It's not guaranteed by the standard, but it's reasonable to expect lambdas with no captures
  // to be empty. This is always the case in GCC and Clang, but is not guaranteed to work in all
  // conforming C++ compilers. If this error happens for a lambda with no captures, please file a
  // bug at https://github.com/google/fruit/issues and indicate the compiler (with version) that
  // you're using.
  static_assert(AlwaysFalse<Lambda>::value,
                "Only lambdas with no captures are supported.");
};





struct LambdaWithCapturesErrorTag {
  template <typename Lambda>
  using apply = LambdaWithCapturesError<Lambda>;
};

struct FactoryReturningPointerErrorTag {
  template <typename Signature>
  using apply = FactoryReturningPointerError<Signature>;
};

struct NoBindingFoundErrorTag {
  template <typename T>
  using apply = NoBindingFoundError<T>;
};

struct RepeatedTypesErrorTag {
  template <typename... Ts>
  using apply = RepeatedTypesError<Ts...>;
};

struct SelfLoopErrorTag {
  template <typename T, typename... Requirements>
  using apply = SelfLoopError<T, Requirements...>;
};

struct NonClassTypeErrorTag {
  template <typename T, typename C>
  using apply = NonClassTypeError<T, C>;
};

struct TypeAlreadyBoundErrorTag {
  template <typename C>
  using apply = TypeAlreadyBoundError<C>;
};

struct RequiredFactoryWithDifferentSignatureErrorTag {
  template <typename RequiredSignature, typename SignatureInInjectTypedef>
  using apply = RequiredFactoryWithDifferentSignatureError<RequiredSignature, SignatureInInjectTypedef>;
};

struct DuplicateTypesInComponentErrorTag {
  template <typename... DuplicatedTypes>
  using apply = DuplicateTypesInComponentError<DuplicatedTypes...>;
};

struct InjectorWithRequirementsErrorTag {
  template <typename... Requirements>
  using apply = InjectorWithRequirementsError<Requirements...>;
};

struct ComponentWithRequirementsInInjectorErrorTag {
  template <typename... ComponentRequirements>
  using apply = ComponentWithRequirementsInInjectorError<ComponentRequirements...>;
};

struct InjectTypedefNotASignatureErrorTag {
  template <typename C, typename TypeInInjectTypedef>
  using apply = InjectTypedefNotASignatureError<C, TypeInInjectTypedef>;
};

struct InjectTypedefForWrongClassErrorTag {
  template <typename C, typename ReturnTypeOfInjectTypedef>
  using apply = InjectTypedefForWrongClassError<C, ReturnTypeOfInjectTypedef>;
};

struct UnsatisfiedRequirementsInNormalizedComponentErrorTag {
  template <typename... UnsatisfiedRequirements>
  using apply = UnsatisfiedRequirementsInNormalizedComponentError<UnsatisfiedRequirements...>;
};

struct TypesInInjectorNotProvidedErrorTag {
  template <typename... TypesNotProvided>
  using apply = TypesInInjectorNotProvidedError<TypesNotProvided...>;
};

struct FunctorUsedAsProviderErrorTag {
  template <typename ProviderType>
  using apply = FunctorUsedAsProviderError<ProviderType>;
};

struct ConstructorDoesNotExistErrorTag {
  template <typename Signature>
  using apply = ConstructorDoesNotExistError<Signature>;
};

struct NotABaseClassOfErrorTag {
  template <typename I, typename C>
  using apply = NotABaseClassOfError<I, C>;
};

struct NotASignatureErrorTag {
  template <typename CandidateSignature>
  using apply = NotASignatureError<CandidateSignature>;
};

struct TypeNotProvidedErrorTag {
  template <typename T>
  using apply = TypeNotProvidedError<T>;
};

struct NoConstructorMatchingInjectSignatureErrorTag {
  template <typename C, typename InjectSignature>
  using apply = NoConstructorMatchingInjectSignatureError<C, InjectSignature>;
};

struct FunctorSignatureDoesNotMatchErrorTag {
  template <typename ExpectedSignature, typename FunctorSignature>
  using apply = FunctorSignatureDoesNotMatchError<ExpectedSignature, FunctorSignature>;
};




} // namespace impl
} // namespace fruit


#endif // FRUIT_INJECTION_ERRORS
