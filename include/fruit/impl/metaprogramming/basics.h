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

#ifndef FRUIT_METAPROGRAMMING_BASICS_H
#define FRUIT_METAPROGRAMMING_BASICS_H

namespace fruit {
namespace impl {

template <typename T>
struct Lazy {
  using type = T;
};

template <typename F>
using Eval = typename F::type;

// Use as LazyApply<MyMetafunction, Arg1, Arg2>.
// Eval<LazyApply<F, Args..>> is equivalent to Apply<F, Args...>.
template <typename F, typename... Args>
using LazyApply = typename F::template apply<Args...>;

// Use as Apply<MyMetafunction, Arg1, Arg2>
template <typename F, typename... Args>
using Apply = typename F::template apply<Args...>::type;

// Use as ApplyC<MyMetafunction, Arg1, Arg2>::value
template <typename F, typename... Args>
struct ApplyC : public F::template apply<Args...> {
};

struct ConditionalHelper {
  template <bool b, typename F1, typename F2>
  struct apply {
    using type = Eval<F1>;
  };
  
  template <typename F1, typename F2>
  struct apply<false, F1, F2> {
    using type = Eval<F2>;
  };
};

// This is a lazy alternative to std::conditional. F1, F2 are nullary metafunctions.
// The result is already evaluated, no need to use Eval<>.
template <bool b, typename F1, typename F2>
using Conditional = typename ConditionalHelper::template apply<b, F1, F2>::type;

} // namespace impl
} // namespace fruit


#endif // FRUIT_METAPROGRAMMING_BASICS_H
