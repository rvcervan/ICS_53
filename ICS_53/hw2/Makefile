CC:=gcc
CFLAGS:=-g
BIN_NAME:=FMCipher
SRC_DIR:=src
INCLUDE_DIR:=include
BIN_DIR:=bin
LIB:= $(shell find lib -type f -name *.o)
PRG_SUFFIX:=.out

# These are called rules. Rules have targets (what to make), and prerequisites (what needs to exist).
# Make won't build a target if it's prerequisites haven't changed; it's smart enough not to re-do it's work. The syntax for rules is:
# target(s): prerequisite(s)
# \trecipe (how to make the target)
# ...
# The tab character is important (no spaces), and the recipe is a series of shell command(s)
all: setup bin/p1test_main bin/linkedlisttest_main FMCipher 
	
FMCipher: src/hw2.c src/hw2_ec.c src/helpers2.c src/linkedlist.c src/FMCipher.c
	$(CC) -I $(INCLUDE_DIR) $(CFLAGS) $(LIB) $(SRC_DIR)/FMCipher.c  $(SRC_DIR)/linkedlist.c $(SRC_DIR)/helpers2.c $(SRC_DIR)/hw2.c $(SRC_DIR)/hw2_ec.c -o $(BIN_DIR)/FMCipher

#compile all the files into object files
bin/p1test_main: setup src/hw2.c src/helpers2.c src/linkedlist.c tests/p1test_main.c
	$(CC) -I $(INCLUDE_DIR) $(CFLAGS) $(LIB)  $(SRC_DIR)/linkedlist.c $(SRC_DIR)/helpers2.c $(SRC_DIR)/hw2.c tests/p1test_main.c -o bin/p1_testing_main

bin/linkedlisttest_main: setup src/hw2.c src/helpers2.c src/linkedlist.c tests/linkedlisttest_main.c
	$(CC) -I $(INCLUDE_DIR) $(CFLAGS) $(LIB)  $(SRC_DIR)/linkedlist.c $(SRC_DIR)/helpers2.c $(SRC_DIR)/hw2.c tests/linkedlisttest_main.c -o bin/linkedlisttest_main

setup:
	@mkdir -p $(BIN_DIR)

# Standard (ish) rule to clean out files we'd like to re-generate
clean:
	rm -rf $(BIN_DIR) 

# By default, make will assume each target is a file. This means that it won't actually run the clean target if a file called ```clean``` exists.
# The following special target overrides this behavior; it's prerequisites are always run.
.PHONY: clean
