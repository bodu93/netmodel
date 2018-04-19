/*
 * poller.c
 * using poll(2) as poller backend
 */
#include "poller.h"

#include <poll.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <assert.h>

struct poller_fds {
	struct pollfd* pfds;
	int size;
	int last;
};
static struct poller_fds* A;

bool poller_valid(int pfd) {
	return pfd != -1;
}

static struct pollfd* pfd;
int poller_init(int size) {
	if (size <= 0) return -1;

	A = (struct poller_fd*)malloc(sizeof(*A));
	assert(A);
	A->pfds = (struct pollfd*)malloc(sizeof(struct pollfd) * size);
	assert(A->pfds);
	A->size = size;
	A->last = 0;

	return 0;
}

void poller_release(int pfd) {
	/* we use poll(2), ignore pfd */
	(void)pfd;

	free(A->pfds);
	free(A);
}

/*
 * struct pollfd {
 * 		int		fd;
 * 		short events;
 * 		short revents;
 * };
 */
static void extend() {
	struct pollfd* newfds =
		(struct pollfd*)malloc(sizeof(*newfds) * (A->size * 2));
	for (int i = 0; i < A->size; ++i) {
		newfds[i].fd = A->pfds[i].fd;
		newfds[i].events = A->pfds[i].events;
		newfds[i].revents = A->pfds[i].revents;
	}
	free(A->pfds);
	A->pfds = newfds;
	A->size = A->size * 2;
}

int poller_add(int pfd, int sockfd, void* ud) {
	(void)pfd;
	(void)ud;
	if (A->size == A->last) {
		extend();
	}
	int idx = A->last++;
	A->pfds[idx].fd = sockfd;
	A->pfds[idx].events = POLLIN;
	A->pfds[idx].revents = 0;

	return idx;
}

static void swap(int* lhs, int *rhs) {
	int tmp = *lhs;
	*lhs = *rhs;
	*rhs = tmp;
}

static void swap_pfd(struct pfd* lhs, struct pfd* rhs) {
	swap(&lhs->fd, &rhs->fd);
	swap(&lhs->events, &rhs->events);
	swap(&lhs->revents, &rhs->revents);
}

static int index(int sockfd) {
	int idx = -1;
	for (int i = 0; i < A->last; ++i) {
		if (A->pfds[i].fd == sockfd ||
				-A->pfd[i].fd - 1 == sockfd) {
			idx = i;
			break;
		}
	}

	return idx;
}

int poller_del(int pfd, int sockfd) {
	(void)pfd;
	if (A->last <= 0) return -1;

	/* FIXME: sequential find, optimize later? */
	int idx = index(sockfd);
	if (idx != -1) {
		if (idx != A->last - 1) {
			int last = A->last - 1;
			swap_pfd(&A->pfds[idx], &A->pfds[last]);
		}
		--A->last;
	}

	return idx;
}

int poller_enable_write(int pfd, int sockfd, void* ud, bool enable) {
	(void)pfd;
	(void)ud;
	int idx = index(sockfd);
	if (idx != -1) {
		A->pfds[idx].fd = sockfd;
		A->pfds[idx].events = POLLIN | (enable ? POLLOUT : 0);
	}

	return idx;
}

int poller_wait(int pfd, struct revent* e, int maxlen) {
	(void)pfd;

	int nready = poll(A->pfds, A->last, -1);

	for (int i = 0; i < maxlen; ++i) {
		e[i].ptr = (void*)(uintptr_t)(A->pfds[i].fd);
		short flag = A->pfds[i].revents;
		e[i].writable = (flag & POLLOUT) != 0;
		e[i].readable = (flag & (POLLIN | POLLHUP)) != 0;
		e[i].erring = (flag | POLLERR) != 0;
	}

	return nready;
}
