#include <stdio.h>
#include <string.h>

#include "minunit.h"

#include "planner.h"
#include "probSpace.h"
#include "trie.h"
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


  // Setup an action so there will be a gap on the second position.
  actL->act->terms[0] = objManag_getObject(problem->objManag, "p1");
  actL->act->terms[1] = objManag_getObject(problem->objManag, "p2");
  //utils_print_actionList(actL); // DEBUG
  //printf("\n"); // DEBUG

  utils_free_gap(gap);
  gap = NULL;

  gap = planner_hasGap(problem->init, problem->goal, actL);
  //utils_print_gap(gap); // DEBUG
  //printf("\n"); // DEBUG
  mu_assert("Error planner_hasGap()",
            gap->position == 2 &&
            gap->literal->isPos &&
            strcmp(gap->literal->atom->pred->name, "visited") == 0 &&
            strcmp(gap->literal->atom->terms[0]->name, "p1") == 0);


  // Set up the an action such that there will be a gap in the first position.
  actL->act->terms[0] = objManag_getObject(problem->objManag, "p2");
  actL->act->terms[1] = objManag_getObject(problem->objManag, "p1");
  //utils_print_actionList(actL); // DEBUG
  //printf("\n"); // DEBUG

  utils_free_gap(gap);
  gap = NULL;

  gap = planner_hasGap(problem->init, problem->goal, actL);
  //utils_print_gap(gap); // DEBUG
  //printf("\n"); // DEBUG
  mu_assert("Error planner_hasGap()",
            gap->position == 1 &&
            gap->literal->isPos &&
            strcmp(gap->literal->atom->pred->name, "at") == 0 &&
            strcmp(gap->literal->atom->terms[0]->name, "p2") == 0);



  struct actionList *actL2 = utils_actionFixesGap(action, gap);
  //utils_print_actionList(actL2); // DEBUG
  //printf("\n"); // DEBUG
  actL2->act->terms[0] = objManag_getObject(problem->objManag, "p1");
  //utils_print_actionList(actL2); // DEBUG
  //printf("\n"); // DEBUG
  actL2->next = actL;
  //utils_print_actionList(actL2); // DEBUG
  //printf("\n"); // DEBUG


  utils_free_gap(gap);
  gap = NULL;



  // If the action list is a solution, then there is no gap anymore.
  // planner_hasGap() should return NULL.
  gap = planner_hasGap(problem->init, problem->goal, actL2);
  mu_assert("Error planner_hasGap() did return a gap when not expected to.",
            gap == NULL);

  utils_free_actionList(actL2);

  libpddl31_problem_free(problem);
  libpddl31_domain_free(domain);
  return 0;
}

static char *
test_planner_getActsToFixGap()
{
  char *domainFilename = "test/planner-domain0.pddl";
  struct domain *domain = libpddl31_domain_parse(domainFilename);
  char *problemFilename = "test/planner-problem0.pddl";
  struct problem *problem = libpddl31_problem_parse(domain, problemFilename);
  struct probSpace *probSpace = ps_init(problem);

  struct gap *gap = NULL;
  struct actionList *actsGap = NULL;
  struct actionList *actsGapBackup = NULL;


  // An empty plan gives the first literal of the goal as gap.
  gap = planner_hasGap(problem->init, problem->goal, NULL);
  //utils_print_gap(gap); // DEBUG
  //printf("\n"); // DEBUG

  actsGap = planner_getActsToFixGap(problem, probSpace, gap);
  actsGapBackup = actsGap;
  //utils_print_actionList(actsGap); // DEBUG

  mu_assert("Error planner_getActsToFixGap()",
            utils_actionList_length(actsGap) == 1 &&

            strcmp(actsGap->act->terms[0]->name, "p2") == 0 &&
            strcmp(actsGap->act->terms[1]->name, "p1") == 0
           );

  utils_free_actionList(actsGapBackup);
  utils_free_gap(gap);
  gap = NULL;

  ps_free(probSpace);
  //trie_cleanupSNBuffer();

  libpddl31_problem_free(problem);
  libpddl31_domain_free(domain);

  return 0;
}

