/*
 * pthreads mutex lock
 * codes from UNPv3
 */
#include "accept_lock.h"

#include <unistd.h>
#include <fcntl.h>
#include <pthread.h>
#include <sys/mman.h>

/* every process owns a copy of this pointer */
static pthread_mutex_t* mptr;

void accept_lock_init(const char* fname) {
	(void)fname;

	int fd = open("/dev/zero", O_RDWR, 0);
	/* protected by pthread mutex */
	mptr = mmap(0, sizeof(pthread_mutex_t), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
	close(fd);

	pthread_mutexattr_t mattr;
	pthread_mutexattr_init(&mattr);
	pthread_mutexattr_setpshared(&mattr, PTHREAD_PROCESS_SHARED);

	/* on my macOS10.13, core dumped ?? */
	pthread_mutex_init(mptr, &mattr);
}

void accept_lock_wait() {
	pthread_mutex_lock(mptr);
}

void accept_lock_release() {
	pthread_mutex_unlock(mptr);
}
