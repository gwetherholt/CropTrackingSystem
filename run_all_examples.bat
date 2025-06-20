@echo off
setlocal enabledelayedexpansion

for %%F in (data\input\*.json) do (
    rem --- convert back-slashes to forward-slashes ---
    set "FILE=%%F"
    set "FILE=!FILE:\=/!"

    rem --- strip extension for folder name ---
    set "BASE=%%~nF"

    echo Running !FILE! ...

    docker run --rm -v "%cd%":/project tracking-solution ^
      --input  /project/!FILE! ^
      --output /project/data/output/!BASE!/tracking_output.json ^
      --vis-dir /project/data/output/!BASE!/visualization
)

echo All datasets processed.
endlocal
