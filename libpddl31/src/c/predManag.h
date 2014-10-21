#ifndef PREDMANAG_H
#define PREDMANAG_H

#include <antlr3collections.h>

#include "pddl31structs.h"

struct predManag
{
    int32_t numOfPreds;
    struct predicate *preds;
};

// This function also accepts NULL.
struct predManag *predManag_create(pANTLR3_LIST preds);
void predManag_free(struct predManag *predManag);
struct predicate *predManag_getPred(struct predManag *manager, char *name);


#endif

