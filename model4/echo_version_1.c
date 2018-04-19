/*
 * accept in every precreated threads with pthread mutex lock.
 * Every thread run a busy loop until user send process a quit signal(Ctrl+C)
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

volatile sig_atomic_t quit;
void sigint(int signo) {
	(void)signo;

	quit = 1;
	exit(0);
}

pthread_mutex_t accept_lock = PTHREAD_MUTEX_INITIALIZER;
void* thread_routine(void* thdarg) {
	pthread_detach(pthread_self());
	int listenfd = (uintptr_t)thdarg;
	/*
	 * accept in every thread.
	 * with pthread mutex locked.
	 * avoid thundering herd problem.
	 */
	while (!quit) { /* is quit a atomic variable on x86?? */
		pthread_mutex_lock(&accept_lock);
		int connfd = accept(listenfd, NULL, NULL);
		pthread_mutex_unlock(&accept_lock);

		handle_request(connfd);

		close(connfd);
	}

	return (void*)0;
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

	for (int i = 0; i < THREADSCOUNT; ++i) {
		/* thread per connection */
		pthread_t tid;
		pthread_create(&tid, NULL, &thread_routine, (void*)(uintptr_t)listen_fd);
	}

	/* wait user's quit signal */
	for (;;) {
		pause();
	}

	exit(0);
}
