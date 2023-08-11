#!/bin/sh

set -xe

cc -Wall -Wextra -pedantic -std=c99 -D_GNU_SOURCE -DNDEBUG -DPM_THREAD_COUNT=32 -DPM_VERSION=$(git rev-parse --short HEAD) -O3 -ggdb source/console/pm_main.c source/console/pm_thread.c source/console/pm_aes.c source/console/pm_b64.c source/console/pm_io.c source/console/pm_parse.c source/console/pm_rand.c source/console/pm_sync.c source/console/pm_glob.c source/console/pm_xcrypt.c -Iinclude -opm-$(uname -s -m | tr ' ' '-' | tr '[:upper:]' '[:lower:]') -lpthread
