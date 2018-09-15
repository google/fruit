from conans import ConanFile, CMake, tools
from conans.errors import ConanException


class FruitConan(ConanFile):
    name = "fruit"
    version = "3.4.0"
    license = "Apache"
    url = "https://github.com/google/fruit"
    description = "C++ dependency injection framework"
    settings = "os", "compiler", "build_type", "arch"
    options = {"shared": [True, False], "use_boost": [True, False]}
    default_options = "shared=False", "use_boost=True"
    generators = "cmake"

    def configure(self):
        min_version = {
            "gcc": "5",
            "clang": "3.5",
            "apple-clang": "7.3",
            "Visual Studio": "14", # MSVC 2015
        }.get(str(self.settings.compiler))
        if not min_version:
            # Unknown minimum version, let's try going ahead with the build to see if it works.
            return
        if str(self.settings.compiler.version) < min_version:
            raise ConanException("%s %s is not supported, must be at least %s" % (self.settings.compiler,
                                                                                  self.settings.compiler.version,
                                                                                  min_version))

    def requirements(self):
        if self.options.use_boost:
            self.requires("boost/1.68.0@conan/stable")

    def source(self):
        self.run("git clone https://github.com/google/fruit")
        self.run("cd fruit && git checkout v%s" % self.version)
        # This small hack might be useful to guarantee proper /MT /MD linkage
        # in MSVC if the packaged project doesn't have variables to set it
        # properly
        tools.replace_in_file("fruit/CMakeLists.txt", "project(Fruit)",
                              '''PROJECT(Myfruit)
include(${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
conan_basic_setup()''')

    def build(self):
        cmake = CMake(self)
        cmake.definitions["FRUIT_IS_BEING_BUILT_BY_CONAN"] = "YES"
        cmake.definitions["BUILD_SHARED_LIBS"] = "YES" if self.options.shared else "NO"
        if self.options.use_boost:
            if self.settings.os == "Windows":
                cmake.definitions["BOOST_DIR"] = "."
        else:
            cmake.definitions["FRUIT_USES_BOOST"] = "NO"
        if self.settings.os == "Windows":
            cmake.definitions["FRUIT_TESTS_USE_PRECOMPILED_HEADERS"] = "NO"
        cmake.definitions["CMAKE_BUILD_TYPE"] = self.settings.build_type
        cmake.configure(source_folder="fruit")
        cmake.build()
        cmake.install()

    def package(self):
        self.copy("*.h", dst="include", src="include")
        self.copy("*.h", dst="include", src="fruit/include")
        self.copy("*fruit.lib", dst="lib", keep_path=False)
        self.copy("*.dll", dst="bin", keep_path=False)
        self.copy("*.so", dst="lib", keep_path=False)
        self.copy("*.dylib", dst="lib", keep_path=False)
        self.copy("*.a", dst="lib", keep_path=False)
        tools.save("LICENSE", tools.load("fruit/COPYING"))
        self.copy("COPYING", dst="licenses", ignore_case=True, keep_path=False)

    def package_info(self):
        self.cpp_info.libs = ["fruit"]
