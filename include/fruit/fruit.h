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

#ifndef FRUIT_FRUIT_H
#define FRUIT_FRUIT_H

// This header includes all the public Fruit headers.
// To limit the amount of included code, you might want to only include the necessary headers (if only forward
// declarations are needed you can include fruit_forward_decls.h).

// This include is not required here, but having it here shortens the include trace in error messages.
#include <fruit/impl/injection_errors.h>

#include <fruit/component.h>
#include <fruit/component_function.h>
#include <fruit/fruit_forward_decls.h>
#include <fruit/injector.h>
#include <fruit/macro.h>
#include <fruit/normalized_component.h>
#include <fruit/provider.h>

#endif // FRUIT_FRUIT_H
