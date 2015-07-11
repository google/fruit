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

#include "basics.h"
#include "logical_operations.h"
#include "numeric_operations.h"
#include <functional>

/*

Types and operations provided by this header:

Vector<Ts...>                : constructs a Vector with the specified elements.
None                         : a Vector element that should be ignored (all transformations should map it to itself).
IsInVector<V, T>             : true if T appears at least once in V.
IsEmptyVector<V>               : true if L is empty (i.e. if all the elements are None)
VectorSize<V>                : the number of (non-None) elements of the vector (as an int).
VectorApparentSize<V>        : the number of elements of the vector *including* any None elements.
PushFront<V, T>              : adds T in front of V.
ConcatVectors<V1, V2>        : returns the concatenation of the given vectors.
ConcatMultipleVectors<Vs...> : extension of ConcatVectors that works with an arbitrary number of vectors. Use sparingly, it's
                               quite slow.
TransformVector<F, V>        : returns the list obtained by applying F to each element of V. F<None>::type must be None.
RemoveFromVector<V, T>       : returns a vector equivalent to V but with all the occurrences of T replaced with None.

*/

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

// None elements in a vector are just placeholders (put in place of real elements when removing an element) and should be ignored.
struct None {};

struct IsInVector {
  template <typename T, typename V>
  struct apply;

  template <typename T, typename... Ts>
  struct apply<T, Vector<Ts...>> {
    using type = Bool<staticOr(std::is_same<T, Ts>::value...)>;
  };
};

struct IsEmptyVector {
  template <typename V>
  struct apply;

  template <typename... Ts>
  struct apply<Vector<Ts...>> {
    using type = Bool<staticAnd(std::is_same<Ts, None>::value...)>;
  };
};

struct VectorSize {
  template <typename V>
  struct apply;

  template <typename... Ts>
  struct apply<Vector<Ts...>> {
    using type = Int<staticSum(!std::is_same<Ts, None>::value...)>;
  };
};

struct VectorApparentSize {
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

struct ConcatMultipleVectors {
  // Empty vector.
  template <typename... V>
  struct apply {
    using type = Vector<>;
  };

  template <typename V>
  struct apply<V> {
    using type = V;
  };

  template <typename... Ts, typename... Us, typename... Vs>
  struct apply<Vector<Ts...>, Vector<Us...>, Vs...> {
    using type = ConcatMultipleVectors(Vector<Ts..., Us...>, Vs...);
  };
};

// CallWithVector(F, Vector<Args...>) is equivalent to Call(F, Args...)
struct CallWithVector {
  template <typename F, typename V>
  struct apply;
  
  template <typename F, typename... Args>
  struct apply<F, Vector<Args...>> {
    using type = F(Args...);
  };
};

// TODO: This is slow when T is not in the vector, consider doing a IsInVector check first.
struct RemoveFromVector {
  template <typename T, typename V>
  struct apply;

  template <typename T, typename... Ts>
  struct apply<T, Vector<Ts...>> {
    using type = ConsVector(typename std::conditional<std::is_same<T, Ts>::value, None, Ts>::type
                            ...);
  };
};

} // namespace meta
} // namespace impl
} // namespace fruit


#endif // FRUIT_META_VECTOR_H
