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

#include <ctime>
#include <vector>
#include <iostream>
#include <iomanip>

#define MULTIPLIER 1000

#if MULTIPLIER == 1
#define REPEAT(X) REPEAT_1(X, _)

#elif MULTIPLIER == 10
#define REPEAT(X) REPEAT_10(X, _)

#elif MULTIPLIER == 100
#define REPEAT(X) REPEAT_100(X, _)

#elif MULTIPLIER == 1000
#define REPEAT(X) REPEAT_1000(X, _)

#else
#error Multiplier not supported.
#endif

#define PLACEHOLDER

#define EVAL0(...) __VA_ARGS__
#define EVAL1(...) EVAL0(EVAL0(EVAL0(EVAL0(__VA_ARGS__))))
#define EVAL2(...) EVAL1(EVAL1(EVAL1(EVAL1(__VA_ARGS__))))
#define EVAL(...) EVAL2(EVAL2(EVAL2(EVAL2(__VA_ARGS__))))

#define META_REPEAT_10(R, X, I) \
R PLACEHOLDER(X, I##0) \
R PLACEHOLDER(X, I##1) \
R PLACEHOLDER(X, I##2) \
R PLACEHOLDER(X, I##3) \
R PLACEHOLDER(X, I##4) \
R PLACEHOLDER(X, I##5) \
R PLACEHOLDER(X, I##6) \
R PLACEHOLDER(X, I##7) \
R PLACEHOLDER(X, I##8) \
R PLACEHOLDER(X, I##9)

#define REPEAT_1(X, I) \
X(I)

#define REPEAT_10(X, I) \
META_REPEAT_10(REPEAT_1, X, I)

#define REPEAT_100(X, I) \
META_REPEAT_10(REPEAT_10, X, I)

#define REPEAT_1000(X, I) \
META_REPEAT_10(REPEAT_100, X, I)

using namespace std;

#define DEFINITIONS(N)       \
struct I##N {                \
  virtual ~I##N() = default; \
};                           \
                             \
struct C##N : public I##N {  \
  virtual ~C##N() = default; \
};

#define ALLOCATE(N)          \
C##N* c##N = new C##N();

#define DEALLOCATE(N)        \
delete c##N;

EVAL(REPEAT(DEFINITIONS))

int main() {
  size_t num_loops;
  cin >> num_loops;
  
  clock_t start_time;
  start_time = clock();
  
  for (size_t i = 0; i < num_loops; i++) {
    EVAL(REPEAT(ALLOCATE))
    EVAL(REPEAT(DEALLOCATE))
  }
  size_t totalTime = clock() - start_time;
  
  std::cout << std::fixed;
  std::cout << std::setprecision(2);
  std::cout << "Total           = " << totalTime * 1.0 / num_loops << std::endl;
  
  return 0;
}
