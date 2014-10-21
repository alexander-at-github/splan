#include <antlr3interfaces.h>

#include "predManag.h"

struct predManag *
predManag_create(pANTLR3_LIST preds)
{
    struct predManag *result = malloc(sizeof(*result));
    result->numOfPreds = preds->size(preds);
    result->preds = malloc(sizeof(*reslut->preds) * result->numOfPreds);
    for (int i = 0; i < result->numOfPreds; ++i) {
        // The antlr3 list index starts from 1.
        result->preds[i] = *(struct predicate *) preds->get(preds, i+1);
    }
    return result;
};

void 
predManag_free(struct predManag)
{
    assert(false);
    // TODO
};

struct predicate *
predManag_getPred(struct predManag *manager, char *name)
{
    for (int i = 0; i < manager->numOfPreds; ++i) {
        if (strcmp(name, manager->preds[i].name) == 0) {
            return &manager->preds[i];
        }
    }
    return NULL;
};
