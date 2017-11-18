
echo on
setlocal EnableDelayedExpansion

SET POWERSHELL_PATH=
FOR /F "delims=" %%F IN ('where powershell.exe') DO (SET POWERSHELL_PATH=!POWERSHELL_PATH!%%~dpF.;)

set NEW_PATH=C:\Windows\system32;C:\Windows;%PYTHON3_PATH%;%PYTHON3_PATH%\Scripts;C:\Program Files (x86)\CMake\bin;
set PATH=%NEW_PATH%

SET CL_PATH=
SET MSBUILD_PATH=

if not "%VCVARSALL_DIR%" == "" (
  CALL "%VCVARSALL_DIR%\vcvarsall.bat" amd64
  echo on
  FOR /F "delims=" %%F IN ('where cl.exe') DO (SET CL_PATH=!CL_PATH!%%~dpF.;)
  FOR /F "delims=" %%F IN ('where msbuild.exe') DO (SET MSBUILD_PATH=!MSBUILD_PATH!%%~dpF.;)
)

set NEW_PATH=%NEW_PATH%%CL_PATH%%MSBUILD_PATH%%POWERSHELL_PATH%

if not "%MINGW_PATH%" == "" SET NEW_PATH=%NEW_PATH%%MINGW_PATH%;

set PATH=%NEW_PATH%
setx PATH "%PATH%"

mkdir C:\Fruit\build-%CONFIGURATION%
cd C:\Fruit\build-%CONFIGURATION%

cmake.exe -G "%CMAKE_GENERATOR%" .. -DCMAKE_BUILD_TYPE=%CONFIGURATION% %ADDITIONAL_CMAKE_ARGS%

echo "Content of CMakeFiles\CMakeError.log:"
if exist "CMakeFiles\CMakeError.log" (
  type "CMakeFiles\CMakeError.log"
)

IF "%CMAKE_GENERATOR%"=="MinGW Makefiles" (
  mingw32-make -j12 || exit /b 1
) ELSE (
  type ALL_BUILD.vcxproj
  msbuild ALL_BUILD.vcxproj || exit /b 1
)

pip3 install pytest
pip3 install pytest-xdist

cd tests
py.test -r a -n 1 || exit /b 1
