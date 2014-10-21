#include <stdio.h>
#include <string.h>

#include "minunit.h"
#include "typeSystem.h"

int tests_run = 0;

static char *
test_createTypeSystem()
{
    struct typeSystem *ts = typeSystem_create();
    typeSystem_free(ts);
    return 0;
}

static char *
test_addType()
{
    struct typeSystem *ts = typeSystem_create();
    bool success = typeSystem_addType(ts, "aType", NULL);
    mu_assert("error adding single new type", success);
    success = typeSystem_addType(ts, "anotherType", "aType");
    mu_assert("error adding new type", success);
    success = typeSystem_addType(ts, "thirdType", NULL);
    mu_assert("error adding new type", success);
    success = typeSystem_addType(ts, "fourthType", "object");
    mu_assert("error adding new type", success);
    typeSystem_print(ts);
    printf("\n");
    typeSystem_free(ts);
    return 0;
}

static char *
test_addTypesInvalid()
{
    struct typeSystem *ts = typeSystem_create();
    bool success;
    
    // test_addType with NULL values!
    success = typeSystem_addType(NULL, "foo", "bar");
    mu_assert("error adding type to non-existing type-system", !success);
    success = typeSystem_addType(ts, NULL, NULL);
    mu_assert("error adding NULL as type", !success);

    // Adding type with non-existing parent
    success = typeSystem_addType(ts, "aType", "non-existing");
    mu_assert("error adding type with non-existing parent", !success);

    success = typeSystem_addType(ts, "foo", NULL);
    mu_assert("error adding type", success);
    
    // Decalring type 'foo' again
    success = typeSystem_addType(ts, "foo", NULL);
    mu_assert("error decalring multiple instances of type", !success);

    // Using type 'object' in an invalid way.
    success = typeSystem_addType(ts, "object", "foo");
    mu_assert("error adding object with invalid parent", !success);

    typeSystem_print(ts);
    printf("\n");
    typeSystem_free(ts);
    return 0;
}

static char *
test_getType()
{
    struct typeSystem *ts = typeSystem_create();
    bool success;

    // Add some types to work with first
    success = typeSystem_addType(ts, "foo", NULL);
    mu_assert("error adding type", success);
    success = typeSystem_addType(ts, "bar", "foo");
    mu_assert("error adding type", success);

    // Querying types
    struct type *type = typeSystem_getType(ts, "foo");
    success = strcmp(type->name, "foo") == 0;
    success = success && type->parent == ts->root;
    mu_assert("error retrieving type", success);
    
    struct type *type2 = typeSystem_getType(ts, "bar");
    success = strcmp(type2->name, "bar") == 0;
    success = success && type2->parent == type && success;
    mu_assert("error retrieving type", success);

    // Some invalid calls to typeSystem_getType
    type = typeSystem_getType(NULL, "foo");
    mu_assert("error querying types", type == NULL);
    type = typeSystem_getType(ts, NULL);
    mu_assert("error querying types", type == NULL);

    // Querying unknown type
    type = typeSystem_getType(ts, "unkown-type");
    mu_assert("error querying unkown type", type == NULL);

    typeSystem_free(ts);
    return 0;
}

static char *
allTests()
{
    mu_run_test(test_createTypeSystem);
    mu_run_test(test_addType);
    mu_run_test(test_addTypesInvalid);
    mu_run_test(test_getType);
    return 0;
}

int main(int argc, char **argv)
{
    char *result = allTests();
    if (result != 0) {
        printf("%s\n", result);
    }
    else {
        printf("ALL TESTS PASSED\n");
    }
    printf("Tests run: %d\n", tests_run);

    return result != 0;
}
