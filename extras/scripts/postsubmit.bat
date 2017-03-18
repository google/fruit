
echo on

set PATH=C:\Windows\system32;C:\Windows;%PYTHON3_PATH%;%PYTHON3_PATH%\Scripts;%CMAKE_PATH%;

if not "%VCVARSALL_DIR%" == "" CALL "%VCVARSALL_DIR%\vcvarsall.bat" amd64

if not "%MINGW_PATH%" == "" SET PATH=%PATH%%MINGW_PATH%;

setx PATH "%PATH%"

rem TODO: Remove this.
dir "%CMAKE_PATH%"
dir "%BOOST_DIR%"
dir "%PYTHON3_PATH%"
dir "%PYTHON3_PATH%\Scripts"
where cmake.exe
where ctest.exe
echo %PATH%

mkdir C:\Fruit\build-%CONFIGURATION%
cd C:\Fruit\build-%CONFIGURATION%

IF "%CMAKE_GENERATOR%"=="MinGW Makefiles" SET CMAKE_EXTRA_ARGS="-DCMAKE_MAKE_PROGRAM=%MINGW_PATH%/mingw32-make.exe" "-DCMAKE_C_COMPILER=%MINGW_PATH%\gcc.exe" "-DCMAKE_CXX_COMPILER=%MINGW_PATH%\g++.exe"

cmake.exe -G "%CMAKE_GENERATOR%" .. -DCMAKE_BUILD_TYPE=%CONFIGURATION% -DBOOST_DIR="%BOOST_DIR%" -DBUILD_TESTS_IN_RELEASE_MODE=True %CMAKE_EXTRA_ARGS% || exit /b 1

IF "%CMAKE_GENERATOR%"=="MinGW Makefiles" (
  mingw32-make -j12 || exit /b 1
) ELSE (
  msbuild ALL_BUILD.vcxproj || exit /b 1
)

pip3 install nose2

ctest.exe -j 1 --output-on-failure -C %CONFIGURATION% || exit /b 1
