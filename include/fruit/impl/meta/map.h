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

#ifndef FRUIT_META_MAP_H
#define FRUIT_META_MAP_H

#include <fruit/impl/meta/set.h>

namespace fruit {
namespace impl {
namespace meta {

// A Map is a Set whose elements have the form Pair<Key, Value>

struct GetMapKeys {
  template <typename M>
  struct apply;

  template <typename... Pairs>
  struct apply<Vector<Pairs...>> {
    using type = Vector<typename Pairs::First...>;
  };
};

// TODO: Consider implementing this by finding the position.
struct MapContainsKey {
  template <typename TToFind>
  struct Helper {
    template <typename CurrentResult, typename T>
    struct apply {
      using type = CurrentResult;
    };
    template <typename CurrentResult, typename Value>
    struct apply<CurrentResult, Pair<TToFind, Value>> {
      using type = Bool<true>;
    };
  };

  template <typename M, typename TToFind>
  struct apply {
    using type = FoldVector(M, Helper<TToFind>, Bool<false>);
  };
};

// TODO: Consider implementing this by finding the position first, then calling VectorRemoveFirstN
// and getting the first element.
struct FindInMap {
  template <typename TToFind>
  struct Helper {
    template <typename CurrentResult, typename T>
    struct apply {
      using type = CurrentResult;
    };
    template <typename CurrentResult, typename Value>
    struct apply<CurrentResult, Pair<TToFind, Value>> {
      using type = Value;
    };
  };

  template <typename M, typename TToFind>
  struct apply {
    using type = FoldVector(M, Helper<TToFind>, None);
  };
};

// TODO: Consider implementing this by finding the position first, then calling VectorRemoveFirstN
// and getting the first element.
struct FindValueInMap {
  template <typename TToFind>
  struct Helper {
    template <typename CurrentResult, typename T>
    struct apply {
      using type = CurrentResult;
    };
    template <typename CurrentResult, typename Value>
    struct apply<CurrentResult, Pair<Value, TToFind>> {
      using type = Value;
    };
  };

  template <typename M, typename TToFind>
  struct apply {
    using type = FoldVector(M, Helper<TToFind>, None);
  };
};

} // namespace meta
} // namespace impl
} // namespace fruit

#endif // FRUIT_META_MAP_H
