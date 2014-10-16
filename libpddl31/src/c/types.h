#ifndef TYPES_H
#define TYPES_H

#include <stdbool.h>
#include <stdint.h>

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

struct typeSystem *types_createTypeSystem();
void types_freeTypeSystem(struct typeSystem *typeSystem);
bool types_addType(struct typeSystem *ts, char *newTypeName, char *parentName);
struct type *types_getType(struct typeSystem *ts, char *name);
void types_printTypeSystem(struct typeSystem *ts);


#endif
