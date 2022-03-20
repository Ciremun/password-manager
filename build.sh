cc -DNDEBUG -DPM_VERSION=$(git rev-parse --short HEAD) -O3 -g0 source/console/*.c -Iinclude -opm-$(uname -s -m | tr ' ' '-' | tr '[:upper:]' '[:lower:]')
