#include <assert.h>
#include <antlr3interfaces.h>

#include "predManag.h"

struct predManag *
predManag_create(pANTLR3_LIST preds)
{
    struct predManag *result = malloc(sizeof(*result));
    result->numOfPreds = preds->size(preds);
    result->preds = malloc(sizeof(*result->preds) * result->numOfPreds);
    for (int i = 0; i < result->numOfPreds; ++i) {
        // The antlr3 list index starts from 1.
        result->preds[i] = *(struct predicate *) preds->get(preds, i+1);
    }
    return result;
};

static void
free_predicate_aux(struct predicate *pred)
{
    if (pred == NULL) {
        return;
    }

    if (pred->name != NULL) {
        free(pred->name);
        pred->name = NULL;
    }

    if (pred->params != NULL) {
        for (int i = 0; i < pred->numOfParams; ++i) {
            libpddl31_term_free(&pred->params[i]);
        }
        free(pred->params);
        pred->params = NULL;
    }
}

void 
predManag_free(struct predManag *predManag)
{
    if (predManag == NULL) {
        return;
    }

    if (predManag->preds != NULL) {
        for (int i = 0; i < predManag->numOfPreds; ++i) {
            free_predicate_aux(&predManag->preds[i]);
        }
        free(predManag->preds),
        predManag->preds = NULL;
    }
    free(predManag);
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
