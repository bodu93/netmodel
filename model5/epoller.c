/*
 * epoller.c
 * using epoll(2) as as poller backend
 * codes from skynet(skynet-src/socket_epoll.h) with a little changes
 *
 * The MIT License (MIT)
 *
 * Copyright (c) 2012-2017 codingnow.com
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of
 * this software and associated documentation files (the "Software"), to deal in
 * the Software without restriction, including without limitation the rights to
 * use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
 * the Software, and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */
#include "poller.h"

#include <sys/epoll.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

bool poller_valid(int pfd) {
	return pfd != -1;
}

int poller_init(int size) {
	/* since Linux2.6.8, ignore size of epoll_create */
	return epoll_create(size);
}

void poller_release(int pfd) {
	close(pfd);
}

int poller_add(int pfd, int sockfd, void* ud) {
	struct epoll_event ev;
	ev->data->ptr = ud;
	ev->events = EPOLLIN;
	return epoll_ctl(pfd, EPOLL_CTL_ADD, sockfd, &ev);
}

int poller_del(int pfd, int sockfd) {
	return epoll_ctl(pfd, EPOLL_CTL_DEL, sockfd, NULL);
}

int poller_enable_write(int pfd, int sockfd, void* ud, bool enable) {
	struct epoll_event ev;
	ev.data.ptr = ud;
	ev.events = EPOLLIN | (enable ? EPOLLOUT : 0);
	return epoll_ctl(pfd, EPOLL_CTL_MOD, sockfd, &ev);
}

int poller_wait(int pfd, struct revent* e, int maxlen) {
	struct epoll_event evs[maxlen];
	/* bzero(evs, sizeof(evs); */
	int nready = epoll_wait(pfd, &evs, maxlen, -1);

	for (int i = 0; i < maxlen; ++i) {
		e[i].ptr = evs[i].data.ptr;
		uint32_t flag = evs[i].events;
		e[i].readable = (flag & (EPOLLIN | EPOLLHUP)) != 0;
		e[i].writable = (flag & EPOLLOUT) != 0;
		e[i].erring = (flag & EPOLLERR) != 0;
	}

	return nready;
}