static char *
test_planner_satisfies()
{
  char *domainFilename = "test/planner-domain0.pddl";
  struct domain *domain = libpddl31_domain_parse(domainFilename);
  char *problemFilename = "test/planner-problem0.pddl";
  struct problem *problem = libpddl31_problem_parse(domain, problemFilename);

  struct literal *lit = NULL;


  lit = planner_satisfies(problem->init, problem->goal);
  // The initial state of the test problem does not fulfill the goal.
  mu_assert("Error planner_satisfies(). Initial state should not fulfill"
            " goal",
            lit != NULL &&
            strcmp(lit->atom->pred->name, "visited") == 0 &&
            strcmp(lit->atom->terms[0]->name, "p1") == 0);


  utils_free_literal(lit);
  lit = NULL;


  // An artificial goal
  struct goal g;
  struct goal *goal = &g;
  goal->numOfPos = 1;
  goal->posLiterals = malloc(sizeof(*goal->posLiterals));
  goal->posLiterals->pred = predManag_getPred(domain->predManag, "at");
  goal->posLiterals->terms = malloc(sizeof(*goal->posLiterals->terms));
  goal->posLiterals->terms[0] = objManag_getObject(problem->objManag, "p1");
  goal->numOfNeg = 1;
  goal->negLiterals = malloc(sizeof(*goal->negLiterals));
  goal->negLiterals->pred = predManag_getPred(domain->predManag, "visited");
  goal->negLiterals->terms = malloc(sizeof(*goal->negLiterals->terms));
  goal->negLiterals->terms[0] = objManag_getObject(problem->objManag, "p2");

  trie_t state = trie_createEmpty(domain);
  struct atom *atom0 = malloc(sizeof(*atom0));
  atom0->pred = predManag_getPred(domain->predManag, "at");
  atom0->terms = malloc(sizeof(*atom0->terms) * atom0->pred->numOfParams);
  atom0->terms[0] = objManag_getObject(problem->objManag, "p1");
  trie_add(state, atom0);
  /* struct state s; */
  /* struct state *state = &s; */
  /* state->numOfFluents = 1; */
  /* state->fluents = malloc(sizeof(*state->fluents)); */
  /* state->fluents[0].pred = predManag_getPred(domain->predManag, "at"); */
  /* state->fluents[0].terms = malloc(sizeof(*state->fluents[0].terms)); */
  /* state->fluents[0].terms[0] = objManag_getObject(problem->objManag, "p1"); */

  // This state must satisfie the goal.
  mu_assert("Error planner_satisfies().",
            planner_satisfies(state, goal) == NULL);

  struct atom *atom1 = malloc(sizeof(*atom1));
  atom1->pred = predManag_getPred(domain->predManag, "visited");
  atom1->terms = malloc(sizeof(*atom1->terms) * atom1->pred->numOfParams);
  atom1->terms[0] = objManag_getObject(problem->objManag, "p2");
  trie_add(state, atom1);
  /* state->numOfFluents = 2; */
  /* state->fluents = realloc(state->fluents, */
  /*                          sizeof(*state->fluents) * state->numOfFluents); */
  /* // The first fluent should be still set from before. */
  /* //state->fl/uents[0].pred = predManag_getPred(domain->predManag, "at"); */
  /* //state->fluents[0].terms = malloc(sizeof(*state->fluents[0].terms)); */
  /* //state->fluents[0].terms[0] = objManag_getObject(problem->objManag, "p1"); */
  /* state->fluents[1].pred = predManag_getPred(domain->predManag, "visited"); */
  /* state->fluents[1].terms = malloc(sizeof(*state->fluents[1].terms)); */
  /* state->fluents[1].terms[0] = objManag_getObject(problem->objManag, "p2"); */

  // This state must not be satisfie the goal.
  lit = planner_satisfies(state, goal);
  //utils_print_literal(lit); // DEBUG
  mu_assert("Error planner_satisfies().",
            lit != NULL &&

            ! lit->isPos &&

            strcmp(lit->atom->pred->name, "visited") == 0 &&
            strcmp(lit->atom->terms[0]->name, "p2") == 0
            );

  utils_free_literal(lit);

  // Clean up this mess.
  trie_free(state);
  libpddl31_atom_free(atom0);
  free(atom0);
  libpddl31_atom_free(atom1);
  free(atom1);
  free(goal->negLiterals->terms);
  free(goal->negLiterals);
  free(goal->posLiterals->terms);
  free(goal->posLiterals);

  //trie_cleanupSNBuffer();

  libpddl31_problem_free(problem);
  libpddl31_domain_free(domain);
  return 0;
}

static char *
test_planner_solveProblem()
{
  char *domainFilename = "test_instances/tsp-neg-prec/domain.pddl";
                        //"test_instances/openstacks-strips/p01-domain.pddl";
                          //"test/planner-domain0.pddl";
  struct domain *domain = libpddl31_domain_parse(domainFilename);
  char *problemFilename = "test_instances/tsp-neg-prec/p3.pddl";
                          //"test_instances/openstacks-strips/p01.pddl";
                          //"test/planner-problem0.pddl";
  struct problem *problem = libpddl31_problem_parse(domain, problemFilename);
  struct probSpace *probSpace = ps_init(problem);

  // The problem instance tsp-neg-prec/p3.pddl shortest solution is of length
  // four.
  struct actionList *result = planner_solveProblem_v2(problem, probSpace, 4);
  mu_assert("Error planner_solveProblem()", result != NULL);
  utils_print_actionList(result);
  utils_free_actionList(result);

  ps_free(probSpace);
  //trie_cleanupSNBuffer();

  libpddl31_problem_free(problem);
  libpddl31_domain_free(domain);
  return 0;
}

static char *
test_planner_iterativeDeepeningSearch()
{
  char *domainFilename = "test_instances/tsp-neg-prec/domain.pddl";
                        //"test_instances/openstacks-strips/p01-domain.pddl";
                          //"test/planner-domain0.pddl";
  struct domain *domain = libpddl31_domain_parse(domainFilename);
  char *problemFilename = "test_instances/tsp-neg-prec/p6.pddl";
                          //"test_instances/openstacks-strips/p01.pddl";
                          //"test/planner-problem0.pddl";
  struct problem *problem = libpddl31_problem_parse(domain, problemFilename);

  struct actionList *result = planner_iterativeDeepeningSearch_v2(problem);
  mu_assert("Error planner_iterativeDeepeningSearch()", result != NULL);
  utils_print_actionList(result);
  utils_free_actionList(result);


  libpddl31_problem_free(problem);
  libpddl31_domain_free(domain);
  return 0;
}

static char *
allTests()
{
  mu_run_test(test_planner_hasGap);
  mu_run_test(test_planner_getActsToFixGap);
  mu_run_test(test_planner_satisfies);
  mu_run_test(test_planner_solveProblem);
  mu_run_test(test_planner_iterativeDeepeningSearch);

  trie_cleanupSNBuffer();

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
