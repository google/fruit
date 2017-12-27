
#include <fruit/fruit.h>
#include <iostream>

using fruit::Component;
using fruit::Injector;

class Writer {
public:
  virtual void write(std::string s) = 0;
};

class StdoutWriter : public Writer {
public:
  // Like "StdoutWriter() = default;" but also marks this constructor as the
  // one to use for injection.
  INJECT(StdoutWriter()) = default;

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
  // Like "GreeterImpl(Writer* writer) {...}" but also marks this constructor
  // as the one to use for injection.
  INJECT(GreeterImpl(Writer* writer)) : writer(writer) {}

  virtual void greet() override {
    writer->write("Hello world!\n");
  }
};

Component<Greeter> getGreeterComponent() {
  return fruit::createComponent().bind<Writer, StdoutWriter>().bind<Greeter, GreeterImpl>();
}

int main() {

  Injector<Greeter> injector(getGreeterComponent);
  Greeter* greeter = injector.get<Greeter*>();

  greeter->greet();

  return 0;
}
