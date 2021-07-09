# password-manager

## Usage

    ./pm [flags]                  read or write data

    flags:

    -d  --data                    data to encrypt
    -df --data-file               read data from file
    -l  --label                   add label for data
    -fl --find-label              find data by label
    -gp --generate-password [N]   put random data
    -k  --key                     key
    -h  --help                    display help

## Build

    cc nobuild.c -o nobuild
    ./nobuild

## Thank

[Tiny AES](https://github.com/kokke/tiny-AES-c)  
[b64.c](https://github.com/littlstar/b64.c)
[nobuild](https://github.com/tsoding/nobuild)
