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

#ifndef FRUIT_COMPONENT_STORAGE_DEFN_H
#define FRUIT_COMPONENT_STORAGE_DEFN_H

#include <fruit/impl/component_storage/component_storage.h>
#include <fruit/impl/component_storage/component_storage_entry.h>

namespace fruit {
namespace impl {

inline ComponentStorage::ComponentStorage(FixedSizeVector<ComponentStorageEntry>&& entries)
    : entries(std::move(entries)) {}

inline ComponentStorage::ComponentStorage(const ComponentStorage& other) {
  *this = other;
}

inline ComponentStorage::ComponentStorage(ComponentStorage&& other) {
  *this = std::move(other);
}

inline void ComponentStorage::destroy() {
  for (ComponentStorageEntry& entry : entries) {
    entry.destroy();
  }
  entries.clear();
}

inline ComponentStorage::~ComponentStorage() {
  destroy();
}

inline FixedSizeVector<ComponentStorageEntry> ComponentStorage::release() && {
  return std::move(entries);
}

inline std::size_t ComponentStorage::numEntries() const {
  return entries.size();
}

inline ComponentStorage& ComponentStorage::operator=(const ComponentStorage& other) {
  destroy();

  entries = FixedSizeVector<ComponentStorageEntry>(other.entries.size());
  for (const ComponentStorageEntry& entry : other.entries) {
    entries.push_back(entry.copy());
  }

  return *this;
}

inline ComponentStorage& ComponentStorage::operator=(ComponentStorage&& other) {
  entries = std::move(other.entries);

  // We don't want other to have any entries after this operation because we might otherwise end up destroying those
  // ComponentStorageEntry objects twice.
  other.entries.clear();

  return *this;
}

} // namespace impl
} // namespace fruit

#endif // FRUIT_COMPONENT_STORAGE_DEFN_H
