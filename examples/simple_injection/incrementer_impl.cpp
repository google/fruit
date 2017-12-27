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

#include "incrementer_impl.h"

class IncrementerImpl : public Incrementer {
private:
  Adder* adder;

public:
  INJECT(IncrementerImpl(Adder* adder)) : adder(adder) {}

  virtual int increment(int x) override {
    return adder->add(x, 1);
  }
};

fruit::Component<fruit::Required<Adder>, Incrementer> getIncrementerImplComponent() {
  return fruit::createComponent().bind<Incrementer, IncrementerImpl>();
}
