#ifndef OBJMANAG_H
#define OBJMANAG_H

#include "pddl31structs.h"

struct objManag {
    int32_t numOfObjs;
    struct term *objs;
};

struct objManag *objManag_create();
struct objManag *objManag_add(  struct objManag *objManag,
                                int32_t count,
                                struct term *objs);
struct term *objManag_getObject(struct objManag *objManag,
                                char *name);
void objManag_free(struct objManag *objManag);

#endif
