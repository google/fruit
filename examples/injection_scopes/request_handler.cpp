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

#include "request_handler.h"

#include "parsed_request.h"
#include "server_context.h"

#include <iostream>

using namespace std;

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
