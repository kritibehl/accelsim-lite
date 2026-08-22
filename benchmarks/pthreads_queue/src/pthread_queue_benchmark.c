#include "pthread_queue.h"

#include <math.h>
#include <pthread.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    pthread_work_queue_t *queue;
    const uint64_t *data;
    size_t workload_size;

    uint64_t checksum;
    uint64_t tasks_completed;
    uint64_t allocations;

    double *latencies_us;
    size_t latency_capacity;
    size_t latency_count;
} worker_context_t;

static int compare_uint64(
    const void *left,
    const void *right
) {
    const uint64_t a =
        *(const uint64_t *)left;

    const uint64_t b =
        *(const uint64_t *)right;

    if (a < b) {
        return -1;
    }

    if (a > b) {
        return 1;
    }

    return 0;
}

static int compare_double(
    const void *a,
    const void *b
) {
    const double da = *(const double *)a;
    const double db = *(const double *)b;

    if (da < db) {
        return -1;
    }

    if (da > db) {
        return 1;
    }

    return 0;
}

static double percentile(
    double *values,
    size_t count,
    double fraction
) {
    double position;
    size_t lower;
    size_t upper;
    double weight;

    if (count == 0) {
        return 0.0;
    }

    qsort(
        values,
        count,
        sizeof(double),
        compare_double
    );

    position =
        fraction * (double)(count - 1);

    lower = (size_t)floor(position);
    upper = (size_t)ceil(position);

    if (lower == upper) {
        return values[lower];
    }

    weight = position - (double)lower;

    return
        values[lower] * (1.0 - weight)
        + values[upper] * weight;
}

static uint64_t process_item(
    const uint64_t *data,
    pthread_work_item_t item,
    uint64_t *allocation_counter
) {
    size_t count;
    uint64_t *temporary;
    uint64_t checksum;
    size_t i;

    count = item.end - item.begin;

    temporary = malloc(
        count * sizeof(uint64_t)
    );

    if (temporary == NULL) {
        return 0;
    }

    (*allocation_counter)++;

    checksum = 0;

    for (i = 0; i < count; ++i) {
        size_t index = item.begin + i;

        temporary[i] =
            data[index]
            ^ (uint64_t)(index * 17);

        checksum += temporary[i];
    }

    qsort(
        temporary,
        count,
        sizeof(uint64_t),
        compare_uint64
    );

    for (i = 1; i < count; ++i) {
        checksum ^=
            temporary[i] - temporary[i - 1];
    }

    free(temporary);

    return checksum;
}

static void *worker_main(void *argument) {
    worker_context_t *context;
    pthread_work_item_t item;

    context = (worker_context_t *)argument;

    while (1) {
        int rc;
        uint64_t start_ns;
        uint64_t end_ns;
        uint64_t item_checksum;

        rc = pthread_work_queue_pop(
            context->queue,
            &item
        );

        if (rc == 0) {
            break;
        }

        if (rc != 1) {
            return NULL;
        }

        start_ns = pthread_queue_now_ns();

        item_checksum = process_item(
            context->data,
            item,
            &context->allocations
        );

        end_ns = pthread_queue_now_ns();

        context->checksum ^= item_checksum;
        context->tasks_completed++;

        if (
            context->latency_count
            < context->latency_capacity
        ) {
            context->latencies_us[
                context->latency_count++
            ] =
                (double)(
                    end_ns - item.queued_at_ns
                ) / 1000.0;
        }

        (void)start_ns;
    }

    return NULL;
}

