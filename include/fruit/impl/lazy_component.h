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

#ifndef FRUIT_LAZY_COMPONENT_H
#define FRUIT_LAZY_COMPONENT_H

#include <fruit/impl/storage/to_port/old_component_storage.h>

namespace fruit {
namespace impl {

/**
 * A component that can be created lazily.
 * Note: this is quite slow; for components that have no parameters prefer using LazyComponentWithNoArgs.
 */
class LazyComponent {
protected:
  // An arbitrary function type, used as type for the field `erased_fun`.
  // Note that we can't use void* here, since data pointers might not have the same size as function pointers.
  using erased_fun_t = void(*)();

  // The function that will be invoked to create the Component.
  // Here we don't know the type, it's only known to the LazyComponent implementation.
  // We store this here instead of in the LazyComponent implementation so that we can do a quick comparison on the
  // pointer without virtual calls (and we can then do the rest of the comparison via virtual call if needed).
  erased_fun_t erased_fun;

public:
  LazyComponent(erased_fun_t erased_fun);

  virtual ~LazyComponent() = default;

  // Checks if *this and other are equal, assuming that this->fun and other.fun are equal.
  virtual bool areParamsEqual(const LazyComponent& other) const = 0;

  bool operator==(const LazyComponent& other) const;

  virtual void addBindings(OldComponentStorage& component_storage) const = 0;
  virtual std::size_t hashCode() const = 0;
  virtual LazyComponent* copy() const = 0;

  /**
   * Returns the type ID of the real `fun` object stored by the implementation.
   * We use this instead of the `typeid` operator so that we don't require RTTI.
   */
  virtual TypeId getFunTypeId() const = 0;
};

} // namespace impl
} // namespace fruit

#include <fruit/impl/lazy_component.defn.h>

#endif // FRUIT_LAZY_COMPONENT_H
