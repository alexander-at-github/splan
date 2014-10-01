CC=clang
CFLAGS=

BUILD_DIR=build
BIN_DIR=bin

LIB_PDDL31_DIR=libpddl31
LIB_PDDL31_BIN_DIR=$(LIB_PDDL31_DIR)/bin
LIB_PDDL31=$(LIB_PDDL31_BIN_DIR)/libpddl31.a

#all: $(LIB_PDDL31)
###

build/test-dev.o: src/planner.c $(BUILD_DIR) $(LIB_PDDL31)
	$(CC) $(CFLAGS) -c -I$(LIB_PDDL31_BIN_DIR) -o $@ $<
###

$(LIB_PDDL31):
	cd $(LIB_PDDL31_DIR) && make all

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

clean:
	cd $(LIB_PDDL31_DIR) && make clean
	rm -rf $(BUILD_DIR) $(BIN_DIR)
