#include <assert.h>
#include <string.h>

#include "planner.h"

#include "libpddl31.h"

// Two predicates. Returns true if they are equal. False otherwise.
// Precondition: Both formulas are not NULL, of type predicate.
bool predicates_equal_aux(struct formula *f1, struct formula *f2)
{
    if (f1 == NULL || f2 == NULL ||
        f1->type != PREDICATE || f2->type != PREDICATE) {

        assert(false && "prconditions not met");
        return false;
    }
    
    // compare names
    int32_t nameCmp = strcmp(f1->item.predicate_formula.name,
                             f2->item.predicate_formula.name);
    if (nameCmp != 0) {
        return false;
    }
    // compare number of arguments
    int32_t nArgsCmp = f1->item.predicate_formula.numOfArguments -
                       f2->item.predicate_formula.numOfArguments;
    if (nArgsCmp != 0) {
        return false;
    }
    // compare arguments
    bool argsEqual = true;
    for (int32_t i = 0;
         argsEqual &&
         i < f1->item.predicate_formula.numOfArguments &&
         i < f2->item.predicate_formula.numOfArguments;
         ++i) {

        struct term *f1A = f1->item.predicate_formula.arguments;
        struct term *f2A = f2->item.predicate_formula.arguments;

        char *f1A_name = NULL;
        char *f2A_name = NULL;
        bool f1A_isTyped = false;
        bool f2A_isTyped = false;
        if (f1A->type == CONSTANT && f2A->type == CONSTANT) {
            f1A_name = f1A->item.constArgument->name;
            f2A_name = f2A->item.constArgument->name;
            f1A_isTyped = f1A->item.constArgument->isTyped;
            f2A_isTyped = f2A->item.constArgument->isTyped;
        } else if (f1A->type == VARIABLE && f2A->type == VARIABLE) {
            f1A_name = f1A->item.varArgument->name;
            f2A_name = f2A->item.varArgument->name;
            f1A_isTyped = f1A->item.varArgument->isTyped;
            f2A_isTyped = f2A->item.varArgument->isTyped;
        } else {
            assert(false && "unkown term type");
        }

        int32_t equal = strcmp(f1A_name, f2A_name);
        if (f1A_isTyped || f2A_isTyped) {
            // Todo: if typing, implement here.
            assert(false && "this planner does not support "
                            "typing");
        }
        argsEqual = equal == 0; // sets argsEqual to false if necessary
    }
    return argsEqual; // returns false if necessary
}

// Forms a conjunction of a formula and a predicate.
// Retruns the conjunction of two formulas. Might return NULL on error.
// This function also changes the argument 'formula'.
// Will not merge nested conjunctions.
// preconditions: both arguments are not NULL. The argument 'predicate' is
// indeed a predicate, i.e. predicate->type == PREDICATE holds.
struct formula *conjunction(struct formula *formula, struct formula *predicate)
{
    if (formula == NULL || predicate == NULL || predicate->type != PREDICATE) {
        assert(false && "precondition not met");
        return NULL;
    }
    // Check whether conjunction is necessary at all. That is, if 'formula' is a
    // conjunction and already includes 'predicate'.
    if (formula->type == AND) {
        for (int i = 0; i < formula->item.and_formula.numOfParameters; ++i) {
            struct formula *andArg = &formula->item.and_formula.p[i];
            if (andArg->type == PREDICATE) {
                if (predicates_equal_aux(andArg, predicate)) {
                    // Nothing to do. Just return formula.
                    return formula;
                }
            }
        }
    }
    
    // Prepare conjunction parameters. Using local variables just to avoid
    // typing long names.
    int32_t numOfParam = 2; // A conjunction of two formulas.
    struct formula *param = malloc(sizeof(*param) * numOfParam);
    // Copy predicates into their new destination.
    memcpy(&param[0], formula, sizeof(*formula));
    memcpy(&param[1], predicate, sizeof(*predicate));
    // Reuse formula data structure. This makes shure that the hole memory
    // can be freed by freeing the result. Without that it would be difficult
    // to know which memory must be freed or must not be freed later.
    struct formula *result = formula;
    // Copy values into result
    result->type = AND;
    result->item.and_formula.numOfParameters = numOfParam;
    result->item.and_formula.p = param;
    return result;
}

