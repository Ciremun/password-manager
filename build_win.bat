@echo off

rem run using VsDevCmd.bat

mkdir build
cd build

set CFLAGS=/FC /nologo /Fepm.exe
set S=../src/
set SOURCES=%S%main.c ^
%S%aes.c ^
%S%b64/encode.c %S%b64/decode.c %S%b64/buffer.c ^
%S%io/win.c %S%io/common.c ^
%S%rand.c

cl.exe %CFLAGS% %INCLUDES% %SOURCES%

cd ../