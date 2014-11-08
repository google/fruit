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
#include "path_handler.h"

#include <iostream>
#include <string>
#include <ctime>
#include <thread>
#include <vector>

using namespace std;
using namespace fruit;

class ServerImpl : public Server {
private:
  std::vector<std::thread> threads;
  
public:
  INJECT(ServerImpl()) {
  }
  
  ServerImpl(ServerImpl&&) = default;
  
  ~ServerImpl() {
    for (std::thread& t : threads) {
      t.join();
    }
  }
  
  void run(Component<Required<Request, ServerContext>>&& requestHandlersComponent) override {
    ServerContext serverContext;
    serverContext.startupTime = getTime();
    
    const NormalizedComponent<Required<Request>> requestHandlersNormalizedComponent(
      createComponent()
      .install(std::move(requestHandlersComponent))
      .bindInstance(serverContext));
    
    printPathHandlers(requestHandlersNormalizedComponent);
    cerr << "Server started." << endl;
    
    while (1) {
      cerr << endl;
      cerr << "Enter the request (absolute path starting one of the registered paths above), or an empty line to exit." << endl;
      Request request;
      getline(cin, request.path);
      cerr << "Server received request: " + request.path << endl;
      if (request.path.empty()) {
        cerr << "Server received empty line, shutting down." << endl;
        break;
      }
      
      // In production code we would use a thread pool.
      // Here we spawn a new thread each time to keep it simple.
      threads.push_back(std::thread(worker_thread_main, requestHandlersNormalizedComponent, request));
    }
  }
  
private:
  static void worker_thread_main(const NormalizedComponent<Required<Request>>& requestHandlerNormalizedComponent,
                                 Request request) {
    
    Injector<> injector(requestHandlerNormalizedComponent,
                        Component<Request>(createComponent()
                                               .bindInstance(request)));
    
    vector<PathHandler*> handlers = injector.getMultibindings<PathHandler>();
    
    PathHandler* handler = nullptr;
    for (PathHandler* candidateHandler : handlers) {
      if (stringStartsWith(request.path, candidateHandler->getPathPrefix())) {
        if (handler != nullptr) {
          cerr << "Error: multiple handlers found for request path: '" << request.path << "' , ignoring request." << endl;
          return;
        }
        handler = candidateHandler;
      }
    }
    if (handler == nullptr) {
      cerr << "Error: no handler found for request path: '" << request.path << "' , ignoring request." << endl;
      return;
    }
    handler->handleRequest();
  }
  
  static bool stringStartsWith(const string& s, const string& candidatePrefix) {
    return s.compare(0, candidatePrefix.size(), candidatePrefix) == 0;
  }
  
  static string getTime() {
    time_t now = time(nullptr);
    tm* localTime = localtime(&now);
    string result = asctime(localTime);
    if (result.size() != 0 && result.back() == '\n') {
      result.pop_back();
    }
    return result;
  }

  void printPathHandlers(const NormalizedComponent<Required<Request>>& requestHandlersNormalizedComponent) {
    
    // The same path handlers are available, independently from the request.
    Request dummyRequest;
    
    Injector<> injector(requestHandlersNormalizedComponent,
                        Component<Request>(createComponent()
                                               .bindInstance(dummyRequest)));
    
    vector<PathHandler*> handlers = injector.getMultibindings<PathHandler>();
    cerr << "Registered paths:" << endl;
    for (PathHandler* handler : handlers) {
      cerr << handler->getPathPrefix() << "*" << endl;
    }
  }
};

fruit::Component<Server> getServerComponent() {
  return fruit::createComponent()
    .bind<Server, ServerImpl>();
}
