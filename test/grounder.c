#include "minunit.h"

#include "grounder.h"

//#include "actionManag.h"
#include "libpddl31.h"

int tests_run = 0;

static char *
test_groundAction1()
{
  char *domainFilename = "test/grounder-domain.pddl";
  char *problemFilename = "test/grounder-problem.pddl";
  struct domain *domain = libpddl31_domain_parse(domainFilename);
  struct problem *problem = libpddl31_problem_parse(domain, problemFilename);

  struct groundAction grA;
  grA.action = &domain->actionManag->actions[0];
  grA.numOfGrnds = 0;
  grA.grnds = NULL;

  struct grounding partialG;
  partialG.terms = malloc(sizeof(*partialG.terms) * grA.action->numOfParams);
  for (size_t i = 0; i < grA.action->numOfParams; ++i) {
    partialG.terms[i] = NULL;
  }

  // Everything set up.

  libpddl31_state_print(problem->init);
  printf("\n");
  grounder_groundAction(problem->init, 0, &partialG, &grA);
  grounder_print_groundAction(&grA);

  return 0;
}

static char *
allTests()
{
  mu_run_test(test_groundAction1);
  return 0;
}

int main (int argc, char **argv)
{
  char *result = allTests();
  if (result != 0) {
    printf("%s\n", result);
  } else {
    printf("ALL TESTS PASSED\n");
  }
  printf("Tests run: %d\n", tests_run);

  return result != 0;
}
