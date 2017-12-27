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

#ifndef FRUIT_META_WRAPPERS_H
#define FRUIT_META_WRAPPERS_H

#include <fruit/impl/fruit-config.h>

#include <memory>

namespace fruit {
namespace impl {
namespace meta {

struct ConsSignature {
  template <typename ReturnType, typename... Args>
  struct apply;

  template <typename ReturnType, typename... Args>
  struct apply<Type<ReturnType>, Type<Args>...> {
    using type = Type<ReturnType(Args...)>;
  };
};

struct ConsStdFunction {
  template <typename Signature>
  struct apply;

  template <typename Signature>
  struct apply<Type<Signature>> {
    using type = Type<std::function<Signature>>;
  };
};

struct ConsUniquePtr {
  template <typename T>
  struct apply;

  template <typename T>
  struct apply<Type<T>> {
    using type = Type<std::unique_ptr<T>>;
  };
};

struct RemoveUniquePtr {
  template <typename T>
  struct apply {
    using type = T;
  };

  template <typename T>
  struct apply<Type<std::unique_ptr<T>>> {
    using type = Type<T>;
  };
};

struct RemovePointer {
  template <typename T>
  struct apply {
    using type = T;
  };

  template <typename T>
  struct apply<Type<T*>> {
    using type = Type<T>;
  };
};

struct ConsReference {
  template <typename T>
  struct apply;

  template <typename T>
  struct apply<Type<T>> {
    using type = Type<T&>;
  };
};

struct ConsConstReference {
  template <typename T>
  struct apply;

  template <typename T>
  struct apply<Type<T>> {
    using type = Type<const T&>;
  };
};

struct IsEmpty {
  template <typename T>
  struct apply;

  template <typename T>
  struct apply<Type<T>> {
    using type = Bool<std::is_empty<T>::value>;
  };
};

struct IsTriviallyCopyable {
  template <typename T>
  struct apply;

  template <typename T>
  struct apply<Type<T>> {
    using type = Bool<FRUIT_IS_TRIVIALLY_COPYABLE(T)>;
  };
};

struct IsPointer {
  template <typename T>
  struct apply;

  template <typename T>
  struct apply<Type<T>> {
    using type = Bool<std::is_pointer<T>::value>;
  };
};

struct IsUniquePtr {
  template <typename T>
  struct apply {
    using type = Bool<false>;
  };

  template <typename T>
  struct apply<Type<std::unique_ptr<T>>> {
    using type = Bool<true>;
  };
};

struct IsAbstract {
  template <typename T>
  struct apply;

  template <typename T>
  struct apply<Type<T>> {
    using type = Bool<std::is_abstract<T>::value>;
  };
};

struct IsBaseOf {
  template <typename I, typename C>
  struct apply;

  template <typename I, typename C>
  struct apply<Type<I>, Type<C>> {
    using type = Bool<std::is_base_of<I, C>::value>;
  };
};

struct HasVirtualDestructor {
  template <typename T>
  struct apply;

  template <typename T>
  struct apply<Type<T>> {
    using type = Bool<std::has_virtual_destructor<T>::value>;
  };
};

} // namespace meta
} // namespace impl
} // namespace fruit

#endif // FRUIT_META_WRAPPERS_H
