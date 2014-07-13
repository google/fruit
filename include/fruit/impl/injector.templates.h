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

#ifndef FRUIT_INJECTOR_TEMPLATES_H
#define FRUIT_INJECTOR_TEMPLATES_H

#include "../component.h"

// Redundant, but makes KDevelop happy.
#include "../injector.h"

namespace fruit {

template <typename... P>
Injector<P...>::Injector(const Component<P...>& component)
  : Super(new fruit::impl::ComponentStorage(component.storage)) {
};

template <typename... P>
Injector<P...>::Injector(Component<P...>&& component)
  : Super(new fruit::impl::ComponentStorage(std::move(component.storage))) {
};

template <typename... P>
Injector<P...>::~Injector() {
  delete this->storage;
}

template <typename... P>
template <typename C>
std::set<C*> Injector<P...>::getMultibindings() {
  return this->storage->template getMultibindings<C>();
}

template <typename... P>
void Injector<P...>::eagerlyInjectAll() {
  // Eagerly inject normal bindings.
  void* unused[] = {reinterpret_cast<void*>(this->storage->template get<P*>())...};
  (void)unused;
  
  this->storage->eagerlyInjectMultibindings();
}

} // namespace fruit


#endif // FRUIT_INJECTOR_TEMPLATES_H
