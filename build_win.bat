@echo off

rem run using VsDevCmd.bat

set CFLAGS=/FC /nologo
set S=../src/
set SOURCES=%S%aes.c ^
%S%b64/encode.c %S%b64/decode.c %S%b64/buffer.c ^
%S%io/win.c %S%io/common.c ^
%S%rand.c ^
%S%parse.c

if "%~1"=="test" goto :test

mkdir build
cd build
cl.exe %S%main.c %SOURCES% %CFLAGS% /Fepm.exe
cd ../

goto :eof

:test

cd tests
cl.exe test.c %SOURCES% %CFLAGS% /Ftest.exe
test.exe
cd ../
