/*
 * prefork processes with non-lock-accept call
 * codes from UNPv3
 */
#include <unistd.h>
#include <errno.h>
#include <signal.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/poll.h>
#include <sys/wait.h>

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "../util/util.h"

#define MAXLINE 4096
#define PORT 2018
#define PROCESSESCOUNT 5
pid_t pids[PROCESSESCOUNT];
typedef struct {
	pid_t 				child_pid; 			/* process id */
	int 					child_pipefd;		/* parent's stream pipe to/from child */
	int 					child_status;		/* 0 == ready */
	int 					child_count;		/* # connections already handled */
} Child;
Child children[PROCESSESCOUNT];

void handle_request(int sockfd);
void child_main(int listenfd);
pid_t make_process(int idx, int listenfd);
typedef void echo_sigfunc(int);
echo_sigfunc* echo_signal(int signo, echo_sigfunc* func);
void echo_sigint(int signo);
ssize_t read_fd(int fd, void* ptr, size_t nbytes, int* recvfd);
ssize_t write_fd(int fd, void* ptr, size_t nbytes, int sendfd);

ssize_t read_fd(int fd, void* ptr, size_t nbytes, int* recvfd) {
	/* keep chars array align with struct cmsghdr */
	union {
		struct cmsghdr cm;
		char control[CMSG_SPACE(sizeof(int))];
	} control_un;

	struct msghdr msg;
	msg.msg_control = control_un.control;
	msg.msg_controllen = sizeof(control_un.control);
	msg.msg_name = NULL;
	msg.msg_namelen = 0;

	struct iovec iov[1];
	iov[0].iov_base = ptr;
	iov[0].iov_len = nbytes;

	msg.msg_iov = iov;
	msg.msg_iovlen = 1;

	ssize_t n;
	if ((n = recvmsg(fd, &msg, 0)) <= 0) {
		return n;
	}

	struct cmsghdr* cmptr;
	if ((cmptr = CMSG_FIRSTHDR(&msg)) != NULL &&
			 cmptr->cmsg_len == CMSG_LEN(sizeof(int))) {
		if (cmptr->cmsg_level != SOL_SOCKET) {
			fprintf(stderr, "control level != SOL_SOCKET\n");
			exit(1);
		}
		if (cmptr->cmsg_type != SCM_RIGHTS) {
			fprintf(stderr, "control type != SCM_RIGHTS\n");
			exit(1);
		}

		*recvfd = *((int*)CMSG_DATA(cmptr));
	} else {
		*recvfd = -1;
	}

	return n;
}

ssize_t write_fd(int fd, void* ptr, size_t nbytes, int sendfd) {
	/* keep chars array control align with struct cmsghdr */
	union {
		struct cmsghdr cm;
		char control[CMSG_SPACE(sizeof(int))];
	} control_un;

	struct msghdr msg;
	msg.msg_control = control_un.control;
	msg.msg_controllen = sizeof(control_un.control);

	struct cmsghdr* cmptr;
	cmptr = CMSG_FIRSTHDR(&msg);
	cmptr->cmsg_len = CMSG_LEN(sizeof(int));
	cmptr->cmsg_level = SOL_SOCKET;
	cmptr->cmsg_type = SCM_RIGHTS;
	*((int*)CMSG_DATA(cmptr)) = sendfd;

	msg.msg_name = NULL;
	msg.msg_namelen = 0;

	struct iovec iov[1];
	iov[0].iov_base = ptr;
	iov[0].iov_len = nbytes;

	msg.msg_iov = iov;
	msg.msg_iovlen = 1;

	return sendmsg(fd, &msg, 0);
}

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
		/* recv connected fd from parent process */
		int connfd;
		char c;
		ssize_t n;
		if ((n = read_fd(STDERR_FILENO, &c, 1, &connfd)) == 0) {
			fprintf(stderr, "read_fd returned 0\n");
			exit(1);
		}

		if (connfd < 0) {
			fprintf(stderr, "no descriptor from read_fd\n");
			exit(1);
		}

		handle_request(connfd);

		close(connfd);

		write(STDERR_FILENO, "", 1); /* wake up poller to tell parent we are ready */
	}
}


pid_t make_process(int idx, int listenfd) {
	int sockfd[2];
	socketpair(AF_LOCAL, SOCK_STREAM, 0, sockfd);

	pid_t pid;
	/* loop forever until fork succeed */
	while ((pid = fork()) == -1) ;

	if (pid > 0) {
		/* parent process */
		close(sockfd[1]);
		children[idx].child_pid = pid;
		children[idx].child_pipefd = sockfd[0]; /* parent read/write sockfd[0] */
		children[idx].child_status = 0; /* ready */
		return pid;
	}

	/* child process */
	dup2(sockfd[1], STDERR_FILENO); /* child read/write sockfd[1] */
	close(sockfd[0]);
	close(sockfd[1]);
	close(listenfd);
	child_main(listenfd);

	return -1; /* disable warning: -Wreturn-type */
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
		pids[i] = make_process(i, listen_fd);
	}

	echo_signal(SIGINT, echo_sigint);

	/* poll for listening fd and socket pair fd */
	struct pollfd pollfds[PROCESSESCOUNT + 1];
	pollfds[0].fd = listen_fd;
	pollfds[0].events = POLLIN;
	pollfds[0].revents = 0;
	for (int i = 1; i < PROCESSESCOUNT + 1; ++i) {
		pollfds[i].fd = children[i - 1].child_pipefd;
		pollfds[i].events = POLLIN;
		pollfds[i].revents = 0;
	}

	int navail = PROCESSESCOUNT;
	for (;;) {
		int nready = poll(pollfds, PROCESSESCOUNT + 1, -1);

		if (pollfds[0].revents & POLLIN) {
			/* new client connection */
			int connfd = accept(listen_fd, NULL, NULL);

			/* find a free child process */
			int i = 0;
			for (; i < PROCESSESCOUNT; ++i) {
				if (children[i].child_status == 0) {
					break;
				}
			}

			/* if we cannot find a free child */
			if (i == PROCESSESCOUNT) {
				struct linger lingo;
				lingo.l_onoff = 1;
				lingo.l_linger = 0;
				setsockopt(connfd, SOL_SOCKET, SO_LINGER, &lingo, sizeof(lingo));
				/* send client a RST */
				close(connfd);

				/* or we cache this connfd and wait for free child process */
			}

			children[i].child_status = 1;
			children[i].child_count++;
			--navail;

			/* send connected fd to child process, let this free child process handle this
			 * connected fd */
			write_fd(children[i].child_pipefd, "", 1, connfd);
			close(connfd);

			/* once we handle all avaliable fds, we exit immediatly */
			if (--nready == 0) continue;
		}

		for (int i = 0; i < PROCESSESCOUNT; ++i) {
			if (pollfds[i + 1].revents & POLLIN) {
				children[i].child_status = 0;
				++navail;
			}

			/* once we handle all avaliable fds, we exit immediatly */
			if (--nready == 0) break;
		}
	}

	for (;;) {
		/* pause process process until user press Ctrl+C */
		pause();
	}

	exit(0);
}
