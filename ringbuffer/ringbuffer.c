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


#include "ringbuffer.h"

int rb_init(struct ringbuffer_t *rb, size_t size)
{
    int rc = 0;
    rb->head = 0;
    rb->tail = 0;
    rb->size = size;
    rb->full_cnt = 0;
    rb->p_data = NULL;
    if (rb->size & (rb->size - 1)) {
        rc = RB_ERROR;
        fprintf(stderr, "[%s:%d] Ring size must power of two\n", __FUNCTION__,
                __LINE__);
        goto exit;
    }
    rb->p_data = (void **) calloc(size, sizeof(void *));
    fprintf(stdout, "[rb_init] rb: 0x%p, rb->p_data 0x%p\n", rb, rb->p_data);
    if (!rb->p_data) {
        rc = RB_ERROR;
        fprintf(stderr, "[%s:%d] Memory Allocate Fail\n", __FUNCTION__,
                __LINE__);
        goto exit;
    }

    rc = pthread_mutex_init(&rb->ring_lock, NULL);
    if (rc < 0) {
        fprintf(stderr, "[%s:%d] mutex_init fail rc = %d %s\n", __FUNCTION__,
                __LINE__, rc, strerror(errno));
        free(rb->p_data);
        goto exit;
    }
exit:
    return rc;
}

int rb_deinit(struct ringbuffer_t *rb)
{
    int rc = 0;
    rc = pthread_mutex_destroy(&rb->ring_lock);
    if (rb->p_data) {
        free(rb->p_data);
        rb->p_data = NULL;
    }
    // fprintf(stderr, "[%s] Full count: %d \n", __FUNCTION__, rb->full_cnt);
    return rc;
}


static inline bool rb_is_empty(const struct ringbuffer_t *rb)
{
    return rb->head == rb->tail;
}

static inline bool rb_is_full(const struct ringbuffer_t *rb)
{
    if (rb->head < rb->tail) {
        return (rb->size - 1) == (rb->tail - rb->head);
    }
    return (rb->head - rb->tail) == 1;
}


int rb_push(struct ringbuffer_t *rb, void *payload)
{
    int rc = 0;
    if (!rb->p_data) {
        rc = RB_ERROR;
        goto exit;
    }
    rc = pthread_mutex_lock(&rb->ring_lock);
    if (rc < 0) {
        fprintf(stderr, "[%s:%d] mutex_lock fail rc = %d %s\n", __FUNCTION__,
                __LINE__, rc, strerror(errno));
        goto exit;
    }

    if (rb_is_full(rb)) {
        //  ring buffer is full
        rb->full_cnt++;
        rc = pthread_mutex_unlock(&rb->ring_lock);
        return rc;
    }
    rb->p_data[rb->tail] = payload;
    rb->tail++;
    rb->tail &= (rb->size - 1);
    rc = pthread_mutex_unlock(&rb->ring_lock);
    if (rc < 0) {
        fprintf(stderr, "[%s:%d] mutex_unlock fail rc = %d %s\n", __FUNCTION__,
                __LINE__, rc, strerror(errno));
        goto exit;
    }
exit:
    return rc;
}

int rb_pop(struct ringbuffer_t *rb, void **payload)
{
    int rc = 0;
    if (!rb->p_data) {
        rc = RB_ERROR;
        goto exit;
    }
    pthread_mutex_lock(&rb->ring_lock);
    if (rc < 0) {
        fprintf(stderr, "[%s:%d] mutex_lock fail rc = %d %s\n", __FUNCTION__,
                __LINE__, rc, strerror(errno));
        goto exit;
    }
    if (rb_is_empty(rb)) {
        pthread_mutex_unlock(&rb->ring_lock);
        return rc;
    }

    *payload = rb->p_data[rb->head];
    rb->head++;
    rb->head &= (rb->size - 1);
    rc = pthread_mutex_unlock(&rb->ring_lock);
    if (rc < 0) {
        fprintf(stderr, "[%s:%d] mutex_unlock fail rc = %d %s\n", __FUNCTION__,
                __LINE__, rc, strerror(errno));
        goto exit;
    }
exit:
    return rc;
}
