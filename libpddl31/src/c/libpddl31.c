// TODO: When freeing memory of struct domain and struct problem do check
// each pointer for NULL first and set pointer to NULL after call to free.


#include <assert.h>

#include "libpddl31.h"
#include "pddl31Lexer.h"
#include "pddl31Parser.h"
#include "pddl31structs.h"
#include "objManag.h"
#include "predManag.h"
#include "actionManag.h"
#include "typeSystem.h"

// In order to read Latin-1 input files (pddl-domain or pddl-problem files)
// set INPUT_STREAM_ENCODING to 7. If you want to read UTF-8 files set it to
// 8.
#define INPUT_STREAM_ENCODING 7 //8

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

/* Prepare parsing with antlr3. The output parameters will be set. All previous
 * values will be lost. The function will set the 'psr' output parameter to
 * NULL on error.
 */
void parse_pre_aux(char *fName,
                   pANTLR3_INPUT_STREAM *input,             // output parameter
                   ppddl31Lexer *lxr,                       // output parameter
                   pANTLR3_COMMON_TOKEN_STREAM *tstream,    // output parameter
                   ppddl31Parser *psr)                      // output parameter
{
    *input = NULL;
    *lxr = NULL;
    *tstream = NULL;
    *psr = NULL;
    // Set character input stream
    // The second agrument is the encoding. For more informaion see:
    // * file:///home/alexander/uni/bac/antlr3/runtime_c/libantlr3c-3.4/api/
    //          struct_a_n_t_l_r3___i_n_p_u_t___s_t_r_e_a_m__struct.html
    //          #acce3c7aa90181c9e636829746ad666b0
    // * /home/alexander/uni/bac/antlr3/runtime_c/libantlr3c-3.4/include/
    //   antlr3input.h
    *input = antlr3FileStreamNew((pANTLR3_UINT8) fName, INPUT_STREAM_ENCODING);
    if (input == NULL) {
        ANTLR3_FPRINTF(stderr, "Unable to open file %s due to malloc() "
                               "failure1\n", fName);
        free_parser_aux(psr, tstream, lxr, input);
        return;
    }

    // Create a new lexer and set lexer input to input-stream
    *lxr = pddl31LexerNew(*input);
    if (*lxr ==NULL) {
        ANTLR3_FPRINTF(stderr, "Unable to create the lexer due to malloc() "
                               "failure1\n");
        free_parser_aux(psr, tstream, lxr, input);
        return;
    }

    // Set token stream source to lexer.
    // Actual lexing the file will only happen later.
    //
    // ANTLR3_API pANTLR3_COMMON_TOKEN_STREAM
    // antlr3CommonTokenStreamSourceNew (ANTLR3_UINT32 hint,
    //                                   pANTLR3_TOKEN_SOURCE source);
    //
    // Note: The TOKENSOURCE macro expands to the arrow operator. That is,
    // you have to place the macro's parameter into another set of parantheses,
    // if it is dereferencing a pointer.
    *tstream = antlr3CommonTokenStreamSourceNew(ANTLR3_SIZE_HINT,
                                                TOKENSOURCE((*lxr)));
    if (*tstream == NULL) {
        ANTLR3_FPRINTF(stderr, "Out of memory trying to allocate token "
                               "stream\n");
        free_parser_aux(psr, tstream, lxr, input);
        return;
    }

    // Create parser. It accepts a token stream.
    *psr = pddl31ParserNew(*tstream);
    if (*psr == NULL) {
        ANTLR3_FPRINTF(stderr, "Out of memory trying to allocate parser\n");
        free_parser_aux(psr, tstream, lxr, input);
        return;
    }

}

struct domain *libpddl31_domain_parse(char *filename)
{
    // ANTLR3 character input stream
    pANTLR3_INPUT_STREAM input;
    // The lexer
    ppddl31Lexer lxr;
    // The token stream is produced by the ANTLR3 generated lexer.
    pANTLR3_COMMON_TOKEN_STREAM tstream;
    // The parser. It accepts a token stream.
    ppddl31Parser psr;

