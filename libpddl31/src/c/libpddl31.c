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

    if (atom->term != NULL) {
        for (int i = 0; i < atom->pred->numOfParams; ++i) {
            libpddl31_term_free(atom->term[i]);
            free(atom->term[i]);
        }
        free(atom->term);
        atom->term = NULL;
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
            free(state->fluents[i].term);
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

/*
void free_struct_constant_aux(struct constant *constant)
{
    //printf("Debug: free_struct_constant_aux( %p )\n", (void *) constant);
    if (constant == NULL) {
        return;
    }
    if (constant->name != NULL) {
        free(constant->name);
        constant->name = NULL;
    }
    if (constant->isTyped && constant->type != NULL) {
        if (constant->type->name != NULL) {
            free(constant->type->name);
            constant->type->name = NULL;
        }
        free(constant->type);
        constant->type = NULL;
    }
}

void free_struct_variable_aux(struct variable *variable)
{
    //printf("Debug: free_struct_variable_aux( %p )\n", (void *) variable);
    if(variable == NULL) {
        return;
    }
    if (variable->name != NULL) {
        free(variable->name);
        variable->name = NULL;
    }
    if (variable->isTyped && variable->type != NULL) {
        if (variable->type->name != NULL) {
            free(variable->type->name);
            variable->type->name = NULL;
        }
        free(variable->type);
        variable->type = NULL;
    }
}

void free_struct_predicate_aux(struct predicate *predicate)
{
    if (predicate == NULL) {
        return;
    }
    if (predicate->name != NULL) {
        free(predicate->name);
    }
    // Free parameters
    if (predicate->parameters != NULL) {
        for (int i = 0; i < predicate->numOfParameters; ++i) {
            free_struct_variable_aux(&predicate->parameters[i]);
        }
        free(predicate->parameters);
    }
}

void free_struct_fluent_aux(struct fluent *fluent)
{
    if (fluent == NULL) {
        return;
    }
    if (fluent->name != NULL) {
        free(fluent->name);
        fluent->name = NULL;
    }
    // Free arguments
    if (fluent->arguments != NULL) {
        for (int i = 0; i < fluent->numOfArguments; ++i) {
            free_struct_constant_aux(&fluent->arguments[i]);
        }
        free(fluent->arguments);
        fluent->arguments = NULL;
    }
}

void free_struct_state_aux(struct state *state)
{
    if (state == NULL) {
        return;
    }
    if (state->fluents != NULL) {
        for (int i = 0; i < state->numOfFluents; ++i) {
            free_struct_fluent_aux(&state->fluents[i]);
        }
        free(state->fluents);
        state->fluents = NULL;
    }
}

void libpddl31_term_free(struct term *term)
{
    if (term == NULL) {
        return;
    }
    if (term->type == CONSTANT) {
        free_struct_constant_aux(term->item.constArgument);
        if (term->item.constArgument != NULL) {
            free(term->item.constArgument);
            term->item.constArgument = NULL;
        }
    } else if (term->type == VARIABLE) {
        free_struct_variable_aux(term->item.varArgument);
        if (term->item.varArgument != NULL) {
            free(term->item.varArgument);
            term->item.varArgument = NULL;
        }
    } else {
        assert(false && "unkown term type");
    }
}

void libpddl31_formula_free_rec(struct formula *formula)
{
    if (formula == NULL) {
        return;
    }
    if (formula->type == PREDICATE) {
        if (formula->item.predicate_formula.name != NULL) {
            free(formula->item.predicate_formula.name);
            formula->item.predicate_formula.name = NULL;
        }
        // Free predicate arguments
        if (formula->item.predicate_formula.arguments != NULL) {
            for (int i = 0;
                 i < formula->item.predicate_formula.numOfArguments;
                 ++i) {
                libpddl31_term_free(
                                &formula->item.predicate_formula.arguments[i]);
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
                libpddl31_formula_free_rec(&formula->item.and_formula.p[i]);
            }
            free(formula->item.and_formula.p);
        }
    } else if (formula->type == NOT) {
        // Free NOT-formula
        if (formula->item.not_formula.p != NULL) {
            libpddl31_formula_free_rec(formula->item.not_formula.p);
            free(formula->item.not_formula.p);
        }
    } else {
        assert(false && "unkown formula type");
    }
    // Do *not* free formula itself, cause it might just be one element of an
    // array of formulas, which will (must) be freed as an entity.
}

void free_struct_action_aux(struct action *action)
{
    if (action == NULL) {
        return;
    }
    if (action->name != NULL) {
        free(action->name);
        action->name = NULL;
    }
    if (action->parameters != NULL) {
        for (int i = 0; i < action->numOfParameters; ++i) {
            free_struct_variable_aux(&action->parameters[i]);
        }
        free(action->parameters);
        action->parameters = NULL;
    }
    libpddl31_formula_free_rec(action->precondition);
    if (action->precondition != NULL) {
        free(action->precondition);
        action->precondition = NULL;
    }
    libpddl31_formula_free_rec(action->effect);
    if (action->effect != NULL) {
        free(action->effect);
        action->effect = NULL;
    }
}

void libpddl31_domain_free(struct domain *domain)
{
    printf("[libpddl31_domain_free]\n");

    if (domain == NULL) {
        return;
    }

    // Free name.
    if (domain->name != NULL) {
        free(domain->name);
    }
 
    // Free requirements
    if (domain->requirements != NULL) {
        free(domain->requirements);
    }
 
    // Free constants
    if (domain->constants != NULL) {
        for (int i = 0; i < domain->numOfConstants; ++i) {
            free_struct_constant_aux(&domain->constants[i]);
        }
        free(domain->constants);
    }

    // Free predicates
    if (domain->predicates != NULL) {
        for (int i = 0; i < domain->numOfPredicates; ++i) {
            free_struct_predicate_aux(&domain->predicates[i]);
        }
        free(domain->predicates);
    }
    
    // Free actions
    if (domain->actions != NULL) {
        for (int i = 0; i < domain->numOfActions; ++i) {
            free_struct_action_aux(&domain->actions[i]);
        }
        free(domain->actions);
    }

    // Free domain itself.
    free(domain);
}

void libpddl31_problem_free(struct problem *problem)
{
    printf("[libpddl31_problem_free]\n");

    if (problem == NULL) {
        return;
    }

    // Free names
    if (problem->name != NULL) {
        free(problem->name);
    }
    if (problem->domainName != NULL) {
        free(problem->domainName);
    }

    // Free requirements
    if (problem->requirements != NULL) {
        free(problem->requirements);
    }

    // Free objects
    if (problem->objects != NULL) {
        for (int i = 0; i < problem->numOfObjects; ++i) {
            free_struct_constant_aux(&problem->objects[i]);
        }
        free(problem->objects);
    }

    // Free initial state
    if (problem->init != NULL) {
        free_struct_state_aux(problem->init);
        free(problem->init);
    }

    // Free goal state
    if (problem->goal != NULL) {
        libpddl31_formula_free_rec(problem->goal);
        free(problem->goal);
    }

    // Free problem itself.
    free(problem);
}

void print_constant_aux(struct constant *constant)
{
    if (constant == NULL) {
        return;
    }
    if (constant->name != NULL) {
        printf("%s", constant->name);
    }
    if (constant->isTyped && constant->type != NULL) {
        printf(" - %s", constant->type->name);
    }
}

void print_term_aux(struct term *term)
{
    if (term->type == CONSTANT) {
        print_constant_aux(term->item.constArgument);
        //printf("%s", term->item.constArgument->name);
    } else if (term->type == VARIABLE) {
        printf("%s", term->item.varArgument->name);
    } else {
        assert(false && "unknown term type");
    }
}

void print_fluent_aux(struct fluent *fluent)
{
    if (fluent == NULL) {
        return;
    }
    printf("(");
    if (fluent->name != NULL) {
        printf("%s", fluent->name);
    }
    if (fluent->arguments != NULL) {
        for (int i = 0; i < fluent->numOfArguments; ++i) {
            printf(" ");
            print_constant_aux(&fluent->arguments[i]);
        }
    }
    printf(")");
}

void libpddl31_state_print(struct state *state)
{
    if (state == NULL) {
        return;
    }
    if (state->fluents != NULL) {
        for (int i = 0; i < state->numOfFluents; ++i) {
            print_fluent_aux(&state->fluents[i]);
            if (i < state->numOfFluents - 1) {
                // separate fluents by blanks
                printf(" ");
            }
        }
    }
}

void libpddl31_formula_print(struct formula *formula)
{
    if (formula == NULL) {
        return;
    }
    if (formula->type == PREDICATE) {
        printf("(%s", formula->item.predicate_formula.name);
        for (int i = 0;
             i < formula->item.predicate_formula.numOfArguments; 
             ++i) {
            printf(" ");
            print_term_aux(&formula->item.predicate_formula.arguments[i]);
        }
        printf(")");
    } else if (formula->type == AND) {
        printf("(and");
        for (int i = 0; i < formula->item.and_formula.numOfParameters; ++i) {
            printf(" ");
            libpddl31_formula_print(&formula->item.and_formula.p[i]);
        }
        printf(")");
    } else if (formula->type == NOT) {
        printf("(not ");
        libpddl31_formula_print(formula->item.not_formula.p);
        printf(")");
    } else {
        assert(false && "unkown formula type");
    }
}

void libpddl31_domain_print(struct domain *domain)
{
    if (domain == NULL) {
        return;
    }

    //printf("[libpddl31_domain_print]\n");
    printf("[libpddl31_domain_print] domain name: %s\n", domain->name);
    printf("[libpddl31_domain_print]\n");

    // Print requirements.
    printf("[libpddl31_domain_print] number of requirements: %d\n",
           domain->numOfRequirements);
    printf("[libpddl31_domain_print] requirements:");
    // Print only integer representation of requirements, since printing
    // text would be overly complicated.
    for (int i = 0; i < domain->numOfRequirements; ++i) {
        printf(" %d", domain->requirements[i]);
    }
    printf("\n");
    printf("[libpddl31_domain_print]\n");

    // Print constants
    printf("[libpddl31_domain_print] number of constants: %d\n",
           domain->numOfConstants);
    for (int i = 0; i < domain->numOfConstants; ++i) {
        struct constant *c = domain->constants;
        printf("[libpddl31_domain_print] constant: %s", c[i].name);
        if (c[i].isTyped) {
            printf(" - %s", c[i].type->name);
        }
        printf("\n");
    }
    printf("[libpddl31_domain_print]\n");

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
                printf(" - %s", par[j].type->name);
            }
        }
        printf("\n");
    }
    printf("[libpddl31_domain_print]\n");

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
                printf(" - %s", par.type->name);
            }
        }
        printf("\n");
        printf("[libpddl31_domain_print]\t precondition:\n");
        printf("[libpddl31_domain_print]\t  ");
        libpddl31_formula_print(a.precondition);
        printf("\n");
        printf("[libpddl31_domain_print]\t effect:\n");
        printf("[libpddl31_domain_print]\t  ");
        libpddl31_formula_print(a.effect);
        printf("\n");
    }
    printf("[libpddl31_domain_print]\n");
}

void libpddl31_problem_print(struct problem *problem)
{
    if (problem == NULL) {
        return;
    }

    //printf("[libpddl32_problem_print]\n");
    printf("[libpddl32_problem_print] problem name: %s\n", problem->name);
    printf("[libpddl32_problem_print] this problem is for domain: %s\n",
           problem->domainName);
    printf("[libpddl32_problem_print]\n");

    // Print requirements.
    printf("[libpddl31_problem_print] number of requirements: %d\n",
           problem->numOfRequirements);
    printf("[libpddl31_problem_print] requirements:");
    // Print only integer representation of requirements, since printing
    // text would be overly complicated.
    for (int i = 0; i < problem->numOfRequirements; ++i) {
        printf(" %d", problem->requirements[i]);
    }
    printf("\n");
    printf("[libpddl31_problem_print]\n");

    // Print objects.
    printf("[libpddl31_problem_print] number of objects: %d\n",
           problem->numOfObjects);
    for (int i = 0; i < problem->numOfObjects; ++i) {
        struct constant obj = problem->objects[i];
        printf("[libpddl31_problem_print] object: %s", obj.name);
        if (obj.isTyped) {
            printf(" - %s", obj.type->name);
        }
        printf("\n");
    }
    printf("[libpddl31_problem_print]\n");

    // Print initial state
    printf("[libpddl31_problem_print] initial state:\n");
    printf("[libpddl31_problem_print]  ");
    libpddl31_state_print(problem->init);
    printf("\n");
    printf("[libpddl31_problem_print]\n");

    // Print goal state
    printf("[libpddl31_problem_print] goal state:\n");
    printf("[libpddl31_problem_print]  ");
    libpddl31_formula_print(problem->goal);
    printf("\n");
    printf("[libpddl31_problem_print]\n");
}

bool libpddl31_problem_is_member_of_domain( struct problem *problem,
                                            struct domain *domain)
{
    int equals = strcmp(problem->domainName, domain->name);
    return equals == 0;
}
*/
