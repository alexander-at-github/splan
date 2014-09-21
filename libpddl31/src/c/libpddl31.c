#include <assert.h>

#include "libpddl31.h"
#include "pddl31Lexer.h"
#include "pddl31Parser.h"
#include "pddl31structs.h"

#define INPUT_STREAM_ENCODING 8

void free_parser_aux(   ppddl31Parser *psr,
                        pANTLR3_COMMON_TOKEN_STREAM *tstream,
                        ppddl31Lexer *lxr,
                        pANTLR3_INPUT_STREAM *input)
{
    // Free pseudo-objects in the reverse order we created them.
    if (*psr != NULL) {
        (*psr)->free(*psr);
        *psr=NULL;
    }
    if (*tstream != NULL) {
        (*tstream)->free(*tstream);
        *tstream=NULL;
    }
    if (*lxr != NULL) {
        (*lxr)->free(*lxr);
        *lxr=NULL;
    }
    if (*input != NULL) {
        (*input)->close(*input);
        *input=NULL;
    }
}

struct domain *libpddl31_domain_parse(char *filename)
{
    // First set all the pointers to NULL, so we know what to free
    // later.

    // Name of the input file.
    pANTLR3_UINT8 fName; 
    // ANTLR3 character input stream
    pANTLR3_INPUT_STREAM input = NULL;

    // The lexer
    ppddl31Lexer lxr = NULL;

    // The token stream is produced by the ANTLR3 generated lexer.
    pANTLR3_COMMON_TOKEN_STREAM tstream = NULL;

    // The parser. It accepts a token stream.
    ppddl31Parser psr = NULL;



    // Set name of the input file.
    fName = (pANTLR3_UINT8) filename;
    // Set character input stream
    // The second agrument is the encoding. For more informaion see:
    // * file:///home/alexander/uni/bac/antlr3/runtime_c/libantlr3c-3.4/api/
    //          struct_a_n_t_l_r3___i_n_p_u_t___s_t_r_e_a_m__struct.html
    //          #acce3c7aa90181c9e636829746ad666b0
    // * /home/alexander/uni/bac/antlr3/runtime_c/libantlr3c-3.4/include/
    //   antlr3input.h
    input = antlr3FileStreamNew(fName, INPUT_STREAM_ENCODING);
    if (input == NULL) {
        ANTLR3_FPRINTF(stderr, "Unable to open file %s due to malloc() "
                               "failure1\n", (char *)fName);
        free_parser_aux(&psr, &tstream, &lxr, &input);
        return NULL;
    }

    // Create a new lexer and set lexer input to input-stream
    lxr = pddl31LexerNew(input);
    if (lxr ==NULL) {
        ANTLR3_FPRINTF(stderr, "Unable to create the lexer due to malloc() "
                               "failure1\n");
        free_parser_aux(&psr, &tstream, &lxr, &input);
        return NULL;
    }

    // Set token stream source to lexer.
    // Actual lexing the file will only happen later.
    //
    // ANTLR3_API pANTLR3_COMMON_TOKEN_STREAM
    // antlr3CommonTokenStreamSourceNew (ANTLR3_UINT32 hint,
    //                                   pANTLR3_TOKEN_SOURCE source);
    tstream = antlr3CommonTokenStreamSourceNew(ANTLR3_SIZE_HINT,
                                               TOKENSOURCE(lxr));
    if (tstream == NULL) {
        ANTLR3_FPRINTF(stderr, "Out of memory trying to allocate token "
                               "stream\n");
        free_parser_aux(&psr, &tstream, &lxr, &input);
        return NULL;
    }

    // Create parser. It accepts a token stream.
    psr = pddl31ParserNew(tstream);
    if (psr == NULL) {
        ANTLR3_FPRINTF(stderr, "Out of memory trying to allocate parser\n");
        free_parser_aux(&psr, &tstream, &lxr, &input);
        return NULL;
    }

    // Run parser.
    // The first parameter to a pseudo-method is the object itself.
    // 'domain' is the starting rule in the grammar.
    //
    // Attention: psr->pParser->rec->state->errorCount!!
    // not psr->pParser->rec->errorCount (as is stated in the example in the
    // documentation)
    struct domain *domain = psr->domain(psr);
    // We will check for parser errors after freeing all the memory, cause we
    // have to do that in any case.
    int errorCount = psr->pParser->rec->state->errorCount;

    free_parser_aux(&psr, &tstream, &lxr, &input);

    // Check for parser error.
    if (errorCount > 0) {
        ANTLR3_FPRINTF(stderr, "The parser returned %d errors, tree walking "
                               "aborted.\n",
                               psr->pParser->rec->state->errorCount);
        // free domain itself, it might be allocated incompletely.
        libpddl31_domain_free(domain);
        return NULL;
    }
    // No error

    return domain;
}

void free_term_aux(struct term *term)
{
    if (term == NULL) {
        return;
    }
    if (term->type == CONSTANT) {
        free(term->item.constArgument);
    } else if (term->type == VARIABLE) {
        free(term->item.varArgument);
    } else {
        assert(false && "unkown term type");
    }
}

