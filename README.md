# password-manager

## Usage

    ./pm [flags]                  read or write data

    flags:

    -d  --data                    data to encrypt
    -df --data-file               data to encrypt from file
    -l  --label                   add label for data
    -fl --find-label              find data by label
    -dl --delete-label            delete label and its data
    -c  --copy                    -fl helper, win32: copy to clipboard, posix: pipe with clip tools
    -gp --generate-password [N]   put random data
    -k  --key                     key (optional flag)
    -h  --help                    display help

## Build

    cc nobuild.c -o nobuild
    ./nobuild

## Thank

[Tiny AES](https://github.com/kokke/tiny-AES-c)  
[b64.c](https://github.com/littlstar/b64.c)  
[nobuild](https://github.com/tsoding/nobuild)  
