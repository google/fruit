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

#ifndef FRUIT_FRUIT_FORWARD_DECLS_H
#define FRUIT_FRUIT_FORWARD_DECLS_H

namespace fruit {

// Used to group the requirements of Component.
template <typename... Types>
struct Required {};

// Used to annotate T as a type that uses assisted injection.
template <typename T>
struct Assisted;

template <typename... Types>
class Component;

template <typename Comp>
class PartialComponent;

template <typename... Types>
class NormalizedComponent;

template <typename... P>
class Provider;

template <typename... P>
class Injector;

namespace impl {
 
template <typename Comp, typename I, typename C>
struct Bind;

template <typename Comp, typename Signature>
struct RegisterConstructor;

template <typename Comp, typename C>
struct RegisterInstance;

template <typename Comp, typename I, typename C>
struct AddMultibinding;

template <typename Comp, typename Function>
struct RegisterProvider;

template <typename Comp, typename C>
struct AddInstanceMultibinding;

template <typename Comp, typename Function>
struct RegisterMultibindingProvider;

template <typename Comp, typename AnnotatedSignature, typename Function>
struct RegisterFactory;

template <typename Comp, typename OtherComp>
struct InstallComponent;

class ComponentStorage;
class NormalizedComponentStorage;
class InjectorStorage;

template <typename T>
struct NoBindingFoundError;

template <typename... Ts>
struct CheckNoRepeatedTypes;

template <bool b, typename T>
struct CheckHasNoSelfLoop;

template <typename T, typename C>
struct CheckClassType;

template <bool b, typename C>
struct CheckTypeAlreadyBound;

template <typename RequiredSignature, typename SignatureInInjectTypedef>
struct CheckSameSignatureInInjectionTypedef;

template <typename DuplicatedTypes>
struct DuplicatedTypesInComponentError;

template <typename... Requirements>
struct CheckNoRequirementsInProvider;

template <typename Rs>
struct CheckNoRequirementsInProviderHelper;

template <typename C, typename CandidateSignature>
struct InjectTypedefNotASignature;

template <typename C, typename SignatureReturnType>
struct InjectTypedefForWrongClass;

template <typename CandidateSignature>
struct ParameterIsNotASignature;

template <typename Signature>
struct ConstructorDoesNotExist; // Not used.

template <typename I, typename C>
struct NotABaseClassOf;

template <typename Signature, typename ProviderType>
struct FunctorUsedAsProvider;

template <typename... ComponentRequirements>
struct ComponentWithRequirementsInInjectorError;

template <typename ComponentRequirements>
struct ComponentWithRequirementsInInjectorErrorHelper;

template <typename... UnsatisfiedRequirements>
struct UnsatisfiedRequirementsInNormalizedComponent;

template <typename UnsatisfiedRequirements>
struct UnsatisfiedRequirementsInNormalizedComponentHelper;

template <typename... TypesNotProvided>
struct TypesInInjectorNotProvided;

template <typename TypesNotProvided>
struct TypesInInjectorNotProvidedHelper;

template <typename T, bool is_provided>
struct TypeNotProvidedError;

template <typename C, typename InjectSignature>
struct NoConstructorMatchingInjectSignature;

template <typename ExpectedSignature, typename FunctorSignature>
struct FunctorSignatureDoesNotMatch;

template <bool returns_pointer, typename Signature>
struct FactoryReturningPointer;

} // namespace impl

} // namespace fruit

#endif // FRUIT_FRUIT_FORWARD_DECLS_H
