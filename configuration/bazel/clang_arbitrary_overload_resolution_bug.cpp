template <typename T, typename U>
struct Pair {};

struct Map : public Pair<int, float>, Pair<int, char> {};

template <typename Value>
Value f(Pair<int, Value>*) { return Value(); }

int main() {
  f((Map*)0);
}
