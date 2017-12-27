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

#ifndef FRUIT_NORMALIZED_COMPONENT_INLINES_H
#define FRUIT_NORMALIZED_COMPONENT_INLINES_H

#include <fruit/normalized_component.h>

#include <fruit/component.h>
#include <fruit/impl/util/type_info.h>

namespace fruit {

template <typename... Params>
template <typename... FormalArgs, typename... Args>
inline NormalizedComponent<Params...>::NormalizedComponent(Component<Params...> (*getComponent)(FormalArgs...),
                                                           Args&&... args)
    : NormalizedComponent(std::move(fruit::Component<Params...>(
                                        fruit::createComponent().install(getComponent, std::forward<Args>(args)...))
                                        .storage),
                          fruit::impl::MemoryPool()) {}

template <typename... Params>
inline NormalizedComponent<Params...>::NormalizedComponent(fruit::impl::ComponentStorage&& storage,
                                                           fruit::impl::MemoryPool memory_pool)
    : storage(std::move(storage),
              fruit::impl::getTypeIdsForList<typename fruit::impl::meta::Eval<fruit::impl::meta::SetToVector(
                  typename fruit::impl::meta::Eval<fruit::impl::meta::ConstructComponentImpl(
                      fruit::impl::meta::Type<Params>...)>::Ps)>>(memory_pool),
              memory_pool, fruit::impl::NormalizedComponentStorageHolder::WithUndoableCompression()) {}

} // namespace fruit

#endif // FRUIT_NORMALIZED_COMPONENT_INLINES_H
