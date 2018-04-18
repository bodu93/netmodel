/*
 * prefork processes with file-locked accept(2) call
 * from UNPv3
 */
#include <unistd.h>
#include <errno.h>
#include <signal.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/wait.h>

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "accept_lock.h"
#include "../util/util.h"

#define MAXLINE 4096
#define PORT 2018
#define PROCESSESCOUNT 5
pid_t pids[PROCESSESCOUNT];

void handle_request(int sockfd);
void child_main(int listenfd);
pid_t make_process(int listenfd);
typedef void echo_sigfunc(int);
echo_sigfunc* echo_signal(int signo, echo_sigfunc* func);
void echo_sigint(int signo);

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


void child_main(int listenfd) {
	/* loop forever until parent process kill us */
	for (;;) {
		/* accept in every process */
		accept_lock_wait();
		/* no thundering herd problem any more */
		int connfd = accept(listenfd, NULL, NULL);
		accept_lock_release();
		handle_request(connfd);
		close(connfd);
	}
}


pid_t make_process(int listenfd) {
	pid_t pid;
	/* forkever until success */
	while ((pid = fork()) == -1) ;

	if (pid > 0) {
		/* parent process */
		return pid;
	}

	/* run a busy loop */
	child_main(listenfd);

	return -1; /* just disable warning */
}

echo_sigfunc* echo_signal(int signo, echo_sigfunc* func) {
	struct sigaction act;
	act.sa_handler = func;
	sigemptyset(&act.sa_mask);
	act.sa_flags = 0;

	struct sigaction old_act;
	if (sigaction(signo, &act, &old_act) < 0) return SIG_ERR;
	return old_act.sa_handler;
}

void echo_sigint(int signo) {
	for (int i = 0; i < PROCESSESCOUNT; ++i) {
		kill(pids[i], SIGTERM);
	}

	/* wait for all children */
	while (wait(NULL) > 0) ;

	if (errno != ECHILD) {
		/* NOTE: non-async-signal-safe */
		fprintf(stderr, "wait(2) error\n");
		exit(1);
	}

	exit(0);
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

	for (int i = 0; i < PROCESSESCOUNT; ++i) {
		accept_lock_init("/tmp/lock.XXXXXX");
		pids[i] = make_process(listen_fd);
	}

	echo_signal(SIGINT, echo_sigint);
	for (;;) {
		/* pause process process until user press Ctrl+C */
		pause();
	}

	exit(0);
}
