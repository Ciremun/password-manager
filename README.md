# password-manager

[![build-console-ci](https://github.com/Ciremun/password-manager/actions/workflows/build-console.yml/badge.svg)](https://github.com/Ciremun/password-manager/actions) [![build-ui-desktop](https://github.com/Ciremun/password-manager/actions/workflows/build-ui-desktop.yml/badge.svg)](https://github.com/Ciremun/password-manager/actions) [![build-ui-android](https://github.com/Ciremun/password-manager/actions/workflows/build-ui-android.yml/badge.svg)](https://github.com/Ciremun/password-manager/actions) [![Coverity Scan Build Status](https://scan.coverity.com/projects/23849/badge.svg)](https://scan.coverity.com/projects/ciremun-password-manager)

## Usage

    ./pm [flags]                      read or write data

    sync:                             set PM_SYNC_REMOTE_URL env var

    flags:

    -d       --data                   data to encrypt
    -df      --data-file              data to encrypt from file
    -l       --label                  label data / find by label
    -dl      --delete-label           delete label and its data
    -gp      --generate-password [N]  put random data
    -c       --copy [label]           win32: copy to clipboard, posix: pipe with clip tools
    -k       --key                    key
    -kf      --key-file               key file path
    -i       --input                  encrypted file path
    -o       --output                 decrypted file path
    -b       --binary                 binary mode
    -b64enc  --base64-encode [str]    base64 encode string to stdout, optional key
    -b64dec  --base64-decode [str]    base64 decode string to stdout, optional key
    -v       --version                display version
    -h       --help                   display help

## Build

    sh build_console.sh

## Examples

### generate and copy a password

	echo "secret_key" > key.txt
	./pm -kf key.txt -gp -c password_name | xclip

### copy saved password

	./pm -kf key.txt -c pass | xclip

### encrypt binary

	./pm -kf key.txt -i binary -o binary.enc -b

### decrypt binary

	./pm -kf key.txt -i binary.enc -o binary.dec -b

### print all passwords

    ./pm -kf key.txt

### base64 encode string to stdout, optional key

    ./pm -b64enc -d "string" [-kf key.txt]

## Thank

[Dear ImGui](https://github.com/ocornut/imgui)  
[stb](https://github.com/nothings/stb)  
[Tiny AES](https://github.com/kokke/tiny-AES-c)  
[rawdraw](https://github.com/cntools/rawdraw)  
[nobuild](https://github.com/tsoding/nobuild)  
[b64.c](https://github.com/littlstar/b64.c)  
