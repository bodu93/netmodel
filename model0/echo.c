/*
 * accept + block io
 * codes from UNPv3
 */
#include <unistd.h>
#include <errno.h>
#include <signal.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>

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

	for (;;) {
		int connect_fd;
		connect_fd = accept(listen_fd, NULL, NULL);

		handle_request(connect_fd);

		close(connect_fd);
	}

	exit(0);
}
