
echo on

echo Postsubmit.bat started.

if exist %VCVARSALL_SCRIPT% (
  echo Running VCVARSALL script: %VCVARSALL_SCRIPT%
  CALL %VCVARSALL_SCRIPT% amd64 || exit /b %errorlevel%
)

echo Creating build directory.

mkdir C:\Fruit\build-%CONFIGURATION%
cd C:\Fruit\build-%CONFIGURATION%

rem TODO: Remove the following 2 commands, they are only for debugging.
echo Looking for CMake executable:
dir %CMAKE_PATH%

echo Running CMake.

%CMAKE_PATH%\cmake.exe -G %CMAKE_GENERATOR% .. -DCMAKE_BUILD_TYPE=%CONFIGURATION% -DBOOST_DIR=%BOOST_DIR% -DBUILD_TESTS_IN_RELEASE_MODE=True || exit /b %errorlevel%

echo Building Fruit.

IF %CMAKE_GENERATOR%=="MinGW Makefiles" (
  mingw32-make -j12 || exit /b %errorlevel%
) ELSE (
  msbuild ALL_BUILD.vcxproj || exit /b %errorlevel%
)

echo Running tests.

%CMAKE_PATH%\ctest.exe -j 1 --output-on-failure -C %CONFIGURATION% || exit /b %errorlevel%
