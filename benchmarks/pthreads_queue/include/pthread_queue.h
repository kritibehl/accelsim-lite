#ifndef ACCELSIM_PTHREAD_QUEUE_H
#define ACCELSIM_PTHREAD_QUEUE_H

#include <pthread.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    uint64_t id;
    size_t begin;
    size_t end;
    uint64_t queued_at_ns;
} pthread_work_item_t;

typedef struct {
    pthread_work_item_t *items;
    size_t capacity;
    size_t head;
    size_t tail;
    size_t count;
    int closed;

    pthread_mutex_t mutex;
    pthread_cond_t not_empty;
    pthread_cond_t not_full;

    uint64_t lock_acquisitions;
    uint64_t contended_lock_acquisitions;
    uint64_t lock_wait_ns;
} pthread_work_queue_t;

int pthread_work_queue_init(
    pthread_work_queue_t *queue,
    size_t capacity
);

void pthread_work_queue_destroy(
    pthread_work_queue_t *queue
);

int pthread_work_queue_push(
    pthread_work_queue_t *queue,
    pthread_work_item_t item
);

int pthread_work_queue_pop(
    pthread_work_queue_t *queue,
    pthread_work_item_t *item
);

void pthread_work_queue_close(
    pthread_work_queue_t *queue
);

uint64_t pthread_queue_now_ns(void);

#ifdef __cplusplus
}
#endif

#endif
