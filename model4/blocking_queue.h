#ifndef ECHO_BLOCKING_QUEUE_H
#define ECHO_BLOCKING_QUEUE_H

struct bq;

struct bq* bq_init(int size);
void bq_release(struct bq* q);
int bq_size(struct bq* q);
void bq_put(struct bq* q, int fd);
int bq_get(struct bq* q);

#endif
