
# Features tested in end-to-end tests

#### INJECT macro
* **TODO** Typical use-case
* **TODO** With assisted params
* **TODO** Check what happens with non-normalized types (all kinds)

#### Binding to an instance
* Using `bind(x)` or `bind<fruit::Annotated<A, T>>(x)`.
* Check that calling bindInstance with a non-normalized type (e.g. const pointer, nonconst ptr, etc.) causes an error
* Abstract class (ok)
* Mismatched type arguments
* Bind to subclass

#### Interface bindings
* Check that bind<T, T> causes an easy-to-understand error
* bind<T, Annotated<A, T>>
* Check that bind<X, Y>, bind<Y, Z> is allowed if Z derives from Y and Y derives from X
* bind<X, Y> with X not a base class of Y
* Check that the types passed to bind<> are normalized
* Check that bind<I, C> also means bind<std::function<std::unique_ptr<I>(Args...)>, std::function<std::unique_ptr<C>(Args...)>> (with and without Args)  

##### Binding to a constructor
* Explicitly, with a non-signature (not ok)
* Implicitly, with a non-signature (not ok)
* Implicitly, with a signature "returning" another type (not ok)
* Implicitly, with a signature "returning" an annotated type (not ok)
* Explicitly, with a signature that doesn't match any of the type's constructors
* Implicitly, with a signature that doesn't match any of the type's constructors
* **TODO** Using the Inject typedef
* **TODO** Using the INJECT macro
* **TODO** Also with no params
* **TODO** Also for a templated class
* **TODO** Also for a templated constructor (only explicitly or using Inject)
* **TODO** With all kinds of non-normalized params (esp. with INJECT)
* **TODO** With a constructor mistakenly taking an Assisted<X> or Annotated<A,X> parameter (instead of just using Assisted/Annotated in the Inject typedef)
* For an abstract type (not ok), both implicit and explicit
* **TODO** Check that a default-constructible type without an Inject typedef can't be auto-injected

##### Binding to a provider
* Returning a value
* **TODO: ownership check** Returning a pointer (also check that Fruit takes ownership)
* Check that lambdas with captures are forbidden
* **TODO** Check that non-lambda functors/functions are forbidden
* **TODO** Check that objects without operator() are forbidden
* Passing a non-signature type
* **TODO** Passing a signature type incompatible with the lambda's signature
* **TODO** With a lambda mistakenly taking an Assisted<X> or Annotated<A,X> parameter (instead of just using Assisted/Annotated in the Inject typedef)
* **TODO** For an abstract type (ok)
* With a provider that returns nullptr (runtime error)

#### Factory bindings
* Explicit, using `registerFactory()`
* Implicitly, with a signature "returning" an annotated type (not ok)
* **TODO** Explicit, using `registerFactory()`, but passing a non-signature
* Explicit, using `registerFactory()`, but with a lambda that has a different signature compared to the one given explicitly
* Implicitly, with a signature that doesn't match any of the type's constructors
* Check that lambdas with captures are forbidden in `registerFactory()`
* **TODO** Check that non-lambda functors/functions are forbidden in `registerFactory()`
* **TODO** Check that objects without operator() are forbidden in `registerFactory()`
* Using the INJECT macro
* With some assisted params and some injected params
* **TODO** With no assisted params but some injected params
* With some assisted params but no injected params
* **TODO** With no assisted params and no injected params
* **TODO** Using the factory in another class' constructor instead of getting it from the injector directly
* **TODO** With a lambda mistakenly taking a Assisted<X>/Annotated<A,X> parameter (instead of just using Assisted/Annotated in the Inject typedef)
* Explicit, for an abstract type (ok)
* Implicit, for an abstract class (not ok)
* Explicit, with a lambda returning a pointer (not supported)
* Explicit, with a lambda returning a unique ptr (ok)
* **TODO** With assisted params of all kinds of non-normalized types (especially in ASSISTED)
* Implicitly, registering a `std::function<T(...)>` instead of a `std::function<std::unique_ptr<T>(...)>`  
* Explicitly, registering a `std::function<T(...)>` instead of a `std::function<std::unique_ptr<T>(...)>`
* Implicitly, generating a binding for std::function<T()> when there is a binding for T
* Implicitly, generating a binding for std::function<std::unique_ptr<T>()> when there is a binding for T
* **TODO** Check that assisted params are passed in the right order when there are multiple
* **TODO** Try calling the factory multiple times
* Injecting a std::function<std::unique_ptr<T>(...)> with T not movable

