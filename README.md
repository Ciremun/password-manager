# password-manager

## Usage

    ./pm [flags]        read or write data

    flags:

    -d  --data          data to encrypt
    -l  --label         add label for data
    -fl --find-label    find data by label
    -h  --help          display help

## Build

### Unix

    $ make

### Windows

```console
cmd.exe
cd "C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\Common7\Tools\"
VsDevCmd.bat
cd password-manager
build_win.bat
```

## Thank

[Tiny AES](https://github.com/kokke/tiny-AES-c)  
[b64.c](https://github.com/littlstar/b64.c)
