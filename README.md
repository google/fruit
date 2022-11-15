
[![Test on Linux](https://github.com/google/fruit/actions/workflows/test-on-linux.yml/badge.svg?branch=master)](https://github.com/google/fruit/actions/workflows/test-on-linux.yml)
[![Test on Windows](https://github.com/google/fruit/actions/workflows/test-on-windows.yml/badge.svg?branch=master)](https://github.com/google/fruit/actions/workflows/test-on-windows.yml)
[![Test on OS X](https://github.com/google/fruit/actions/workflows/test-on-osx.yml/badge.svg?branch=master)](https://github.com/google/fruit/actions/workflows/test-on-osx.yml)
[![Coverity Scan Status](https://img.shields.io/coverity/scan/8486.svg?label=Coverity%20scan)](https://scan.coverity.com/projects/google-fruit)
[![CII Best Practices](https://bestpractices.coreinfrastructure.org/projects/1040/badge)](https://bestpractices.coreinfrastructure.org/projects/1040)

Fruit is a [dependency injection](http://en.wikipedia.org/wiki/Dependency_injection) framework for C++, loosely inspired by the Guice framework for Java. It uses C++ metaprogramming together with some C++11 features to detect most injection problems at compile-time.
It allows to split the implementation code in "components" (aka modules) that can be assembled to form other components.
From a component with no requirements it's then possible to create an injector, that provides an instance of the interfaces exposed by the component.

See the [wiki](https://github.com/google/fruit/wiki) for more information, including installation instructions, tutorials and reference documentation.