// Function to merge nested conjunctions.
// May return NULL on error.
// Does alter argument.
// TODO
struct formula *merge_conjunctions(struct formula *formula)
{
    switch (formula->type) {
    case PREDICATE: {
        // Nothing to do
        return formula;
        break;
    }
    case AND: {
        for (int i = 0; i < formula->item.and_formula.numOfParameters; ++i) {
            // TODO
            struct formula *localF = &formula->item.and_formula.p[i];
            // Recurse
            merge_conjunctions(localF);
            // Check for nested conjunction
            if (localF->type == AND) {
                // Do merge nested conjunctions
                int32_t numOfParams = formula->item.and_formula.numOfParameters
                                     + localF->item.and_formula.numOfParameters;
                // Note: We are using realloc. No need to free the old memory.
                struct formula *newParams =
                                    realloc(formula->item.and_formula.p,
                                            sizeof(*newParams) * numOfParams);
                // Check for NULL, cause in that case old memory is untouched.
                if (newParams == NULL) {
                    // error. What to do?
                    assert(false && "error locating memory");
                    continue;
                }
                memcpy(&newParams[formula->item.and_formula.numOfParameters],
                       localF->item.and_formula.p,
                       localF->item.and_formula.numOfParameters);
                formula->item.and_formula.numOfParameters = numOfParams;
                formula->item.and_formula.p = newParams;
                // TODO: Check for memory leaks
                // Free memory, which we do not need anymore.
                free(localF->item.and_formula.p);
            }
        }
        return formula;
        break;
    }
    case NOT: {
        merge_conjunctions(formula->item.not_formula.p);
        return formula;
        break;
    }
    } // switch (formula->type)
    return formula; // TODO: check if right
}

// This function returns a pointer to the new state. It might change the old
// state (the argument) though. This function might return NULL on error.
// TODO
struct formula *planner_apply_effect(   struct formula *stateOld,
                                        struct formula *effect)
{
    if (stateOld == NULL || effect == NULL) {
        return NULL;
    }
    switch (effect->type) {
    case PREDICATE: {
        switch (stateOld->type) {
        case PREDICATE: {
            if (predicates_equal_aux(stateOld, effect)) {
                // Just return old state. Nothing more to do.
                return stateOld;
            } else {
                // Result is a conjunction.
                return conjunction(stateOld, effect);
            }
            break;
	    }
        case AND: {
            return conjunction(stateOld, effect);
            break;
	    }
        case NOT: {
            // TODO: error? What to do?
            break;
	    }
        case EMPTY: {
            // TODO: Free stateOld?
            free(stateOld);
            return effect;
            break;
        }
        default: {
            assert(false && "unkown formula type");
            break;
	    }
        } // switch (stateOld->type)
        break;
	}
    case AND: {
        struct formula *stateNew = stateOld;
        // Apply one effect after another
        for (int i = 0; i < effect->item.and_formula.numOfParameters; ++i) {
            stateNew = planner_apply_effect(stateNew,
                                            &effect->item.and_formula.p[i]);
        }
        return stateNew;
        break;
	}
    case NOT: {
        struct formula *delEff = effect->item.not_formula.p;
        if (delEff->type != PREDICATE) {
            // Error
            assert(false && "can not apply malformed effect");
            return stateOld;
        }
        // TODO: What about two nested NOT-formulas?


        // Delete predicate from state
        struct formula *stateNew = merge_conjunctions(stateOld);
        switch (stateNew->type) {
        case PREDICATE: {
            if (predicates_equal_aux(stateNew, delEff)) {
                libpddl31_formula_free_rec(stateNew);
                stateNew->type = EMPTY;
            }
            return stateNew;
            break;
        }
        case AND: {
            for (int i = 0;
                 i < stateNew->item.and_formula.numOfParameters;
                 ++i) {

                // TODO: remove 
            }
            break;
        }
        case NOT: {
            break;
        }
        } // switch (stateOld->type)
        break;
	}
    default: {
        assert(false && "unkown formula type");
        break;
	}
    } // switch (effect->type)

    assert(false);
    return NULL;
}


// TODO: Introduce a struct state?
