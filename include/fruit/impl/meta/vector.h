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

#ifndef FRUIT_META_VECTOR_H
#define FRUIT_META_VECTOR_H

#include <fruit/impl/meta/basics.h>
#include <fruit/impl/meta/eval.h>
#include <fruit/impl/meta/fold.h>
#include <fruit/impl/meta/logical_operations.h>
#include <fruit/impl/meta/numeric_operations.h>
#include <functional>

namespace fruit {
namespace impl {
namespace meta {

// Used to pass around a Vector<Types...>, no meaning per se.
template <typename... Types>
struct Vector {};

// Using ConsVector(MetaExpr...) instead of Vector<MetaExpr...> in a meta-expression allows the
// types to be evaluated. Avoid using Vector<...> directly in a meta-expression, unless you're sure
// that the arguments have already been evaluated (e.g. if Args... are arguments of a metafunction,
// Vector<Args...> is ok but Vector<MyFunction(Args)...> is wrong.
struct ConsVector {
  template <typename... Types>
  struct apply {
    using type = Vector<Types...>;
  };
};

struct GenerateIntSequenceEvenHelper {
  template <typename Half>
  struct apply;

  template <int... ns>
  struct apply<Vector<Int<ns>...>> {
    using type = Vector<Int<ns>..., Int<sizeof...(ns) + ns>...>;
  };
};

struct GenerateIntSequenceOddHelper {
  template <typename Half>
  struct apply;

  template <int... ns>
  struct apply<Vector<Int<ns>...>> {
    using type = Vector<Int<ns>..., Int<sizeof...(ns)>, Int<sizeof...(ns) + 1 + ns>...>;
  };
};

struct GenerateIntSequence {
  template <typename N>
  struct apply {
    using type = If(Bool<(N::value % 2) == 0>, GenerateIntSequenceEvenHelper(GenerateIntSequence(Int<N::value / 2>)),
                    GenerateIntSequenceOddHelper(GenerateIntSequence(Int<N::value / 2>)));
  };
};

template <>
struct GenerateIntSequence::apply<Int<0>> {
  using type = Vector<>;
};

template <>
struct GenerateIntSequence::apply<Int<1>> {
  using type = Vector<Int<0>>;
};

struct IsInVector {
  template <typename T>
  struct AlwaysFalseBool {
    constexpr static bool value = false;
  };

  template <bool... bs>
  struct BoolVector;

  template <typename T, typename V>
  struct apply;

  template <typename T, typename... Ts>
  struct apply<T, Vector<Ts...>> {
    using type = Bool<
        !std::is_same<BoolVector<AlwaysFalseBool<Ts>::value...>, BoolVector<std::is_same<T, Ts>::value...>>::value>;
  };
};

struct IsVectorContained {
  template <typename V1, typename V2>
  struct apply;

  template <typename T>
  struct AlwaysTrueBool {
    constexpr static bool value = true;
  };

  template <bool... bs>
  struct BoolVector;

  template <typename... Ts, typename V2>
  struct apply<Vector<Ts...>, V2> {
    using type = Bool<std::is_same<BoolVector<AlwaysTrueBool<Ts>::value...>,
                                   BoolVector<Id<typename IsInVector::template apply<Ts, V2>::type>::value...>>::value>;
  };
};

struct VectorSize {
  template <typename V>
  struct apply;

  template <typename... Ts>
  struct apply<Vector<Ts...>> {
    using type = Int<sizeof...(Ts)>;
  };
};

struct PushFront {
  template <typename V, typename T>
  struct apply;

  template <typename T, typename... Ts>
  struct apply<Vector<Ts...>, T> {
    using type = Vector<T, Ts...>;
  };
};

struct PushBack {
  template <typename V, typename T>
  struct apply;

  template <typename T, typename... Ts>
  struct apply<Vector<Ts...>, T> {
    using type = Vector<Ts..., T>;
  };
};

struct ConcatVectors {
  template <typename V1, typename V2>
  struct apply;

  template <typename... Ts, typename... Us>
  struct apply<Vector<Ts...>, Vector<Us...>> {
    using type = Vector<Ts..., Us...>;
  };
};

struct TransformVector {
  template <typename V, typename F>
  struct apply;

  template <typename... Ts, typename F>
  struct apply<Vector<Ts...>, F> {
    using type = Vector<Eval<typename F::template apply<Ts>::type>...>;
  };
};

struct ReplaceInVectorHelper {
  template <typename ToReplace, typename NewElem, typename T>
  struct apply {
    using type = T;
  };

  template <typename ToReplace, typename NewElem>
  struct apply<ToReplace, NewElem, ToReplace> {
    using type = NewElem;
  };
};

struct ReplaceInVector {
  template <typename V, typename ToReplace, typename NewElem>
  struct apply {
    using type = TransformVector(V, PartialCall(ReplaceInVectorHelper, ToReplace, NewElem));
  };
};

// If V is Vector<T1, ..., Tn> this calculates F(InitialValue, F(T1, F(..., F(Tn) ...))).
// If V is Vector<> this returns InitialValue.
struct FoldVector {
  template <typename V, typename F, typename InitialValue>
  struct apply;

  template <typename... Ts, typename F, typename InitialValue>
  struct apply<Vector<Ts...>, F, InitialValue> {
    using type = Fold(F, InitialValue, Ts...);
  };
};

template <typename Unused>
using AlwaysVoidPtr = void*;

// Returns a copy of V but without the first N elements.
// N must be at least 0 and at most VectorSize(V).
struct VectorRemoveFirstN {
  template <typename V, typename N, typename Indexes = Eval<GenerateIntSequence(N)>>
  struct apply;

  template <typename... Types, typename N, typename... Indexes>
  struct apply<Vector<Types...>, N, Vector<Indexes...>> {
    template <typename... RemainingTypes>
    static Vector<RemainingTypes...> f(AlwaysVoidPtr<Indexes>..., RemainingTypes*...);

    using type = decltype(f((Types*)nullptr...));
  };
};

struct VectorEndsWith {
  template <typename V, typename T>
  struct apply {
    using N = Int<Eval<VectorSize(V)>::value - 1>;
    using type = IsSame(VectorRemoveFirstN(V, N), Vector<T>);
  };

  template <typename T>
  struct apply<Vector<>, T> {
    using type = Bool<false>;
  };
};

// Removes all None elements from the vector.
// O(n) instantiations.
struct VectorRemoveNone {
  template <typename V>
  struct apply {
    using type = Vector<>;
  };

  template <typename T, typename... Ts>
  struct apply<Vector<T, Ts...>> {
    using type = PushFront(VectorRemoveNone(Vector<Ts...>), T);
  };

  template <typename... Ts>
  struct apply<Vector<None, Ts...>> {
    using type = VectorRemoveNone(Vector<Ts...>);
  };
};

struct ConstructErrorWithArgVectorHelper {
  template <typename ErrorTag, typename ArgsVector, typename... OtherArgs>
  struct apply;

  template <typename ErrorTag, typename... Args, typename... OtherArgs>
  struct apply<ErrorTag, Vector<Args...>, OtherArgs...> {
    using type = ConstructError(ErrorTag, OtherArgs..., Args...);
  };
};

struct ConstructErrorWithArgVector {
  template <typename ErrorTag, typename ArgsVector, typename... OtherArgs>
  struct apply {
    using type = ConstructErrorWithArgVectorHelper(ErrorTag, VectorRemoveNone(ArgsVector), OtherArgs...);
  };
};

} // namespace meta
} // namespace impl
} // namespace fruit

#endif // FRUIT_META_VECTOR_H
