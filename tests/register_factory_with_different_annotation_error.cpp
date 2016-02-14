// expect-compile-error NoBindingFoundError<fruit::Annotated<Annotation1,std::\(__1::\)\?function<std::\(__1::\)\?unique_ptr<ScalerImpl\(,std::\(__1::\)\?default_delete<ScalerImpl>\)\?>(double)>>>
// The __1:: prefix appears when using libc++

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
using fruit::Assisted;
using fruit::createComponent;

struct Annotation1 {};
struct Annotation2 {};

class Scaler {
public:
  virtual double scale(double x) = 0;
};

class ScalerImpl : public Scaler {
private:
  double factor;
  
public:
  ScalerImpl(double factor)
    : factor(factor) {
  }
  
  double scale(double x) override {
    return x * factor;
  }
};

using ScalerFactory = std::function<std::unique_ptr<Scaler>(double)>;
using ScalerFactoryAnnot = fruit::Annotated<Annotation1, ScalerFactory>;
using ScalerAnnot = fruit::Annotated<Annotation1, Scaler>;
using ScalerImplAnnot = fruit::Annotated<Annotation2, ScalerImpl>;
using WrongScalerImplAnnot = fruit::Annotated<Annotation1, ScalerImpl>;

Component<ScalerFactoryAnnot> getScalerComponent() {
  return createComponent()
    .bind<ScalerAnnot, WrongScalerImplAnnot>()
    .registerFactory<ScalerImplAnnot(Assisted<double>)>([](double factor) { return ScalerImpl(factor); });
}

int main() {
  Injector<ScalerFactoryAnnot> injector(getScalerComponent());
  ScalerFactory scalerFactory = injector.get<ScalerFactoryAnnot>();
  std::unique_ptr<Scaler> scaler = scalerFactory(12.1);
  std::cout << scaler->scale(3) << std::endl;
  
  return 0;
}
