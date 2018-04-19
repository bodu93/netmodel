#include "blocking_queue.h"

#include <pthread.h>
#include <stdlib.h>
#include <assert.h>

/* a circular blocking thread-safe queue */
struct bq {
	int* buf;
	int  len;
	int  head;
	int  tail;
	pthread_mutex_t mutex;
	pthread_cond_t  cond;
};

struct bq* bq_init(int size) {
	if (size <= 0) return NULL;

	struct bq* q = (struct bq*)malloc(sizeof(*q));
	assert(q);
	q->len = size + 1;
	q->buf = (int*)malloc(sizeof(int) * (q->len));
	q->head = q->tail = 0;
	pthread_mutex_init(&q->mutex, NULL);
	pthread_cond_init(&q->cond, NULL);
	return q;
}

void bq_release(struct bq* q) {
	if (!q) return;

	free(q->buf);
	pthread_mutex_destroy(&q->mutex);
	pthread_cond_destroy(&q->cond);
	free(q);
}

int bq_size(struct bq* q) {
	assert(q);
	int size = 0;
	pthread_mutex_lock(&q->mutex);
	size = q->tail - q->head;
	if (size < 0) {
		size += q->len;
	}
	pthread_mutex_unlock(&q->mutex);
	return size;
}

static int bq_full(struct bq* q) {
	return (q->tail + 1) % (q->len) == (q->head);
}

void bq_put(struct bq* q, int fd) {
	pthread_mutex_lock(&q->mutex);
	while (bq_full(q)) { /* full buffer */
		pthread_cond_wait(&q->cond, &q->mutex);
	}
	assert(!bq_full(q));
	q->buf[q->tail++] = fd;
	/* pthread_cond_signal(&q->cond); */
	pthread_cond_broadcast(&q->cond);
	pthread_mutex_unlock(&q->mutex);
}

static int bq_empty(struct bq* q) {
	return q->head == q->tail;
}

int bq_get(struct bq* q) {
	int fd = -1;
	pthread_mutex_lock(&q->mutex);
	while (bq_empty(q)) {
		pthread_cond_wait(&q->cond, &q->mutex);
	}
	assert(!bq_empty(q));
	fd = q->buf[q->head++];
	/* pthread_cond_signal(&q->cond); */
	pthread_cond_broadcast(&q->cond);
	pthread_mutex_unlock(&q->mutex);
	return fd;
}
