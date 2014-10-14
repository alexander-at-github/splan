#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#define ARRAY_SIZE_INIT 16
#define ARRAY_SIZE_SCALE 2

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

char *string_malloc_copy_aux(char *string)
{
    int32_t length = strlen(string) + 1; // +1 for treminating null byte
    char *dest = malloc(sizeof(*dest) * length);
    if (dest == NULL) {
        return NULL;
    }
    strncpy(dest, string, length);
    dest[length - 1] = '\0';
    return dest;
}

struct typeSystem *types_createTypeSystem()
{
    struct typeSystem *typeSystem = malloc(sizeof(*typeSystem));

    typeSystem->numAlloced = ARRAY_SIZE_INIT;
    typeSystem->types = malloc(sizeof(*typeSystem->types) * ARRAY_SIZE_INIT);

    // The root node of the type hierarchy is 'object'.
    struct type *root = malloc(sizeof(*root));
    char *objStr = "object";
    root->name = string_malloc_copy_aux(objStr);
    root->parent = NULL;

    typeSystem->root = root;

    // Only one node. The root node.
    typeSystem->numOfTypes = 1;
    typeSystem->types[0] = root;

    return typeSystem;
}

// Frees the give datastructure. The given pointer is invalid afterwards.
void types_freeTypeSystem(struct typeSystem *typeSystem)
{
    if (typeSystem == NULL) {
        return;
    }
    if (typeSystem->types != NULL) {
        for (int i = 0; i < typeSystem->numOfTypes; ++i) {
            struct type *type = typeSystem->types[i];
            if (type != NULL) {
                if (type->name != NULL) {
                    free(type->name);
                }
                free(type);
            }
        }
        free(typeSystem->types);
    }
    free(typeSystem);
}

// Precondition: arguments typeSystem and newTypeName are not NULL.
// Arguemtn parentName may be NULL.
// This function returns true on success and false on error.
bool
types_addType(  struct typeSystem *ts,
                char *newTypeName,
                char *parentName)
{
    if (ts == NULL || newTypeName == NULL) {
        return false;
    }
    if (strcmp(newTypeName, ts->root->name) == 0) {
        // new types name is same as roots name
        if (parentName != NULL) {
            fprintf(stderr, "invalid declaration of root node '%s' of type "
                            "system\n", newTypeName);
            return false;
        }
        return true;
    }
    if(parentName != NULL && strcmp(newTypeName, parentName) == 0) {
        // A type can not be its own parent.
        fprintf(stderr, "invalid decalration of type '%s'\n", newTypeName);
        return false;
    }
    // Searching for parent type with this pointer.
    struct type *parentType = NULL;
    if (parentName == NULL) {
        // Use root as parent
        parentName = ts->root->name;
        parentType = ts->root;
    }
    // Go over all types
    for (int i = 0; i < ts->numOfTypes; ++i) {
        struct type *currType = ts->types[i];
        if (strcmp(currType->name, newTypeName) == 0) {
            // Multiple types with same name.
            fprintf(stderr, "multiple declarations of type '%s'\n",newTypeName);
            return false;
        }
        if (parentType == NULL && strcmp(currType->name, parentName) == 0) {
            // Remember struct of parent type for later use.
            parentType = currType;
        }
    }
    if (parentType == NULL) {
        fprintf(stderr,
                "unkown parent type '%s' for type '%s'\n",
                parentName,
                newTypeName);
        return false;
    }
    // Grow array in data structue, if neccessary.
    if (ts->numOfTypes >= ts->numAlloced) {
        struct type **t = realloc(ts->types,
                                   sizeof(*t) *ts->numAlloced*ARRAY_SIZE_SCALE);
        if (t == NULL) {
            // Error allocating
            return false;
        }
        ts->types = t;
        ts->numAlloced *= ARRAY_SIZE_SCALE;
    }
    // Create new type 
    struct type *newType = malloc(sizeof(*newType));
    newType->name = string_malloc_copy_aux(newTypeName);
    newType->parent = parentType;
    ts->types[ts->numOfTypes] = newType;
    ts->numOfTypes++;
    return true;
}

void types_printTypeSystem(struct typeSystem *ts)
{
    printf("[");
    for (int i = 0; i < ts->numOfTypes; ++i) {
        struct type *type = ts->types[i];
        printf("[%s - %s]", type->name, type->parent->name);
        if (i < ts->numOfTypes - 1) {
            printf(", ");
        }
    }
    printf("[");
}
