from conans import ConanFile, CMake, tools


class FruitConan(ConanFile):
    name = "fruit"
    version = "3.2.0"
    license = "Apache"
    url = "https://github.com/google/fruit"
    description = "C++ dependency injection framework"
    settings = "os", "compiler", "build_type", "arch"
    options = {"shared": [True, False]}
    default_options = "shared=False"
    generators = "cmake"

    def source(self):
        self.run("git clone https://github.com/google/fruit")
        self.run("cd fruit && git checkout v3.2.0")

    def build(self):
        cmake = CMake(self)
        if self.options.shared:
            cmake.definitions["BUILD_SHARED_LIBS"] = "YES"
        else:
            cmake.definitions["BUILD_SHARED_LIBS"] = "NO"
        cmake.definitions["FRUIT_USES_BOOST"] = "NO"
        cmake.definitions["RUN_TESTS_UNDER_VALGRIND"] = "NO"
        cmake.definitions["FRUIT_TESTS_USE_PRECOMPILED_HEADERS"] = "NO"
        cmake.definitions["FRUIT_ENABLE_COVERAGE"] = "NO"
        cmake.configure(source_folder="fruit")
        cmake.build()
        cmake.install()

    def package(self):
        pass

    def package_info(self):
        self.cpp_info.libs = ["fruit"]
