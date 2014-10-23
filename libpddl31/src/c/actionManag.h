#ifndef ACTIONMANAG_H
#define ACTIONMANAG_H

#include <antlr3collections.h>

#include "pddl31structs.h"

struct actionManag
{
    int32_t numOfActions;
    struct action *actions;
};

struct actionManag *actionManag_create(pANTLR3_LIST actions);
void actionManag_free(struct actionManag *actionManag);
struct action *actionManag_getAction(struct actionManag *manager, char *name);

#endif
