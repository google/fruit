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

#ifndef FRUIT_META_SIGNATURES_H
#define FRUIT_META_SIGNATURES_H

#include <fruit/impl/meta/basics.h>
#include <fruit/impl/meta/vector.h>

namespace fruit {
namespace impl {
namespace meta {

// Similar to ConsSignature, but takes a Vector of args instead of individual args.
struct ConsSignatureWithVector {
  template <typename ReturnType, typename ArgVector>
  struct apply;

  template <typename ReturnType, typename... Args>
  struct apply<Type<ReturnType>, Vector<Type<Args>...>> {
    using type = Type<ReturnType(Args...)>;
  };
};

struct SignatureType {
  template <typename Signature>
  struct apply;

  template <typename T, typename... Types>
  struct apply<Type<T(Types...)>> {
    using type = Type<T>;
  };
};

struct SignatureArgs {
  template <typename Signature>
  struct apply;

  template <typename T, typename... Types>
  struct apply<Type<T(Types...)>> {
    using type = Vector<Type<Types>...>;
  };
};

struct IsSignature {
  template <typename Signature>
  struct apply {
    using type = Bool<false>;
  };

  template <typename C, typename... Args>
  struct apply<Type<C(Args...)>> {
    using type = Bool<true>;
  };
};

} // namespace meta
} // namespace impl
} // namespace fruit

#endif // FRUIT_META_SIGNATURES_H
