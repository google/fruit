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

#include <fruit/component.h>

#include <exception>
#include <iostream>

namespace fruit {

// TODO: reimplement this check somehow.
/*
EmptyPartialComponent::~EmptyPartialComponent() {
  // If the user of Fruit didn't cast the result of createComponent() (possibly after adding some bindings) to a
Component<>, we abort
  // because that's a misuse of the Fruit API. If we went ahead, there might be some PartialComponent<> instances that
point
  // to the ComponentStorage in this EmptyComponent, and any use of those would cause undefined behavior.
  // If an exception is in flight, don't abort; that's likely to be an unexpected flow so we don't want to alert the
user of Fruit,
  // and there can't be any leftover instances of PartialComponent<> referring to this EmptyComponent anyway.
  if (!already_converted_to_component && !std::uncaught_exception()) {
    std::cerr << "The result of fruit::createComponent() was not converted to a Component before the end of the
expression! "
        << "This is a misuse of the Fruit API. This is likely to cause undefined behavior, aborting now to be safe." <<
std::endl;
    std::abort();
  }
}
*/

// We need a LCOV_EXCL_BR_LINE below because for some reason gcov/lcov think there's a branch there.
} // namespace fruit LCOV_EXCL_BR_LINE
