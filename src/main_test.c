#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "planner.h"
#include "libpddl31.h"

int main(int argc, char **argv)
{
    char *name1 = malloc(sizeof(*name1) * 256);
    char *name2 = malloc(sizeof(*name2) * 256);
    strcpy(name1, "predicate1");
    strcpy(name2, "predicate2");

    struct formula *state = malloc(sizeof(*state));
    state->type = PREDICATE;
    state->item.predicate_formula.name = name1;
    state->item.predicate_formula.numOfArguments = 0;
    state->item.predicate_formula.arguments = NULL;
    libpddl31_formula_print(state);
    printf("\n");

    struct formula *effect = malloc(sizeof(*effect));
    effect->type = PREDICATE;
    effect->item.predicate_formula.name = name2;
    effect->item.predicate_formula.numOfArguments = 0;
    effect->item.predicate_formula.arguments = NULL;
    libpddl31_formula_print(effect);
    printf("\n");

    struct formula *result = planner_apply_effect(state, effect);
    libpddl31_formula_print(result);
    printf("\n");

    libpddl31_formula_free_rec(result);
    free(result);
    // Effect is not freed by libpddl31_formula_free_rec because it if it
    // is part of the result it will be copied. Effect stays independent of
    // result.
    free(effect);

    return EXIT_SUCCESS;
}
