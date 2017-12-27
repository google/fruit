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
#include <iostream>

using fruit::Component;
using fruit::Injector;

class Listener {
public:
  virtual void notify() = 0;
};

class Listener1 : public Listener {
public:
  INJECT(Listener1()) = default;

  void notify() override {
    std::cout << "Listener 1 notified" << std::endl;
  }
};

class Writer {
public:
  virtual void write(std::string s) = 0;
};

// To show that we can inject parameters of multibindings
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
  INJECT(Listener2(Writer* writer)) : writer(writer) {}

  void notify() override {
    writer->write("Listener 2 notified");
  }
};

Component<> getListenersComponent() {
  // Here they are in the same component to keep it simple, but Fruit collects all multibindings in installed
  // components.
  return fruit::createComponent()
      .bind<Writer, StdoutWriter>()
      .addMultibinding<Listener, Listener1>()
      .addMultibinding<Listener, Listener2>();
}

int main() {
  Injector<> injector(getListenersComponent);
  std::vector<Listener*> listeners = injector.getMultibindings<Listener>();

  // The order of the returned listeners is unspecified, so the lines in output may have any order.
  for (Listener* listener : listeners) {
    listener->notify();
  }

  return 0;
}
