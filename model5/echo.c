/*
 * single thread reactor model
 *
 * using poll in a busy loop to waiting multiple fds, once
 * readable/writable event arrive, use non-block io handle it
 *
 * codes from UNPv3
 */
#include <unistd.h>
#include <errno.h>
#include <signal.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <poll.h>
#include <limits.h>

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "../util/util.h"

#define MAXLINE 4096

void handle_request(int sockfd) {
	ssize_t n;
	char buf[MAXLINE];
again:
	while ((n = read(sockfd, buf, MAXLINE)) > 0) {
		writen(sockfd, buf, n);
		//write(sockfd, buf, n);
	}

	if (n < 0 && errno == EINTR) /* interrupted by a signal */
		goto again;
	else if (n < 0) { /* RST ?? */
		fprintf(stderr, "read(2) error\n");
	} else {
		fprintf(stdout, "disconnected\n");
	}
}

#define PORT 2018

int main() {
	int listenfd = socket(AF_INET, SOCK_STREAM, 0);

	struct sockaddr_in serv_addr;
	bzero(&serv_addr, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(PORT);
	serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);

	int ok = 1;
	socklen_t len = sizeof(ok);
	setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &ok, len);

	bind(listenfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr));
	listen(listenfd, SOMAXCONN);

	struct pollfd clients[OPEN_MAX];
	clients[0].fd = listenfd;
	clients[0].events = POLLIN;
	clients[0].revents = 0;
	for (int i = 1; i < OPEN_MAX; ++i) {
		clients[i].fd = -1;
	}

	int maxidx = 0;
	/* run a busy loop */
	for (;;) {
		int nready = poll(clients, maxidx + 1, -1);

		if (clients[0].revents & POLLIN) {
			int connfd = accept(listenfd, NULL, NULL);

			int i = 0;
			for (i = 1; i < OPEN_MAX; ++i) {
				if (clients[i].fd < 0) {
					clients[i].fd = connfd; /* register in poller */
					break;
				}
			}
			if (i == OPEN_MAX) {
				/* trivial: exit */
				fprintf(stderr, "too many clients\n");
				exit(1);
			}
			clients[i].events = POLLIN;
			if (i > maxidx) maxidx = i;

			if (--nready <= 0) continue;
		}

		char buf[MAXLINE];
		for (int i = 1; i <= maxidx; ++i) {
			int sockfd;
			if ((sockfd = clients[i].fd) < 0)
				continue;

			if (clients[i].revents & (POLLIN | POLLERR)) {
				ssize_t n;
				if ((n = read(sockfd, buf, MAXLINE)) < 0) {
					if (errno == ECONNRESET) {
						close(sockfd);
						clients[i].fd = -1; /* unregister from poller */
					} else {
						fprintf(stderr, "read error\n");
						exit(1);
					}
				} else if (n == 0) {
					close(sockfd);
					clients[i].fd = -1; /* unregister from poller */
				} else {
					writen(sockfd, buf, n);
				}

				if (--nready <= 0) break;
			}
		}
	}

	exit(0);
}
