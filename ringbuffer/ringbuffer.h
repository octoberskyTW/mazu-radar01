/***************************************************************************
* MIT License
*
* Copyright (c) [2020] [Dung-Ru Tsai] octobersky.tw@gmail.com
*
* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in all
* copies or substantial portions of the Software.

* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
* AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
*OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
* SOFTWARE.
*
****************************************************************************/
#ifndef __RINGBUFFER_H__
#define __RINGBUFFER_H__

#include <errno.h>
#include <pthread.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
struct ringbuffer_t {
    uint32_t head;
    uint32_t tail;
    uint32_t size;
    uint32_t full_cnt;
    pthread_mutex_t ring_lock;
    void **p_data;
};

enum { RB_ERROR = -1, RB_SUCCESS = 0 };

#ifdef __cplusplus
extern "C" {
#endif
int rb_init(struct ringbuffer_t *rb, size_t size);
int rb_deinit(struct ringbuffer_t *rb);
int rb_push(struct ringbuffer_t *rb, void *payload);
int rb_pop(struct ringbuffer_t *rb, void **payload);
#ifdef __cplusplus
}
#endif


#endif  //  __RINGBUFFER_H__