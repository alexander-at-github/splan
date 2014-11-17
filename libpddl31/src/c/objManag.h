#ifndef OBJMANAG_H
#define OBJMANAG_H

#include "pddl31structs.h"

/* This might better be called term-manager. */

struct objManag {
    int32_t numOfObjs;
    struct term **objs;
};

struct objManag *objManag_create();
struct objManag *objManag_add(  struct objManag *objManag,
                                int32_t count,
                                struct term **objs);
struct objManag *objManag_add_v2( struct objManag *objManag,
                                  int32_t count,
                                  struct term *objs);
struct term *objManag_getObject(struct objManag *objManag,
                                char *name);
void objManag_free(struct objManag *objManag);
void objManag_freeWthtTerms(struct objManag *objManag);
void objManag_print(struct objManag *objManag);
// Does not alter the source object manager.
struct objManag *objManag_clone(struct objManag *src);

#endif
