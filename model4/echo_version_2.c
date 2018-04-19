/*
 * accept in main thread and transfer
 * connected fd through a bounded blocking queue
 * codes from UNPv3
 */
#include <unistd.h>
#include <errno.h>
#include <signal.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <pthread.h>

#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "blocking_queue.h"
#include "../util/util.h"

#define MAXLINE 4096
void handle_request(int sockfd) {
	ssize_t n;
	char buf[MAXLINE];
again:
	while ((n = read(sockfd, buf, MAXLINE)) > 0) {
		//writen(sockfd, buf, n);
		write(sockfd, buf, n);
	}

	if (n < 0 && errno == EINTR) { /* interrupted by a signal */
		goto again;
	}
	else if (n < 0) { /* RST ?? */
		fprintf(stderr, "read(2) error\n");
	} else {
		fprintf(stdout, "disconnected\n");
	}
}

#define PORT 2018
#define THREADSCOUNT 5
pthread_t tids[THREADSCOUNT];
struct bq* g_fdqueue;

volatile sig_atomic_t quit;
void sigint(int signo) {
	(void)signo;

	quit = 1;

	/* wait all threads quit */
	for (int i = 0; i < THREADSCOUNT; ++i) {
		pthread_join(tids[i], NULL);
	}
	bq_release(g_fdqueue);

	exit(0);
}

void* thread_routine(void* thd_arg) {
	(void)thd_arg;

	/* run a busy loop to get connected fd from fd queue */
	while (!quit) {
		int connfd = bq_get(g_fdqueue);
		/* printf("thread got connected fd from queue: %d\n", connfd); */
		handle_request(connfd);
		close(connfd);
	}

	return (void*)0; /* disable warning: -Wreturn-type */
}

int main() {
	int listen_fd = socket(AF_INET, SOCK_STREAM, 0);

	struct sockaddr_in serv_addr;
	bzero(&serv_addr, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(PORT);
	serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);

	int ok = 1;
	socklen_t len = sizeof(ok);
	setsockopt(listen_fd, SOL_SOCKET, SO_REUSEADDR, &ok, len);

	bind(listen_fd, (struct sockaddr*)&serv_addr, sizeof(serv_addr));
	listen(listen_fd, 5);

	quit = 0;
	signal(SIGINT, sigint);

	g_fdqueue = bq_init(THREADSCOUNT);
	for (int i = 0; i < THREADSCOUNT; ++i) {
		pthread_create(&tids[i], NULL, &thread_routine, NULL);
	}

	/* main thread run a busy loop to accept(2) request */
	for (;;) {
		int connect_fd = accept(listen_fd, NULL, NULL);
		bq_put(g_fdqueue, connect_fd);
	}

	/* wait user's quit signal */
	for (;;) {
		pause();
	}

	exit(0);
}
