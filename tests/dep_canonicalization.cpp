// expect-success
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

#include <fruit/fruit.h>

using namespace fruit;
using namespace fruit::impl;

template <typename Dep1, typename Dep2>
struct is_same_dep {
  static constexpr bool value = std::is_same<typename Dep1::Type, typename Dep2::Type>::value
                             && is_same_set<typename Dep1::Requirements, typename Dep2::Requirements>::value;
};

template <typename T, typename L>
struct is_dep_in_list {}; // Not used

template <typename T, typename... Ts>
struct is_dep_in_list<T, List<Ts...>> {
  static constexpr bool value = static_or<is_same_dep<T, Ts>::value...>::value;
};

template <typename S1, typename S2>
struct dep_set_difference_impl {}; // Not used.

template <typename... Ts, typename S>
struct dep_set_difference_impl<List<Ts...>, S> {
  using type = List<typename std::conditional<is_dep_in_list<Ts, S>::value, None, Ts>::type...>;
};

template <typename S1, typename S2>
using dep_set_difference = typename dep_set_difference_impl<S1, S2>::type;

template <typename S1, typename S2>
struct is_same_dep_set {
  static constexpr bool value = is_empty_list<dep_set_difference<S1, S2>>::value
                             && is_empty_list<dep_set_difference<S2, S1>>::value;
};

template <typename T1, typename T2>
struct CheckSameDepHelper {
  static_assert(is_same_dep<T1, T2>::value, "Deps differ");
};

template <typename T1, typename T2>
struct CheckSameDepSetHelper {
  static_assert(is_same_dep_set<T1, T2>::value, "Dep sets differ");
};

#define CHECK_SAME_DEP(...) static_assert(true || sizeof(CheckSameDepHelper<__VA_ARGS__>), "")
#define CHECK_SAME_DEP_SET(...) static_assert(true || sizeof(CheckSameDepSetHelper<__VA_ARGS__>), "")

struct A{};
struct B{};
struct C{};
  
int main() {
  using Dep1 = ConstructDep<A, List<B>>;
  using Dep2 = ConstructDep<B, List<C>>;
  
  static_assert(is_same_set<fruit::impl::List<C, fruit::impl::None>, fruit::impl::List<C> >::value, "");
  static_assert(is_same_dep<fruit::impl::ConsDep<A, fruit::impl::List<C, fruit::impl::None> >, fruit::impl::ConsDep<A, fruit::impl::List<C> > >::value, "");
  static_assert(is_same_dep_set<fruit::impl::List<fruit::impl::ConsDep<A, fruit::impl::List<C, fruit::impl::None> > >, fruit::impl::List<fruit::impl::ConsDep<A, fruit::impl::List<C> > > >::value, "");

  
  CHECK_SAME_DEP(CanonicalizeDepWithDep<Dep1, Dep2>,
                 ConstructDep<A, List<C>>);
  CHECK_SAME_DEP(CanonicalizeDepWithDep<Dep2, Dep1>,
                 Dep2);
  
  CHECK_SAME_DEP_SET(typename CanonicalizeDepsWithDep<List<Dep1>, Dep2>::type,
                     List<ConstructDep<A, List<C>>>);
  CHECK_SAME_DEP_SET(typename CanonicalizeDepsWithDep<List<Dep2>, Dep1>::type,
                     List<Dep2>);
  
  CHECK_SAME_DEP(CanonicalizeDepWithDeps<Dep1, List<Dep2>,
                 List<typename Dep2::Type>>, ConstructDep<A, List<C>>);
  CHECK_SAME_DEP(CanonicalizeDepWithDeps<Dep2, List<Dep1>,
                 List<typename Dep2::Type>>, Dep2);
  
  using Deps1 = AddDep<Dep1, List<Dep2>, List<typename Dep2::Type>>;
  using Deps2 = AddDep<Dep2, List<Dep1>, List<typename Dep2::Type>>;
  
  CHECK_SAME_DEP_SET(Deps1,
                     List<ConsDep<A, List<C>>, ConsDep<B, List<C>>>);
  CHECK_SAME_DEP_SET(Deps2,
                     List<ConsDep<B, List<C>>, ConsDep<A, List<C>>>);
  
  return 0;
}
