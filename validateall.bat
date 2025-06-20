@echo off
rem --------------------------------------------------------------
rem validateall.bat  –  verify every tracking_output.json (or
rem output.json) under data\output\*\
rem --------------------------------------------------------------
chcp 65001 > nul    &  echo === checking all tracking_output.json files ===

set "FAIL=0"

rem --- 1. build a list with DIR (recurses sub-folders) -------------
for /F "usebackq delims=" %%F in (`
        dir /b /s "data\output\tracking_output.json" 2^>nul ^&^&
        dir /b /s "data\output\output.json"          2^>nul
    `) do (
    echo Validating %%F …
    python "%~dp0tools\check_ids.py" "%%F"
    if errorlevel 1 set "FAIL=1"
)

if "%FAIL%"=="1" (
    echo.
    echo Validation FAILED ✗
    exit /B 1
) else (
    echo.
    echo All datasets ✓
    exit /B 0
)
