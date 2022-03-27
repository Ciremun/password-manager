#!/bin/bash
cc -DNDEBUG -DPM_VERSION=$(git rev-parse --short HEAD) -O3 -g0 source/console/pm_main.c source/console/pm_aes.c source/console/pm_b64.c source/console/pm_io.c source/console/pm_parse.c source/console/pm_rand.c source/console/pm_sync.c source/console/pm_xcrypt.c -Iinclude -opm-$(uname -s -m | tr ' ' '-' | tr '[:upper:]' '[:lower:]')
