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

struct IsSameDep {
  template <typename Dep1, typename Dep2>
  struct apply {
    static constexpr bool value = std::is_same<typename Dep1::Type, typename Dep2::Type>::value
                               && ApplyC<IsSameSet, typename Dep1::Requirements, typename Dep2::Requirements>::value;
  };
};

struct IsDepInList {
  template <typename T, typename L>
  struct apply;

  template <typename T, typename... Ts>
  struct apply<T, List<Ts...>> {
    static constexpr bool value = StaticOr<ApplyC<IsSameDep, T, Ts>::value...>::value;
  };
};

struct DepSetDifference {
  template <typename S1, typename S2>
  struct apply;

  template <typename... Ts, typename S>
  struct apply<List<Ts...>, S> {
    using type = List<typename std::conditional<ApplyC<IsDepInList, Ts, S>::value, None, Ts>::type...>;
  };
};

struct IsSameDepSet {
  template <typename S1, typename S2>
  struct apply {
    static constexpr bool value = ApplyC<IsEmptyList, Apply<DepSetDifference, S1, S2>>::value
                               && ApplyC<IsEmptyList, Apply<DepSetDifference, S2, S1>>::value;
  };
};

template <typename T1, typename T2>
struct CheckSameDepHelper {
  static_assert(ApplyC<IsSameDep, T1, T2>::value, "Deps differ");
};

template <typename T1, typename T2>
struct CheckSameDepSetHelper {
  static_assert(ApplyC<IsSameDepSet, T1, T2>::value, "Dep sets differ");
};

#define CHECK_SAME_DEP(...) static_assert(true || sizeof(CheckSameDepHelper<__VA_ARGS__>), "")
#define CHECK_SAME_DEP_SET(...) static_assert(true || sizeof(CheckSameDepSetHelper<__VA_ARGS__>), "")

struct A{};
struct B{};
struct C{};
  
int main() {
  using Dep1 = Apply<ConstructDep, A, List<B>>;
  using Dep2 = Apply<ConstructDep, B, List<C>>;
  
  static_assert(ApplyC<IsSameSet,
                       List<C, None>,
                       List<C>
                      >::value,
                "");
  static_assert(ApplyC<IsSameDep,
                       ConsDep<A, List<C, None>>,
                       ConsDep<A, List<C>>
                      >::value,
                "");
  static_assert(ApplyC<IsSameDepSet,
                       List<ConsDep<A, List<C, None>>>,
                       List<ConsDep<A, List<C>>>
                      >::value,
                "");

  
  CHECK_SAME_DEP(Apply<CanonicalizeDepWithDep, Dep1, Dep2>,
                 Apply<ConstructDep, A, List<C>>);
  CHECK_SAME_DEP(Apply<CanonicalizeDepWithDep, Dep2, Dep1>,
                 Dep2);
  
  CHECK_SAME_DEP_SET(Apply<CanonicalizeDepsWithDep, List<Dep1>, Dep2>,
                     List<Apply<ConstructDep, A, List<C>>>);
  CHECK_SAME_DEP_SET(Apply<CanonicalizeDepsWithDep, List<Dep2>, Dep1>,
                     List<Dep2>);
  
  CHECK_SAME_DEP(Apply<CanonicalizeDepWithDeps, Dep1, List<Dep2>,
                 List<typename Dep2::Type>>, Apply<ConstructDep, A, List<C>>);
  CHECK_SAME_DEP(Apply<CanonicalizeDepWithDeps, Dep2, List<Dep1>,
                 List<typename Dep2::Type>>, Dep2);
  
  using Deps1 = Apply<AddDep, Dep1, List<Dep2>, List<typename Dep2::Type>>;
  using Deps2 = Apply<AddDep, Dep2, List<Dep1>, List<typename Dep2::Type>>;
  
  CHECK_SAME_DEP_SET(Deps1,
                     List<ConsDep<A, List<C>>, ConsDep<B, List<C>>>);
  CHECK_SAME_DEP_SET(Deps2,
                     List<ConsDep<B, List<C>>, ConsDep<A, List<C>>>);
  
  return 0;
}
