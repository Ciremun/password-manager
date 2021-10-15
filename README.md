# password-manager

<a href="https://scan.coverity.com/projects/ciremun-password-manager"><img alt="Coverity Scan Build Status" src="https://scan.coverity.com/projects/23849/badge.svg"/></a>

## Usage

    ./pm [flags]                  read or write data

    sync:                         set PM_SYNC_REMOTE_URL env var

    flags:

    -d  --data                    data to encrypt
    -df --data-file               data to encrypt from file
    -l  --label                   add label for data
    -fl --find-label              find data by label
    -dl --delete-label            delete label and its data
    -gp --generate-password [N]   put random data
    -c  --copy                    -fl, -gp helper, win32: copy to clipboard, posix: pipe with clip tools
    -k  --key                     key
    -kf --key-file                key file path
    -i  --input                   stored data path
    -v  --version                 display version
    -h  --help                    display help

## Build

    cc nobuild.c -o nobuild
    ./nobuild

## Thank

[Tiny AES](https://github.com/kokke/tiny-AES-c)  
[b64.c](https://github.com/littlstar/b64.c)  
[nobuild](https://github.com/tsoding/nobuild)  
