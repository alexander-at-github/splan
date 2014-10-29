#include <stdlib.h>
#include <string.h>

#include "libpddl31.h"
#include "objManag.h"

struct objManag *objManag_create()
{
    struct objManag *result = malloc(sizeof(*result));
    result->numOfObjs = 0;
    result->objs = NULL;
    return result;
}
struct objManag *objManag_add(  struct objManag * objManag,
                                int32_t count,
                                struct term *newObjs)
{
    if (objManag == NULL) {
        return NULL;
    }

    struct term *tmp = realloc(objManag->objs,
                               (objManag->numOfObjs + count) *sizeof(*newObjs));
    if (tmp == NULL) {
        return NULL;
    }
    objManag->objs = tmp;
    memcpy( &objManag->objs[objManag->numOfObjs],
            newObjs,
            count * sizeof(*newObjs));
    objManag->numOfObjs += count;
    return objManag;
}

struct term *objManag_getObject(struct objManag *objManag, char *name)
{
    if (objManag == NULL) {
        return NULL;
    }
    for (size_t i = 0; i < objManag->numOfObjs; ++i) {
        struct term *obj = &objManag->objs[i];
        if (strcmp(obj->name, name) == 0) {
            return obj;
        }
    }
    return NULL;
}

void objManag_free(struct objManag *objManag)
{
    if (objManag == NULL) {
        return;
    }
    for (size_t i = 0; i < objManag->numOfObjs; ++i) {
        libpddl31_term_free(&objManag->objs[i]);
    }
    free(objManag->objs);
    free(objManag);
}
