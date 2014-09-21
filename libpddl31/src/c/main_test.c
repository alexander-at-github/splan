//#include "pddl31Lexer.h"
//#include "pddl31Parser.h"
#include "libpddl31.h"
#include "pddl31structs.h"

int
main (int agrc, char **argv)
{
    char *filename = "test_instances/blocks_domain_altered.pddl";
    struct domain *domain = libpddl31_domain_parse(filename);
    libpddl31_domain_print(domain);
    libpddl31_domain_free(domain);

    /*
    // Name of the input file.
    pANTLR3_UINT8 fName; 
    // ANTLR3 character input stream
    pANTLR3_INPUT_STREAM input;

    // The lexer
    ppddl31Lexer lxr;

    // The token stream is produced by the ANTLR3 generated lexer.
    pANTLR3_COMMON_TOKEN_STREAM tstream;

    // The parser. It accepts a token stream.
    ppddl31Parser psr;

    // The parser produces an AST, which is returned as a member of the
    // return type of the starting rule.
    ////pddl31Parser_domain_return pddlAST;

    // The tree nodes are managed by a tree adaptor, which doles out the
    // nodes upon request.
    pANTLR3_COMMON_TREE_NODE_STREAM nodes;

    // The tree parser
    //TODO

    // Set input file name.
    fName = (pANTLR3_UINT8)"test_instances/blocks_domain_altered.pddl";
    // Create input stream from file name.
    input = antlr3FileStreamNew(fName, 8); // TODO: What is the second argument?
    if (input == NULL) {
        ANTLR3_FPRINTF(stderr, "Unable to open file %s due to malloc() "
                               "failure1\n", (char *)fName);
    }

    // Create a new lexer and set lexer input to input-stream
    lxr = pddl31LexerNew(input);
    if (lxr ==NULL) {
        ANTLR3_FPRINTF(stderr, "Unable to create the lexer due to malloc() "
                               "failure1\n");
        exit(ANTLR3_ERR_NOMEM);
    }

    // Set token stream source to lexer.
    // Actual lexing the file will only happen later.
    //
    // ANTLR3_API pANTLR3_COMMON_TOKEN_STREAM  antlr3CommonTokenStreamSourceNew        (ANTLR3_UINT32 hint, pANTLR3_TOKEN_SOURCE source);
    //
    // pLexer is of type struct ANTLR3_LEXER_struct which is defined
    // in libantlr3c-3.4/include/antlr3lexer.h:76
    tstream = antlr3CommonTokenStreamSourceNew(ANTLR3_SIZE_HINT,
                                               TOKENSOURCE(lxr));
                                               //lxr->pLexer->tokSource);
    if (tstream == NULL) {
        ANTLR3_FPRINTF(stderr, "Out of memory trying to allocate token "
                               "stream\n");
        exit(ANTLR3_ERR_NOMEM);
    }

    // Create the parser
    psr = pddl31ParserNew(tstream);
    if (psr == NULL) {
        ANTLR3_FPRINTF(stderr, "Out of memory trying to allocate parser\n");
        exit(ANTLR3_ERR_NOMEM);
    }

    // Run.
    // The first parameter to a pseudo-method is the object itself.
    // 'domain' is the starting rule in the grammar.
    //
    // Attention: psr->pParser->rec->state->errorCount!!
    // not psr->pParser->rec->errorCount (as is stated in the example in the
    // documentation)
    ////pddlAST = psr->domain(psr);
    struct domain *domain = psr->domain(psr);
    if (psr->pParser->rec->state->errorCount > 0) {
        ANTLR3_FPRINTF(stderr, "The parser returned %d errors, tree walking "
                               "aborted.\n",
                               psr->pParser->rec->state->errorCount);
    } else {
        // No error
        printf("Debug: parser returned domain with name %s\n", domain->name);

        printf("Debug: number of constants: %d\n", domain->numOfConstants);
        printf("Debug: first constant name: %s\n", domain->constants[0].name);
        
        printf("Debug: number of predicates: %d\n", domain->numOfPredicates);
        printf("Debug: first predicate: '%s' with %d parameters\n",
               domain->predicates[0].name,
               domain->predicates[0].numOfParameters);

        // SIZE HINT WILL SOON BE DEPRECATED!!
        ////nodes = antlr3CommonTreeNodeStreamNewTree(pddlAST.tree,
        ////                                          ANTLR3_SIZE_HINT);
 
        // Tree parsers are given a common tree node stream (or your override)
        //treePsr = LangDumpDeclNew(nodes);

        //treePsr->decl(treePsr);
 
        ////nodes->free(nodes);
        ////nodes   = NULL;
        //treePsr->free(treePsr);
        //treePsr = NULL;
    }

    // Free pseudo-objects in the reverse order we created them.
    psr->free(psr);
    psr=NULL;
    tstream->free(tstream);
    tstream=NULL;
    lxr->free(lxr);
    lxr=NULL;
    input->close(input);
    input=NULL;
    */
}
