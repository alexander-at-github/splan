#include <stdio.h>

#include "minunit.h"

#include "planner.h"
#include "utils.h"

int tests_run = 0;

static char *
test_planner_hasGap()
{
  char *domainFilename = "test/planner-domain0.pddl";
  struct domain *domain = libpddl31_domain_parse(domainFilename);
  char *problemFilename = "test/planner-problem0.pddl";
  struct problem *problem = libpddl31_problem_parse(domain, problemFilename);

  // An empty plan must give the first literal of the goal as gap.
  struct gap *gap = planner_hasGap(problem->init, problem->goal, NULL);
  //utils_print_gap(gap); // DEBUG
  //printf("\n"); // DEBUG
  mu_assert("Error planner_hasGap()",
            gap->position == 1 &&
            gap->literal->isPos &&
            utils_atom_equal(gap->literal->atom,
                             &problem->goal->posLiterals[0]));

  // There is only one action in the domain
  struct action *action = actionManag_getAction(domain->actionManag, "move");
  struct actionList *actL = utils_actionFixesGap(action, gap);
  //utils_print_actionList(actL); // DEBUG
  //printf("\n"); // DEBUG
  actL->act->terms[0] = objManag_getObject(problem->objManag, "p2");
  //utils_print_actionList(actL); // DEBUG
  //printf("\n"); // DEBUG

  // TODO: Free gap

  gap = planner_hasGap(problem->init, problem->goal, actL);
  //utils_print_gap(gap); // DEBUG
  //printf("\n"); // DEBUG
  mu_assert("Error planner_hasGap()",
            gap->position == 1 &&
            gap->literal->isPos &&
            strcmp(gap->literal->atom->pred->name, "at") == 0 &&
            strcmp(gap->literal->atom->terms[0]->name, "p2") == 0);


  struct actionList *actL2 = utils_actionFixesGap(action, gap);
  utils_print_actionList(actL2); // DEBUG
  printf("\n"); // DEBUG
  actL2->act->terms[0] = objManag_getObject(problem->objManag, "p1");
  utils_print_actionList(actL2); // DEBUG
  printf("\n"); // DEBUG
  actL2->next = actL;

  // If the action list is a solution, then there is no gap anymore.
  // planner_hasGap() should return NULL.
  gap = planner_hasGap(problem->init, problem->goal, actL2);
  mu_assert("Error planner_hasGap() did return a gap when not expected to.",
            gap == NULL);

  libpddl31_problem_free(problem);
  libpddl31_domain_free(domain);
  return 0;
}

static char *
test_planner_solveProblem()
{
  char *domainFilename = "test/planner-domain0.pddl";
  struct domain *domain = libpddl31_domain_parse(domainFilename);
  char *problemFilename = "test/planner-problem0.pddl";
  struct problem *problem = libpddl31_problem_parse(domain, problemFilename);

  // FIXME: A random depth limit.
  struct actionList *result = planner_solveProblem(problem, 2);

  return 0;
}

static char *
allTests()
{
  mu_run_test(test_planner_hasGap);
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
