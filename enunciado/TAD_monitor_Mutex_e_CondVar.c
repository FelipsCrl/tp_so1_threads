#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

#define BUFFER_SIZE 5

typedef struct {
    int buffer[BUFFER_SIZE];
    int count, in, out;
    pthread_mutex_t mutex;
    pthread_cond_t not_empty;
    pthread_cond_t not_full;
} monitor_buffer_t;

void monitor_init(monitor_buffer_t *mb) {
    mb->count = mb->in = mb->out = 0;
    pthread_mutex_init(&mb->mutex, NULL);
    pthread_cond_init(&mb->not_empty, NULL);
    pthread_cond_init(&mb->not_full, NULL);
}

void monitor_insert(monitor_buffer_t *mb, int item) {
    pthread_mutex_lock(&mb->mutex);

    while (mb->count == BUFFER_SIZE)
        pthread_cond_wait(&mb->not_full, &mb->mutex);

    mb->buffer[mb->in] = item;
    mb->in = (mb->in + 1) % BUFFER_SIZE;
    mb->count++;

    pthread_cond_signal(&mb->not_empty);
    pthread_mutex_unlock(&mb->mutex);
}

int monitor_remove(monitor_buffer_t *mb) {
    pthread_mutex_lock(&mb->mutex);

    while (mb->count == 0)
        pthread_cond_wait(&mb->not_empty, &mb->mutex);

    int item = mb->buffer[mb->out];
    mb->out = (mb->out + 1) % BUFFER_SIZE;
    mb->count--;

    pthread_cond_signal(&mb->not_full);
    pthread_mutex_unlock(&mb->mutex);

    return item;
}