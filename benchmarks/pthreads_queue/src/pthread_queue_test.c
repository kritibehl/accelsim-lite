#include "pthread_queue.h"

#include <assert.h>
#include <stdint.h>
#include <stdio.h>

int main(void) {
    pthread_work_queue_t queue;
    pthread_work_item_t input;
    pthread_work_item_t output;

    assert(
        pthread_work_queue_init(
            &queue,
            4
        ) == 0
    );

    input.id = 42;
    input.begin = 10;
    input.end = 20;
    input.queued_at_ns =
        pthread_queue_now_ns();

    assert(
        pthread_work_queue_push(
            &queue,
            input
        ) == 0
    );

    assert(
        pthread_work_queue_pop(
            &queue,
            &output
        ) == 1
    );

    assert(output.id == 42);
    assert(output.begin == 10);
    assert(output.end == 20);

    pthread_work_queue_close(&queue);

    assert(
        pthread_work_queue_pop(
            &queue,
            &output
        ) == 0
    );

    pthread_work_queue_destroy(&queue);

    printf(
        "pthread_queue_test: PASS\n"
    );

    return 0;
}
