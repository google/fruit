#include <fruit/fruit.h>

struct X {
  INJECT(X()) = default;
};

fruit::Component<X> getXComponent() {
  return fruit::createComponent();
}

int main() {
  fruit::Injector<X> injector(getXComponent);
  injector.get<X*>();
}
