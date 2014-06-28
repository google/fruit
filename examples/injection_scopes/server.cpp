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

#include "server.h"

#include "server_context.h"
#include "request_context.h"

// We could split the interface in a separate file, and get rid of the dependency on the get*Component()
// by injecting a Component<RequestHandler> in the main injector.
// We don't do it in this example to make it simpler to understand.
#include "request_handler.h"

#include <iostream>
#include <string>
#include <ctime>

using namespace std;

class ServerImpl : public Server {
private:
  fruit::Injector<ServerContext> injector;
  
public:
  INJECT(ServerImpl(fruit::Injector<ServerContext> injector))
    : injector(injector) {
  }
  
  void run() override {
    ServerContext* serverContext(injector);
    serverContext->startupTime = getTime();
    
    cout << "Server started." << endl;
    cout << "Enter the request (words separated by spaces, in a single line), or an empty line to exit." << endl;
    while (1) {
      string line;
      getline(cin, line);
      cout << "Server received request: " + line << endl;
      if (line.empty()) {
        cout << "Server received empty line, shutting down." << endl;
        break;
      }
      RequestContext requestContext{line};
      
      fruit::Component<fruit::Required<ServerContext>, RequestHandler> childComponent =
        fruit::createComponent()
          .install(getRequestHandlerComponent())
          .bindInstance(requestContext);
      
      fruit::Injector<RequestHandler> childInjector(injector, childComponent);
      RequestHandler* requestHandler(childInjector);
      requestHandler->handleRequest();
    }
  }
  
private:
  static string getTime() {
    time_t now = time(nullptr);
    tm* localTime = localtime(&now);
    return {asctime(localTime)};
  }
};

fruit::Component<fruit::Required<ServerContext>, Server> getServerComponent() {
  return fruit::createComponent()
    .bind<Server, ServerImpl>();
}
