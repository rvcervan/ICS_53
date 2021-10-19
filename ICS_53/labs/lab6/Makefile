CC = gcc
DFLAGS := -g -DDEBUG
CFLAGS = -Wall -Werror -Iinclude $(DFLAGS) 
TEST_SRCS := $(wildcard tests/*.c)
TESTS := $(patsubst %.c,%,$(TEST_SRCS))

LDFLAGS := ../lib/allocator.o 
EFLAGS := $(DFLAGS) -I../include
PRG_SUFFIX=.out

export LDFLAGS
export EFLAGS
export PRG_SUFFIX

all: setup
	$(MAKE) -C tests
	mv tests/*$(PRG_SUFFIX) bin/

setup:
	@mkdir -p bin 

clean:
	rm -rf bin/ 
