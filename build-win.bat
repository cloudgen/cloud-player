@echo off
setlocal
rem Build cloud-player.exe with Visual Studio 2022.
rem See docs/requirements/requirement-cpp-windows-environment.md.
set "VS=C:\Program Files\Microsoft Visual Studio\2022\Community"
set "CMAKE=%VS%\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\cmake.exe"
set "VCVARS=%VS%\VC\Auxiliary\Build\vcvars64.bat"
if not exist "%VCVARS%" (
  echo vcvars64.bat is missing. Install the Desktop development with C++ workload in Visual Studio 2022.
  exit /b 1
)
if not exist "%CMAKE%" (
  echo The Visual Studio CMake package is missing.
  exit /b 1
)
call "%VCVARS%"
if errorlevel 1 exit /b 1
cd /d "%~dp0"
"%CMAKE%" -S . -B build-win -G "Visual Studio 17 2022" -A x64
if errorlevel 1 exit /b 1
"%CMAKE%" --build build-win --config Debug
if errorlevel 1 exit /b 1
echo Built build-win\Debug\cloud-player.exe
exit /b 0
