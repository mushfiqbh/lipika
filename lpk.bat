@echo off
rem %1 receives the filename passed to the script
g++ -std=c++17 src/*.cpp -o lipika.exe || exit /b %errorlevel%
lipika.exe %1 || exit /b %errorlevel%
python output.py