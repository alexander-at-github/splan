CC=clang
CFLAGS=#-dNDEBUG # turns off assertions


BUILD_DIR=build
BIN_DIR=bin

LIB_PDDL31_DIR=libpddl31
LIB_PDDL31_BIN_DIR=$(LIB_PDDL31_DIR)/bin
LIB_PDDL31=$(LIB_PDDL31_BIN_DIR)/libpddl31.a

#all: $(LIB_PDDL31)
### TEST-START

ANTLR3.4_C_RUNTIME=antlr3c 	# linker will search the library path for a file
							# called libantlr3c.a (and libantlr3c.so?).

# link antlr3 runtime here? Maybe better in the parser.
bin/test: src/main_test.c build/planner.o $(BIN_DIR)
	$(CC) $(CFLAGS) -o $@ -I$(BIN_DIR) -I$(LIB_PDDL31_BIN_DIR) $< build/planner.o $(LIB_PDDL31) -l$(ANTLR3.4_C_RUNTIME)

build/planner.o: src/planner.c $(BUILD_DIR) $(LIB_PDDL31)
	$(CC) $(CFLAGS) -c -I$(LIB_PDDL31_BIN_DIR) -o $@ $<
### TEST-END

$(LIB_PDDL31):
	cd $(LIB_PDDL31_DIR) && make all

$(BUILD_DIR):
	mkdir -p $@

$(BIN_DIR):
	mkdir -p $@

clean:
	cd $(LIB_PDDL31_DIR) && make clean
	rm -rf $(BUILD_DIR) $(BIN_DIR)
