CC ?= clang
CFLAGS ?= -g -Wall
LIBS += -lpthread

targets = echo_version_1.out echo_version_2.out
debug_folders = $(patsubst %, %.dSYM, $(targets))

vpath %.c  ../util
vpath %.c  .

define compile
	$(CC) $(CFLAGS) $^ -o $@ $(LIBS)
endef

all : $(targets)

echo_version_1.out : echo_version_1.c util.c
	$(compile)

echo_version_2.out : echo_version_2.c util.c blocking_queue.c
	$(compile)

.PHONY : all clean

clean :
	-rm -rf $(targets) $(debug_folders)
