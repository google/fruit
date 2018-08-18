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

#ifndef FRUIT_BINDINGS_H
#define FRUIT_BINDINGS_H

#include <fruit/impl/meta/metaprogramming.h>

namespace fruit {
namespace impl {

// The types here represent individual entries added in a PartialComponent.

/**
 * Binds the base class I to the implementation C.
 * I must be a base class of C. I=C is not allowed.
 * I and/or C may be annotated using fruit::Annotated<>.
 */
template <typename I, typename C>
struct Bind {};

/**
 * Registers Signature as the constructor signature to use to inject a type.
 * Signature must be a valid signature and its return type must be constructible with those argument
 * types.
 * The arguments and the return type can be annotated using fruit::Annotated<>.
 */
template <typename Signature>
struct RegisterConstructor {};

/**
 * Binds an instance (i.e., object) to the type C.
 * AnnotatedC may be annotated using fruit::Annotated<>.
 * NOTE: for this binding, the runtime binding is added in advance.
 */
template <typename AnnotatedC, typename C>
struct BindInstance {};

/**
 * A variant of BindInstance that binds a constant reference.
 */
template <typename AnnotatedC, typename C>
struct BindConstInstance {};

template <typename... Params>
struct RegisterProvider;

/**
 * Registers `provider' as a provider of C, where provider is a lambda with no captures returning
 * either C or C*.
 */
template <typename Lambda>
struct RegisterProvider<Lambda> {};

/**
 * Registers `provider' as a provider of C, where provider is a lambda with no captures returning
 * either C or C*. Lambda must have the signature AnnotatedSignature (ignoring annotations).
 */
template <typename AnnotatedSignature, typename Lambda>
struct RegisterProvider<Lambda, AnnotatedSignature> {};

/**
 * Adds a multibinding for an instance (as a C&).
 */
template <typename C>
struct AddInstanceMultibinding {};

/**
 * Adds multibindings for a vector of instances (as a std::vector<C>&).
 */
template <typename C>
struct AddInstanceVectorMultibindings {};

/**
 * Similar to Bind<I, C>, but adds a multibinding instead.
 */
template <typename I, typename C>
struct AddMultibinding {};

template <typename... Params>
struct AddMultibindingProvider;

/**
 * Similar to RegisterProvider, but adds a multibinding instead.
 */
template <typename Lambda>
struct AddMultibindingProvider<Lambda> {};

/**
 * Similar to RegisterProvider, but adds a multibinding instead.
 * Lambda must have the signature AnnotatedSignature (ignoring annotations).
 */
template <typename AnnotatedSignature, typename Lambda>
struct AddMultibindingProvider<AnnotatedSignature, Lambda> {};

/**
 * Registers `Lambda' as a factory of C, where `Lambda' is a lambda with no captures returning C.
 * Lambda must have signature DecoratedSignature (ignoring any fruit::Annotated<> and
 * fruit::Assisted<>).
 * Lambda must return a C by value, or a std::unique_ptr<C>.
 */
template <typename DecoratedSignature, typename Lambda>
struct RegisterFactory {};

/**
 * Adds the bindings (and multibindings) in `component' to the current component.
 * OtherComponent must be of the form Component<...>.
 * NOTE: for this binding, the runtime binding is added in advance.
 */
template <typename GetComponentFunction>
struct InstallComponent {};

/**
 * Installs all the specified ComponentFunction objects.
 */
template <typename... ComponentFunctions>
struct InstallComponentFunctions {};

/**
 * An in-progress ReplaceComponent operation, where we don't have all the required information yet.
 */
template <typename GetReplacedComponent>
struct PartialReplaceComponent {};

/**
 * Replaces install()s for a component with install()s for another one.
 * The two Get*Component function signatures must return the same Component<...> type.
 */
template <typename GetReplacedComponent, typename GetReplacementComponent>
struct ReplaceComponent {};

} // namespace impl
} // namespace fruit

#endif // FRUIT_BINDINGS_H
