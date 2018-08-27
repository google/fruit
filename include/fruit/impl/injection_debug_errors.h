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

#ifndef FRUIT_INJECTION_DEBUG_ERRORS_H
#define FRUIT_INJECTION_DEBUG_ERRORS_H

#include <fruit/impl/injection_errors.h>

namespace fruit {
namespace impl {
namespace meta {

template <typename... MissingProvides>
struct ComponentDoesNotEntailDueToProvidesError {
  static_assert(AlwaysFalse<MissingProvides...>::value, "");
};

struct ComponentDoesNotEntailDueToProvidesErrorTag {
  template <typename... MissingProvides>
  using apply = ComponentDoesNotEntailDueToProvidesError<MissingProvides...>;
};

template <typename... MissingInterfaceBindings>
struct ComponentDoesNotEntailDueToInterfaceBindingsError {
  static_assert(AlwaysFalse<MissingInterfaceBindings...>::value, "");
};

struct ComponentDoesNotEntailDueToInterfaceBindingsErrorTag {
  template <typename... MissingInterfaceBindings>
  using apply = ComponentDoesNotEntailDueToInterfaceBindingsError<MissingInterfaceBindings...>;
};

template <typename... AdditionalRequirements>
struct ComponentDoesNotEntailDueToRequirementsError {
  static_assert(AlwaysFalse<AdditionalRequirements...>::value, "");
};

struct ComponentDoesNotEntailDueToRequirementsErrorTag {
  template <typename... AdditionalRequirements>
  using apply = ComponentDoesNotEntailDueToProvidesError<AdditionalRequirements...>;
};

template <typename Deps, typename CandidateEntailedDeps>
struct ComponentDoesNotEntailDueToIncompatibleDepsError {
  static_assert(AlwaysFalse<Deps>::value, "");
};

struct ComponentDoesNotEntailDueToIncompatibleDepsErrorTag {
  template <typename Deps, typename CandidateEntailedDeps>
  using apply = ComponentDoesNotEntailDueToIncompatibleDepsError<Deps, CandidateEntailedDeps>;
};

template <typename... RequirementsWithConstMismatch>
struct ComponentDoesNotEntailDueToDifferentConstnessOfRequirementsError {
  static_assert(AlwaysFalse<RequirementsWithConstMismatch...>::value, "");
};

struct ComponentDoesNotEntailDueToDifferentConstnessOfRequirementsErrorTag {
  template <typename... RequirementsWithConstMismatch>
  using apply = ComponentDoesNotEntailDueToDifferentConstnessOfRequirementsError<RequirementsWithConstMismatch...>;
};

template <typename... ProvidesWithConstMismatch>
struct ComponentDoesNotEntailDueToDifferentConstnessOfProvidesError {
  static_assert(AlwaysFalse<ProvidesWithConstMismatch...>::value, "");
};

struct ComponentDoesNotEntailDueToDifferentConstnessOfProvidesErrorTag {
  template <typename... ProvidesWithConstMismatch>
  using apply = ComponentDoesNotEntailDueToDifferentConstnessOfProvidesError<ProvidesWithConstMismatch...>;
};

template <typename ProofTh, typename ForestThs>
struct ProofNotEntailedByForestBecauseThNotFoundError {
  static_assert(AlwaysFalse<ProofTh>::value, "");
};

struct ProofNotEntailedByForestBecauseThNotFoundErrorTag {
  template <typename ProofTh, typename ForestThs>
  using apply = ProofNotEntailedByForestBecauseThNotFoundError<ProofTh, ForestThs>;
};

template <typename ForestHps, typename ProofHps, typename Difference>
struct ProofNotEntailedByForestBecauseHpsNotASubsetError {
  static_assert(AlwaysFalse<ForestHps>::value, "");
};

struct ProofNotEntailedByForestBecauseHpsNotASubsetErrorTag {
  template <typename ForestHps, typename ProofHps, typename Difference>
  using apply = ProofNotEntailedByForestBecauseHpsNotASubsetError<ForestHps, ProofHps, Difference>;
};

} // namespace meta
} // namespace impl
} // namespace fruit

#endif // FRUIT_INJECTION_DEBUG_ERRORS_H
