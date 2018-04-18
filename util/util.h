/*
 * util.h
 * codes from UNPv3
 */
#ifndef NETMODEL_UTIL_H
#define NETMODEL_UTIL_H

#include <unistd.h>

ssize_t readn(int fd, void* buffer, size_t nb);
ssize_t writen(int fd, const void* buffer, size_t nb);
ssize_t readline(int fd, void* buffer, size_t maxlen);

#endif
