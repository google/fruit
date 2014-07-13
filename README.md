
Fruit is a [dependency injection](http://en.wikipedia.org/wiki/Dependency_injection) framework for C++, loosely inspired by the Guice framework for Java. It uses C++ metaprogramming together with some new C++11 features to detect most injection problems at compile-time.
It allows to split the implementation code in "components" (aka modules) that can be assembled to form other components.
From a component with no requirements it's then possible to create an injector, that provides an instance of the interfaces exposed by the component.

See the [Fruit website](https://sites.google.com/site/fruitlib) for more information, including installation instructions, tutorials and reference documentation.
