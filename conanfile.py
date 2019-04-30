from conans import ConanFile, CMake, tools
from conans.errors import ConanException
import os


class FruitConan(ConanFile):
    name = "fruit"
    version = "3.4.0"
    license = "Apache"
    url = "https://github.com/google/fruit"
    homepage = "https://github.com/google/fruit"
    description = "C++ dependency injection framework"
    settings = "os", "compiler", "build_type", "arch"
    options = {"shared": [True, False], "use_boost": [True, False]}
    default_options = {"shared": False, "use_boost": True}
    generators = "cmake"
    exports = "COPYING"
    _source_subfolder = "source_subfolder"

    def configure(self):
        min_version = {
            "gcc": 5,
            "clang": 3.5,
            "apple-clang": 7.3,
            "Visual Studio": 14, # MSVC 2015
        }.get(str(self.settings.compiler))
        if not min_version:
            # Unknown minimum version, let's try going ahead with the build to see if it works.
            return
        if float(str(self.settings.compiler.version)) < min_version:
            raise ConanException("%s %s is not supported, must be at least %s" % (self.settings.compiler,
                                                                                  self.settings.compiler.version,
                                                                                  min_version))

    def requirements(self):
        if self.options.use_boost:
            self.requires("boost/1.68.0@conan/stable")

    def source(self):
        tools.get("{0}/archive/v{1}.tar.gz".format(self.homepage, self.version))
        extracted_dir = self.name + "-" + self.version
        os.rename(extracted_dir, self._source_subfolder)
        # This small hack might be useful to guarantee proper /MT /MD linkage
        # in MSVC if the packaged project doesn't have variables to set it
        # properly
        tools.replace_in_file(os.path.join(self._source_subfolder, "CMakeLists.txt"),
                              "project(Fruit)",
                              '''PROJECT(Myfruit)
include(${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
conan_basic_setup()''')

    def _configure_cmake(self):
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
        cmake.configure(source_folder=self._source_subfolder)
        return cmake

    def build(self):
        cmake = self._configure_cmake()
        cmake.build()

    def package(self):
        cmake = self._configure_cmake()
        cmake.install()

        self.copy("COPYING", dst="licenses", ignore_case=True, keep_path=False,
                  src=self._source_subfolder)

    def package_info(self):
        self.cpp_info.includedirs = ["include"]
        self.cpp_info.libs = ["fruit"]