int main(
    int argc,
    char **argv
) {
    size_t workers;
    size_t workload_size;
    size_t chunk_size;
    size_t queue_capacity;

    uint64_t *data;
    pthread_t *threads;
    worker_context_t *contexts;
    pthread_work_queue_t queue;

    size_t task_count;
    size_t i;
    size_t worker_index;

    uint64_t start_ns;
    uint64_t end_ns;
    uint64_t checksum;
    uint64_t total_allocations;
    uint64_t total_tasks;

    double *all_latencies;
    size_t all_latency_count;

    double elapsed_ms;
    double throughput;
    double p50;
    double p95;
    double p99;
    double contention_rate;

    if (argc != 3) {
        fprintf(
            stderr,
            "usage: pthread-queue-benchmark "
            "<workers> <workload_size>\n"
        );

        return 2;
    }

    workers =
        (size_t)strtoull(argv[1], NULL, 10);

    workload_size =
        (size_t)strtoull(argv[2], NULL, 10);

    chunk_size = 256;
    queue_capacity = 1024;

    if (
        workers == 0
        || workload_size == 0
    ) {
        fprintf(
            stderr,
            "workers and workload must be > 0\n"
        );

        return 2;
    }

    task_count =
        (workload_size + chunk_size - 1)
        / chunk_size;

    data = malloc(
        workload_size * sizeof(uint64_t)
    );

    threads = calloc(
        workers,
        sizeof(pthread_t)
    );

    contexts = calloc(
        workers,
        sizeof(worker_context_t)
    );

    all_latencies = malloc(
        task_count * sizeof(double)
    );

    if (
        data == NULL
        || threads == NULL
        || contexts == NULL
        || all_latencies == NULL
    ) {
        fprintf(stderr, "allocation failed\n");

        free(data);
        free(threads);
        free(contexts);
        free(all_latencies);

        return 1;
    }

    for (i = 0; i < workload_size; ++i) {
        data[i] =
            ((uint64_t)i * 2654435761ULL)
            ^ ((uint64_t)i >> 3);
    }

    if (
        pthread_work_queue_init(
            &queue,
            queue_capacity
        ) != 0
    ) {
        fprintf(
            stderr,
            "queue initialization failed\n"
        );

        free(data);
        free(threads);
        free(contexts);
        free(all_latencies);

        return 1;
    }

    for (
        worker_index = 0;
        worker_index < workers;
        ++worker_index
    ) {
        contexts[worker_index].queue =
            &queue;

        contexts[worker_index].data =
            data;

        contexts[worker_index].workload_size =
            workload_size;

        contexts[worker_index].latency_capacity =
            task_count;

        contexts[worker_index].latencies_us =
            calloc(
                task_count,
                sizeof(double)
            );

        if (
            contexts[worker_index].latencies_us
            == NULL
        ) {
            fprintf(
                stderr,
                "worker latency allocation failed\n"
            );

            return 1;
        }
    }

    start_ns = pthread_queue_now_ns();

    for (
        worker_index = 0;
        worker_index < workers;
        ++worker_index
    ) {
        if (
            pthread_create(
                &threads[worker_index],
                NULL,
                worker_main,
                &contexts[worker_index]
            ) != 0
        ) {
            fprintf(
                stderr,
                "pthread_create failed\n"
            );

            return 1;
        }
    }

    for (i = 0; i < task_count; ++i) {
        pthread_work_item_t item;

        item.id = (uint64_t)i;
        item.begin = i * chunk_size;
        item.end = item.begin + chunk_size;

        if (item.end > workload_size) {
            item.end = workload_size;
        }

        item.queued_at_ns =
            pthread_queue_now_ns();

        if (
            pthread_work_queue_push(
                &queue,
                item
            ) != 0
        ) {
            fprintf(
                stderr,
                "queue push failed\n"
            );

            return 1;
        }
    }

    pthread_work_queue_close(&queue);

    for (
        worker_index = 0;
        worker_index < workers;
        ++worker_index
    ) {
        pthread_join(
            threads[worker_index],
            NULL
        );
    }

    end_ns = pthread_queue_now_ns();

    checksum = 0;
    total_allocations = 0;
    total_tasks = 0;
    all_latency_count = 0;

    for (
        worker_index = 0;
        worker_index < workers;
        ++worker_index
    ) {
        worker_context_t *context =
            &contexts[worker_index];

        checksum ^= context->checksum;

        total_allocations +=
            context->allocations;

        total_tasks +=
            context->tasks_completed;

        for (
            i = 0;
            i < context->latency_count;
            ++i
        ) {
            all_latencies[
                all_latency_count++
            ] =
                context->latencies_us[i];
        }
    }

    elapsed_ms =
        (double)(end_ns - start_ns)
        / 1000000.0;

    throughput =
        (double)workload_size
        / (elapsed_ms / 1000.0);

    p50 = percentile(
        all_latencies,
        all_latency_count,
        0.50
    );

    p95 = percentile(
        all_latencies,
        all_latency_count,
        0.95
    );

    p99 = percentile(
        all_latencies,
        all_latency_count,
        0.99
    );

    contention_rate =
        queue.lock_acquisitions == 0
            ? 0.0
            : (double)
                queue.contended_lock_acquisitions
              / (double)
                queue.lock_acquisitions;

    printf(
        "implementation,workers,workload_size,"
        "tasks_processed,elapsed_ms,"
        "throughput_items_per_second,"
        "p50_task_latency_us,"
        "p95_task_latency_us,"
        "p99_task_latency_us,"
        "allocations,"
        "lock_acquisitions,"
        "contended_lock_acquisitions,"
        "contention_rate,"
        "lock_wait_ms,"
        "checksum\n"
    );

    printf(
        "c_pthreads,%zu,%zu,%llu,%.4f,"
        "%.4f,%.4f,%.4f,%.4f,"
        "%llu,%llu,%llu,%.6f,%.6f,%llu\n",
        workers,
        workload_size,
        (unsigned long long)total_tasks,
        elapsed_ms,
        throughput,
        p50,
        p95,
        p99,
        (unsigned long long)total_allocations,
        (unsigned long long)
            queue.lock_acquisitions,
        (unsigned long long)
            queue.contended_lock_acquisitions,
        contention_rate,
        (double)queue.lock_wait_ns
            / 1000000.0,
        (unsigned long long)checksum
    );

    pthread_work_queue_destroy(&queue);

    for (
        worker_index = 0;
        worker_index < workers;
        ++worker_index
    ) {
        free(
            contexts[worker_index]
                .latencies_us
        );
    }

    free(data);
    free(threads);
    free(contexts);
    free(all_latencies);

    return 0;
}
