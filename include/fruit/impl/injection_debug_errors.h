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

#ifdef FRUIT_EXTRA_DEBUG

template <typename AdditionalProvidedTypes>
struct CheckNoAdditionalProvidedTypes {
  static_assert(meta::Apply<meta::IsEmptyVector, AdditionalProvidedTypes>::value, 
                "The types in AdditionalProvidedTypes are provided by the new component but weren't provided before.");
};

template <typename AdditionalBindings>
struct CheckNoAdditionalBindings {
  static_assert(meta::Apply<meta::IsEmptyVector, AdditionalBindings>::value, 
                "The types in AdditionalBindings are bindings in the new component but weren't bindings before.");
};

template <typename NoLongerRequiredTypes>
struct CheckNoTypesNoLongerRequired {
  static_assert(meta::Apply<meta::IsEmptyVector, NoLongerRequiredTypes>::value, 
                "The types in NoLongerRequiredTypes were required before but are no longer required by the new component.");
};

template <typename T, typename U>
struct CheckSame {
  static_assert(std::is_same<T, U>::value, "T and U should be the same type");
};

struct CheckDepsNormalized {
  template <typename NormalizedDeps, typename Deps>
  struct apply {
    static_assert(meta::Apply<meta::IsForestEqualTo, NormalizedDeps, Deps>::value,
                  "Internal error: non-normalized deps");
    using type = int;
  };
};

struct CheckComponentEntails {
  template <typename Comp, typename EntailedComp>
  struct apply {
    using AdditionalProvidedTypes = meta::Apply<meta::SetDifference, typename EntailedComp::Ps, typename Comp::Ps>;
    FruitDelegateCheck(CheckNoAdditionalProvidedTypes<AdditionalProvidedTypes>);
    using AdditionalInterfaceBindings = 
        meta::Apply<meta::SetDifference, typename EntailedComp::InterfaceBindings, typename Comp::InterfaceBindings>;
    FruitDelegateCheck(CheckNoAdditionalBindings<AdditionalInterfaceBindings>);
    using NoLongerRequiredTypes = meta::Apply<meta::SetDifference, typename Comp::Rs, typename EntailedComp::Rs>;
    FruitDelegateCheck(CheckNoTypesNoLongerRequired<NoLongerRequiredTypes>);
    static_assert(meta::Apply<meta::IsForestEntailedByForest, typename EntailedComp::Deps, typename Comp::Deps>::value,
                  "Component deps not entailed.");
    using type = int;
  };
};

#endif


} // namespace impl
} // namespace fruit


#endif // FRUIT_INJECTION_DEBUG_ERRORS
