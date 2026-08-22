#include "pthread_queue.h"

#include <pthread.h>
#include <stdint.h>
#include <stdio.h>

typedef struct {
    pthread_work_queue_t *queue;
    pthread_work_item_t item;
    int pop_result;
} consumer_context_t;

static int check(
    int condition,
    const char *message
) {
    if (!condition) {
        fprintf(
            stderr,
            "FAIL: %s\n",
            message
        );
        return 0;
    }

    return 1;
}

static void *consumer_main(
    void *argument
) {
    consumer_context_t *context =
        (consumer_context_t *)argument;

    context->pop_result =
        pthread_work_queue_pop(
            context->queue,
            &context->item
        );

    return NULL;
}

int main(void) {
    pthread_work_queue_t queue;
    pthread_work_item_t input;
    pthread_work_item_t output;

    consumer_context_t consumer;
    pthread_t thread;

    int rc;

    rc = pthread_work_queue_init(
        &queue,
        4
    );

    if (!check(
        rc == 0,
        "queue initialization"
    )) {
        return 1;
    }

    input.id = 42;
    input.begin = 10;
    input.end = 20;
    input.queued_at_ns =
        pthread_queue_now_ns();

    rc = pthread_work_queue_push(
        &queue,
        input
    );

    if (!check(
        rc == 0,
        "queue push"
    )) {
        pthread_work_queue_destroy(&queue);
        return 1;
    }

    rc = pthread_work_queue_pop(
        &queue,
        &output
    );

    if (!check(
        rc == 1,
        "queue pop"
    )) {
        pthread_work_queue_destroy(&queue);
        return 1;
    }

    if (!check(
        output.id == 42,
        "work item id"
    )) {
        pthread_work_queue_destroy(&queue);
        return 1;
    }

    if (!check(
        output.begin == 10 &&
        output.end == 20,
        "work item range"
    )) {
        pthread_work_queue_destroy(&queue);
        return 1;
    }

    consumer.queue = &queue;
    consumer.pop_result = -1;

    rc = pthread_create(
        &thread,
        NULL,
        consumer_main,
        &consumer
    );

    if (!check(
        rc == 0,
        "pthread_create"
    )) {
        pthread_work_queue_destroy(&queue);
        return 1;
    }

    input.id = 99;
    input.begin = 100;
    input.end = 200;
    input.queued_at_ns =
        pthread_queue_now_ns();

    rc = pthread_work_queue_push(
        &queue,
        input
    );

    if (!check(
        rc == 0,
        "concurrent queue push"
    )) {
        pthread_work_queue_close(&queue);
        pthread_join(thread, NULL);
        pthread_work_queue_destroy(&queue);
        return 1;
    }

    rc = pthread_join(
        thread,
        NULL
    );

    if (!check(
        rc == 0,
        "pthread_join"
    )) {
        pthread_work_queue_destroy(&queue);
        return 1;
    }

    if (!check(
        consumer.pop_result == 1,
        "consumer queue pop"
    )) {
        pthread_work_queue_destroy(&queue);
        return 1;
    }

    if (!check(
        consumer.item.id == 99,
        "consumer received correct item"
    )) {
        pthread_work_queue_destroy(&queue);
        return 1;
    }

    pthread_work_queue_close(&queue);

    rc = pthread_work_queue_pop(
        &queue,
        &output
    );

    if (!check(
        rc == 0,
        "closed empty queue"
    )) {
        pthread_work_queue_destroy(&queue);
        return 1;
    }

    pthread_work_queue_destroy(&queue);

    printf(
        "pthread_queue_test: PASS\n"
    );

    return 0;
}
