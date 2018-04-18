#ifndef ECHO_ACCEPT_LOCK_H
#define ECHO_ACCEPT_LOCK_H

void accept_lock_init(const char* fname);
void accept_lock_wait();
void accept_lock_release();

#endif
