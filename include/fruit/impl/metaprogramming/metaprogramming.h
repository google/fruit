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

#ifndef FRUIT_METAPROGRAMMING_H
#define FRUIT_METAPROGRAMMING_H

#include "list.h"

namespace fruit {
namespace impl {

template <int...>
struct IntList {};

#ifdef FRUIT_EXTRA_DEBUG

template <typename T>
struct DebugTypeHelper {
  static_assert(sizeof(T*)*0 != 0, "");
  using type = T;
};

template <typename T>
using DebugType = typename DebugTypeHelper<T>::type;

#endif

template <typename C, typename L>
struct is_constructible_with_list {}; // Not used.

template <typename C, typename... Types>
struct is_constructible_with_list<C, List<Types...>> : public std::is_constructible<C, Types...> {};

template <typename Signature>
struct SignatureTraits {
  FruitStaticAssert(false && sizeof(Signature), "Not a signature");
};

template <typename T, typename... Types>
struct SignatureTraits<T(Types...)> {
  using type = T;
  using Args = List<Types...>;
};

template <typename Signature>
using SignatureType = typename SignatureTraits<Signature>::type;

template <typename Signature>
using SignatureArgs = typename SignatureTraits<Signature>::Args;

template <typename T, typename L>
struct ConstructSignatureImpl {};

template <typename T, typename... Types>
struct ConstructSignatureImpl<T, List<Types...>> {
  using type = T(Types...);
};

template <typename T, typename L>
using ConstructSignature = typename ConstructSignatureImpl<T, L>::type;

template <typename L>
struct AddPointerToListHelper {};

template <typename... Ts>
struct AddPointerToListHelper<List<Ts...>> {
  using type = List<Ts*...>;
};

template <typename L>
using AddPointerToList = typename AddPointerToListHelper<L>::type;

template<int n, int... ns>
struct GenerateIntSequenceHelper : public GenerateIntSequenceHelper<n-1, n-1, ns...> {};

template<int... ns>
struct GenerateIntSequenceHelper<0, ns...> {
  typedef IntList<ns...> type;
};

template <int n>
using GenerateIntSequence = typename GenerateIntSequenceHelper<n>::type;

template <int n, typename L>
struct GetNthTypeHelper {};

template <typename T, typename... Ts>
struct GetNthTypeHelper<0, List<T, Ts...>> {
  using type = T;
};

template <int n, typename T, typename... Ts>
struct GetNthTypeHelper<n, List<T, Ts...>> : public GetNthTypeHelper<n-1, List<Ts...>> {};

template <int n, typename L>
using GetNthType = typename GetNthTypeHelper<n, L>::type;

template <typename MethodSignature>
struct FunctorResultHelper {};

template <typename Result, typename Functor, typename... Args>
struct FunctorResultHelper<Result(Functor::*)(Args...)> {
  using type = Result;
};

template <typename F>
using FunctorResult = typename FunctorResultHelper<decltype(&F::operator())>::type;

template <typename LambdaMethod>
struct FunctionSignatureHelper2 {};

template <typename Result, typename LambdaObject, typename... Args>
struct FunctionSignatureHelper2<Result(LambdaObject::*)(Args...) const> {
  using type = Result(Args...);
};

template <typename Function>
struct FunctionSignatureHelper {
  using type = typename FunctionSignatureHelper2<decltype(&Function::operator())>::type;
  FruitDelegateCheck(fruit::impl::FunctorUsedAsProvider<type, Function>);
};

template <typename Result, typename... Args>
struct FunctionSignatureHelper<Result(Args...)> {
  using type = Result(Args...);
};

template <typename Result, typename... Args>
struct FunctionSignatureHelper<Result(*)(Args...)> {
  using type = Result(Args...);
};

// Function is either a plain function type of the form T(*)(Args...) or a lambda.
template <typename Function>
using FunctionSignature = typename FunctionSignatureHelper<Function>::type;

} // namespace impl
} // namespace fruit


#endif // FRUIT_METAPROGRAMMING_H
