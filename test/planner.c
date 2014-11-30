#include <stdio.h>

#include "minunit.h"
#include "planner.h"

int tests_run = 0;

static char *
test_planner_solveProblem()
{
  char *domainFilename = "test/planner-domain0.pddl";
  struct domain *domain = libpddl31_domain_parse(domainFilename);
  char *problemFilename = "test/planner-problem0.pddl";
  struct problem *problem = libpddl31_problem_parse(domain, problemFilename);

  // FIXME: A random depth limit.
  struct actionList *result = planner_solveProblem(problem, 8);

  return 0;
}

static char *
allTests()
{
  mu_run_test(test_planner_solveProblem);

  return 0;
}

int main (int argc, char **argv)
{
  char *result = allTests();
  printf("\n");
  if (result != 0) {
    printf("%s\n", result);
  } else {
    printf("ALL TESTS PASSED\n");
  }
  printf("Tests run: %d\n", tests_run);

  return result != 0;
}
