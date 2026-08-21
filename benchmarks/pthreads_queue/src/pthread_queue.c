#include "pthread_queue.h"

#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

uint64_t pthread_queue_now_ns(void) {
    struct timespec ts;

    if (clock_gettime(CLOCK_MONOTONIC, &ts) != 0) {
        return 0;
    }

    return
        ((uint64_t)ts.tv_sec * 1000000000ULL)
        + (uint64_t)ts.tv_nsec;
}

static int lock_queue(
    pthread_work_queue_t *queue
) {
    uint64_t start_ns;
    uint64_t end_ns;
    int rc;

    start_ns = pthread_queue_now_ns();

    rc = pthread_mutex_trylock(&queue->mutex);

    if (rc == EBUSY) {
        queue->contended_lock_acquisitions++;
        rc = pthread_mutex_lock(&queue->mutex);
    }

    if (rc != 0) {
        return rc;
    }

    end_ns = pthread_queue_now_ns();

    queue->lock_acquisitions++;
    queue->lock_wait_ns += end_ns - start_ns;

    return 0;
}

int pthread_work_queue_init(
    pthread_work_queue_t *queue,
    size_t capacity
) {
    int rc;

    if (queue == NULL || capacity == 0) {
        return EINVAL;
    }

    memset(queue, 0, sizeof(*queue));

    queue->items = calloc(
        capacity,
        sizeof(pthread_work_item_t)
    );

    if (queue->items == NULL) {
        return ENOMEM;
    }

    queue->capacity = capacity;

    rc = pthread_mutex_init(&queue->mutex, NULL);
    if (rc != 0) {
        free(queue->items);
        queue->items = NULL;
        return rc;
    }

    rc = pthread_cond_init(&queue->not_empty, NULL);
    if (rc != 0) {
        pthread_mutex_destroy(&queue->mutex);
        free(queue->items);
        queue->items = NULL;
        return rc;
    }

    rc = pthread_cond_init(&queue->not_full, NULL);
    if (rc != 0) {
        pthread_cond_destroy(&queue->not_empty);
        pthread_mutex_destroy(&queue->mutex);
        free(queue->items);
        queue->items = NULL;
        return rc;
    }

    return 0;
}

void pthread_work_queue_destroy(
    pthread_work_queue_t *queue
) {
    if (queue == NULL) {
        return;
    }

    pthread_cond_destroy(&queue->not_full);
    pthread_cond_destroy(&queue->not_empty);
    pthread_mutex_destroy(&queue->mutex);

    free(queue->items);
    queue->items = NULL;
}

int pthread_work_queue_push(
    pthread_work_queue_t *queue,
    pthread_work_item_t item
) {
    int rc;

    if (queue == NULL) {
        return EINVAL;
    }

    rc = lock_queue(queue);
    if (rc != 0) {
        return rc;
    }

    while (
        queue->count == queue->capacity
        && !queue->closed
    ) {
        rc = pthread_cond_wait(
            &queue->not_full,
            &queue->mutex
        );

        if (rc != 0) {
            pthread_mutex_unlock(&queue->mutex);
            return rc;
        }
    }

    if (queue->closed) {
        pthread_mutex_unlock(&queue->mutex);
        return ECANCELED;
    }

    queue->items[queue->tail] = item;
    queue->tail =
        (queue->tail + 1) % queue->capacity;

    queue->count++;

    pthread_cond_signal(&queue->not_empty);
    pthread_mutex_unlock(&queue->mutex);

    return 0;
}

int pthread_work_queue_pop(
    pthread_work_queue_t *queue,
    pthread_work_item_t *item
) {
    int rc;

    if (queue == NULL || item == NULL) {
        return EINVAL;
    }

    rc = lock_queue(queue);
    if (rc != 0) {
        return rc;
    }

    while (
        queue->count == 0
        && !queue->closed
    ) {
        rc = pthread_cond_wait(
            &queue->not_empty,
            &queue->mutex
        );

        if (rc != 0) {
            pthread_mutex_unlock(&queue->mutex);
            return rc;
        }
    }

    if (
        queue->count == 0
        && queue->closed
    ) {
        pthread_mutex_unlock(&queue->mutex);
        return 0;
    }

    *item = queue->items[queue->head];

    queue->head =
        (queue->head + 1) % queue->capacity;

    queue->count--;

    pthread_cond_signal(&queue->not_full);
    pthread_mutex_unlock(&queue->mutex);

    return 1;
}

void pthread_work_queue_close(
    pthread_work_queue_t *queue
) {
    if (queue == NULL) {
        return;
    }

    if (pthread_mutex_lock(&queue->mutex) != 0) {
        return;
    }

    queue->closed = 1;

    pthread_cond_broadcast(&queue->not_empty);
    pthread_cond_broadcast(&queue->not_full);

    pthread_mutex_unlock(&queue->mutex);
}
