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

#ifndef FRUIT_META_OPERATION_WRAPPERS_H
#define FRUIT_META_OPERATION_WRAPPERS_H

#include <fruit/impl/meta/basics.h>

namespace fruit {
namespace impl {

template <typename T>
struct RemoveAnnotationsHelper {
  using type = fruit::impl::meta::UnwrapType<
      fruit::impl::meta::Eval<fruit::impl::meta::RemoveAnnotations(fruit::impl::meta::Type<T>)>>;
};
template <typename T>
using RemoveAnnotations = typename RemoveAnnotationsHelper<T>::type;

} // namespace impl
} // namespace fruit

#endif // FRUIT_META_OPERATION_WRAPPERS_H
