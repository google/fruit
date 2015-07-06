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

#include <memory>

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

struct IsConstructible {
  template <typename C, typename... Args>
  struct apply;

  template <typename C, typename... Args>
  struct apply<Type<C>, Type<Args>...> {
    using type = Bool<std::is_constructible<C, Args...>::value>;
  };
};

struct IsConstructibleWithVector {
  template <typename C, typename V>
  struct apply;

  template <typename C, typename... Types>
  struct apply<Type<C>, Vector<Type<Types>...>> {
    using type = Bool<std::is_constructible<C, Types...>::value>;
  };
};

struct AddPointer {
  template <typename T>
  struct apply;
  
  template <typename T>
  struct apply<Type<T>> {
    using type = Type<T*>;
  };
};

struct AddPointerToVector {
  template <typename V>
  struct apply;

  template <typename... Ts>
  struct apply<Vector<Type<Ts>...>> {
    using type = Vector<Type<Ts*>...>;
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
  template <typename N, typename... Ts>
  struct apply;

  template <typename T, typename... Ts>
  struct apply<Int<0>, T, Ts...> {
    using type = T;
  };

  template <int n, typename T, typename... Ts>
  struct apply<Int<n>, T, Ts...> {
    using type = GetNthTypeHelper(Int<n-1>, Ts...);
  };
};

struct GetNthType {
  template <typename N, typename V>
  struct apply;
  
  template <typename N, typename... Ts>
  struct apply<N, Vector<Ts...>> {
    using type = GetNthTypeHelper(N, Ts...);
  };
};

struct FunctorResultHelper {
  template <typename MethodSignature>
  struct apply;

  template <typename Result, typename Functor, typename... Args>
  struct apply<Type<Result(Functor::*)(Args...)>> {
    using type = Type<Result>;
  };
};

struct FunctorResult {
  template <typename F>
  struct apply;
  
  template <typename F>
  struct apply<Type<F>> {
    using type = FunctorResultHelper(Type<decltype(&F::operator())>);
  };
};

struct FunctionSignatureHelper {
  template <typename LambdaMethod>
  struct apply;

  template <typename Result, typename LambdaObject, typename... Args>
  struct apply<Type<Result(LambdaObject::*)(Args...) const>> {
    using type = Type<Result(Args...)>;
  };
};

// Function is either a plain function type of the form T(*)(Args...) or a lambda.
struct FunctionSignature {
  template <typename Function>
  struct apply;
  
  template <typename Function>
  struct apply<Type<Function>> {
    using CandidateSignature = FunctionSignatureHelper(Type<decltype(&Function::operator())>);
    using type = If(Not(IsConstructible(AddPointer(CandidateSignature), Type<Function>)),
                    ConstructError(FunctorUsedAsProviderErrorTag, Type<Function>),
                    CandidateSignature);
  };

  template <typename Result, typename... Args>
  struct apply<Type<Result(Args...)>> {
    using type = Type<Result(Args...)>;
  };

  template <typename Result, typename... Args>
  struct apply<Type<Result(*)(Args...)>> {
    using type = Type<Result(Args...)>;
  };
};

} // namespace meta
} // namespace impl
} // namespace fruit


#endif // FRUIT_META_METAPROGRAMMING_H
