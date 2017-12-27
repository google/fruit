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

#ifndef BAR_HANDLER_H
#define BAR_HANDLER_H

#include "request.h"
#include "server_context.h"

#include <fruit/fruit.h>

class BarHandler {
public:
  // Handles a request for a subpath of "/bar/".
  // The request is injected, no need to pass it directly here.
  virtual void handleRequest() = 0;
};

fruit::Component<fruit::Required<Request, ServerContext>, BarHandler> getBarHandlerComponent();

#endif // BAR_HANDLER_H
