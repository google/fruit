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

#include "request_handler.h"
#include "server_context.h"

#include <iostream>

using namespace std;

class BarHandler : public RequestHandler {
private:
  const std::string prefix = "/bar/";
  
public:
  INJECT(BarHandler()) {
  }
  
  const std::string& getPathPrefix() override {
    return prefix;
  }
  
  void handleRequest(const ServerContext& serverContext, const Request& request) override {
    cout << "BarHandler handling request on server started at " << serverContext.startupTime << " for path: " << request.path << endl;
  }
};

fruit::Component<> getBarHandler() {
  return fruit::createComponent()
    .addMultibinding<RequestHandler, BarHandler>();
}