void free_formula_rec_aux(struct formula *formula)
{
    if (formula == NULL) {
        return;
    }
    if (formula->type == PREDICATE) {
        // Free predicate arguments
        if (formula->item.predicate_formula.arguments != NULL) {
            for (int i = 0;
                 i < formula->item.predicate_formula.numOfArguments;
                 ++i) {
                free_term_aux(&formula->item.predicate_formula.arguments[i]);
            }
            free(formula->item.predicate_formula.arguments);
        }
    } else if (formula->type == AND) {
        // Free AND-formula
        if (formula->item.and_formula.p != NULL) {
            // Recurse on formulas combined by this AND
            for (int i = 0;
                 i < formula->item.and_formula.numOfParameters;
                 ++i) {
                free_formula_rec_aux(&formula->item.and_formula.p[i]);
            }
            free(formula->item.and_formula.p);
        }
    } else if (formula->type == NOT) {
        // Free NOT-formula
        if (formula->item.not_formula.p != NULL) {
            free_formula_rec_aux(formula->item.not_formula.p);
        }
    } else {
        assert(false && "unkown formula type");
    }
    // Do *not* free formula itself, cause it might just be one element of an
    // array of formulas, which will (must) be freed as an entity.
}

void libpddl31_domain_free(struct domain *domain)
{
    printf("[libpddl31_domain_free]\n");

    if (domain == NULL) {
        return;
    }

    // TODO: Free name, if neccessary
 
    // Free requirements
    if (domain->requirements != NULL) {
        free(domain->requirements);
    }
 
    // Free constants
    if (domain->constants != NULL) {
        free(domain->constants);
    }

    // Free predicates
    if (domain->predicates != NULL) {
        for (int i = 0; i < domain->numOfPredicates; ++i) {
            struct predicate p = domain->predicates[i];
            // Free parameters
            if (p.parameters != NULL) {
                free(p.parameters);
            }
        }
        free(domain->predicates);
    }
    
    // Free actions
    if (domain->actions != NULL) {
        for (int i = 0; i < domain->numOfActions; ++i) {
            struct action a = domain->actions[i];
            free(a.parameters);
            free_formula_rec_aux(a.precondition);
            free(a.precondition);
            free_formula_rec_aux(a.effect);
            free(a.effect);
        }
        free(domain->actions);
    }
}

void print_term_aux(struct term *term)
{
    if (term->type == CONSTANT) {
        printf(" %s", term->item.constArgument->name);
    } else if (term->type == VARIABLE) {
        printf(" %s", term->item.varArgument->name);
    } else {
        assert(false && "unknown term type");
    }
}

void print_formula_aux(struct formula *formula)
{
    if (formula == NULL) {
        return;
    }
    int32_t indent_inc = 2; // increment of spaces for indent
    if (formula->type == PREDICATE) {
        printf(" (%s", formula->item.predicate_formula.predicate_name);
        for (int i = 0;
             i < formula->item.predicate_formula.numOfArguments; 
             ++i) {
            print_term_aux(&formula->item.predicate_formula.arguments[i]);
        }
        printf(")");
    } else if (formula->type == AND) {
        printf(" (and");
        for (int i = 0; i < formula->item.and_formula.numOfParameters; ++i) {
            print_formula_aux(&formula->item.and_formula.p[i]);
        }
        printf(")");
    } else if (formula->type == NOT) {
        printf(" (not");
        print_formula_aux(formula->item.not_formula.p);
        printf(")");
    } else {
        assert(false && "unkown formula type");
    }
}

void libpddl31_domain_print(struct domain *domain)
{
    //printf("[libpddl31_domain_print] \n");
    printf("[libpddl31_domain_print] domain name: %s\n", domain->name);
    printf("[libpddl31_domain_print] \n");

    // Print only number of requirements. Printing the requirements themselfs
    // would be complicated, and we don't think it is neccessary.
    printf("[libpddl31_domain_print] number of requirements: %d\n",
           domain->numOfRequirements);
    printf("[libpddl31_domain_print] requirements:");
    for (int i = 0; i < domain->numOfRequirements; ++i) {
        printf(" %d", domain->requirements[i]);
    }
    printf("\n");
    printf("[libpddl31_domain_print] \n");

    // Print constants
    printf("[libpddl31_domain_print] number of constants: %d\n",
           domain->numOfConstants);
    for (int i = 0; i < domain->numOfConstants; ++i) {
        struct constant *c = domain->constants;
        printf("[libpddl31_domain_print] constant: %s", c[i].name);
        if (c[i].isTyped) {
            printf(" - %s", c[i].type.name);
        }
        printf("\n");
    }
    printf("[libpddl31_domain_print] \n");

    // Print predicates
    printf("[libpddl31_domain_print] number of predicates: %d\n",
           domain->numOfPredicates);
    for (int i = 0; i < domain->numOfPredicates; ++i) {
        struct predicate *p = domain->predicates;
        printf("[libpddl31_domain_print] predicate: '%s' with parameters:",
               p[i].name);
        for (int j = 0; j < p[i].numOfParameters; ++j) {
            struct variable *par = p[i].parameters;
            printf(" %s", par[j].name);
            if (par[j].isTyped) {
                printf(" - %s", par[j].type.name);
            }
        }
        printf("\n");
    }
    printf("[libpddl31_domain_print] \n");

    // Print actions
    printf("[libpddl31_domain_print] number of actions: %d\n",
           domain->numOfActions);
    for (int i = 0; i < domain->numOfActions; ++i) {
        struct action a = domain->actions[i];
        printf("[libpddl31_domain_print] action: '%s' with parameters:",
               a.name);
        for (int j = 0; j < a.numOfParameters; j++) {
            struct variable par = a.parameters[j];
            printf(" %s", par.name);
            if (par.isTyped) {
                printf(" - %s", par.type.name);
            }
        }
        printf("\n");
        printf("[libpddl31_domain_print]\t precondition:\n");
        printf("[libpddl31_domain_print]\t ");
        print_formula_aux(a.precondition);
        printf("\n");
        printf("[libpddl31_domain_print]\t effect:\n");
        printf("[libpddl31_domain_print]\t ");
        print_formula_aux(a.effect);
        printf("\n");
    }
    printf("[libpddl31_domain_print] \n");
}
