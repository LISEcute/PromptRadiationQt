@echo off

set "APPDIR=C:\PromptRadiation\_install"
set "WDEP=C:\Qt\6.7.0\mingw_64\bin\windeployqt.exe"

if not exist "%APPDIR%\PromptRadiation.exe" (
  echo ERROR: Cannot find "%APPDIR%\PromptRadiation.exe"
  echo Check APPDIR path.
  pause
  exit /b 1
)

if not exist "%WDEP%" (
  echo ERROR: Cannot find "%WDEP%"
  echo Check Qt path / version / kit folder.
  pause
  exit /b 1
)

cd /d "%APPDIR%" || (
  echo ERROR: cd failed to "%APPDIR%"
  pause
  exit /b 1
)

"%WDEP%" --release --compiler-runtime --dir . "PromptRadiation.exe"

  echo DONE
  pause

