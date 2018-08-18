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

#ifndef FRUIT_COMPONENT_INSTALL_ARG_CHECKS_DEFN_H
#define FRUIT_COMPONENT_INSTALL_ARG_CHECKS_DEFN_H

#include <fruit/impl/component_install_arg_checks.h>

#include <utility>
#include <functional>

namespace fruit {
namespace impl {

template <typename T>
FRUIT_ALWAYS_INLINE inline int checkAcceptableComponentInstallArg() {
    // This lambda checks that the required operations on T exist.
    // Note that the lambda is never actually executed.
    auto checkRequirements = [](const T& constRef, T value) {
        T x1(constRef);
        T x2(std::move(value));
        x1 = constRef;
        x2 = std::move(value);
        bool b = (constRef == constRef);
        std::size_t h = std::hash<T>()(constRef);
        (void)x1;
        (void)x2;
        (void)b;
        (void)h;
    };
    (void)checkRequirements;
    return 0;
}

} // namespace impl
} // namespace fruit

#endif // FRUIT_COMPONENT_INSTALL_ARG_CHECKS_DEFN_H
