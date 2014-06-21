
## What is Fruit

Fruit is a [dependency injection](http://en.wikipedia.org/wiki/Dependency_injection) framework for C++, loosely inspired by the Guice framework for Java. It uses C++ metaprogramming together with some new C++11 features to detect most injection problems at compile-time.

### Features

* Basic features:
  * Binding of a type to an interface
  * Inject annotations for constructors
  * Binding to a provider
  * Binding to an instance/value
  * Assisted injection
* Unlike most DI frameworks, most checks are done at **compile time**, so that errors can be caught early.
  Some examples of checks done at compile time:
  * Checking that all required types are bound (implicitly or explicitly)
  * Checking that there are no dependency loops in the bound types.
* The only injection error that can't be detected at compile time is when a type has multiple inconsistent bindings in different modules. This is checked at run-time.
* No code generation. Just include fruit/fruit.h and link with the fruit library.
* Not intrusive. You can bind interfaces and classes without modifying them (e.g. from a third party library that doesn't use Fruit).
* Reduces the need of #includes. The header file of a module only includes the interfaces exposed by the module.
  The implementation classes and the interfaces that the module doesn't expose (for example, private interfaces that the client code doesn't need
  to know about) don't need to be included. So after changing the binding of a type in a module (as long as the interfaces exposed by the module
  remain the same) only the module itself needs to be re-compiled. Yes, any modules that install that module **don't** need to. This makes
  compilation of large projects much faster than an include-all-the-classes-I-need-to-inject approach.
* Helps with binary compatibility: as a consequence of the previous point, since the client code doesn't include the implementation classes (not even the header files) if the interfaces exported by the module didn't change the compiled client code is binary compatible with the new implementation.
* No static data. This allows the creation of separate injectors in different parts of a system, which might bind the same type in different ways.
* Conditional injection based on runtime conditions. This allows to decide what to inject based on e.g. flags passed to the executable or an XML file loaded at runtime.
  * Note that you don't need special support in Fruit for the way that you use to decide what to inject.
    For example, if you'd like to determine the classes to inject based on the result of an RPC to a server sent using a proprietary RPC
    framework, you can do this and you don't need to modify Fruit.
* The combination of the previous two features means that at runtime you can decide to create a separate injector with a different configuration.
  E.g. think of a web server that receives a notification to reconfigure itself, creates an injector with the new configuration for new requests
  and then deletes the old injector when there are no more requests using it, never stopping serving requests.

#### Planned features

* Full thread-safety: by implementing eager injection support, multiple threads will be able to share a single injector, with no locking.
* Injection scopes: e.g. will allow to bind a type/value only for the duration of a request, while sharing the non-request-specific bindings across all worker threads (with no locking).
* Multi-bindings: unlike the typical binding when in an injector there's a single binding for each type, multi-bindings allow modules to specify several bindings and injected classes to access the collection of bound instances.
  This can be useful for plugin loading/hooks, or to register RPC services in a server.

#### Rejected features

* Compile-time detection of multiple inconsistent bindings. This feature has been rejected because it would interfere with some of the features above that are considered more important (conditional injection, binary compatibility, few includes).

Do you have a feature in mind that's not in the above list? Drop me an email ([poletti.marco@gmail.com](mailto:poletti.marco@gmail.com)), I'm interested to hear your idea and I'll implement it if feasible.

## Using Fruit

### Fruit hello world

```C++
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
  // Like "StdoutWriter() = default;" but also marks this constructor as the
  // one to use for injection.
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
  // Like "GreeterImpl(Writer* writer) {...}" but also marks this constructor
  // as the one to use for injection.
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
```

More documentation is coming soon, stay tuned.

### Example code

For examples on how code looks like when using Fruit, look at the [examples/](https://github.com/google/fruit/tree/master/examples) directory in the source tree.

### Dependencies

There are no library dependencies, but this project uses many C++11 features so you need to use a relatively recent compiler.
The supported compilers are GCC 4.8.1 or later (released in May 2013) and Clang 3.1.0 or later (released in May 2012).
For building Fruit, you'll also need to have cmake and make installed.

### How to build and install

To configure and build:

    cmake . && make -j

To install (under Linux uses /usr/local):

    sudo make install

To configure for installation in a specific directory, e.g. /usr:

    cmake -DCMAKE_INSTALL_PREFIX=/usr . && make -j

The above instructions are the simplest to get started, but out-of-source builds are also supported.

### License

The code is released unser the Apache 2.0 license. See the [COPYING](https://github.com/google/fruit/blob/master/COPYING) file for more details.

This project is not an official Google project. It is not supported by Google and Google specifically disclaims all warranties as to its quality, merchantability, or fitness for a particular purpose.

### Contact information

Currently I (Marco Poletti) am the only developer of this project.
You can contact me by email: [poletti.marco@gmail.com](mailto:poletti.marco@gmail.com).

## FAQ

### How mature is this project?

This project was started as a weekend side-project in May 2014.
It's currently in alpha/beta stage. While the codebase is small and has extensive tests, some features are missing (see the "Planned features" section above) and some bugs might still remain. If you're considering using Fruit you should probably wait for 1-2 months for it to stabilize. If you can't wait, send me an email and we can discuss it.

If you try it out and find a bug or surprising behavior, let me know.

### Why the name "Fruit"?

Fruit is inspired by Guice (pronounced as "juice"), which uses run-time checks.

*If you'd like some Guice but don't want to wait (for run-time), have some Fruit.*

It also hints to the fact that Fruit modules have clearly-defined boundaries while Guice modules don't (all Guice modules have the same type) and to the fact that both Fruit and Guice are 100% natural (no code generation).

### Does Fruit use Run-Time Type Identification (RTTI)?

Yes and no. In some sense, Fruit uses compile-time RTTI only.
For an injected type T, Fruit calls typeid(T) at compile time (using constexpr to ensure that it's evaluated at compile time).
However, Fruit never calls typeid on an object, and never calls it at all at runtime.
This means that RTTI has to be enabled for the build, but you won't have any performance degradation due to RTTI, since it's only used at compile time.

### Fruit uses templates heavily. Will this have an impact on the executable size?

Fruit uses templates heavily for metaprogramming, but the storage that backs injectors and modules is *not* templated.
All templated methods are just wrappers and will most likely be inlined by the compiler.
So you should not expect an increase in the executable size due to the use of templates.
See also the next question.

### Does Fruit have an impact on the executable size?

If before using Fruit you were already using virtual classes, the impact on the executable size will be negligible.
Otherwise, there will be a (likely small, but noticeable) increase due to the RTTI information for classes with virtual methods.

### Does Fruit have an impact on performance?

If before using Fruit you were already using virtual classes, the overhead of injection will be the cost of looking up a pointer in a hash table, once per injected object.
Once the desired objects have been created from the injector, there will be no further overhead.
I haven't made any benchmarks yet, but I expect this to be fairly small.

Otherwise, there will be the additional cost due to the memory allocations for implementation classes (that before were probably allocated on the stack).
In this case, the speedup gained by inlining the implementation into the caller will also be lost.

Avoid creating injectors inside a tight loop where performance is critical.
