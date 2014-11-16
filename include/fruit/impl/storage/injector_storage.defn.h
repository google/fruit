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

#ifndef FRUIT_INJECTOR_STORAGE_DEFN_H
#define FRUIT_INJECTOR_STORAGE_DEFN_H

#include "../util/demangle_type_name.h"
#include "../util/type_info.h"
#include "../fruit_assert.h"

#include <cassert>

// Redundant, but makes KDevelop happy.
#include "injector_storage.h"

namespace fruit {
namespace impl {

template <typename AnnotatedSignature>
struct BindAssistedFactory;

// General case, value
template <typename C>
struct GetHelper {
  C operator()(InjectorStorage& injector) {
    return *(injector.getPtr<C>());
  }
  C operator()(InjectorStorage& injector, NormalizedComponentStorage::Graph::edge_iterator deps, std::size_t dep_index) {
    return *(injector.getPtr<C>(deps, dep_index));
  }
};

template <typename C>
struct GetHelper<const C> {
  const C operator()(InjectorStorage& injector) {
    return *(injector.getPtr<C>());
  }
  const C operator()(InjectorStorage& injector, NormalizedComponentStorage::Graph::edge_iterator deps, std::size_t dep_index) {
    return *(injector.getPtr<C>(deps, dep_index));
  }
};

template <typename C>
struct GetHelper<std::shared_ptr<C>> {
  std::shared_ptr<C> operator()(InjectorStorage& injector) {
    return std::shared_ptr<C>(std::shared_ptr<char>(), injector.getPtr<C>());
  }
  std::shared_ptr<C> operator()(InjectorStorage& injector, NormalizedComponentStorage::Graph::edge_iterator deps, std::size_t dep_index) {
    return std::shared_ptr<C>(std::shared_ptr<char>(), injector.getPtr<C>(deps, dep_index));
  }
};

template <typename C>
struct GetHelper<C*> {
  C* operator()(InjectorStorage& injector) {
    return injector.getPtr<C>();
  }
  C* operator()(InjectorStorage& injector, NormalizedComponentStorage::Graph::edge_iterator deps, std::size_t dep_index) {
    return injector.getPtr<C>(deps, dep_index);
  }
};

template <typename C>
struct GetHelper<const C*> {
  const C* operator()(InjectorStorage& injector) {
    return injector.getPtr<C>();
  }
  const C* operator()(InjectorStorage& injector, NormalizedComponentStorage::Graph::edge_iterator deps, std::size_t dep_index) {
    return injector.getPtr<C>(deps, dep_index);
  }
};

template <typename C>
struct GetHelper<C&> {
  C& operator()(InjectorStorage& injector) {
    return *(injector.getPtr<C>());
  }
  C& operator()(InjectorStorage& injector, NormalizedComponentStorage::Graph::edge_iterator deps, std::size_t dep_index) {
    return *(injector.getPtr<C>(deps, dep_index));
  }
};

template <typename C>
struct GetHelper<const C&> {
  const C& operator()(InjectorStorage& injector) {
    return *(injector.getPtr<C>());
  }
  const C& operator()(InjectorStorage& injector, NormalizedComponentStorage::Graph::edge_iterator deps, std::size_t dep_index) {
    return *(injector.getPtr<C>(deps, dep_index));
  }
};

template <typename C>
struct GetHelper<Provider<C>> {
  Provider<C> operator()(InjectorStorage& injector) {
    return Provider<C>(&injector, injector.lazyGetPtr<C>());
  }
  Provider<C> operator()(InjectorStorage& injector, NormalizedComponentStorage::Graph::edge_iterator deps, std::size_t dep_index) {
    return Provider<C>(&injector, injector.lazyGetPtr<C>(deps, dep_index));
  }
};

template <typename C>
inline InjectorStorage::Graph::node_iterator InjectorStorage::lazyGetPtr() {
  return lazyGetPtr(getTypeId<C>());
}

template <typename C>
inline InjectorStorage::Graph::node_iterator InjectorStorage::lazyGetPtr(Graph::edge_iterator deps, std::size_t dep_index) {
  Graph::node_iterator itr = lazyGetPtr(deps, dep_index);
  assert(typeRegistry.find(getTypeId<C>()) == itr);
  assert(!(typeRegistry.end() == itr));
  return itr;
}

template <typename C>
inline C* InjectorStorage::getPtr() {
  void* p = getPtr(getTypeId<C>());
  return reinterpret_cast<C*>(p);
}

template <typename C>
inline C* InjectorStorage::getPtr(Graph::edge_iterator deps, std::size_t dep_index) {
  void* p = getPtr(lazyGetPtr(deps, dep_index));
  return reinterpret_cast<C*>(p);
}

template <typename C>
inline C* InjectorStorage::getPtr(Graph::node_iterator itr) {
  assert(typeRegistry.find(getTypeId<C>()) == itr);
  assert(!(typeRegistry.end() == itr));
  void* p = getPtr(itr);
  return reinterpret_cast<C*>(p);
}

template <typename C>
inline C* InjectorStorage::unsafeGet() {
  void* p = unsafeGetPtr(getTypeId<C>());
  return reinterpret_cast<C*>(p);
}

inline void* InjectorStorage::getPtr(TypeId typeInfo) {
  return getPtr(lazyGetPtr(typeInfo));
}

inline void* InjectorStorage::getPtr(Graph::node_iterator itr) {
  ensureConstructed(itr);
  return itr.getNode().getStoredSingleton();
}

inline InjectorStorage::Graph::node_iterator InjectorStorage::lazyGetPtr(TypeId typeInfo) {
  return typeRegistry.at(typeInfo);
}

inline InjectorStorage::Graph::node_iterator InjectorStorage::lazyGetPtr(Graph::edge_iterator deps, std::size_t dep_index) {
  return deps.getNodeIterator(dep_index, typeRegistry);
}

inline void* InjectorStorage::unsafeGetPtr(TypeId typeInfo) {
  Graph::node_iterator itr = typeRegistry.find(typeInfo);
  if (itr == typeRegistry.end()) {
    return nullptr;
  }
  ensureConstructed(itr);
  return itr.getNode().getStoredSingleton();
}

inline std::size_t InjectorStorage::maximumRequiredSpace(fruit::impl::TypeId typeId) {
  return typeId.type_info->alignment() + typeId.type_info->size() - 1;
}

template <typename C, typename... Args>
inline C* InjectorStorage::constructSingleton(Args&&... args) {
  char* p = singletonStorageLastUsed;
  size_t misalignment = std::uintptr_t(p) % alignof(C);
  p += alignof(C) - misalignment;
  assert(std::uintptr_t(p) % alignof(C) == 0);
  C* c = reinterpret_cast<C*>(p);
  new (c) C(std::forward<Args>(args)...);
  singletonStorageLastUsed = p + sizeof(C) - 1;
  return c;
}

template <typename C>
void InjectorStorage::destroySingleton(void* p) {
  C* cPtr = reinterpret_cast<C*>(p);
  cPtr->C::~C();
}

template <typename C>
void InjectorStorage::destroyExternalSingleton(void* p) {
  C* cPtr = reinterpret_cast<C*>(p);
  delete cPtr;
}

template <typename C>
inline const std::vector<C*>& InjectorStorage::getMultibindings() {
  void* p = getMultibindings(getTypeId<C>());
  if (p == nullptr) {
    static std::vector<C*> empty_vector;
    return empty_vector;
  } else {
    return *reinterpret_cast<std::vector<C*>*>(p);
  }
}

inline void InjectorStorage::ensureConstructed(typename SemistaticGraph<TypeId, NormalizedBindingData>::node_iterator nodeItr) {
  NormalizedBindingData& bindingData = nodeItr.getNode();
  if (!nodeItr.isTerminal()) {
    BindingData::destroy_t destroy;
    void* p;
    std::tie(destroy, p) = bindingData.create(*this, nodeItr.neighborsBegin());
    nodeItr.setTerminal();
    if (destroy != nullptr) {
      onDestruction.push_back(std::make_pair(destroy, p));
    }
  }
}

inline NormalizedMultibindingData* InjectorStorage::getNormalizedMultibindingData(TypeId typeInfo) {
  auto itr = typeRegistryForMultibindings.find(typeInfo);
  if (itr != typeRegistryForMultibindings.end())
    return &(itr->second);
  else
    return nullptr;
}

} // namespace fruit
} // namespace impl


#endif // FRUIT_INJECTOR_STORAGE_DEFN_H
