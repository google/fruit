
echo on

set PATH=C:\Windows\system32;C:\Windows;%PYTHON3_PATH%;%CMAKE_PATH%;

if not "%VCVARSALL_DIR%" == "" CALL "%VCVARSALL_DIR%\vcvarsall.bat" amd64

if not "%MINGW_PATH%" == "" SET PATH=%PATH%%MINGW_PATH%;

rem TODO: Remove this.
dir "C:\Program Files (x86)" 
dir "C:\Program Files (x86)\Microsoft Visual Studio" 
dir "C:\Program Files (x86)\Microsoft Visual Studio\2017" 
dir "C:\Program Files (x86)\Microsoft Visual Studio\2017\Community" 
dir "C:\Program Files (x86)\Microsoft Visual Studio\2017\Community\Common7" 
dir "C:\Program Files (x86)\Microsoft Visual Studio\2017\Community\Common7\IDE" 
dir "C:\Program Files (x86)\Microsoft Visual Studio\2017\Community\Common7\IDE" 
dir "C:\Program Files (x86)\Microsoft Visual Studio\2017\Community\Common7\IDE\CommonExtensions" 
dir "C:\Program Files (x86)\Microsoft Visual Studio\2017\Community\Common7\IDE\CommonExtensions\Microsoft" 
dir "C:\Program Files (x86)\Microsoft Visual Studio\2017\Community\Common7\IDE\CommonExtensions\Microsoft\CMake" 
dir "C:\Program Files (x86)\Microsoft Visual Studio\2017\Community\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake" 
dir "%CMAKE_PATH%"
dir "%PYTHON3_PATH%"
where python.exe
where python3.exe
where pip.exe
where pip3.exe

mkdir C:\Fruit\build-%CONFIGURATION%
cd C:\Fruit\build-%CONFIGURATION%


cmake.exe -G "%CMAKE_GENERATOR%" .. -DCMAKE_BUILD_TYPE=%CONFIGURATION% -DBOOST_DIR="%BOOST_DIR%" -DBUILD_TESTS_IN_RELEASE_MODE=True || exit /b 1

IF "%CMAKE_GENERATOR%"=="MinGW Makefiles" (
  mingw32-make -j12 || exit /b 1
) ELSE (
  msbuild ALL_BUILD.vcxproj || exit /b 1
)

pip3 install nose2

ctest.exe -j 1 --output-on-failure -C %CONFIGURATION% || exit /b 1
