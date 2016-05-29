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

#ifndef FRUIT_SPARSEHASH_HELPERS_FORWARD_DECLS_H
#define FRUIT_SPARSEHASH_HELPERS_FORWARD_DECLS_H

#include <boost/unordered/unordered_set_fwd.hpp>
#include <boost/unordered/unordered_map_fwd.hpp>

namespace fruit {
namespace impl {

template <typename T>
using HashSet = boost::unordered_set<T, std::hash<T>>;

template <typename Key, typename Value>
using HashMap = boost::unordered_map<Key, Value, std::hash<Key>>;

} // namespace impl
} // namespace fruit

#endif // FRUIT_SPARSEHASH_HELPERS_FORWARD_DECLS_H
