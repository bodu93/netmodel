CC ?= clang
CFLAGS ?= -g -Wall -std=c11
LIBS += -lpthread

targets = echo.out
debug_folders = $(patsubst %, %.dSYM, $(targets))

define compile
	$(CC) $(CFLAGS) $^ -o $@ $(LIBS)
endef

all : $(targets)

echo.out : echo.c poller.c buffer.c
	$(compile)

.PHONY : all clean

clean :
	-rm -rf $(targets) $(debug_folders)
