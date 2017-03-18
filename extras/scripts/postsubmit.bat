
echo on

set PATH=C:\Windows\system32;C:\Windows;%PYTHON3_PATH%;%CMAKE_PATH%;

if not "%VCVARSALL_DIR%" == "" CALL "%VCVARSALL_DIR%\vcvarsall.bat" amd64

if not "%MINGW_PATH%" == "" SET PATH=%MINGW_PATH%;%PATH%

mkdir C:\Fruit\build-%CONFIGURATION%
cd C:\Fruit\build-%CONFIGURATION%


cmake.exe -G "%CMAKE_GENERATOR%" .. -DCMAKE_BUILD_TYPE=%CONFIGURATION% -DBOOST_DIR="%BOOST_DIR%" -DBUILD_TESTS_IN_RELEASE_MODE=True || exit /b 1

IF "%CMAKE_GENERATOR%"=="MinGW Makefiles" (
  mingw32-make -j12 || exit /b 1
) ELSE (
  msbuild ALL_BUILD.vcxproj || exit /b 1
)

ctest.exe -j 1 --output-on-failure -C %CONFIGURATION% || exit /b 1
