@echo off

rem run using VsDevCmd.bat

mkdir build
cd build

set CFLAGS=/FC /nologo /Fepm.exe
set SOURCES=../src/main.c ../src/aes.c ../src/b64/encode.c ../src/b64/decode.c ../src/b64/buffer.c ../src/io_win.c

cl.exe %CFLAGS% %INCLUDES% %SOURCES%

cd ../