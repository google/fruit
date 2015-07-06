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

#ifndef FRUIT_INJECTION_DEBUG_ERRORS
#define FRUIT_INJECTION_DEBUG_ERRORS

namespace fruit {
namespace impl {
namespace meta {

#ifdef FRUIT_EXTRA_DEBUG

struct CheckNoAdditionalProvidedTypes {
  template <typename AdditionalProvidedTypes>
  struct apply {
    FruitStaticAssert(IsEmptyVector(AdditionalProvidedTypes));
    using type = int;
  };
};

struct CheckNoAdditionalBindings {
  template <typename AdditionalBindings>
  struct apply {
    FruitStaticAssert(IsEmptyVector(AdditionalBindings));
    using type = int;
  };
};

struct CheckNoTypesNoLongerRequired {
  template <typename NoLongerRequiredTypes>
  struct apply {
    FruitStaticAssert(IsEmptyVector(NoLongerRequiredTypes));
    using type = int;
  };
};

struct CheckSame {
  template <typename T, typename U>
  struct apply {
    FruitStaticAssert(IsSame(T, U));
    using type = int;
  };
};

struct CheckDepsNormalized {
  template <typename NormalizedDeps, typename Deps>
  struct apply {
    FruitStaticAssert(IsForestEqualTo(NormalizedDeps, Deps));
    using type = int;
  };
};

struct CheckComponentEntails {
  template <typename Comp, typename EntailedComp>
  struct apply {
    using AdditionalProvidedTypes = SetDifference(typename EntailedComp::Ps, typename Comp::Ps);
    FruitDelegateCheck(CheckNoAdditionalProvidedTypes(AdditionalProvidedTypes));
    using AdditionalInterfaceBindings = SetDifference(typename EntailedComp::InterfaceBindings,
                                                      typename Comp::InterfaceBindings);
    FruitDelegateCheck(CheckNoAdditionalBindings(AdditionalInterfaceBindings));
    using NoLongerRequiredTypes = SetDifference(typename Comp::Rs, typename EntailedComp::Rs);
    FruitDelegateCheck(CheckNoTypesNoLongerRequired(NoLongerRequiredTypes));
    FruitStaticAssert(IsForestEntailedByForest(typename EntailedComp::Deps, typename Comp::Deps));
    using type = int;
  };
};

#endif


} // namespace meta
} // namespace impl
} // namespace fruit


#endif // FRUIT_INJECTION_DEBUG_ERRORS
