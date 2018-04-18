CC ?= clang
CFLAGS ?= -g -Wall

targets = echo_version_1.out echo_version_2.out echo_version_3.out echo_version_4.out
debug_folders = $(patsubst %, %.dSYM, $(targets))

vpath %.c  ../util
vpath %.c  .

define compile
	$(CC) $(CFLAGS) $^ -o $@
endef

all : $(targets)

echo_version_1.out : echo_version_1.c util.c
	$(compile)

echo_version_2.out : echo_version_2.c util.c accept_file_lock.c
	$(compile)

echo_version_3.out : echo_version_3.c util.c accept_mutex_lock.c
	$(compile) -lpthread

echo_version_4.out : echo_version_4.c util.c
	$(compile)

.PHONY : all clean

clean :
	-rm -rf $(targets) $(debug_folders)
