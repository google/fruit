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

#if MULTIPLIER == 1
#define REPEAT(X) REPEAT_1(X, _)

#elif MULTIPLIER == 2
#define REPEAT(X) REPEAT_2(X, _)

#elif MULTIPLIER == 4
#define REPEAT(X) REPEAT_4(X, _)

#elif MULTIPLIER == 8
#define REPEAT(X) REPEAT_8(X, _)

#elif MULTIPLIER == 16
#define REPEAT(X) REPEAT_16(X, _)

#elif MULTIPLIER == 32
#define REPEAT(X) REPEAT_32(X, _)

#elif MULTIPLIER == 64
#define REPEAT(X) REPEAT_64(X, _)

#elif MULTIPLIER == 128
#define REPEAT(X) REPEAT_128(X, _)

#elif MULTIPLIER == 256
#define REPEAT(X) REPEAT_256(X, _)

#elif MULTIPLIER == 512
#define REPEAT(X) REPEAT_512(X, _)

#elif MULTIPLIER == 1024
#define REPEAT(X) REPEAT_1024(X, _)

#else
#error Multiplier not supported.
#endif

#define PLACEHOLDER

#define EVAL0(...) __VA_ARGS__
#define EVAL1(...) EVAL0(EVAL0(EVAL0(EVAL0(__VA_ARGS__))))
#define EVAL2(...) EVAL1(EVAL1(EVAL1(EVAL1(__VA_ARGS__))))
#define EVAL(...) EVAL2(EVAL2(EVAL2(EVAL2(__VA_ARGS__))))

#define META_REPEAT_2(R, X, I) R PLACEHOLDER(X, I##0) R PLACEHOLDER(X, I##1)

#define REPEAT_1(X, I) X(I)

#define REPEAT_2(X, I) META_REPEAT_2(REPEAT_1, X, I)

#define REPEAT_4(X, I) META_REPEAT_2(REPEAT_2, X, I)

#define REPEAT_8(X, I) META_REPEAT_2(REPEAT_4, X, I)

#define REPEAT_16(X, I) META_REPEAT_2(REPEAT_8, X, I)

#define REPEAT_32(X, I) META_REPEAT_2(REPEAT_16, X, I)

#define REPEAT_64(X, I) META_REPEAT_2(REPEAT_32, X, I)

#define REPEAT_128(X, I) META_REPEAT_2(REPEAT_64, X, I)

#define REPEAT_256(X, I) META_REPEAT_2(REPEAT_128, X, I)

#define REPEAT_512(X, I) META_REPEAT_2(REPEAT_256, X, I)

#define REPEAT_1024(X, I) META_REPEAT_2(REPEAT_512, X, I)

using namespace fruit;

#define DEFINITIONS(N)                                                                                                 \
  struct A##N {                                                                                                        \
    INJECT(A##N()) = default;                                                                                          \
  };                                                                                                                   \
                                                                                                                       \
  struct B##N {};                                                                                                      \
                                                                                                                       \
  struct C##N {};                                                                                                      \
                                                                                                                       \
  struct I##N {                                                                                                        \
    virtual void f() = 0;                                                                                              \
  };                                                                                                                   \
                                                                                                                       \
  struct X##N : public I##N {                                                                                          \
    INJECT(X##N(A##N, B##N*, const C##N&)){};                                                                          \
                                                                                                                       \
    virtual void f();                                                                                                  \
  };                                                                                                                   \
                                                                                                                       \
  struct Y##N {};                                                                                                      \
                                                                                                                       \
  struct Z##N {};                                                                                                      \
                                                                                                                       \
  Component<Required<Y##N>, Z##N> getZ##N##Component();

#define REQUIREMENTS(N) C##N,

#define PARAMETERS(N) B##N &b##N,

#if USE_FRUIT_2_X_SYNTAX
#define BINDINGS(N)                                                                                                    \
  .bind<I##N, X##N>().bindInstance(b##N).install(getZ##N##Component()).registerProvider([]() { return Y##N(); })
#else

#define BINDINGS(N)                                                                                                    \
  .bind<I##N, X##N>().bindInstance(b##N).install(getZ##N##Component).registerProvider([]() { return Y##N(); })
#endif

EVAL(REPEAT(DEFINITIONS))

Component<Required<EVAL(REPEAT(REQUIREMENTS)) int>> getComponent(EVAL(REPEAT(PARAMETERS)) int) {
  return createComponent() EVAL(REPEAT(BINDINGS));
}
