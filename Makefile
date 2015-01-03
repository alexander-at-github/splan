CC=gcc #clang

CFLAGS=-D_DEFAULT_SOURCE -std=c11 #-g -pg -O3 -DNDEBUG #-g # turns off assertions

SRC_DIR=src
BUILD_DIR=build
BIN_DIR=bin

LIB_PDDL31_DIR=libpddl31
LIB_PDDL31_BIN_DIR=$(LIB_PDDL31_DIR)/bin
LIB_PDDL31=$(LIB_PDDL31_BIN_DIR)/libpddl31.a
TOOL_DIR=tool

#all: $(LIB_PDDL31)
### EXPERIMENTAL-START

ANTLR3.4_C_RUNTIME=antlr3c 	# linker will search the library path for a file
							# called libantlr3c.a (and libantlr3c.so?).

# Simple Plan (splan)
bin/splan: src/main.c build/planner.o build/utils.o $(BIN_DIR)
	$(CC) $(CFLAGS) -o $@ -I$(BIN_DIR) -I$(LIB_PDDL31_BIN_DIR) $< build/planner.o build/utils.o $(LIB_PDDL31) -l$(ANTLR3.4_C_RUNTIME)

## link antlr3 runtime here? Maybe better in the parser.
#bin/test: src/main_test.c build/planner.o $(BIN_DIR)
#	$(CC) $(CFLAGS) -o $@ -I$(BIN_DIR) -I$(LIB_PDDL31_BIN_DIR) $< build/planner.o $(LIB_PDDL31) -l$(ANTLR3.4_C_RUNTIME)

#build/planner.o: src/planner.c $(BUILD_DIR) $(LIB_PDDL31)
#	$(CC) $(CFLAGS) -c -I$(LIB_PDDL31_BIN_DIR) -o $@ $<

build/%.o: src/%.c $(BUILD_DIR) $(LIB_PDDL31)
	$(CC) $(CFLAGS) -c -I$(LIB_PDDL31_BIN_DIR) -o $@ $<


### EXPERIMENTAL-END

$(LIB_PDDL31):
	cd $(LIB_PDDL31_DIR) && make all

$(BUILD_DIR):
	mkdir -p $@

$(BIN_DIR):
	mkdir -p $@

clean:
	cd $(LIB_PDDL31_DIR) && make clean
	rm -rf $(BUILD_DIR) $(BIN_DIR)

### Testing ###

#bin/test_grounder : test/grounder.c src/grounder.c $(LIB_PDDL31) $(BIN_DIR)
#	$(CC) $(CFLAGS) -o $@ -I$(SRC_DIR) -I$(TOOL_DIR) -I$(LIB_PDDL31_BIN_DIR) test/grounder.c src/grounder.c $(LIB_PDDL31) -l$(ANTLR3.4_C_RUNTIME)

bin/test_utils : test/utils.c src/utils.c $(LIB_PDDL31) $(BIN_DIR)
	$(CC) $(CFLAGS) -o $@ -I$(SRC_DIR) -I$(TOOL_DIR) -I$(LIB_PDDL31_BIN_DIR) test/utils.c src/utils.c $(LIB_PDDL31) -l$(ANTLR3.4_C_RUNTIME)

#bin/test_state: test/state.c src/state.c src/utils.c $(LIB_PDDL31) $(BIN_DIR)
#	$(CC) $(CFLAGS) -o $@ -I$(SRC_DIR) -I$(TOOL_DIR) -I$(LIB_PDDL31_BIN_DIR) test/state.c src/state.c src/utils.c $(LIB_PDDL31) -l$(ANTLR3.4_C_RUNTIME)

bin/test_planner: test/planner.c src/planner.c src/utils.c $(LIB_PDDL31) $(BIN_DIR)
	$(CC) $(CFLAGS) -o $@ -I$(SRC_DIR) -I$(TOOL_DIR) -I$(LIB_PDDL31_BIN_DIR) test/planner.c src/planner.c src/utils.c $(LIB_PDDL31) -l$(ANTLR3.4_C_RUNTIME)
