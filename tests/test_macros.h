
#ifndef FRUIT_TEST_MACROS
#define FRUIT_TEST_MACROS

#include <iostream>

#define Assert(...) do { if (!(__VA_ARGS__)) { std::cerr << __FILE__ << ":" << __LINE__ << ": " << __func__ << ": Assertion \"" << #__VA_ARGS__ << "\" failed." << std::endl; abort(); } } while(false)


#endif // FRUIT_TEST_MACROS
