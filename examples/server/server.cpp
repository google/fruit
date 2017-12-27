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

#include <ctime>
#include <iostream>
#include <thread>

using namespace std;
using namespace fruit;

class ServerImpl : public Server {
private:
  std::vector<std::thread> threads;

public:
  INJECT(ServerImpl()) {}

  ~ServerImpl() {
    for (std::thread& t : threads) {
      t.join();
    }
  }

  void run(Component<Required<Request, ServerContext>, RequestDispatcher> (*getRequestDispatcherComponent)()) override {
    ServerContext serverContext;
    serverContext.startupTime = getTime();

    const NormalizedComponent<Required<Request>, RequestDispatcher> requestDispatcherNormalizedComponent(
        getRequestDispatcherComponentWithContext, getRequestDispatcherComponent, &serverContext);

    cerr << "Server started." << endl;

    while (1) {
      cerr << endl;
      cerr << "Enter the request (absolute path starting with \"/foo/\" or \"/bar/\"), or an empty line to exit."
           << endl;
      Request request;
      getline(cin, request.path);
      cerr << "Server received request: " + request.path << endl;
      if (request.path.empty()) {
        cerr << "Server received empty line, shutting down." << endl;
        break;
      }

      // In production code we would use a thread pool.
      // Here we spawn a new thread each time to keep it simple.
      threads.push_back(std::thread(worker_thread_main, std::ref(requestDispatcherNormalizedComponent), request));
    }
  }

private:
  static void worker_thread_main(
      const NormalizedComponent<Required<Request>, RequestDispatcher>& requestDispatcherNormalizedComponent,
      Request request) {
    Injector<RequestDispatcher> injector(requestDispatcherNormalizedComponent, getRequestComponent, &request);

    RequestDispatcher* requestDispatcher(injector);
    requestDispatcher->handleRequest();
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

  static Component<Request> getRequestComponent(Request* request) {
    return createComponent().bindInstance(*request);
  }

  static Component<Required<Request>, RequestDispatcher> getRequestDispatcherComponentWithContext(
      Component<Required<Request, ServerContext>, RequestDispatcher> (*getRequestDispatcherComponent)(),
      ServerContext* serverContext) {
    return createComponent().install(getRequestDispatcherComponent).bindInstance(*serverContext);
  }
};

fruit::Component<Server> getServerComponent() {
  return fruit::createComponent().bind<Server, ServerImpl>();
}