    // Helper function will do some work for us
    parse_pre_aux(filename, &input, &lxr, &tstream, &psr);
    if (psr == NULL) {
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

void
libpddl31_term_free(struct term *term)
{
    if (term == NULL) {
        return;
    }

    if (term->name != NULL) {
        free(term->name);
        term->name = NULL;
    }

    // Don't free type of term, it should be freed by the type system.
}

// Frees also the pointer handed as argument.
void
libpddl31_domain_free(struct domain *domain)
{
    if (domain == NULL) {
        return;
    }

    if (domain->name != NULL) {
        free(domain->name);
        domain->name = NULL;
    }

    if (domain->requirements != NULL) {
        free(domain->requirements);
        domain->requirements = NULL;
    }

    if (domain->objManag != NULL) {
        objManag_free(domain->objManag);
        domain->objManag = NULL;
    }

    // Order of frees of these is important.
    // Action Manager uses Predicate Manager. So, Predicate Manager must still
    // exist when freeing Action Manager.
    if (domain->actionManag != NULL) {
       actionManag_free(domain->actionManag);
       domain->actionManag = NULL;
    }

    if (domain->predManag != NULL) {
        predManag_free(domain->predManag);
        domain->predManag = NULL;
    }

    if (domain->typeSystem != NULL) {
        typeSystem_free(domain->typeSystem);
        domain->typeSystem = NULL;
    }

    free(domain);
}

void
libpddl31_atom_free(struct atom *atom)
{
    if (atom == NULL) {
        return;
    }

    // Ne need to free predicate, it just points to a member of the predicate
    // manager.

    if (atom->terms != NULL) {
        for (int i = 0; i < atom->pred->numOfParams; ++i) {
            libpddl31_term_free(atom->terms[i]);
            free(atom->terms[i]);
        }
        free(atom->terms);
        atom->terms = NULL;
    }   
}

void
free_goal(struct goal *goal)
{
    if (goal == NULL) {
        return;
    }

    if (goal->posLiterals != NULL) {
        for (int i = 0; i < goal->numOfPos; ++i) {
            libpddl31_atom_free(&goal->posLiterals[i]);
        }
        free(goal->posLiterals);
        goal->posLiterals = NULL;
    }
    if (goal->negLiterals != NULL) {
        for (int i = 0; i < goal->numOfNeg; ++i) {
            libpddl31_atom_free(&goal->negLiterals[i]);
        }
        free(goal->negLiterals);
        goal->negLiterals = NULL;
    }
}

// Forward declaration
void free_effect(struct effect *e);


void free_forall(struct forall *forall)
{
    if (forall == NULL) {
        return;
    }

    if (forall->vars != NULL) {
        for (int i = 0; i < forall->numOfVars; ++i) {
            libpddl31_term_free(&forall->vars[i]);
        }
        free(forall->vars);
        forall->vars = NULL;
    }

    if (forall->effect != NULL) {
        free_effect(forall->effect);
        free(forall->effect);
        forall->effect = NULL;
    }
}

void free_when(struct when *when)
{
    if (when == NULL) {
        return;
    }

    if (when->precond != NULL) {
        free_goal(when->precond);
        free(when->precond);
    }

    if (when->posLiterals != NULL) {
        for (int i = 0; i < when->numOfPos; ++i) {
            libpddl31_atom_free(&when->posLiterals[i]);
        }
        free(when->posLiterals);
        when->posLiterals = NULL;
    }
    if (when->negLiterals != NULL) {
        for (int i = 0; i < when->numOfNeg; ++i) {
            libpddl31_atom_free(&when->negLiterals[i]);
        }
        free(when->negLiterals);
        when->negLiterals = NULL;
    }
}

void free_effectElem(struct effectElem *eElem)
{
    if (eElem == NULL) {
        return;
    }

    switch(eElem->type) {
    case POS_LITERAL: // drop into next case
    case NEG_LITERAL: {
        libpddl31_atom_free(eElem->it.literal);
        free(eElem->it.literal);
        break;
    }
    case FORALL: {
        free_forall(eElem->it.forall);
        free(eElem->it.forall);
        break;
    }
    case WHEN: {
        free_when(eElem->it.when);
        free(eElem->it.when);
        break;
    }
    default: {
        assert(false);
        break;
    }
    }
}

void free_effect(struct effect *effect)
{
    if(effect == NULL) {
        return;
    }

    if (effect->elems != NULL) {
        for (int i = 0; i < effect->numOfElems; ++i) {
            free_effectElem(&effect->elems[i]);
        }
        free(effect->elems);
        effect->elems = NULL;
    }
}

void libpddl31_action_free(struct action *action)
{
    if (action == NULL) {
        return;
    }

    if (action->name != NULL) {
        free(action->name);
        action->name = NULL;
    }

    if (action->params != NULL) {
        for (int i = 0; i < action->numOfParams; ++i) {
            libpddl31_term_free(&action->params[i]);
        }
        free(action->params);
        action->params = NULL;
    }

    if (action->precond != NULL) {
        free_goal(action->precond);
        free(action->precond);
        action->precond = NULL;
    }

    if (action->effect != NULL) {
        free_effect(action->effect);
        free(action->effect);
        action->effect = NULL;
    }
}

struct problem *libpddl31_problem_parse(struct domain *domain, char *filename)
{
    // ANTLR3 character input stream
    pANTLR3_INPUT_STREAM input;
    // The lexer
    ppddl31Lexer lxr;
    // The token stream is produced by the ANTLR3 generated lexer.
    pANTLR3_COMMON_TOKEN_STREAM tstream;
    // The parser. It accepts a token stream.
    ppddl31Parser psr;

    // Helper function will do some work for us
    parse_pre_aux(filename, &input, &lxr, &tstream, &psr);
    if (psr == NULL) {
        return NULL;
    }

    // Run parser.
    // The first parameter to a pseudo-method is the object itself.
    // 'problem' is the starting rule in the grammar.
    //
    // Attention: psr->pParser->rec->state->errorCount!!
    // not psr->pParser->rec->errorCount (as is stated in the example in the
    // documentation)
    struct problem *problem = psr->problem(psr, domain);
    // We will check for parser errors after freeing all the memory, cause we
    // have to do that in any case.
    int errorCount = psr->pParser->rec->state->errorCount;

    free_parser_aux(&psr, &tstream, &lxr, &input);

    // Check for parser error.
    if (errorCount > 0) {
        ANTLR3_FPRINTF(stderr, "The parser returned %d errors, tree walking "
                               "aborted.\n",
                               psr->pParser->rec->state->errorCount);
        // free problem itself, it might be allocated incompletely.
        libpddl31_problem_free(problem);
        return NULL;
    }
    // No error

    return problem;
}

void free_state(struct state *state)
{
    if (state == NULL) {
        return;
    }

    if (state->fluents != NULL) {
        for (size_t i = 0; i < state->numOfFluents; ++i) {
            // These atoms only point to prediactes and terms which will be
            // free'd by coresponding managers. The array of pointers to terms
            // of atom needs to be free'd though.
            // TODO: Maybe improve that!
            free(state->fluents[i].terms);
        }
        free(state->fluents);
        state->fluents = NULL;
    }
}

void libpddl31_problem_free(struct problem *problem)
{
    if (problem == NULL) {
        return;
    }

    if (problem->name != NULL) {
        free(problem->name);
        problem->name = NULL;
    }

    if (problem->domainName != NULL) {
        free(problem->domainName);
        problem->domainName = NULL;
    }

    if (problem->requirements != NULL) {
        free(problem->requirements);
        problem->requirements = NULL;
    }

    if (problem->init != NULL) {
        free_state(problem->init);
        free(problem->init);
        problem->init = NULL;
    }

    if (problem->goal != NULL) {
        free_goal(problem->goal);
        free(problem->goal);
        problem->goal = NULL;
    }

    free(problem);
}


void libpddl31_domain_print(struct domain *domain)
{
    printf("Domain:[");
    printf("Name: %s,", domain->name);

    printf("Requirements:[");
    for (size_t i = 0; i < domain->numOfRequirements; ++i) {
        printf("%d", domain->requirements[i]);
        if (i < domain->numOfRequirements - 1) {
            printf(", ");
        }
    }
    printf("], ");

    objManag_print(domain->objManag);
    printf(", ");
    predManag_print(domain->predManag);
    printf(", ");
    actionManag_print(domain->actionManag);

    printf("]\n");
}

void libpddl31_term_print(struct term *term)
{
    printf( "Term:[%s,name:%s,type:%s]",
            term->isVariable ? "variable" : "constant",
            term->name,
            term->type->name);
}

void libpddl31_predicate_print(struct predicate *pred)
{
    printf("Predicate:[name:%s,parameter:[", pred->name);
    for (size_t i = 0; i < pred->numOfParams; ++i) {
        libpddl31_term_print(&pred->params[i]);
        if (i < pred->numOfParams - 1) {
            printf(",");
        }
    }
    printf("]]");
}

void libpddl31_atom_print(struct atom *atom)
{
    printf("Atom:[predicate:%s,arguments:[", atom->pred->name);
    for (size_t i = 0; i < atom->pred->numOfParams; ++i) {
        libpddl31_term_print(atom->terms[i]);
        if (i < atom->pred->numOfParams - 1) {
            printf(",");
        }
    }
    printf("]");
}

void libpddl31_forall_print (struct forall *forall)
{

}

void libpddl31_when_print (struct when *when)
{

}

void libpddl31_effectElem_print (struct effectElem *effectElem)
{
    switch (effectElem->type) {
    case POS_LITERAL: {
        libpddl31_atom_print(effectElem->it.literal);
        break;
    }
    case NEG_LITERAL: {
        printf("[NOT ");
        libpddl31_atom_print(effectElem->it.literal);
        printf("]");
        break;
    }
    case FORALL: {
        libpddl31_forall_print(effectElem->it.forall);
        break;
    }
    case WHEN: {
        libpddl31_when_print(effectElem->it.when);
        break;
    }
    }
}

void libpddl31_effect_print (struct effect *effect)
{
    if (effect == NULL) {
        return;
    }
    printf("Effect: [");
    if (effect->numOfElems > 1) {
        printf("AND ");
    }
    for (size_t i = 0; i < effect->numOfElems; ++i) {
        libpddl31_effectElem_print(&effect->elems[i]);
        if (i < effect->numOfElems - 1) {
            printf(",");
        }
    }
    printf("]");
}

void libpddl31_goal_print (struct goal *goal)
{
    printf("Goal:[");
    for (size_t i = 0; i < goal->numOfPos; ++i) {
        libpddl31_atom_print(&goal->posLiterals[i]);
        if (i < goal->numOfPos - 1 || goal->numOfNeg > 0) {
            printf(",");
        }
    }
    for (size_t i = 0; i < goal->numOfNeg; ++i) {
        printf("[NOT ");
        libpddl31_atom_print(&goal->negLiterals[i]);
        printf("]");
        if (i < goal->numOfNeg - 1) {
            printf(",");
        }
    }
    printf("]");
}

void libpddl31_action_print(struct action *action)
{
    printf( "Action:[name:%s,parameters:[",
            action->name);
    for (size_t i = 0; i < action->numOfParams; ++i) {
        libpddl31_term_print(&action->params[i]);
        if (i < action->numOfParams - 1) {
            printf(",");
        }
    }
    printf("],");
    printf("precondition:");
    libpddl31_goal_print(action->precond);
    printf(",effect:");
    libpddl31_effect_print(action->effect);
    printf("]");
}

