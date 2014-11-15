#ifndef TYPESYSTEM_H
#define TYPESYSTEM_H

#include <stdbool.h>
#include <stdint.h>

#include "pddl31structs.h"

struct type
{
    char *name;
    struct type *parent;
};

struct typeSystem
{
    struct type *root;
    int32_t numOfTypes;
    // Size of memory alloced for following array. In array elements.
    int32_t numAlloced;
    // An array of pointer to types
    struct type **types;
};

struct typeSystem *typeSystem_create();
void typeSystem_free(struct typeSystem *typeSystem);
bool typeSystem_addType(struct typeSystem *ts,
                        char *newTypeName,
                        char *parentName);
struct type *typeSystem_getType(struct typeSystem *ts, char *name);
void typeSystem_print(struct typeSystem *ts);
struct type *typeSystem_getRoot(struct typeSystem *ts);
bool typeSystem_isa(struct type *t1,
                    struct type *t2);


#endif
