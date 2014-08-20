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

template <typename... Types>
struct Required;

template <typename T>
struct Assisted;

template <typename... Types>
class Component;

template <typename... Types>
class PartialComponent;

template <typename... P>
class Provider;

template <typename... P>
class Injector;

namespace impl {

template <typename Comp>
struct Identity;

template <typename Comp, typename I, typename C>
struct Bind;

template <typename Comp, typename I, typename C>
struct BindNonFactory;

template <typename Comp, typename I, typename C>
struct AddMultibinding;

template <typename Comp, typename Signature>
struct RegisterProvider;

template <typename Comp, typename Signature>
struct RegisterMultibindingProvider;

template <typename Comp, typename AnnotatedSignature>
struct RegisterFactory;

template <typename Comp, typename C>
struct RegisterInstance;

template <typename Comp, typename C>
struct AddInstanceMultibinding;

template <typename Comp, typename Signature>
struct RegisterConstructor;

template <typename Comp, typename AnnotatedSignature>
struct RegisterConstructorAsValueFactory;

template <typename Comp, typename AnnotatedSignature>
struct RegisterConstructorAsPointerFactory;

template <typename Comp, typename OtherComp>
struct InstallComponent;

template <typename Comp, typename ToRegister>
struct ComponentConversionHelper;

template <typename Comp, typename TargetRequirements, bool is_already_provided, typename C>
struct EnsureProvidedTypeHelper;

} // namespace impl

}

#endif // FRUIT_FRUIT_FORWARD_DECLS_H
