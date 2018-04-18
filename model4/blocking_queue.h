#ifndef ECHO_BLOCKING_QUEUE_H
#define ECHO_BLOCKING_QUEUE_H

struct bq;

struct bq* bq_init(int size);
void bq_release(struct bq*);
void bq_put(int fd);
int bq_get();

#endif
