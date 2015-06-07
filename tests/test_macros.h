
#ifndef FRUIT_TEST_MACROS
#define FRUIT_TEST_MACROS

#include <iostream>

#define Assert(e) do { if (!(e)) { std::cerr << __FILE__ << ":" << __LINE__ << ": " << __func__ << ": Assertion \"" << #e << "\" failed." << std::endl; abort(); } } while(false)


#endif // FRUIT_TEST_MACROS
