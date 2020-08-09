int f() {
  static int x = 1;
  if (x == 1) {
    return 0;
  } else {
    __assume(0);
    // Note: the lack of return here is intentional
  }
}

int main() {
  return f();
}
