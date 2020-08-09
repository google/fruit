int main() {
  bool b = __has_trivial_copy(int);
  (void) b;
  return 0;
}
