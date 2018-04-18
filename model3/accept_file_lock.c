/*
 * file lock
 * codes from UNPv3
 */
#include "accept_lock.h"

#include <unistd.h>
#include <fcntl.h>
#include <sys/file.h>
#include <errno.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

static struct flock lock_it;
static struct flock unlock_it;
static int lock_fd = -1;

void accept_lock_init(const char* fname) {
	/* mkstemp will change file name buffer, so we use a stack buffer */
	char lock_file[1024];
	strncpy(lock_file, fname, sizeof(lock_file));
	lock_fd = mkstemp(lock_file);

	unlink(lock_file);

	lock_it.l_type = F_WRLCK;
	lock_it.l_whence = SEEK_SET;
	lock_it.l_start = 0;
	lock_it.l_len = 0;

	unlock_it.l_type = F_UNLCK;
	unlock_it.l_whence = SEEK_SET;
	unlock_it.l_start = 0;
	unlock_it.l_len = 0;
}

void accept_lock_wait() {
	int rc;
	while ((rc = fcntl(lock_fd, F_SETLKW, &lock_it)) < 0) {
		if (errno == EINTR) {
			continue;
		} else {
			fprintf(stderr, "fcntl(2) error\n");
			exit(1);
		}
	}
}

void accept_lock_release() {
	if (fcntl(lock_fd, F_SETLKW, &unlock_it) < 0) {
		fprintf(stderr, "fcntl(2) error\n");
		exit(1);
	}
}
