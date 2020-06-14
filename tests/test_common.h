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

#ifndef FRUIT_COMMON_H
#define FRUIT_COMMON_H

// This file includes headers used in various tests.
// This allows to improve compilation speed (and therefore test time) by pre-compiling this header.

#include "class_construction_tracker.h"
#include "test_macros.h"
#include <fruit/fruit.h>
#include <fruit/impl/injector/injector_accessor_for_tests.h>
#include <map>
#include <vector>

// These are here because including Boost in test code would require depending on its headers but those files don't have
// public visibility in the bazel repo.
#include <fruit/impl/data_structures/semistatic_graph.h>
using Graph = fruit::impl::SemistaticGraph<int, const char*>;
using node_iterator = Graph::node_iterator;
using edge_iterator = Graph::edge_iterator;
struct SimpleNode {
  int id;
  const char* value;
  const std::vector<int>* neighbors;
  bool is_terminal;

  int getId() { return id; }
  const char* getValue() { return value; }
  bool isTerminal() { return is_terminal; }
  std::vector<int>::const_iterator getEdgesBegin() { return neighbors->begin(); }
  std::vector<int>::const_iterator getEdgesEnd() { return neighbors->end(); }
};

#endif // FRUIT_COMMON_H
