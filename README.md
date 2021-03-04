# password-manager

## Usage

    ./pm [flags]                        read or write data

    flags:

    -d  --data                          data to encrypt
    -df --data-file                     read data from file
    -l  --label                         add label for data
    -fl --find-label                    find data by label
    -gp --generate-password [Nchars]    put random data
    -k  --key                           key
    -h  --help                          display help

## Build

### Unix

    $ make

### Windows

[Build Tools for Visual Studio 2019](https://visualstudio.microsoft.com/downloads/)  

In Visual Studio Installer:  
Windows 10 SDK  
MSVC Build Tools & Libraries  

```console
cmd.exe
"C:\Program Files (x86)\Microsoft Visual Studio\2019\BuildTools\Common7\Tools\VsDevCmd.bat"
cd password-manager
build_win.bat
```

## Thank

[Tiny AES](https://github.com/kokke/tiny-AES-c)  
[b64.c](https://github.com/littlstar/b64.c)
