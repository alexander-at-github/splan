#include <assert.h>
#include <string.h>

#include "planner.h"

#include "libpddl31.h"

// Two grounded predicates. Returns true if they are equal. False otherwise.
// Precondition: Both formulas are of type predicate and grounded.
bool grounded_predicates_equal_aux(struct formula *f1, struct formula *f2)
{
    if (f1 == NULL || f2 == NULL ||
        f1->type != PREDICATE || f2->type != PREDICATE) {

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
        if (f1A->type == CONSTANT && f2A->type == CONSTANT) {
            int32_t equal = strcmp(f1A->item.constArgument->name,
                                   f2A->item.constArgument->name);
            if (f1A->item.constArgument->isTyped ||
                f2A->item.constArgument->isTyped) {
                
                // Todo: if typing, implement here.
                assert(false && "this planner does not support "
                                "typing");
            }
            argsEqual = equal == 0; // sets argsEqual to false if necessary
        } else {
            assert(false && "ungrounded predicate");
        }
    }
    return argsEqual; // returns false if necessary
}


// This function returns a pointer to the new state. It might change the old
// state (the argument) though. This junction might return NULL on error.
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

            if (grounded_predicates_equal_aux(stateOld, effect)) {
                // Just return old state. Nothing more to do.
                return stateOld;
            } else {
                // Result is a conjunction.
                // Prepare conjunction parameters
                int32_t numOfParam = 2; // A conjunction of two prediactes.
                struct formula *param = malloc(sizeof(*param) * numOfParam);
                // Copy predicates into their new destination.
                memcpy(&param[0], stateOld, sizeof(*stateOld));
                memcpy(&param[1], effect, sizeof(*effect));

                // Reuse memory of stateOld and copy values.
                struct formula *stateNew = stateOld;
                stateNew->type = AND;
                stateNew->item.and_formula.numOfParameters = numOfParam;
                stateNew->item.and_formula.p = param;

                return stateNew;
            }
            break;
	    }
        case AND: {
            // TODO: continue here
            break;
	    }
        case NOT: {
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
        break;
	}
    case NOT: {
        break;
	}
    default: {
        assert(false && "unkown formula type");
        break;
	}
    } // switch (effect->type)

    return NULL;
}
