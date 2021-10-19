CC:=gcc
CFLAGS:=-g
BIN_NAME:=formattxt
SRC_DIR:=src
INCLUDE_DIR:=include
BIN_DIR:=bin

# These are called rules. Rules have targets (what to make), and prerequisites (what needs to exist).
# Make won't build a target if it's prerequisites haven't changed; it's smart enough not to re-do it's work. The syntax for rules is:
# target(s): prerequisite(s)
# \trecipe (how to make the target)
# ...
# The tab character is important (no spaces), and the recipe is a series of shell command(s)
default: src/hw1.c src/helpers1.c
	mkdir -p $(BIN_DIR)
	$(CC) -I $(INCLUDE_DIR) $(CFLAGS) $(SRC_DIR)/hw1.c $(SRC_DIR)/helpers1.c -o $(BIN_DIR)/formattxt

# Standard (ish) rule to clean out files we'd like to re-generate
clean:
	rm -rf $(BIN_DIR)

# By default, make will assume each target is a file. This means that it won't actually run the clean target if a file called ```clean``` exists.
# The following special target overrides this behavior; it's prerequisites are always run.
.PHONY: clean
