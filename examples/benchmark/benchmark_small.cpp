#include <fruit/fruit.h>
struct Y1 { INJECT(Y1()) {} };
struct X1 { INJECT(X1(Y1)) {} };
struct Y2 { INJECT(Y2()) {} };
struct X2 { INJECT(X2(Y2)) {} };
struct Y3 { INJECT(Y3()) {} };
struct X3 { INJECT(X3(Y3)) {} };
struct Y4 { INJECT(Y4()) {} };
struct X4 { INJECT(X4(Y4)) {} };
struct Y5 { INJECT(Y5()) {} };
struct X5 { INJECT(X5(Y5)) {} };
struct Y6 { INJECT(Y6()) {} };
struct X6 { INJECT(X6(Y6)) {} };
struct Y7 { INJECT(Y7()) {} };
struct X7 { INJECT(X7(Y7)) {} };
struct Y8 { INJECT(Y8()) {} };
struct X8 { INJECT(X8(Y8)) {} };
struct Y9 { INJECT(Y9()) {} };
struct X9 { INJECT(X9(Y9)) {} };
struct Y10 { INJECT(Y10()) {} };
struct X10 { INJECT(X10(Y10)) {} };
fruit::Component<
X1,
X2,
X3,
X4,
X5,
X6,
X7,
X8,
X9,
X10
> getComponent() { return fruit::createComponent(); }
int main() {
for (int i = 0; i < 1000; i++) {
fruit::Injector<
X1,
X2,
X3,
X4,
X5,
X6,
X7,
X8,
X9,
X10
> injector(getComponent());
injector.get<X1*>();
injector.get<X2*>();
injector.get<X3*>();
injector.get<X4*>();
injector.get<X5*>();
injector.get<X6*>();
injector.get<X7*>();
injector.get<X8*>();
injector.get<X9*>();
injector.get<X10*>();
}
return 0;
}
