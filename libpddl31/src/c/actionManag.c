#include <assert.h>

#include "actionManag.h"
#include "libpddl31.h"

struct actionManag *actionManag_create(pANTLR3_LIST actions)
{
    if (actions == NULL) {
        return NULL;
    }

    struct actionManag *result = malloc(sizeof(*result));
    result->numOfActions = actions->size(actions);
    if (result->numOfActions == 0) {
        result->actions = NULL;
        return result;
    }
    result->actions = malloc(sizeof(*result->actions) * result->numOfActions);
    for (int i = 0; i < result->numOfActions; ++i) {
        // antlr3 list index starts from 1
        result->actions[i] = *(struct action *) actions->get(actions, i+1);
    }
    return result;
}

void actionManag_free(struct actionManag *actionManag)
{
    if (actionManag == NULL) {
        return;
    }

    if (actionManag->actions != NULL) {
        for (int i = 0; i < actionManag->numOfActions; ++i) {
            libpddl31_action_free(&actionManag->actions[i]);
        }
        free(actionManag->actions);
        actionManag->actions = NULL;
    }
    free(actionManag);
}

struct action *actionManag_getAction(  struct actionManag *manager,
                                       char *name)
{
    assert(false);
    // TODO
    return NULL;
}


void actionManag_print(struct actionManag *actionManag)
{
    printf("Actions:[");
    for (size_t i = 0; i < actionManag->numOfActions; ++i) {
        libpddl31_action_print(&actionManag->actions[i]);
        if (i < actionManag->numOfActions - 1) {
            printf(", ");
        }
    }
    printf("]");
}
