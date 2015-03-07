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

#ifndef FRUIT_META_BASICS_H
#define FRUIT_META_BASICS_H

#include <functional>

namespace fruit {
namespace impl {
namespace meta {

template <typename T>
struct Lazy {
  using type = T;
};

template <bool b>
struct Bool {
  static constexpr bool value = b;
};

template <int n>
struct Int {
  static constexpr int value = n;
};

template <typename F>
using Eval = typename F::type;

// Use as Apply<MyMetafunction, Arg1, Arg2>.
template <typename F, typename... Args>
using Apply = typename F::template apply<Args...>::type;

template <typename EvaluatedB, typename F1, typename F2>
struct ConditionalHelper;

template <typename F1, typename F2>
struct ConditionalHelper<Bool<true>, F1, F2> {
  using type = Eval<F1>;
};

template <typename F1, typename F2>
struct ConditionalHelper<Bool<false>, F1, F2> {
  using type = Eval<F2>;
};

// Use as Conditional<B, F1, F2>.
// All parameters are lazy values (even B) and the result is a lazy value.
template <typename B, typename T1, typename T2>
struct Conditional {
  using type = Eval<ConditionalHelper<Eval<B>, T1, T2>>;
};

// A functor equivalent to F, but that takes parameters lazily and returns the result lazily.
template <typename F>
struct LazyFunctor {
  template <typename... Args>
  struct apply {
    struct X {
      using type = Apply<F, Eval<Args>...>;
    };
    // We need to do this because an inner type can't have the same name of the enclosing class.
    using type = X;
  };
};

// Apply<ApplyAndPostponeFirstArgument<F, Args...>, Arg> is the same as Apply<F, Arg, Args...>
template <typename F, typename... Args>
struct ApplyAndPostponeFirstArgument {
  template <typename Arg>
  struct apply {
    using type = Apply<F, Arg, Args...>;
  };
};

struct IsSame {
  template <typename T, typename U>
  struct apply {
    using type = Bool<std::is_same<T, U>::value>;
  };
};

struct Not {
  template <typename B>
  struct apply {
    using type = Bool<!B::value>;
  };
};

} // namespace meta
} // namespace impl
} // namespace fruit


#endif // FRUIT_META_BASICS_H
