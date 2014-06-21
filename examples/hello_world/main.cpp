
#include "fruit/fruit.h"
#include <iostream>

using fruit::Module;
using fruit::Injector;

class Writer {
public:
  virtual void write(std::string s) = 0;
};

class StdoutWriter : public Writer {
public:
  INJECT(StdoutWriter()) = default;
  
  virtual ~StdoutWriter() = default;
  
  virtual void write(std::string s) override {
    std::cout << s;
  }
};

class Greeter {
public:
  virtual void greet() = 0;
};

class GreeterImpl : public Greeter {
private:
  Writer* writer;

public:
  INJECT(GreeterImpl(Writer* writer))
    : writer(writer) {
  }
  
  virtual ~GreeterImpl() = default;
  
  virtual void greet() override {
    writer->write("Hello world!\n");
  }
};

Module<Greeter> getGreeterModule() {
  return fruit::createModule()
    .bind<Writer, StdoutWriter>()
    .bind<Greeter, GreeterImpl>();
}

int main() {

  Injector<Greeter> injector(getGreeterModule());
  Greeter* greeter(injector);
  
  greeter->greet();
  
  return 0;
}
