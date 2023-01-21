from conans import ConanFile, CMake, tools
from conans.errors import ConanException
import os


class FruitConan(ConanFile):
    name = "fruit"
    version = "3.7.1"
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
    _cmake = None

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

    def build_requirements(self):
        if self.options.use_boost:
            self.build_requires("boost/1.68.0@conan/stable")

    def source(self):
        tools.get("{0}/archive/v{1}.tar.gz".format(self.homepage, self.version))
        extracted_dir = self.name + "-" + self.version
        os.rename(extracted_dir, self._source_subfolder)

    def _configure_cmake(self):
        if not self._cmake:
            self._cmake = CMake(self)
            self._cmake.definitions["FRUIT_IS_BEING_BUILT_BY_CONAN"] = "YES"
            self._cmake.definitions["BUILD_SHARED_LIBS"] = "YES" if self.options.shared else "NO"
            self._cmake.definitions["FRUIT_USES_BOOST"] = self.options.use_boost
            if self.options.use_boost:
                self._cmake.definitions["Boost_INCLUDE_DIR"] = os.path.join(
                    self.deps_cpp_info["boost"].rootpath, "include")
            if self.settings.os == "Windows":
                self._cmake.definitions["FRUIT_TESTS_USE_PRECOMPILED_HEADERS"] = "NO"
            self._cmake.definitions["CMAKE_BUILD_TYPE"] = self.settings.build_type
            self._cmake.configure(source_folder=self._source_subfolder)

        return self._cmake

    def build(self):
        cmake = self._configure_cmake()
        cmake.build()

    def package(self):
        cmake = self._configure_cmake()
        cmake.install()

        self.copy("COPYING", dst="licenses", ignore_case=True, keep_path=False,
                  src=self._source_subfolder)

    def package_info(self):
        self.cpp_info.libs = tools.collect_libs(self)
