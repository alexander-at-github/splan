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
                                struct term **newObjs)
{
    if (objManag == NULL) {
        return NULL;
    }

    // Important: New objects are added to the back.
    struct term **tmp = realloc(objManag->objs,
                                (objManag->numOfObjs + count) * sizeof(*newObjs));
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

struct objManag *objManag_add_v2( struct objManag *objManag,
                                  int32_t count,
                                  struct term *newObjs)
{
    if (objManag == NULL) {
        return NULL;
    }

    // Important: New objects are added to the back.
    struct term **tmp = realloc(objManag->objs,
                                (objManag->numOfObjs + count) * sizeof(*newObjs));
    if (tmp == NULL) {
        return NULL;
    }
    objManag->objs = tmp;
    for (int i = 0; i < count; ++i) {
      objManag->objs[objManag->numOfObjs + i] = &newObjs[i];
    }
    objManag->numOfObjs += count;
    return objManag;
}

struct term *objManag_getObject(struct objManag *objManag, char *name)
{
    //DEBUG
    //printf("objManag_getObject() called with name '%s'\n", name);
    //objManag_print(objManag);
    //printf("\n");
    // DEBUG

    if (objManag == NULL) {
        return NULL;
    }
    // IMPORTANT: Starts iterating over the array from the back. So it finds
    // the instance, which was defined last.
    for (int32_t i = objManag->numOfObjs - 1; i >= 0; --i) {
        struct term *obj = objManag->objs[i];

        //DEBUG
        //printf("objManag_getObject() object in object manager'%s'\n",obj->name);
        // DEBUG

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
        libpddl31_term_free(objManag->objs[i]);
        free(objManag->objs[i]);
    }
    objManag_freeWthtTerms(objManag);
}

void objManag_freeWthtTerms(struct objManag *objManag)
{
    if (objManag == NULL) {
        return;
    }
    free(objManag->objs);
    free(objManag);
}

struct objManag *
objManag_clone(struct objManag *src)
{
  struct objManag *result = objManag_create();
  return objManag_add(result, src->numOfObjs, src->objs);
}

void objManag_print(struct objManag *objManag)
{
    printf("Objects:[");
    for (size_t i = 0; i < objManag->numOfObjs; ++i) {
        libpddl31_term_print(objManag->objs[i]);
        if (i < objManag->numOfObjs - 1) {
            printf(", ");
        }
    }
    printf("]");
}
