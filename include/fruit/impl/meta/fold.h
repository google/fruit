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

#ifndef FRUIT_META_FOLD_H
#define FRUIT_META_FOLD_H

#include <functional>

namespace fruit {
namespace impl {
namespace meta {

struct Fold {
  template <typename F, typename InitialValue, typename... Types>
  struct apply;

  template <typename F, typename InitialValue>
  struct apply<F, InitialValue> {
    using type = InitialValue;
  };

  template <typename F, typename InitialValue, typename T0>
  struct apply<F, InitialValue, T0> {
    using type = typename F::template apply<InitialValue, T0>::type;
  };

  template <typename F, typename InitialValue, typename T0, typename T1>
  struct apply<F, InitialValue, T0, T1> {
    using type =
        typename F::template apply<typename DoEval<typename F::template apply<InitialValue, T0>::type>::type, T1>::type;
  };

  template <typename F, typename InitialValue, typename T0, typename T1, typename T2>
  struct apply<F, InitialValue, T0, T1, T2> {
    using type = typename F::template apply<
        typename DoEval<typename F::template apply<
            typename DoEval<typename F::template apply<InitialValue, T0>::type>::type, T1>::type>::type,
        T2>::type;
  };

  template <typename F, typename InitialValue, typename T0, typename T1, typename T2, typename T3>
  struct apply<F, InitialValue, T0, T1, T2, T3> {
    using type = typename F::template apply<
        typename DoEval<typename F::template apply<
            typename DoEval<typename F::template apply<
                typename DoEval<typename F::template apply<InitialValue, T0>::type>::type, T1>::type>::type,
            T2>::type>::type,
        T3>::type;
  };

  template <typename F, typename InitialValue, typename T0, typename T1, typename T2, typename T3, typename T4>
  struct apply<F, InitialValue, T0, T1, T2, T3, T4> {
    using type = typename F::template apply<
        typename DoEval<typename F::template apply<
            typename DoEval<typename F::template apply<
                typename DoEval<typename F::template apply<
                    typename DoEval<typename F::template apply<InitialValue, T0>::type>::type, T1>::type>::type,
                T2>::type>::type,
            T3>::type>::type,
        T4>::type;
  };

  template <typename F, typename InitialValue, typename T0, typename T1, typename T2, typename T3, typename T4,
            typename... Types>
  struct apply<F, InitialValue, T0, T1, T2, T3, T4, Types...> {
    using type = Fold(
        F, typename F::template apply<
               typename DoEval<typename F::template apply<
                   typename DoEval<typename F::template apply<
                       typename DoEval<typename F::template apply<
                           typename DoEval<typename F::template apply<InitialValue, T0>::type>::type, T1>::type>::type,
                       T2>::type>::type,
                   T3>::type>::type,
               T4>::type,
        Types...);
  };

  // Optimized specialization, processing 10 values at a time.
  template <typename F, typename InitialValue, typename T0, typename T1, typename T2, typename T3, typename T4,
            typename T5, typename T6, typename T7, typename T8, typename T9, typename... Types>
  struct apply<F, InitialValue, T0, T1, T2, T3, T4, T5, T6, T7, T8, T9, Types...> {
    using type = Fold(
        F,
        typename F::template apply<
            typename DoEval<typename F::template apply<
                typename DoEval<typename F::template apply<
                    typename DoEval<typename F::template apply<
                        typename DoEval<typename F::template apply<
                            typename DoEval<typename F::template apply<
                                typename DoEval<typename F::template apply<
                                    typename DoEval<typename F::template apply<
                                        typename DoEval<typename F::template apply<
                                            typename DoEval<typename F::template apply<InitialValue, T0>::type>::type,
                                            T1>::type>::type,
                                        T2>::type>::type,
                                    T3>::type>::type,
                                T4>::type>::type,
                            T5>::type>::type,
                        T6>::type>::type,
                    T7>::type>::type,
                T8>::type>::type,
            T9>::type,
        Types...);
  };
};

} // namespace meta
} // namespace impl
} // namespace fruit

#endif // FRUIT_META_FOLD_H
