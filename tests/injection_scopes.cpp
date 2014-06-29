// expect-success
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

#include <fruit/fruit.h>
#include <string>
#include <iterator>
#include <algorithm>
#include <sstream>
#include <vector>
#include <iostream>

using namespace std;
using fruit::Component;
using fruit::Injector;

struct ServerContext {
  // This is to show that the requests can get non-request-specific information.
  std::string startupTime;
};

struct RequestContext {
  std::string rawRequest;
};

struct ParsedRequest {
  INJECT(ParsedRequest(const RequestContext& requestContext)) {
    std::istringstream stream(requestContext.rawRequest);
    std::copy(std::istream_iterator<std::string>(stream),
              std::istream_iterator<std::string>(),
              std::back_inserter(words));
  }
  
  std::vector<std::string> words;
};

class RequestHandler {
public:
  virtual void handleRequest() = 0;
};

class RequestHandlerImpl : public RequestHandler {
private:
  ServerContext* serverContext;
  ParsedRequest* parsedRequest;
  
public:
  INJECT(RequestHandlerImpl(ServerContext* serverContext, ParsedRequest* parsedRequest))
    : serverContext(serverContext), parsedRequest(parsedRequest) {
  }
  
  void handleRequest() override {
    cout << "Handling request on server started at " << serverContext->startupTime;
    for (const string& word : parsedRequest->words) {
      cout << "Found word: " << word << endl;
    }
    cout << "Request processed successfully." << endl << endl;
  }
};

fruit::Component<fruit::Required<ServerContext, RequestContext>, RequestHandler> getRequestHandlerComponent() {
  return fruit::createComponent()
    .bind<RequestHandler, RequestHandlerImpl>();
}

class Server {
public:
  virtual void run() = 0;
  virtual void processRequest(std::string line) = 0;
};

class ServerImpl : public Server {
private:
  fruit::Provider<ServerContext> provider;
  
public:
  INJECT(ServerImpl(fruit::Provider<ServerContext> provider))
    : provider(provider) {
  }
  
  void run() override {
    ServerContext* serverContext(provider);
    serverContext->startupTime = getTime();
  }
  
  void processRequest(std::string line) override {
    RequestContext requestContext{line};
    
    fruit::Component<fruit::Required<ServerContext>, RequestHandler> childComponent =
      fruit::createComponent()
        .install(getRequestHandlerComponent())
        .bindInstance(requestContext);
    
    fruit::Injector<RequestHandler> childInjector(provider, childComponent);
    RequestHandler* requestHandler(childInjector);
    requestHandler->handleRequest();
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


int main() {
  ServerContext serverContext;
  
  Injector<Server> injector(
    fruit::createComponent()
      .install(getServerComponent())
      .bindInstance(serverContext));
  // Not necessary in this case. Would be necessary if multiple thread called server->processRequest() in parallel.
  injector.eagerlyInjectAll();
  
  Server* server(injector);
  server->run();
  
  server->processRequest("aa bb");
  
  return 0;
}
