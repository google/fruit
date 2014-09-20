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

#include "foo_handler.h"

#include "path_handler.h"
#include "server_context.h"

#include <iostream>

using namespace std;

using namespace std;
using namespace fruit;

class FooHandler {
private:
  const Request& request;
  const ServerContext& serverContext;
  
public:
  INJECT(FooHandler(const Request& request, const ServerContext& serverContext))
    : request(request), serverContext(serverContext) {
  }
  
  void handleRequest() {
    cout << "FooHandler handling request on server started at " << serverContext.startupTime << " for path: " << request.path << endl;
  }
};

class FooPathHandler : public PathHandler {
private:
  Provider<FooHandler> barHandlerProvider;
  
public:
  INJECT(FooPathHandler(Provider<FooHandler> fooHandlerProvider))
    : barHandlerProvider(fooHandlerProvider) {
  }
  
  const std::string& getPathPrefix() override {
    static const string path_prefix = "/foo/";
    return path_prefix;
  }
  
  void handleRequest() override {
    barHandlerProvider.get<FooHandler*>()->handleRequest();
  }
};

Component<Required<Request, ServerContext>> getFooHandlerComponent() {
  return fruit::createComponent()
    .addMultibinding<PathHandler, FooPathHandler>();
}

