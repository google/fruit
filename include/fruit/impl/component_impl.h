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

#ifndef FRUIT_COMPONENT_IMPL_H
#define FRUIT_COMPONENT_IMPL_H

#include "../fruit_forward_decls.h"
#include "component_storage.h"
#include "component.utils.h"
#include "injection_errors.h"
#include "fruit_assert.h"

namespace fruit {

namespace impl {

template <typename RsParam, typename PsParam, typename DepsParam, typename BindingsParam>
struct ConsComp {
  // Just for convenience.
  using Rs = RsParam;
  using Ps = PsParam;
  using Deps = DepsParam;
  using Bindings = BindingsParam;
  
  // Invariants:
  // * all types appearing as arguments of Deps are in Rs
  // * all types in Ps are at the head of one (and only one) Dep.
  //   (note that the types in Rs can appear in deps any number of times, 0 is also ok)
  // * Deps is of the form List<Dep...> with each Dep of the form T(Args...) and where List<Args...> is a set (no repetitions).
  // * Bindings is of the form List<ConsBinding<I1, C1>, ..., ConsBinding<In, Cn>> and is a set (no repetitions).
  
  FruitStaticAssert(true || sizeof(CheckDepsNormalized<AddDeps<Deps, List<>, List<>>, Deps>), "");
};

// Non-specialized case: no requirements.
template <typename... Ps>
struct ConstructComponentImplHelper {
  FruitDelegateCheck(fruit::impl::CheckNoRepeatedTypes<Ps...>);
  FruitDelegateChecks(fruit::impl::CheckClassType<Ps, fruit::impl::GetClassForType<Ps>>);
  using type = ConsComp<List<>,
                        List<Ps...>,
                        ConstructDeps<List<>, Ps...>,
                        List<>>;
};

// Non-specialized case: no requirements.
template <typename... Rs, typename... Ps>
struct ConstructComponentImplHelper<Required<Rs...>, Ps...> {
  FruitDelegateCheck(fruit::impl::CheckNoRepeatedTypes<Rs..., Ps...>);
  FruitDelegateChecks(fruit::impl::CheckClassType<Rs, fruit::impl::GetClassForType<Rs>>);
  FruitDelegateChecks(fruit::impl::CheckClassType<Ps, fruit::impl::GetClassForType<Ps>>);
  using type = ConsComp<List<Rs...>,
                        List<Ps...>,
                        ConstructDeps<List<Rs...>, Ps...>,
                        List<>>;
};

template <typename... Types>
using ConstructComponentImpl = typename ConstructComponentImplHelper<Types...>::type;


} // namespace impl

} // namespace fruit

#endif // FRUIT_COMPONENT_IMPL_H
