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

#define IN_FRUIT_CPP_FILE

#include <fruit/impl/data_structures/fixed_size_allocator.h>
#include <fruit/impl/data_structures/fixed_size_vector.templates.h>
#include "../test_macros.h"

#include <vector>

using namespace std;
using namespace fruit::impl;

struct X {
  int y;
  
  static int num_instances;
  
  X(int y) : y(y) {
    Assert(std::uintptr_t(this) % alignof(X) == 0);
    ++num_instances;
  }
  
  ~X() {
    --num_instances;
  }
};

int X::num_instances = 0;

struct Y {
  static int num_instances;
  
  Y() {
    Assert(std::uintptr_t(this) % alignof(Y) == 0);
    ++num_instances;
  }
  
  ~Y() {
    --num_instances;
  }
};

int Y::num_instances = 0;

template <int n>
struct alignas(n) TypeWithAlignment {
  TypeWithAlignment() {
    Assert(std::uintptr_t(this) % n == 0);
  }
};

void test_empty_allocator() {
  FixedSizeAllocator allocator;
}

void test_2_types() {
  {
    FixedSizeAllocator::FixedSizeAllocatorData allocator_data;
    allocator_data.addType(getTypeId<X>());
    allocator_data.addType(getTypeId<Y>());
    FixedSizeAllocator allocator(allocator_data);
    allocator.constructObject<X>(15);
    allocator.constructObject<Y>();
    Assert(X::num_instances == 1);
    Assert(Y::num_instances == 1);
  }
  Assert(X::num_instances == 0);
  Assert(Y::num_instances == 0);
}

void test_externally_allocated_only() {
  {
    FixedSizeAllocator::FixedSizeAllocatorData allocator_data;
    allocator_data.addExternallyAllocatedType(getTypeId<X>());
    FixedSizeAllocator allocator(allocator_data);
    allocator.registerExternallyAllocatedObject(new X(15));
    // The allocator takes ownership.  Valgrind will report an error if  X is not deleted.
    Assert(X::num_instances == 1);
  }
  Assert(X::num_instances == 0);
}

void test_mix() {
  {
    FixedSizeAllocator::FixedSizeAllocatorData allocator_data;
    allocator_data.addExternallyAllocatedType(getTypeId<X>());
    allocator_data.addType(getTypeId<Y>());
    FixedSizeAllocator allocator(allocator_data);
    allocator.registerExternallyAllocatedObject(new X(15));
    // The allocator takes ownership.  Valgrind will report an error if  X is not deleted.
    allocator.constructObject<Y>();
    Assert(X::num_instances == 1);
    Assert(Y::num_instances == 1);
  }
  Assert(X::num_instances == 0);
  Assert(Y::num_instances == 0);
}

void test_remove_type() {
  {
    FixedSizeAllocator::FixedSizeAllocatorData allocator_data;
    allocator_data.addType(getTypeId<X>());
    allocator_data.addType(getTypeId<Y>());
    allocator_data.removeType(getTypeId<X>());
    FixedSizeAllocator allocator(allocator_data);
    allocator.constructObject<Y>();
    Assert(Y::num_instances == 1);
  }
  Assert(Y::num_instances == 0);
}

void test_alignment() {
  FixedSizeAllocator::FixedSizeAllocatorData allocator_data;
  allocator_data.addType(getTypeId<TypeWithAlignment<1>>());
  allocator_data.addType(getTypeId<TypeWithAlignment<8>>());
  allocator_data.addType(getTypeId<TypeWithAlignment<2>>());
  allocator_data.addType(getTypeId<TypeWithAlignment<128>>());
  allocator_data.addType(getTypeId<TypeWithAlignment<2>>());
  allocator_data.addType(getTypeId<TypeWithAlignment<8>>());
  allocator_data.addType(getTypeId<TypeWithAlignment<1>>());
  FixedSizeAllocator allocator(allocator_data);
  // TypeWithLargeAlignment::TypeWithLargeAlignment() will assert that the alignment is correct.
  allocator.constructObject<TypeWithAlignment<2>>();
  allocator.constructObject<TypeWithAlignment<8>>();
  allocator.constructObject<TypeWithAlignment<1>>();
  allocator.constructObject<TypeWithAlignment<128>>();
  allocator.constructObject<TypeWithAlignment<1>>();
  allocator.constructObject<TypeWithAlignment<8>>();
  allocator.constructObject<TypeWithAlignment<2>>();
}

void test_move_constructor() {
  {
    FixedSizeAllocator::FixedSizeAllocatorData allocator_data;
    allocator_data.addType(getTypeId<X>());
    allocator_data.addType(getTypeId<Y>());
    FixedSizeAllocator allocator(allocator_data);
    allocator.constructObject<X>(15);
    FixedSizeAllocator allocator2(std::move(allocator));
    allocator2.constructObject<Y>();
    Assert(X::num_instances == 1);
    Assert(Y::num_instances == 1);
  }
  Assert(X::num_instances == 0);
  Assert(Y::num_instances == 0);
}

int main() {
  test_empty_allocator();
  test_2_types();
  test_externally_allocated_only();
  test_mix();
  test_remove_type();
  test_alignment();
  test_move_constructor();
  
  return 0;
}
