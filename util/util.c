/*
 * util.c
 * codes from UNPv3 with a little change
 */
#include "util.h"

#include <unistd.h>
#include <errno.h>

ssize_t readn(int fd, void* vptr, size_t n) {
	size_t nleft = n;
	char* ptr = vptr;

	ssize_t nread;
	while (nleft > 0) {
		if ((nread = read(fd, ptr, nleft)) < 0) {
				if (errno == EINTR)
					nread = 0; /* call read again */
				else
					return -1;
		} else if (nread == 0) /* eof */
			break;

		nleft -= nread;
		ptr += nread;
	}

	return n - nleft;
}

ssize_t writen(int fd, const void* vptr, size_t n) {
	size_t nleft = n;
	const char* ptr = vptr;

	ssize_t nwritten;
	while (nleft > 0) {
		if ((nwritten = write(fd, ptr, nleft)) <= 0) {
			if (nwritten < 0 && errno == EINTR)
				nwritten  = 0; /* call write again */
			else
				return -1;
		}

		nleft -= nwritten;
		ptr += nwritten;
	}

	return n;
}


static int read_cnt;
static char* read_ptr;
#define MAXLINE 4096
static char read_buf[MAXLINE];

static ssize_t my_read(int fd, char* ptr) {
	if (read_cnt <= 0) { /* no more buffer in read_buf array */
again:
		if ((read_cnt = read(fd, read_buf, sizeof(read_buf))) < 0) {
			if (errno == EINTR) /* signal */
				goto again;
			return -1;
		} else if (read_cnt == 0) /* eof */
			return 0;

		read_ptr = read_buf; /* rewind pointer */
	}

	--read_cnt;
	*ptr = *read_ptr++;
	return 1;
}


/* NOTE: non-thread-safe */
ssize_t readline(int fd, void* vptr, size_t maxlen) {
	char* ptr = vptr;
	ssize_t n;
	for (n = 1; n < maxlen; n++) {
		ssize_t rc;
		char c;
		if ((rc = my_read(fd, &c)) == 1) {
			*ptr++ = c;
			if (c == '\n') /* newline is stored, like fgets() */
				break;
		} else if (rc == 0) { /* a little diff with UNPv3: eof, we append a newline like some text editors */
			*ptr++ = '\n';
			break;
		} else {
			return -1;
		}
	}

	*ptr = '\0';
	return n;
}
