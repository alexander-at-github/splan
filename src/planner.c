#include <assert.h>
//#include <stdbool.h>

#include "libpddl31.h"


// This function returns a pointer to the new state. It might change the old
// state (the argument) though.
struct formula *apply_effect(struct formula **state, struct formula *effect)
{
    if (state == NULL || effect == NULL) {
        return;
    }
    switch (effect->type) {
    case PREDICATE:
        switch (state->type) {
            case PREDICATE:
                // compare names
                int32_t nameCmp = strcmp(effect->item.predicate_formula.name,
                                         state->item.predicate_formula.name);
                // compare number of arguments
                int32_t nArgsCmp =effect->item.predicate_formula.numOfArguments-
                                  state->item.predicate_formula.numOfArguments;
                // compare arguments
                bool argsEqual = true;
                for (int32_t i = 0;
                     argsEqual &&
                     i < effect->item.predicate_formula.numOfArguments &&
                     i < state->item.predicate_formula.numOfArguments;
                     ++i) {

                    struct term *effectA =
                                       effect->item.predicate_formula.arguments;
                    struct term *stateA =
                                       state->item.predicate_formula.arguments;
                    if (effectA->type == CONSTANT && stateA->type == CONSTANT) {
                       int32_t equal = strcmp(effectA->constArgument->name,
                                              stateA->constArgument->name);
                        // Todo: If typing, check types.
                        if (equal != 0) {
                            argsEqual = false;
                        }
                    } else {
                        assert(false && "ungrounded effect");
                    }
                }

                if (nameCmp == 0 && nArgsCmp == 0 && argsEqual) {
                    // Do nothing, cause this effect does not change anything.
                } else {
                    // TODO!!!
                    struct formula *old_state = state;
                    state =
                    state->type = AND;

                    state->item.
                }
                break;
            case AND:
                break;
            case NOT:
                break;
            default:
                assert(false && "unkown formula type");
                break;
        }
        break;
    case AND:
        break;
    case NOT:
        break;
    default:
        assert(false && "unkown formula type");
        break;
    }
}
