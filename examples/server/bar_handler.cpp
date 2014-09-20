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

#include "bar_handler.h"

#include "path_handler.h"
#include "server_context.h"

#include <iostream>

using namespace std;
using namespace fruit;

class BarHandler {
private:
  const Request& request;
  const ServerContext& serverContext;
  
public:
  INJECT(BarHandler(const Request& request, const ServerContext& serverContext))
    : request(request), serverContext(serverContext) {
  }
  
  void handleRequest() {
    cout << "BarHandler handling request on server started at " << serverContext.startupTime << " for path: " << request.path << endl;
  }
};

class BarPathHandler : public PathHandler {
private:
  Provider<BarHandler> barHandlerProvider;
  
public:
  INJECT(BarPathHandler(Provider<BarHandler> barHandlerProvider))
    : barHandlerProvider(barHandlerProvider) {
  }
  
  const std::string& getPathPrefix() override {
    static const string path_prefix = "/bar/";
    return path_prefix;
  }
  
  void handleRequest() override {
    barHandlerProvider.get<BarHandler*>()->handleRequest();
  }
};

Component<Required<Request, ServerContext>> getBarHandlerComponent() {
  return fruit::createComponent()
    .addMultibinding<PathHandler, BarPathHandler>();
}

