CC=clang
AR=ar

BUILD_DIR=build
BIN_DIR=bin
SRC_DIR=src
TOOL_DIR=tool

ANTLR3_C_FILES=$(BUILD_DIR)/pddl31Lexer.c $(BUILD_DIR)/pddl31Lexer.h $(BUILD_DIR)/pddl31Parser.c $(BUILD_DIR)/pddl31Parser.h $(BUILD_DIR)/pddl31_pddl31core.c $(BUILD_DIR)/pddl31_pddl31core.h $(BUILD_DIR)/pddl31.tokens $(BUILD_DIR)/pddl31core.tokens
LEXER_OBJ_FILE=$(BUILD_DIR)/pddl31Lexer.o
PARSER_OBJ_FILE=$(BUILD_DIR)/pddl31Parser.o
LIB_FILE=$(BIN_DIR)/libpddl31.a

#all: $(ANTLR3_C_FILES)
#all: $(LEXER_OBJ_FILE) $(PARSER_OBJ_FILE)
all: $(LIB_FILE)

$(BUILD_DIR):
	mkdir $@

$(ANTLR3_C_FILES): $(SRC_DIR)/antlr3/pddl31.g $(SRC_DIR)/antlr3/pddl31core.g $(BUILD_DIR)
	# The -make switch should cause the command to only build if generated
	# files are older than grammar.
	java -jar $(TOOL_DIR)/antlr-3.4-complete.jar -make -fo $(BUILD_DIR) $<

$(LEXER_OBJ_FILE): $(BUILD_DIR)/pddl31Lexer.c $(BUILD_DIR)/pddl31Lexer.h
	$(CC) -c -I$(BUILD_DIR) -o $@ $<

$(PARSER_OBJ_FILE): $(BUILD_DIR)/pddl31Parser.c $(BUILD_DIR)/pddl31Parser.h
	$(CC) -c -I$(BUILD_DIR) -o $@ $<

$(LIB_FILE): $(LEXER_OBJ_FILE) $(PARSER_OBJ_FILE)
	mkdir $(BIN_DIR)
	$(AR) rcs $@ $^
	cp $(BUILD_DIR)/pddl31Lexer.h $(BUILD_DIR)/pddl31Parser.h $(BIN_DIR)

clean:
	rm -rf $(BUILD_DIR) $(BIN_DIR)
