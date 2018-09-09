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

#ifndef FRUIT_COMPONENT_FUNCTION_DEFN_H
#define FRUIT_COMPONENT_FUNCTION_DEFN_H

#include <fruit/component_function.h>
#include <fruit/impl/util/call_with_tuple.h>
#include <fruit/impl/component_install_arg_checks.h>

namespace fruit {

template <typename ComponentType, typename... ComponentFunctionArgs>
inline ComponentFunction<ComponentType, ComponentFunctionArgs...>::ComponentFunction(
        ComponentType (*getComponent)(ComponentFunctionArgs...), ComponentFunctionArgs... args)
    : getComponent(getComponent), args_tuple{args...} {
    using IntCollector = int[];
    (void)IntCollector{0, fruit::impl::checkAcceptableComponentInstallArg<ComponentFunctionArgs>()...};
}

template <typename ComponentType, typename... ComponentFunctionArgs>
template <typename... ActualArgs>
inline ComponentFunction<ComponentType, ComponentFunctionArgs...>
ComponentFunction<ComponentType, ComponentFunctionArgs...>::create(
	ComponentType(*getComponent)(ComponentFunctionArgs...), ActualArgs&&... args) {
  return ComponentFunction<ComponentType, ComponentFunctionArgs...>(getComponent, std::forward<ActualArgs>(args)...);
}

template <typename ComponentType, typename... ComponentFunctionArgs>
inline ComponentType ComponentFunction<ComponentType, ComponentFunctionArgs...>::operator()() {
    return fruit::impl::callWithTuple(getComponent, args_tuple);
}

template <typename... ComponentParams, typename... FormalArgs, typename... ActualArgs>
inline ComponentFunction<fruit::Component<ComponentParams...>, FormalArgs...> componentFunction(
        fruit::Component<ComponentParams...> (*getComponent)(FormalArgs...),
        ActualArgs&&... args) {
    return ComponentFunction<fruit::Component<ComponentParams...>, FormalArgs...>::create(
        getComponent, std::forward<ActualArgs>(args)...);
}

}

#endif // FRUIT_COMPONENT_FUNCTION_DEFN_H
