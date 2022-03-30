// Copyright (c) 2011-2012,2013,2016,2018,2019,2020 <>< Charles Lohr
// This file may be licensed under the MIT/x11 license, NewBSD or CC0 licenses
// Permission is hereby granted, free of charge, to any person obtaining a
// copy of this software and associated documentation files (the "Software"),
// to deal in the Software without restriction, including without limitation
// the rights to use, copy, modify, merge, publish, distribute, sublicense,
// and/or sell copies of the Software, and to permit persons to whom the
// Software is furnished to do so, subject to the following conditions:
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of this file.
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
// FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
// IN THE SOFTWARE.

#ifndef PM_THREAD_H_
#define PM_THREAD_H_

#include <stdint.h>
#include <stdlib.h>

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#else
#include <pthread.h>
#endif // _WIN32

#include "console/io.h"
#include "console/util.h"

typedef struct
{
    size_t load;
    size_t remainder;
    size_t offset;
    uint8_t *aes_key;
    String s;
    File f;
    int thread_count;
} thread_load_info;

typedef void *og_thread_t;

thread_load_info calc_thread_count_and_load(size_t size);
og_thread_t OGCreateThread(void(routine)(thread_load_info *), void *parameter);
void *OGJoinThread(og_thread_t ot);

#endif // PM_THREAD_H_
