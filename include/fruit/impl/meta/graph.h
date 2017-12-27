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

#ifndef FRUIT_META_GRAPH_H
#define FRUIT_META_GRAPH_H

#include <fruit/impl/meta/immutable_map.h>
#include <fruit/impl/meta/map.h>
#include <fruit/impl/meta/set.h>
#include <fruit/impl/meta/triplet.h>

namespace fruit {
namespace impl {
namespace meta {

// A Graph is a Map from each node to a set containing its neighbors.

using GetGraphNodes = GetImmutableMapKeys;

using GraphFindNeighbors = FindInImmutableMap;

using GraphContainsNode = ImmutableMapContainsKey;

// Returns a loop in the given graph as a Vector<N1, ..., Nk> such that the graph contains a loop
// N1->...->Nk->N1, or None if there are no loops.
struct GraphFindLoop {
  template <typename G>
  struct apply {
    using ImmutableG = VectorToImmutableMap(G);

    // DfsVisit(VisitedSet, VisitingSet, Node) does a DFS visit starting at Node and returns a
    // Triplet<NewVisitedSet, Loop, IsLoopComplete>, where Loop is the Vector representing the part
    // of the loop starting at Node (if any loop was found) or Null otherwise.
    struct DfsVisit {
      template <typename VisitedSet, typename VisitingSet, typename Node>
      struct apply {
        using NewVisitingSet = AddToSetUnchecked(VisitingSet, Node);

        struct VisitSingleNeighbor {
          // CurrentResult is a Triplet<VisitedSet, Loop, IsLoopComplete> (where IsLoopComplete
          // is only meaningful when Loop is not None).
          template <typename CurrentResult, typename Neighbor>
          struct apply {
            using type = If(IsNone(typename CurrentResult::Second),
                            // Go ahead, no loop found yet.
                            DfsVisit(typename CurrentResult::First, NewVisitingSet, Neighbor),
                            // Found a loop in another neighbor of the same node, we don't need to
                            // visit this neighbor.
                            CurrentResult);
          };
        };

        using NewVisitedSet = AddToSet(VisitedSet, Node);
        using Neighbors = GraphFindNeighbors(ImmutableG, Node);
        using Result = FoldVector(Neighbors, VisitSingleNeighbor, ConsTriplet(NewVisitedSet, None, Bool<false>));
        using type = If(IsNone(Neighbors),
                        // No neighbors.
                        ConsTriplet(NewVisitedSet, None, Bool<false>),
                        If(IsInSet(Node, VisitingSet),
                           // We've just found a loop, since Node is another node that we're currently
                           // visiting
                           Triplet<VisitedSet, Vector<Node>, Bool<false>>,
                           If(IsNone(GetSecond(Result)),
                              // No loop found.
                              Result,
                              // Found a loop
                              If(GetThird(Result) /* IsLoopComplete */, Result,
                                 If(VectorEndsWith(GetSecond(Result) /* Loop */, Node),
                                    // The loop is complete now.
                                    ConsTriplet(GetFirst(Result), GetSecond(Result), Bool<true>),
                                    // Loop still not complete, add the current node.
                                    ConsTriplet(GetFirst(Result), PushFront(GetSecond(Result), Node), Bool<false>))))));
      };
    };

    struct VisitStartingAtNode {
      // CurrentResult is a Pair<CurrentVisitedSet, Loop>
      template <typename CurrentResult, typename Node>
      struct apply {
        using DfsResult = DfsVisit(GetFirst(CurrentResult), EmptySet, Node);
        using type = If(IsNone(GetSecond(CurrentResult)),
                        // No loop found yet.
                        MakePair(GetFirst(DfsResult), GetSecond(DfsResult)),
                        // Found a loop, return early
                        CurrentResult);
      };
    };

    using type = GetSecond(FoldVector(GetMapKeys(G), VisitStartingAtNode, Pair<EmptySet, None>));
  };
};

} // namespace meta
} // namespace impl
} // namespace fruit

#endif // FRUIT_META_GRAPH_H
