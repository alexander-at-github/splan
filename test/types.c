#include <stdio.h>

#include "minunit.h"
#include "types.h"

static char *
test_createTypeSystem()
{
    struct typeSystem *ts = types_createTypeSystem();
    types_freeTypeSystem(ts);
    return 0;
}

static char *
test_addType()
{
    struct typeSystem *ts = types_createTypeSystem();
    bool success = types_addType(ts, "aType", NULL);
    mu_assert("error adding single new type", success);
    types_printTypeSystem(ts);
    printf("\n");
    types_freeTypeSystem(ts);
    return 0;
}

static char *
allTests()
{
    mu_run_test(test_createTypeSystem);
    mu_run_test(test_addType);
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
