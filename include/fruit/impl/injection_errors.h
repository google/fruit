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

namespace fruit {
namespace impl {

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

template <typename C, typename RequiredParameters, typename ParametersInInjectAnnotation>
struct CheckSameParametersInInjectionAnnotation {
  static_assert(std::is_same<RequiredParameters, ParametersInInjectAnnotation>::value,
                "The required C factory doesn't have the same parameters as the Inject annotation in C.");
};

template <typename I, typename C>
struct CheckBaseClass {
  static_assert(std::is_base_of<I, C>::value,
                "I should be a base class (or equal to) C.");
};

template <typename DuplicatedTypes>
struct DuplicatedTypesInComponentError {
  static_assert(is_empty_list<DuplicatedTypes>::value,
                "The installed component provides some types that are already provided by the current component.");
};

template <typename DuplicatedTypes>
struct InstalledTypesAlreadyPresentError {
  static_assert(is_empty_list<DuplicatedTypes>::value, 
                "The installed component provides some types that are already provided by this component.");
};

template <typename Rs>
struct CheckNoRequirementsInProvider {
  static_assert(is_empty_list<Rs>::value, 
                "A provider (including injectors) can't have requirements. To try auto-resolving the requirements in the current scope, cast the component to a component with no requirements before constructing the injector with it.");
};

} // namespace impl
} // namespace fruit


#endif // FRUIT_INJECTION_ERRORS
