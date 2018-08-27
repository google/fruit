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

#include "cached_greeter.h"
#include "key_value_storage.h"

class CachedGreeterImpl : public Greeter {
private:
  Greeter* greeter;
  KeyValueStorage* keyValueStorage;

public:
  INJECT(CachedGreeterImpl(Greeter* greeter, KeyValueStorage* keyValueStorage))
    : greeter(greeter), keyValueStorage(keyValueStorage) {
  }

  std::string greet() override {
    std::string greeting = keyValueStorage->get("greeting");
    if (!greeting.empty()) {
      return greeting;
    }

    // Not in the cache, we need to compute the greeting.
    greeting = greeter->greet();

    // We also add it in the cache so that later calls don't need to call greeter->greet().
    keyValueStorage->put("greeting", greeting);

    return greeting;
  }
};

fruit::Component<fruit::Annotated<Cached, Greeter>> getCachedGreeterComponent() {
  return fruit::createComponent()
      .bind<fruit::Annotated<Cached, Greeter>, CachedGreeterImpl>()
      .install(getKeyValueStorageComponent)
      .install(getGreeterComponent);
}
