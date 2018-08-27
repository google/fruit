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

#ifndef FRUIT_KEY_VALUE_STORAGE_H
#define FRUIT_KEY_VALUE_STORAGE_H

#include <string>
#include <fruit/fruit.h>

class KeyValueStorage {
public:
  // Stores a value associated with the given key.
  virtual void put(std::string key, std::string value) = 0;

  // Returns the value previously associated with the given key, or an empty string otherwise.
  virtual std::string get(std::string key) = 0;
};

fruit::Component<KeyValueStorage> getKeyValueStorageComponent();

#endif // FRUIT_KEY_VALUE_STORAGE_H
