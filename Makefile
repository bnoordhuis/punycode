CC	= gcc
CFLAGS	= -Wall -Wextra -ansi

CFLAGS_DEBUG	= -g -O0
CFLAGS_RELEASE	= -s -O2

OBJS	= punycode.o
TESTS	= test-punycode

.PHONY: $(TESTS)

all:	release

debug:	CFLAGS += $(CFLAGS_DEBUG)
debug:	build

release:	CFLAGS += $(CFLAGS_RELEASE)
release:	build

build:	$(OBJS)

clean:
	rm -f $(OBJS) $(TESTS)

test:	$(TESTS)

$(TESTS):	CFLAGS += $(CFLAGS_DEBUG)

test-punycode:	punycode.c
	$(CC) $(CFLAGS) -o $(@) $(@).c $<
	./$(@)

punycode.o:	punycode.c punycode.h
