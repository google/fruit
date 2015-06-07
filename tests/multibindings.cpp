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
#include "test_macros.h"
#include <iostream>

using fruit::Component;
using fruit::Injector;

static int numNotificationsToListener1 = 0;
static int numNotificationsToListener2 = 0;

class Listener {
public:
  virtual ~Listener() = default;
  
  virtual void notify() = 0;
};

class Listener1 : public Listener {
public:
  INJECT(Listener1()) = default;
  
  virtual ~Listener1() = default;
  
  void notify() override {
    ++numNotificationsToListener1;
  }
};

class Writer {
public:
  virtual void write(std::string s) = 0;
};

class StdoutWriter : public Writer {
public:
  INJECT(StdoutWriter()) = default;
  
  void write(std::string s) override {
    std::cout << s << std::endl;
  }
};

class Listener2 : public Listener {
private:
  Writer* writer;
  
public:
  INJECT(Listener2(Writer* writer))
    : writer(writer) {
  }
  
  virtual ~Listener2() = default;
  
  void notify() override {
    (void) writer;
    ++numNotificationsToListener2;
  }
};

Component<> getListenersComponent() {
  return fruit::createComponent()
    .bind<Writer, StdoutWriter>()
    // Note: this is just to exercise the other method, but in real code you should split this in
    // an addMultibinding<Listener, Listener1> and a registerProvider with the lambda.
    .addMultibindingProvider([]() {
      Listener1* listener1 = new Listener1();
      return static_cast<Listener*>(listener1);
    })
    .addMultibinding<Listener, Listener2>();
}

int main() {
  Injector<> injector(getListenersComponent());
  std::vector<Listener*> listeners = injector.getMultibindings<Listener>();
  
  for (Listener* listener : listeners) {
    listener->notify();
  }
  
  std::vector<Listener*> listeners2 = injector.getMultibindings<Listener>();
  Assert(listeners == listeners2);
  
  if (numNotificationsToListener1 != 1 || numNotificationsToListener2 != 1)
    abort();
  
  return 0;
}
