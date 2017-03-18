
echo on

echo Postsubmit.bat started.

set PATH=%PATH:C:\Program Files\Git\usr\bin;=%

echo PATH changed.

if %VCVARSALL_SCRIPT% != "" (
  echo Running VCVARSALL script: %VCVARSALL_SCRIPT%
  CALL %VCVARSALL_SCRIPT% amd64 || exit /b 1
)

echo Creating build directory.

mkdir C:\Fruit\build-%CONFIGURATION%
cd C:\Fruit\build-%CONFIGURATION%

rem TODO: Remove the following 5 commands, they are only for debugging.
echo Looking for CMake executable:
dir "C:\Program Files (x86)\Microsoft Visual Studio\2017\Community\Common7\IDE\CommonExtensions\Microsoft\"
dir "C:\Program Files (x86)\Microsoft Visual Studio\2017\Community\Common7\IDE\CommonExtensions\Microsoft\CMake"
dir "C:\Program Files (x86)\Microsoft Visual Studio\2017\Community\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake"
dir %CMAKE_PATH%

echo Running CMake.

%CMAKE_PATH%\cmake.exe -G %CMAKE_GENERATOR% .. -DCMAKE_BUILD_TYPE=%CONFIGURATION% -DBOOST_DIR=%BOOST_DIR% -DBUILD_TESTS_IN_RELEASE_MODE=True || exit /b 1

echo Building Fruit.

IF %CMAKE_GENERATOR%=="MinGW Makefiles" (
  mingw32-make -j12 || exit /b 1
) ELSE (
  msbuild ALL_BUILD.vcxproj || exit /b 1
)

echo Running tests.

%CMAKE_PATH%\ctest.exe -j 1 --output-on-failure -C %CONFIGURATION% || exit /b 1
