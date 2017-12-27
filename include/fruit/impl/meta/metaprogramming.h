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

#include <fruit/impl/meta/basics.h>
#include <fruit/impl/meta/vector.h>

#include <fruit/impl/fruit_assert.h>
#include <fruit/impl/fruit_internal_forward_decls.h>
#include <fruit/impl/injection_errors.h>
#include <fruit/impl/meta/errors.h>

#include <memory>

namespace fruit {
namespace impl {
namespace meta {

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

struct IsCallable {
  template <typename T>
  struct apply;

  template <typename C>
  struct apply<Type<C>> {
    template <typename C1>
    static Bool<true> test(decltype(&C1::operator()));

    template <typename>
    static Bool<false> test(...);

    using type = decltype(test<C>(nullptr));
  };
};

struct GetCallOperatorSignature {
  template <typename T>
  struct apply;

  template <typename C>
  struct apply<Type<C>> {
    using type = Type<decltype(&C::operator())>;
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

struct GetNthTypeHelper {
  template <typename N, typename... Ts>
  struct apply;

  template <typename T, typename... Ts>
  struct apply<Int<0>, T, Ts...> {
    using type = T;
  };

  template <int n, typename T, typename... Ts>
  struct apply<Int<n>, T, Ts...> {
    using type = GetNthTypeHelper(Int<n - 1>, Ts...);
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
  struct apply<Type<Result (Functor::*)(Args...)>> {
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
  struct apply<Type<Result (LambdaObject::*)(Args...) const>> {
    using type = Type<Result(Args...)>;
  };
};

// Function is either a plain function type of the form T(*)(Args...) or a lambda.
struct FunctionSignature {
  template <typename Function>
  struct apply;

  template <typename Function>
  struct apply<Type<Function>> {
    using CandidateSignature = FunctionSignatureHelper(GetCallOperatorSignature(Type<Function>));
    using type = If(Not(IsCallable(Type<Function>)), ConstructError(NotALambdaErrorTag, Type<Function>),
                    If(Not(IsConstructible(AddPointer(CandidateSignature), Type<Function>)),
                       ConstructError(FunctorUsedAsProviderErrorTag, Type<Function>), CandidateSignature));
  };

  template <typename Result, typename... Args>
  struct apply<Type<Result(Args...)>> {
    using type = Type<Result(Args...)>;
  };

  template <typename Result, typename... Args>
  struct apply<Type<Result (*)(Args...)>> {
    using type = Type<Result(Args...)>;
  };
};

} // namespace meta
} // namespace impl
} // namespace fruit

#endif // FRUIT_META_METAPROGRAMMING_H
