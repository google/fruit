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

#ifndef FRUIT_META_METAPROGRAMMING_H
#define FRUIT_META_METAPROGRAMMING_H

#include "basics.h"
#include "vector.h"

#include "../fruit_assert.h"
#include "../fruit_internal_forward_decls.h"
#include "../injection_errors.h"
#include "errors.h"

namespace fruit {
namespace impl {
namespace meta {
  
template <int...>
struct IntVector {};

template <typename T>
struct DebugTypeHelper {
  static_assert(sizeof(T*)*0 != 0, "");
  using type = T;
};

template <typename T>
using DebugType = typename DebugTypeHelper<T>::type;

struct IsConstructibleWithVector {
  template <typename C, typename V>
  struct apply;

  template <typename C, typename... Types>
  struct apply<C, Vector<Types...>> {
    using type = Bool<std::is_constructible<C, Types...>::value>;
  };
};

struct SignatureType {
  template <typename Signature>
  struct apply;

  template <typename T, typename... Types>
  struct apply<T(Types...)> {
    using type = T;
  };
};

struct SignatureArgs {
  template <typename Signature>
  struct apply;

  template <typename T, typename... Types>
  struct apply<T(Types...)> {
    using type = Vector<Types...>;
  };
};

struct ConstructSignature {
  template <typename T, typename V>
  struct apply;

  template <typename T, typename... Types>
  struct apply<T, Vector<Types...>> {
    using type = T(Types...);
  };
};

struct AddPointerToVector {
  template <typename V>
  struct apply;

  template <typename... Ts>
  struct apply<Vector<Ts...>> {
    using type = Vector<Ts*...>;
  };
};

template<int n, int... ns>
struct GenerateIntSequenceHelper : public GenerateIntSequenceHelper<n-1, n-1, ns...> {};

template<int... ns>
struct GenerateIntSequenceHelper<0, ns...> {
  using type = IntVector<ns...>;
};

template <int n>
using GenerateIntSequence = typename GenerateIntSequenceHelper<n>::type;

struct GetNthTypeHelper {
  template <int n, typename... Ts>
  struct apply;

  template <typename T, typename... Ts>
  struct apply<0, T, Ts...> {
    using type = T;
  };

  template <int n, typename T, typename... Ts>
  struct apply<n, T, Ts...> : public apply<n-1, Ts...> {};
};

struct GetNthTypeImpl {
  template <int n, typename V>
  struct apply;

  template <int n, typename... Ts>
  struct apply<n, Vector<Ts...>> : public GetNthTypeHelper::template apply<n, Ts...>{
  };
};

template <int n, typename V>
using GetNthType = typename GetNthTypeImpl::template apply<n, V>::type;

struct FunctorResultHelper {
  template <typename MethodSignature>
  struct apply {};

  template <typename Result, typename Functor, typename... Args>
  struct apply<Result(Functor::*)(Args...)> {
    using type = Result;
  };
};

struct FunctorResult {
  template <typename F>
  struct apply {
    using type = Apply<FunctorResultHelper, decltype(&F::operator())>;
  };
};

struct FunctionSignatureHelper {
  template <typename LambdaMethod>
  struct apply {};

  template <typename Result, typename LambdaObject, typename... Args>
  struct apply<Result(LambdaObject::*)(Args...) const> {
    using type = Result(Args...);
  };
};

// Function is either a plain function type of the form T(*)(Args...) or a lambda.
struct FunctionSignature {
  template <typename Function>
  struct apply {
    using CandidateSignature = Apply<FunctionSignatureHelper, decltype(&Function::operator())>;
    using type = Eval<typename std::conditional<!std::is_constructible<CandidateSignature*, Function>::value,
                                                Error<FunctorUsedAsProviderErrorTag, Function>,
                                                CandidateSignature
                                                >>;
  };

  template <typename Result, typename... Args>
  struct apply<Result(Args...)> {
    using type = Result(Args...);
  };

  template <typename Result, typename... Args>
  struct apply<Result(*)(Args...)> {
    using type = Result(Args...);
  };
};

} // namespace meta
} // namespace impl
} // namespace fruit


#endif // FRUIT_META_METAPROGRAMMING_H
