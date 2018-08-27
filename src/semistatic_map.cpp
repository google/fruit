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

#define IN_FRUIT_CPP_FILE 1

#include <fruit/impl/data_structures/semistatic_graph.h>
#include <fruit/impl/data_structures/semistatic_map.h>
#include <fruit/impl/data_structures/semistatic_map.templates.h>

#include <fruit/impl/util/type_info.h>

using namespace fruit::impl;

// Clang requires the following instantiation to be in its namespace.
namespace fruit {
namespace impl {

template class SemistaticMap<TypeId, SemistaticGraphInternalNodeId>;

} // namespace impl
} // namespace fruit
