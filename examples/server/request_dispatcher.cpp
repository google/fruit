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

#include "request_dispatcher.h"

#include "bar_handler.h"
#include "foo_handler.h"

#include <iostream>

using namespace std;
using namespace fruit;

class RequestDispatcherImpl : public RequestDispatcher {
private:
  const Request& request;
  // We hold providers here for lazy injection; we only want to inject the handler that is actually used for the
  // request.
  // In a large system, there will be many handlers, and many will have lots of dependencies that also have to be
  // injected.
  Provider<FooHandler> fooHandler;
  Provider<BarHandler> barHandler;

public:
  INJECT(RequestDispatcherImpl(const Request& request, Provider<FooHandler> fooHandler,
                               Provider<BarHandler> barHandler))
      : request(request), fooHandler(fooHandler), barHandler(barHandler) {}

  void handleRequest() override {
    if (stringStartsWith(request.path, "/foo/")) {
      fooHandler.get()->handleRequest();
    } else if (stringStartsWith(request.path, "/bar/")) {
      barHandler.get()->handleRequest();
    } else {
      cerr << "Error: no handler found for request path: '" << request.path << "' , ignoring request." << endl;
    }
  }

private:
  static bool stringStartsWith(const string& s, const string& candidatePrefix) {
    return s.compare(0, candidatePrefix.size(), candidatePrefix) == 0;
  }
};

Component<Required<Request, ServerContext>, RequestDispatcher> getRequestDispatcherComponent() {
  return createComponent()
      .bind<RequestDispatcher, RequestDispatcherImpl>()
      .install(getFooHandlerComponent)
      .install(getBarHandlerComponent);
}