#### Annotated bindings
* **TODO** Using `fruit::Annotated<>`
* **TODO** Using the ANNOTATED macro (only in constructors using INJECT)
* **TODO** Check possibly-misleading behavior of binding Annotated<A1, I> and Annotated<A2, I> to C (only 1 C instance is created and shared)  
* **TODO** With assisted params of all kinds of non-normalized types (especially in ANNOTATED)

#### Multibindings
* Interface multibindings
* **TODO** Check that addMultibinding<I, I> causes an easy-to-understand error
* Instance multibindings
* **TODO** Check that calling addInstanceMultibinding with a non-normalized type (e.g. const pointer, nonconst ptr, etc.) causes an error
* **TODO** `addInstanceMultibindings(x)`, `addInstanceMultibindings<T>(x)` and `addInstanceMultibindings<Annotated<A, T>>(x)`
* **TODO** `addInstanceMultibindings()` with an empty vector
* **TODO** Check that calling `addInstanceMultibindings()` with a non-normalized type causes an error
* `addMultibindingProvider`:
  * Returning a value
  * **TODO: ownership check** Returning a pointer (also check that Fruit takes ownership)
  * Check that lambdas with captures are forbidden
  * **TODO** Check that non-lambda functors/functions are forbidden
  * **TODO** Check that objects without operator() are forbidden
  * Passing a non-signature type
  * **TODO** Passing a signature type incompatible with the lambda's signature
  * **TODO** With a lambda mistakenly taking an Assisted<X> or Annotated<A,X> parameter (instead of just using Assisted/Annotated in the Inject typedef)
  * For an abstract type (not ok)
  * With a provider that returns nullptr (runtime error)

#### PartialComponent and Component
* copy a Component
* move a Component
* move a PartialComponent
* construction of a Component from another Component
* construction of a Component from a PartialComponent
* install() (old and new style)
* Type already bound (various combinations, incl. binding+install)
* No binding found for abstract class
* Dependency loops
* Run-time error for multiple inconsistent bindings in different components
* Class-level static_asserts in Component
  * Check that there are no repeated types
  * Check that no type is both in Required<> and outside
  * Check that all types are normalized
  * Check that Required only appears once
  * Check that Required only appears as first parameter (if at all)

#### Normalized components
* Constructing an injector from NC + C
* **TODO** Constructing an injector from NC + C with empty NC or empty C
* With requirements
* Class-level static_asserts
  * Check that there are no repeated types
  * Check that no type is both in Required<> and outside
  * **TODO** Check that all types are normalized
  * Check that Required only appears once
  * Check that Required only appears as first parameter (if at all)

#### Components with requirements
* Usual case (where the required type is only forward-declared, with no definition available)
* Usual case (where the required type is defined but it's an abstract class)
* **TODO** Check that requirements aren't allowed in injectors
* Check that multiple Required<...> params are not allowed
* Check that the Required<...> param is only allowed if it's the 1st 
* **TODO** Check that an empty Required<...> param is allowed 

#### Injectors
* **TODO** `std::move()`-ing an injector
* Getting instances from an Injector:
  * **TODO** Using `get<T>` (for all type variations)
  * **TODO** Using `get()` or casting to try to get a value that the injector doesn't provide
  * **TODO** Casting the injector to the desired type
* Getting multibindings from an Injector
  * for a type that has no multibindings
  * for a type that has 1 multibinding
  * for a type that has >1 multibindings
* **TODO** Eager injection
* **TODO** Check that the component (in the constructor from C) has no requirements
* **TODO** Check that the resulting component (in the constructor from C+NC) has no requirements
* **TODO: partial** Empty injector (construct, get multibindings, eager injection, etc.)
* **TODO** Injector with a single instance type bound and nothing else
* **TODO** Injector with a single bindProvider and nothing else
* **TODO** Injector with a single multibinding and nothing else
* **TODO** Injector with a single factory and nothing else
* Injector<T> where the C doesn't provide T
* Injector<T> where the C+NC don't provide T
* Class-level static_asserts
  * Check that there are no repeated types
  * Check that all types are normalized
  * Check that there are no Required types

#### Injecting Provider<>s
* **TODO** In constructors
* Getting a Provider<> from an injector using get<> or casting the injector)
* **TODO** Getting a Provider<> from an injector by casting the injector
* In a constructor and calling get() before the constructor completes
* **TODO** casting a Provider to the desired value instead of calling `get()`
* **TODO** Calling either `get<T>()` or `get()` on the Provider
* **TODO** Check that a Provider's type argument is normalized and not annotated
* Copying a Provider and using the copy
* Using `get()` to try to get a value that the provider doesn't provide
* Class-level static_asserts
  * Check that the type is normalized
  * Check that the type is not annotated
