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

#include "key_value_storage.h"

class KeyValueStorageImpl : public KeyValueStorage {
public:
  INJECT(KeyValueStorageImpl()) = default;

  void put(std::string key, std::string value) override {
    // Imagine the real implementation here, with network communication.
    (void) key;
    (void) value;
    throw "Not implemented";
  }

  std::string get(std::string key) override {
    // Imagine the real implementation here, with network communication.
    (void) key;
    throw "Not implemented";
  }
};

fruit::Component<KeyValueStorage> getKeyValueStorageComponent() {
  return fruit::createComponent()
      .bind<KeyValueStorage, KeyValueStorageImpl>();
}
