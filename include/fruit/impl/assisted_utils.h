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

#ifndef FRUIT_ASSISTED_UTILS_H
#define FRUIT_ASSISTED_UTILS_H

#include "basic_utils.h"

namespace fruit {

// Used to annotate T as a type that uses assisted injection.
template <typename T>
struct Assisted;
  
namespace impl {

template <typename L>
struct ExtractRequirementsFromAssistedParamsHelper {};

template <>
struct ExtractRequirementsFromAssistedParamsHelper<List<>> {
  using type = List<>;
};

// Assisted case
template <typename T, typename... Ts>
struct ExtractRequirementsFromAssistedParamsHelper<List<Assisted<T>, Ts...>> {
  using type = typename ExtractRequirementsFromAssistedParamsHelper<List<Ts...>>::type;
};

// Non-assisted case
template <typename T, typename... Ts>
struct ExtractRequirementsFromAssistedParamsHelper<List<T, Ts...>> {
  using type = add_to_list<GetClassForType<T>, typename ExtractRequirementsFromAssistedParamsHelper<List<Ts...>>::type>;
};

// Takes a list of types, considers only the assisted ones, transforms them to classes with
// GetClassForType and returns the resulting list.
template <typename L>
using ExtractRequirementsFromAssistedParams = typename ExtractRequirementsFromAssistedParamsHelper<L>::type;

template <typename L>
struct RemoveNonAssistedHelper {};

template <>
struct RemoveNonAssistedHelper<List<>> {
  using type = List<>;
};

// Non-assisted case
template <typename T, typename... Ts>
struct RemoveNonAssistedHelper<List<T, Ts...>> {
  using type = typename RemoveNonAssistedHelper<List<Ts...>>::type;
};

// Assisted case
template <typename T, typename... Ts>
struct RemoveNonAssistedHelper<List<Assisted<T>, Ts...>> {
  using type = add_to_list<T, typename RemoveNonAssistedHelper<List<Ts...>>::type>;
};

template <typename L>
using RemoveNonAssisted = typename RemoveNonAssistedHelper<L>::type;

template <typename L>
struct RemoveAssistedHelper {};

template <>
struct RemoveAssistedHelper<List<>> {
  using type = List<>;
};

// Non-assisted case
template <typename T, typename... Ts>
struct RemoveAssistedHelper<List<T, Ts...>> {
  using type = add_to_list<T, typename RemoveAssistedHelper<List<Ts...>>::type>;
};

// Assisted case
template <typename T, typename... Ts>
struct RemoveAssistedHelper<List<Assisted<T>, Ts...>> {
  using type = typename RemoveAssistedHelper<List<Ts...>>::type;
};

template <typename L>
using RemoveAssisted = typename RemoveAssistedHelper<L>::type;

template <typename L>
struct UnlabelAssistedHelper {};

template <>
struct UnlabelAssistedHelper<List<>> {
  using type = List<>;
};

// Non-assisted case
template <typename T, typename... Ts>
struct UnlabelAssistedHelper<List<T, Ts...>> {
  using type = add_to_list<T, typename UnlabelAssistedHelper<List<Ts...>>::type>;
};

// Assisted case
template <typename T, typename... Ts>
struct UnlabelAssistedHelper<List<Assisted<T>, Ts...>> {
  using type = add_to_list<T, typename UnlabelAssistedHelper<List<Ts...>>::type>;
};

template <typename L>
using UnlabelAssisted = typename UnlabelAssistedHelper<L>::type;

template <typename AnnotatedSignature>
using RequiredSignatureForAssistedFactory = ConstructSignature<SignatureType<AnnotatedSignature>, UnlabelAssisted<SignatureArgs<AnnotatedSignature>>>;

template <typename AnnotatedSignature>
using InjectedFunctionTypeForAssistedFactory = ConstructSignature<SignatureType<AnnotatedSignature>, RemoveNonAssisted<SignatureArgs<AnnotatedSignature>>>;

template <int index, typename L>
class NumAssistedBefore {}; // Not used. Instantiated only if index is out of bounds.

template <typename T, typename... Ts>
class NumAssistedBefore<0, List<T, Ts...>> : public std::integral_constant<int, 0> {};

// This is needed because the previous is not more specialized than the specialization with assisted T.
template <typename T, typename... Ts>
class NumAssistedBefore<0, List<Assisted<T>, Ts...>> : public std::integral_constant<int, 0> {};

// Non-assisted T, index!=0.
template <int index, typename T, typename... Ts>
class NumAssistedBefore<index, List<T, Ts...>> : public NumAssistedBefore<index-1, List<Ts...>> {};

// Assisted T, index!=0.
template <int index, typename T, typename... Ts>
class NumAssistedBefore<index, List<Assisted<T>, Ts...>> : public std::integral_constant<int, 1 + NumAssistedBefore<index-1, List<Ts...>>::value> {};


} // namespace impl
} // namespace fruit

#endif // FRUIT_ASSISTED_UTILS_H
