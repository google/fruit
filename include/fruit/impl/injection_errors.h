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

#ifndef FRUIT_INJECTION_ERRORS_H
#define FRUIT_INJECTION_ERRORS_H

#include <fruit/impl/fruit_assert.h>
#include <fruit/impl/meta/set.h>

namespace fruit {
namespace impl {

template <typename... Ts>
struct AlwaysFalse {
  static constexpr bool value = false;
};

template <typename T>
struct NoBindingFoundError {
  static_assert(AlwaysFalse<T>::value, "No explicit binding nor C::Inject definition was found for T.");
};

template <typename T, typename C>
struct NoBindingFoundForAbstractClassError {
  static_assert(AlwaysFalse<T>::value,
                "No explicit binding was found for T, and note that C is an abstract class (so Fruit can't auto-inject "
                "this type, "
                "even if it has an Inject typedef or an INJECT annotation that will be ignored).");
};

template <typename... Ts>
struct RepeatedTypesError {
  static_assert(AlwaysFalse<Ts...>::value,
                "A type was specified more than once. Requirements and provided types should be unique.");
};

template <typename... TypesInLoop>
struct SelfLoopError {
  static_assert(AlwaysFalse<TypesInLoop...>::value,
                "Found a loop in the dependencies! The types in TypesInLoop all depend on the next, and the "
                "last one depends on the first.");
};

template <typename T, typename C>
struct NonClassTypeError {
  static_assert(AlwaysFalse<T>::value, "A non-class type T was specified. Use C instead.");
};

template <typename AnnotatedT, typename T>
struct AnnotatedTypeError {
  static_assert(AlwaysFalse<T>::value, "An annotated type was specified where a non-annotated type was expected.");
};

template <typename C>
struct TypeAlreadyBoundError {
  static_assert(AlwaysFalse<C>::value, "Trying to bind C but it is already bound.");
};

template <typename RequiredSignature, typename SignatureInInjectTypedef>
struct RequiredFactoryWithDifferentSignatureError {
  static_assert(AlwaysFalse<RequiredSignature>::value,
                "The required C factory doesn't have the same signature as the Inject annotation in C.");
};

template <typename Signature, typename SignatureInLambda>
struct AnnotatedSignatureDifferentFromLambdaSignatureError {
  static_assert(AlwaysFalse<Signature>::value,
                "The annotated signature specified is not the same as the lambda's signature (after removing "
                "annotations).");
};

template <typename... DuplicatedTypes>
struct DuplicateTypesInComponentError {
  static_assert(AlwaysFalse<DuplicatedTypes...>::value,
                "The installed component provides some types that are already provided by the current "
                "component.");
};

template <typename... Requirements>
struct InjectorWithRequirementsError {
  static_assert(AlwaysFalse<Requirements...>::value,
                "Injectors can't have requirements. If you want Fruit to try auto-resolving the requirements "
                "in the injector's scope, cast the component to a component with no requirements before "
                "constructing the injector with it.");
};

template <typename C, typename CandidateSignature>
struct InjectTypedefNotASignatureError {
  static_assert(AlwaysFalse<C>::value, "C::Inject should be a typedef to a signature, e.g. C(int)");
};

template <typename C, typename SignatureReturnType>
struct InjectTypedefForWrongClassError {
  static_assert(AlwaysFalse<C>::value,
                "C::Inject is a signature, but does not return a C. Maybe the class C has no Inject typedef "
                "and inherited the base class' one? If that's not the case, make sure it returns just C, not "
                "C* or other types.");
};

template <typename C>
struct InjectTypedefWithAnnotationError {
  static_assert(AlwaysFalse<C>::value,
                "C::Inject is a signature that returns an annotated type. The annotation must be removed, "
                "Fruit will deduce the correct annotation based on how the required binding.");
};

template <typename CandidateSignature>
struct NotASignatureError {
  static_assert(AlwaysFalse<CandidateSignature>::value,
                "CandidateSignature was specified as parameter, but it's not a signature. Signatures are of "
                "the form MyClass(int, float).");
};

template <typename CandidateLambda>
struct NotALambdaError {
  static_assert(AlwaysFalse<CandidateLambda>::value,
                "CandidateLambda was specified as parameter, but it's not a lambda.");
};

template <typename Signature>
struct ConstructorDoesNotExistError {
  static_assert(AlwaysFalse<Signature>::value, "The specified constructor does not exist.");
};

template <typename I, typename C>
struct NotABaseClassOfError {
  static_assert(AlwaysFalse<I>::value, "I is not a base class of C.");
};

template <typename ProviderType>
struct FunctorUsedAsProviderError {
  static_assert(AlwaysFalse<ProviderType>::value,
                "A stateful lambda or a non-lambda functor was used as provider. Only functions and stateless "
                "lambdas can be used as providers.");
};

template <typename... ComponentRequirements>
struct ComponentWithRequirementsInInjectorError {
  static_assert(AlwaysFalse<ComponentRequirements...>::value,
                "When using the two-argument constructor of Injector, the component used as second parameter "
                "must not have requirements (while the normalized component can), but the specified component "
                "requires ComponentRequirements.");
};

template <typename... UnsatisfiedRequirements>
struct UnsatisfiedRequirementsInNormalizedComponentError {
  static_assert(AlwaysFalse<UnsatisfiedRequirements...>::value,
                "The requirements in UnsatisfiedRequirements are required by the NormalizedComponent but are "
                "not provided by the Component (second parameter of the Injector constructor).");
};

template <typename... TypesNotProvided>
struct TypesInInjectorNotProvidedError {
  static_assert(AlwaysFalse<TypesNotProvided...>::value,
                "The types in TypesNotProvided are declared as provided by the injector, but none of the two "
                "components passed to the Injector constructor provides them.");
};

template <typename... TypesProvidedAsConstOnly>
struct TypesInInjectorProvidedAsConstOnlyError {
  static_assert(
      AlwaysFalse<TypesProvidedAsConstOnly...>::value,
      "The types in TypesProvidedAsConstOnly are declared as non-const provided types by the injector, but the "
      "components passed to the Injector constructor provide them as const only. You should mark them as const in the "
      "injector (e.g., switching from Injector<T> to Injector<const T>) or mark them as non-const in the "
      "Component/NormalizedComponent (e.g. switching from [Normalized]Component<const T> to "
      "[Normalized]Component<T>).");
};

template <typename T>
struct TypeNotProvidedError {
  static_assert(AlwaysFalse<T>::value,
                "Trying to get an instance of T, but it is not provided by this Provider/Injector.");
};

template <typename T>
struct TypeProvidedAsConstOnlyError {
  static_assert(
      AlwaysFalse<T>::value,
      "Trying to get an instance of T, but it is only provided as a constant by this Provider/Injector and a non-const "
      "pointer/reference/Provider was requested. You should either switch to injecting a const value (e.g. switching "
      "from"
      " injecting T*, T&, std::unique_ptr<T> or Provider<T> to injecting a T, const T*, const T& or Provider<const T>) "
      "or get the value from an Injector/Provider that provides it as a non-const type (e.g. switching from calling "
      "get "
      "on an Injector<const T> or on a Provider<const T> to calling get on an Injector<T> or a Provider<T>).");
};

template <typename T>
struct NonConstBindingRequiredButConstBindingProvidedError {
  static_assert(
      AlwaysFalse<T>::value,
      "The type T was provided as constant, however one of the constructors/providers/factories in this component "
      "requires it as a non-constant (or this Component declares it as a non-const provided/required type). "
      "If you want to only have a const binding for this type, you should change the places that use the type to "
      "inject "
      "a constant value (e.g. T, const T*, const T& and Provider<const T> are ok while you should avoid injecting T*, "
      "T&,"
      " std::unique_ptr<T> and Provider<T>) and if the type is in Component<...> make sure that it's marked as const "
      "there"
      " (e.g. Component<const T> and Component<Required<const T>> are ok while Component<T> and Component<Required<T>> "
      "are "
      "not. "
      "On the other hand, if you want to have a non-const binding for this type, you should switch to a non-const "
      "bindInstance (if you're binding an instance) or changing any installed component functions to declare the type "
      "as "
      "non-const, e.g. Component<T> or Component<Required<T>> instead of Component<const T> and "
      "Component<Required<const T>>.");
};

template <typename C, typename InjectSignature>
struct NoConstructorMatchingInjectSignatureError {
  static_assert(AlwaysFalse<C>::value,
                "C contains an Inject typedef but it's not constructible with the specified types");
};

template <typename ExpectedSignature, typename FunctorSignature>
struct FunctorSignatureDoesNotMatchError {
  static_assert(AlwaysFalse<ExpectedSignature>::value,
                "Unexpected functor signature (it should be the same as ExpectedSignature minus any Assisted "
                "types).");
};

template <typename Signature>
struct FactoryReturningPointerError {
  static_assert(AlwaysFalse<Signature>::value,
                "The specified factory returns a pointer. This is not supported; return a value or an "
                "std::unique_ptr instead.");
};

template <typename Lambda>
struct LambdaWithCapturesError {
  // It's not guaranteed by the standard, but it's reasonable to expect lambdas with no captures
  // to be empty. This is always the case in GCC and Clang, but is not guaranteed to work in all
  // conforming C++ compilers. If this error happens for a lambda with no captures, please file a
  // bug at https://github.com/google/fruit/issues and indicate the compiler (with version) that
  // you're using.
  static_assert(AlwaysFalse<Lambda>::value, "Only lambdas with no captures are supported.");
};

template <typename Lambda>
struct NonTriviallyCopyableLambdaError {
  // It's not guaranteed by the standard, but it's reasonable to expect lambdas with no captures
  // to be trivially copyable. This is always the case in GCC and Clang, but is not guaranteed to
  // work in all conforming C++ compilers. If this error happens for a lambda with no captures,
  // please file a bug at https://github.com/google/fruit/issues and indicate the compiler (with
  // version) that you're using.
  static_assert(AlwaysFalse<Lambda>::value,
                "Only trivially copyable lambdas are supported. Make sure that your lambda has no captures.");
};

template <typename C>
struct CannotConstructAbstractClassError {
  static_assert(AlwaysFalse<C>::value, "The specified class can't be constructed because it's an abstract class.");
};

template <typename C>
struct InterfaceBindingToSelfError {
  static_assert(AlwaysFalse<C>::value,
                "The type C was bound to itself. If this was intentional, to \"tell Fruit to inject the type"
                " C\", this binding is unnecessary, just remove it. bind<I,C>() is to tell Fruit about"
                " base-derived class relationships.");
};

template <typename TypeParameter, typename TypeOfValue>
struct TypeMismatchInBindInstanceError {
  static_assert(AlwaysFalse<TypeParameter>::value,
                "A type parameter was specified in bindInstance() but it doesn't match the value type"
                " (even after removing the fruit::Annotation<>, if any). Please change the type parameter"
                " to be the same as the type of the value (or a subclass).");
};

template <typename RequiredType>
struct RequiredTypesInComponentArgumentsError {
  static_assert(AlwaysFalse<RequiredType>::value,
                "A Required<...> type was passed as a non-first template parameter to fruit::Component or "
                "fruit::NormalizedComponent. "
                "All required types (if any) should be passed together as a single Required<> type passed as the first "
                "type argument of fruit::Component (and fruit::NormalizedComponent). For example, write "
                "fruit::Component<fruit::Required<Foo, Bar>, Baz> instead of "
                "fruit::Component<fruit::Required<Foo>, fruit::Required<Bar>, Baz>.");
};

template <typename T>
struct NonInjectableTypeError {
  static_assert(
      AlwaysFalse<T>::value,
      "The type T is not injectable. Injectable types are of the form X, X*, X&, const X, const X*, const X&, "
      "std::shared_ptr<X>, or Provider<X> where X is a fundamental type (excluding void), a class, a struct or "
      "an enum.");
};

template <typename T>
struct ConstBindingDeclaredAsRequiredButNonConstBindingRequiredError {
  static_assert(
      AlwaysFalse<T>::value,
      "The type T was declared as a const Required type in the returned Component, however a non-const binding is "
      "required. You should either change all the usages of this type so that they no longer require a non-const "
      "binding "
      "(i.e., you shouldn't inject T*, T& or std::shared_ptr<T>) or you should remove the 'const' in the type of the "
      "returned Component, e.g. changing fruit::Component<fruit::Required<const T, ...>, ...> to "
      "fruit::Component<fruit::Required<T, ...>, ...>.");
};

template <typename T>
struct ProviderReturningPointerToAbstractClassWithNoVirtualDestructorError {
  static_assert(
      AlwaysFalse<T>::value,
      "registerProvider() was called with a lambda that returns a pointer to T, but T is an abstract class with no "
      "virtual destructor so when the injector is deleted Fruit will be unable to call the right destructor (the one "
      "of "
      "the concrete class that was then casted to T). You must either add a virtual destructor to T or change the "
      "registerProvider() call to return a pointer to the concrete class (and then add a bind<T, TImpl>() so that T is "
      "bound).");
};

template <typename T>
struct MultibindingProviderReturningPointerToAbstractClassWithNoVirtualDestructorError {
  static_assert(
      AlwaysFalse<T>::value,
      "registerMultibindingProvider() was called with a lambda that returns a pointer to T, but T is an abstract class "
      "with no virtual destructor so when the injector is deleted Fruit will be unable to call the right destructor "
      "(the "
      "one of the concrete class that was then casted to T). You must add a virtual destructor to T or replace the "
      "registerMultibindingProvider() with a registerProvider() for the concrete class and an addMultibinding() for T. "
      "Note that with the latter, if you end up with multiple addMultibinding() calls for the same concrete class, "
      "there will be only one instance of the concrete class in the injector, not one per addMultibdinding() call; if "
      "you want separate instances you might want to use annotated injection for the concrete class (so that there's "
      "one "
      "instance per annotation).");
};

template <typename T>
struct RegisterFactoryForUniquePtrOfAbstractClassWithNoVirtualDestructorError {
  static_assert(AlwaysFalse<T>::value,
                "registerFactory() was called with a lambda that returns a std::unique_ptr<T>, but T is an abstract "
                "class with no "
                "virtual destructor so when the returned std::unique_ptr<T> object is deleted the wrong destructor "
                "will be called "
                "(T's destructor instead of the one of the concrete class that was then casted to T). You must add a "
                "virtual destructor to T.");
};

template <typename BaseFactory, typename DerivedFactory>
struct FactoryBindingForUniquePtrOfClassWithNoVirtualDestructorError {
  static_assert(
      AlwaysFalse<BaseFactory>::value,
      "Fruit was trying to bind BaseFactory to DerivedFactory but the return type of BaseFactory is a std::unique_ptr "
      "of "
      "a class with no virtual destructor, so when the std::unique_ptr object is destroyed the wrong destructor would "
      "be "
      "called (the one in the base class instead of the derived class). To avoid this, you must add a virtual "
      "destructor to the base class.");
};

template <typename Arg>
struct IncorrectArgTypePassedToInstallComponentFuntionsError {
    static_assert(
        AlwaysFalse<Arg>::value,
        "All arguments passed to installComponentFunctions() must be fruit::ComponentFunction<...> objects but an "
        "argument with type Arg was passed instead.");
};

struct LambdaWithCapturesErrorTag {
  template <typename Lambda>
  using apply = LambdaWithCapturesError<Lambda>;
};

struct NonTriviallyCopyableLambdaErrorTag {
  template <typename Lambda>
  using apply = NonTriviallyCopyableLambdaError<Lambda>;
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
  template <typename... TypesInLoop>
  using apply = SelfLoopError<TypesInLoop...>;
};

struct NonClassTypeErrorTag {
  template <typename T, typename C>
  using apply = NonClassTypeError<T, C>;
};

struct AnnotatedTypeErrorTag {
  template <typename T, typename C>
  using apply = AnnotatedTypeError<T, C>;
};

struct TypeAlreadyBoundErrorTag {
  template <typename C>
  using apply = TypeAlreadyBoundError<C>;
};

struct RequiredFactoryWithDifferentSignatureErrorTag {
  template <typename RequiredSignature, typename SignatureInInjectTypedef>
  using apply = RequiredFactoryWithDifferentSignatureError<RequiredSignature, SignatureInInjectTypedef>;
};

struct AnnotatedSignatureDifferentFromLambdaSignatureErrorTag {
  template <typename Signature, typename SignatureInLambda>
  using apply = AnnotatedSignatureDifferentFromLambdaSignatureError<Signature, SignatureInLambda>;
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

struct InjectTypedefWithAnnotationErrorTag {
  template <typename C>
  using apply = InjectTypedefWithAnnotationError<C>;
};

struct UnsatisfiedRequirementsInNormalizedComponentErrorTag {
  template <typename... UnsatisfiedRequirements>
  using apply = UnsatisfiedRequirementsInNormalizedComponentError<UnsatisfiedRequirements...>;
};

struct TypesInInjectorNotProvidedErrorTag {
  template <typename... TypesNotProvided>
  using apply = TypesInInjectorNotProvidedError<TypesNotProvided...>;
};

struct TypesInInjectorProvidedAsConstOnlyErrorTag {
  template <typename... TypesProvidedAsConstOnly>
  using apply = TypesInInjectorProvidedAsConstOnlyError<TypesProvidedAsConstOnly...>;
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

struct NotALambdaErrorTag {
  template <typename CandidateLambda>
  using apply = NotALambdaError<CandidateLambda>;
};

struct TypeNotProvidedErrorTag {
  template <typename T>
  using apply = TypeNotProvidedError<T>;
};

struct TypeProvidedAsConstOnlyErrorTag {
  template <typename T>
  using apply = TypeProvidedAsConstOnlyError<T>;
};

struct NonConstBindingRequiredButConstBindingProvidedErrorTag {
  template <typename T>
  using apply = NonConstBindingRequiredButConstBindingProvidedError<T>;
};

struct NoConstructorMatchingInjectSignatureErrorTag {
  template <typename C, typename InjectSignature>
  using apply = NoConstructorMatchingInjectSignatureError<C, InjectSignature>;
};

struct FunctorSignatureDoesNotMatchErrorTag {
  template <typename ExpectedSignature, typename FunctorSignature>
  using apply = FunctorSignatureDoesNotMatchError<ExpectedSignature, FunctorSignature>;
};

struct CannotConstructAbstractClassErrorTag {
  template <typename C>
  using apply = CannotConstructAbstractClassError<C>;
};

struct NoBindingFoundForAbstractClassErrorTag {
  template <typename T, typename C>
  using apply = NoBindingFoundForAbstractClassError<T, C>;
};

struct InterfaceBindingToSelfErrorTag {
  template <typename C>
  using apply = InterfaceBindingToSelfError<C>;
};

struct TypeMismatchInBindInstanceErrorTag {
  template <typename TypeParameter, typename TypeOfValue>
  using apply = TypeMismatchInBindInstanceError<TypeParameter, TypeOfValue>;
};

struct RequiredTypesInComponentArgumentsErrorTag {
  template <typename RequiredType>
  using apply = RequiredTypesInComponentArgumentsError<RequiredType>;
};

struct NonInjectableTypeErrorTag {
  template <typename T>
  using apply = NonInjectableTypeError<T>;
};

struct ConstBindingDeclaredAsRequiredButNonConstBindingRequiredErrorTag {
  template <typename T>
  using apply = ConstBindingDeclaredAsRequiredButNonConstBindingRequiredError<T>;
};

struct ProviderReturningPointerToAbstractClassWithNoVirtualDestructorErrorTag {
  template <typename T>
  using apply = ProviderReturningPointerToAbstractClassWithNoVirtualDestructorError<T>;
};

struct MultibindingProviderReturningPointerToAbstractClassWithNoVirtualDestructorErrorTag {
  template <typename T>
  using apply = MultibindingProviderReturningPointerToAbstractClassWithNoVirtualDestructorError<T>;
};

struct RegisterFactoryForUniquePtrOfAbstractClassWithNoVirtualDestructorErrorTag {
  template <typename T>
  using apply = RegisterFactoryForUniquePtrOfAbstractClassWithNoVirtualDestructorError<T>;
};

struct FactoryBindingForUniquePtrOfClassWithNoVirtualDestructorErrorTag {
  template <typename BaseFactory, typename DerivedFactory>
  using apply = FactoryBindingForUniquePtrOfClassWithNoVirtualDestructorError<BaseFactory, DerivedFactory>;
};

struct IncorrectArgTypePassedToInstallComponentFuntionsErrorTag {
  template <typename Arg>
  using apply = IncorrectArgTypePassedToInstallComponentFuntionsError<Arg>;
};

} // namespace impl
} // namespace fruit

#endif // FRUIT_INJECTION_ERRORS_H
