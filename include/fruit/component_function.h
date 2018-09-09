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

#ifndef FRUIT_COMPONENT_FUNCTION_H
#define FRUIT_COMPONENT_FUNCTION_H

#include <fruit/impl/fruit_internal_forward_decls.h>

namespace fruit {

/**
 * See fruit::componentFunction() helper for how to construct a ComponentFunction, and see
 * PartialComponent::installComponentFunctions() for more information on using ComponentFunction objects.
 */
template <typename ComponentType, typename... ComponentFunctionArgs>
class ComponentFunction {
private:
    ComponentType (*getComponent)(ComponentFunctionArgs...);
    std::tuple<ComponentFunctionArgs...> args_tuple;

    /**
     * This is (intentionally) private, use fruit::componentFunction() to construct ComponentFunction objects.
     */
    ComponentFunction(ComponentType (*getComponent)(ComponentFunctionArgs...), ComponentFunctionArgs... args);

    friend struct fruit::impl::ComponentStorageEntry;

public:
	// Prefer using the simpler componentFunction() below instead of this.
	template <typename... ActualArgs>
    static ComponentFunction<ComponentType, ComponentFunctionArgs...> create(
		ComponentType (*getComponent)(ComponentFunctionArgs...), ActualArgs&&... args);

    ComponentFunction(const ComponentFunction&) = default;
    ComponentFunction(ComponentFunction&&) = default;

    ComponentFunction& operator=(const ComponentFunction&) = default;
    ComponentFunction& operator=(ComponentFunction&&) = default;

    ComponentType operator()();
};


/**
 * This function allows to easily construct a ComponentFunction without explicitly mentioning its type.
 * See PartialComponent::installComponentFunctions() for more information on using ComponentFunction.
 */
template <typename... ComponentParams, typename... FormalArgs, typename... ActualArgs>
ComponentFunction<fruit::Component<ComponentParams...>, FormalArgs...> componentFunction(
    fruit::Component<ComponentParams...> (*getComponent)(FormalArgs...),
    ActualArgs&&... args);

}

#include <fruit/impl/component_function.defn.h>

#endif // FRUIT_COMPONENT_FUNCTION_H
