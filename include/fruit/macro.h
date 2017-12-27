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

#ifndef FRUIT_MACRO_H
#define FRUIT_MACRO_H

// This include is not required here, but having it here shortens the include trace in error messages.
#include <fruit/impl/injection_errors.h>

#include <fruit/fruit_forward_decls.h>

/**
 * A convenience macro to define the Inject typedef while declaring/defining the constructor that will be used for
 * injection.
 * It also supports assisted injection and injection of annotated types.
 *
 * Example usage:
 *
 * class MyClass {
 * public:
 *    INJECT(MyClass(Foo* foo, Bar* bar)) {...}
 * };
 *
 * is equivalent to:
 *
 * class MyClass {
 * public:
 *    using Inject = MyClass(Foo*, Bar*);
 *
 *    MyClass(Foo* foo, Bar* y) {...}
 * };
 *
 * Example usage for assisted injection (see PartialComponent::registerFactory):
 *
 * class MyClass {
 * public:
 *    INJECT(MyClass(Foo* foo, ASSISTED(int) n) {...}
 * };
 *
 * is equivalent to:
 *
 * class MyClass {
 * public:
 *    using Inject = MyClass(Foo*, Assisted<int>);
 *
 *    MyClass(Foo* foo, int n) {...}
 * };
 *
 * Example usage for annotated types:
 *
 * class MyClass {
 * public:
 *    INJECT(MyClass(ANNOTATED(SomeAnnotation, Foo*) foo, Bar* bar)) {...}
 * };
 *
 * ASSISTED and ANNOTATED *can* be used together in the same INJECT() annotation, but they can't both be used for a
 * single parameter (as this wouldn't make sense, parameters that use assisted injection are user-supplied, they aren't
 * injected from a binding).
 *
 * NOTE: This can't be used if the constructor is templated (the class can be templated, however), if there are any
 * default arguments or if the constructor is marked `explicit'.
 * In those cases, define the Inject annotation manually or use registerConstructor()/registerFactory() instead.
 *
 * NOTE: ASSISTED takes just one argument, but it's declared as variadic to make sure that the preprocessor doesn't
 * choke on multi-argument templates like the map above, that the processor is unable to parse correctly.
 *
 * NOTE: ASSISTED takes just 2 arguments, but it's declared as variadic to make sure that the preprocessor doesn't choke
 * on multi-argument templates, that the processor is unable to parse correctly.
 *
 * NOTE: In addition to the public Inject typedef, two typedefs (FruitAssistedTypedef and FruitAnnotatedTypedef) will be
 * defined inside the class, make sure you don't define another typedef/field/method with the same name if you use the
 * INJECT macro (unlikely but possible) these typedefs are an implementation detail of Fruit and should not be used.
 *
 * NOTE: The return type (MyClass in this case) should not be annotated. However an annotated
 * MyClass (or MyClass factory) can be injected from any INJECT declaration.
 */
#define INJECT(Signature)                                                                                              \
  using Inject = Signature;                                                                                            \
                                                                                                                       \
  template <typename FruitAssistedDeclarationParam>                                                                    \
  using FruitAssistedTypedef = FruitAssistedDeclarationParam;                                                          \
  template <typename Annotation, typename FruitAnnotatedDeclarationParam>                                              \
  using FruitAnnotatedTypedef = FruitAnnotatedDeclarationParam;                                                        \
                                                                                                                       \
  Signature

#define ASSISTED(...) FruitAssistedTypedef<__VA_ARGS__>
#define ANNOTATED(Annotation, ...) FruitAnnotatedTypedef<Annotation, __VA_ARGS__>

/**
 * These are intentionally NOT in the fruit namespace, they can't be there for technical reasons.
 *
 * NOTE: don't use these directly, they're only used to implement the INJECT macro.
 * Consider them part of fruit::impl.
 */
template <typename T>
using FruitAssistedTypedef = fruit::Assisted<T>;
template <typename Annotation, typename T>
using FruitAnnotatedTypedef = fruit::Annotated<Annotation, T>;

#endif // FRUIT_MACRO_H
