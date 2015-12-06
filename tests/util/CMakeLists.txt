set(UTIL_TEST_SOURCES
lambda_invoker.cpp
type_info.cpp
)

add_fruit_tests(util ${UTIL_TEST_SOURCES})
check_all_tests_listed(util ${UTIL_TEST_SOURCES})
