#include "libpddl31.h"
#include "minunit.h"
#include "trie.h"
#include "utils.h"

/* All the test cases use test instances (that is, domains and problems)
 * from the test/c/utils-domain.pddl and test/c/utils-problem0.pddl files.
 */

int tests_run = 0;

static char *
test_actionFixesGap_simple()
{
  char *domainFilename = "test/c/utils-domain.pddl";
  struct domain *domain = libpddl31_domain_parse(domainFilename);
  char *problemFilename = "test/c/utils-problem0.pddl";
  struct problem *problem = libpddl31_problem_parse(domain, problemFilename);

  // Prepare gap.
  struct gap *gap = malloc(sizeof(*gap));
  gap->literal = malloc(sizeof(*gap->literal));
  gap->position = 0;
  gap->literal->isPos = true;
  // Just take first positive literal from goal.
  gap->literal->atom = &problem->goal->posLiterals[0];
  //libpddl31_atom_print(gap->literal->atom); // DEBUG
  //printf("\n"); // DEBUG

  struct action *action = NULL;
  struct actionList *result = NULL;


  /* First action */
  // Prepare action.
  action = actionManag_getAction(domain->actionManag, "a0");

  // Get partial groundings for action, which fix the gap.
  result = utils_actionFixesGap(action, gap);
  //utils_print_actionList(result); // DEBUG
  //printf("\n"); // DEBUG
  mu_assert("Error utils_actionFixesGap(): wrong number of results.\n",
            utils_actionList_length(result) == 1);
  mu_assert("Error utils_actionFixesGap(), wrong partial grounding.\n",
            strcmp(result->act->terms[0]->name, "obj0") == 0);
  utils_free_actionList(result);


  /* Second action */
  // Prepare action.
  action = actionManag_getAction(domain->actionManag, "a1");

  // Get partial groundings for action, which fix the gap.
  result = utils_actionFixesGap(action, gap);
  //utils_print_actionList(result); // DEBUG
  //printf("\n"); // DEBUG
  mu_assert("Error utils_actionFixesGap(): wrong number of results.\n",
            utils_actionList_length(result) == 1);
  mu_assert("Error utils_actionFixesGap(), wrong partial grounding.\n",
            strcmp(result->act->terms[0]->name, "obj0") == 0);
  for (int32_t i = 1; i < action->numOfParams; ++i) {
    mu_assert("Error utils_actionFixesGap():"
              " partial grounding too restrictive",
              result->act->terms[i] == NULL);
  }
  utils_free_actionList(result);


  free(gap->literal);
  free(gap);
  libpddl31_problem_free(problem);
  libpddl31_domain_free(domain);
  return 0;
}

static char *
test_actionFixesGap_advanced_positive()
{
  char *domainFilename = "test/c/utils-domain.pddl";
  struct domain *domain = libpddl31_domain_parse(domainFilename);
  char *problemFilename = "test/c/utils-problem0.pddl";
  struct problem *problem = libpddl31_problem_parse(domain, problemFilename);

  // Prepare gap.
  struct gap *gap = malloc(sizeof(*gap));
  gap->literal = malloc(sizeof(*gap->literal));
  gap->position = 0;
  gap->literal->isPos = true;
  // Just take first positive literal from goal.
  gap->literal->atom = &problem->goal->posLiterals[1];
  //libpddl31_atom_print(gap->literal->atom); // DEBUG
  //printf("\n"); // DEBUG

  struct action *action = NULL;
  struct actionList *result = NULL;
  struct actionList *resultHead = NULL;


  /* Third action */
  // Prepare action.
  action = actionManag_getAction(domain->actionManag, "a2");

  // Get partial groundings for action, which fix the gap.
  result = utils_actionFixesGap(action, gap);
  //utils_print_actionList(result); // DEBUG
  //printf("\n"); // DEBUG
  mu_assert("Error utils_actionFixesGap(): wrong number of results.\n",
            utils_actionList_length(result) == 1);
  mu_assert("Error utils_actionFixesGap(), wrong partial grounding.\n",
            strcmp(result->act->terms[0]->name, "obj0") == 0);
  mu_assert("Error utils_actionFixesGap(), wrong partial grounding.\n",
            strcmp(result->act->terms[1]->name, "obj1") == 0);
  utils_free_actionList(result);


  /* Fourth action */
  // Prepare action.
  action = actionManag_getAction(domain->actionManag, "a3");

  // Get partial groundings for action, which fix the gap.
  result = utils_actionFixesGap(action, gap);
  resultHead = result;
  //utils_print_actionList(result); // DEBUG
  //printf("\n"); // DEBUG
  mu_assert("Error utils_actionFixesGap(): wrong number of results.\n",
            utils_actionList_length(result) == 2);
  mu_assert("Error utils_actionFixesGap(), wrong partial grounding.\n",
            strcmp(result->act->terms[0]->name, "obj1") == 0);
  mu_assert("Error utils_actionFixesGap(), wrong partial grounding.\n",
            strcmp(result->act->terms[1]->name, "obj0") == 0);
  result = result->next;
  mu_assert("Error utils_actionFixesGap(), wrong partial grounding.\n",
            strcmp(result->act->terms[0]->name, "obj0") == 0);
  mu_assert("Error utils_actionFixesGap(), wrong partial grounding.\n",
            strcmp(result->act->terms[1]->name, "obj1") == 0);
  result = resultHead;
  // Check that other variables of action are not set.
  while (result != NULL) {
    for (int32_t i = 2; i < action->numOfParams; ++i) {
      mu_assert("Error utils_actionFixesGap():"
                " partial grounding too restrictive",
                result->act->terms[i] == NULL);
    }
    result = result->next;
  }
  utils_free_actionList(resultHead);


  free(gap->literal);
  free(gap);
  libpddl31_problem_free(problem);
  libpddl31_domain_free(domain);
  return 0;
}

static char *
test_actionFixesGap_advanced_negative()
{
  char *domainFilename = "test/c/utils-domain.pddl";
  struct domain *domain = libpddl31_domain_parse(domainFilename);
  char *problemFilename = "test/c/utils-problem0.pddl";
  struct problem *problem = libpddl31_problem_parse(domain, problemFilename);

  // Prepare gap.
  struct gap *gap = malloc(sizeof(*gap));
  gap->literal = malloc(sizeof(*gap->literal));
  gap->position = 0;
  gap->literal->isPos = false;
  // Just take first positive literal from goal.
  gap->literal->atom = &problem->goal->negLiterals[0];
  //libpddl31_atom_print(gap->literal->atom); // DEBUG
  //printf("\n"); // DEBUG

  struct action *action = NULL;
  struct actionList *result = NULL;
  struct actionList *resultHead = NULL;


  /* Fifth action */
  // Prepare action.
  action = actionManag_getAction(domain->actionManag, "a4");

  // Get partial groundings for action, which fix the gap.
  result = utils_actionFixesGap(action, gap);
  //utils_print_actionList(result); // DEBUG
  //printf("\n"); // DEBUG
  mu_assert("Error utils_actionFixesGap(): wrong number of results.\n",
            utils_actionList_length(result) == 1);
  mu_assert("Error utils_actionFixesGap(), wrong partial grounding.\n",
            strcmp(result->act->terms[0]->name, "obj0") == 0);
  mu_assert("Error utils_actionFixesGap(): partial grounding too restrictive",
            result->act->terms[1] == NULL);
  mu_assert("Error utils_actionFixesGap(), wrong partial grounding.\n",
            strcmp(result->act->terms[2]->name, "obj1") == 0);
  mu_assert("Error utils_actionFixesGap(): partial grounding too restrictive",
            result->act->terms[3] == NULL);
  utils_free_actionList(result);

  /* Sixt action */
  // Prepare action.
  action = actionManag_getAction(domain->actionManag, "a5");

  // Get partial groundings for action, which fix the gap.
  result = utils_actionFixesGap(action, gap);
  resultHead = result;
  //printf("utils_actionList_length(result): %d\n", // DEBUG
  //       utils_actionList_length(result)); // DEBUG
  //utils_print_actionList(result); // DEBUG
  //printf("\n"); // DEBUG

  mu_assert("Error utils_actionFixesGap(): wrong number of results.\n",
            utils_actionList_length(result) == 2);
  mu_assert("Error utils_actionFixesGap(), wrong partial grounding.\n",
            result->act->terms[0] == NULL);
  mu_assert("Error utils_actionFixesGap(): partial grounding too restrictive",
            result->act->terms[1] == NULL);
  mu_assert("Error utils_actionFixesGap(), wrong partial grounding.\n",
            strcmp(result->act->terms[2]->name, "obj0") == 0);
  mu_assert("Error utils_actionFixesGap(): partial grounding too restrictive",
            strcmp(result->act->terms[3]->name, "obj1") == 0);
  result = result->next;
  mu_assert("Error utils_actionFixesGap(), wrong partial grounding.\n",
            result->act->terms[0] == NULL);
  mu_assert("Error utils_actionFixesGap(): partial grounding too restrictive",
            strcmp(result->act->terms[1]->name, "obj0") == 0);
  mu_assert("Error utils_actionFixesGap(), wrong partial grounding.\n",
            strcmp(result->act->terms[2]->name, "obj1") == 0);
  mu_assert("Error utils_actionFixesGap(): partial grounding too restrictive",
            result->act->terms[3] == NULL);

  utils_free_actionList(resultHead);


  // Use a different gap
  gap->literal->isPos = false;
  // Just take first positive literal from goal.
  gap->literal->atom = &problem->goal->negLiterals[1];
  //libpddl31_atom_print(gap->literal->atom); // DEBUG
  //printf("\n"); // DEBUG

  // Get partial groundings for action, which fix the gap.
  result = utils_actionFixesGap(action, gap);
  resultHead = result;
  //printf("utils_actionList_length(result): %d\n", // DEBUG
  //       utils_actionList_length(result)); // DEBUG
  //utils_print_actionList(result); // DEBUG
  //printf("\n"); // DEBUG

  mu_assert("Error utils_actionFixesGap(): wrong number of results.\n",
            utils_actionList_length(result) == 3);
  mu_assert("Error utils_actionFixesGap(), wrong partial grounding.\n",
            strcmp(result->act->terms[0]->name, "obj1") == 0);
  mu_assert("Error utils_actionFixesGap(): partial grounding too restrictive",
            result->act->terms[1] == NULL);
  mu_assert("Error utils_actionFixesGap(), wrong partial grounding.\n",
            result->act->terms[2] == NULL);
  mu_assert("Error utils_actionFixesGap(): partial grounding too restrictive",
            result->act->terms[3] == NULL);
  result = result->next;
  mu_assert("Error utils_actionFixesGap(), wrong partial grounding.\n",
            result->act->terms[0] == NULL);
  mu_assert("Error utils_actionFixesGap(): partial grounding too restrictive",
            result->act->terms[1] == NULL);
  mu_assert("Error utils_actionFixesGap(), wrong partial grounding.\n",
            strcmp(result->act->terms[2]->name, "const0") == 0);
  mu_assert("Error utils_actionFixesGap(): partial grounding too restrictive",
            strcmp(result->act->terms[3]->name, "obj1") == 0);
  result = result->next;
  mu_assert("Error utils_actionFixesGap(), wrong partial grounding.\n",
            result->act->terms[0] == NULL);
  mu_assert("Error utils_actionFixesGap(): partial grounding too restrictive",
            strcmp(result->act->terms[1]->name, "const0") == 0);
  mu_assert("Error utils_actionFixesGap(), wrong partial grounding.\n",
            strcmp(result->act->terms[2]->name, "obj1") == 0);
  mu_assert("Error utils_actionFixesGap(): partial grounding too restrictive",
            result->act->terms[3] == NULL);

  utils_free_actionList(resultHead);


  free(gap->literal);
  free(gap);
  libpddl31_problem_free(problem);
  libpddl31_domain_free(domain);
  return 0;
}

static char *
test_actionFixesGap_advanced_when()
{
  // TODO
  char *domainFilename = "test/c/utils-domain.pddl";
  struct domain *domain = libpddl31_domain_parse(domainFilename);
  char *problemFilename = "test/c/utils-problem0.pddl";
  struct problem *problem = libpddl31_problem_parse(domain, problemFilename);

  // Prepare gap.
  struct gap *gap = malloc(sizeof(*gap));
  gap->literal = malloc(sizeof(*gap->literal));
  gap->position = 0;
  gap->literal->isPos = true;
  // Just take first positive literal from goal.
  gap->literal->atom = &problem->goal->posLiterals[0];
  //libpddl31_atom_print(gap->literal->atom); // DEBUG
  //printf("\n"); // DEBUG

  struct action *action = NULL;
  struct actionList *result = NULL;
  struct actionList *resultHead = NULL;


  /* Seventh action */
  // Prepare action.
  action = actionManag_getAction(domain->actionManag, "a6");

  // Get partial groundings for action, which fix the gap.
  result = utils_actionFixesGap(action, gap);
  //utils_print_actionList(result); // DEBUG
  //printf("\n"); // DEBUG
  mu_assert("Error utils_actionFixesGap(): wrong number of results.\n",
            utils_actionList_length(result) == 1);
  mu_assert("Error utils_actionFixesGap(), wrong partial grounding.\n",
            result->act->terms[0] == NULL);
  mu_assert("Error utils_actionFixesGap(): partial grounding too restrictive",
            strcmp(result->act->terms[1]->name, "obj0") == 0);
  mu_assert("Error utils_actionFixesGap(), wrong partial grounding.\n",
            result->act->terms[2] == NULL);
  mu_assert("Error utils_actionFixesGap(): partial grounding too restrictive",
            result->act->terms[3] == NULL);
  utils_free_actionList(result);

  /* Eighth action */
  // Prepare action.
  action = actionManag_getAction(domain->actionManag, "a7");

  gap->literal->isPos = false;
  // Just take first positive literal from goal.
  gap->literal->atom = &problem->goal->negLiterals[0];
  //libpddl31_atom_print(gap->literal->atom); // DEBUG
  //printf("\n"); // DEBUG

  // Get partial groundings for action, which fix the gap.
  result = utils_actionFixesGap(action, gap);
  resultHead = result;
  //printf("utils_actionList_length(result): %d\n", // DEBUG
  //       utils_actionList_length(result)); // DEBUG
  //utils_print_actionList(result); // DEBUG
  //printf("\n"); // DEBUG

  mu_assert("Error utils_actionFixesGap(): wrong number of results.\n",
            utils_actionList_length(result) == 3);
  mu_assert("Error utils_actionFixesGap(), wrong partial grounding.\n",
            result->act->terms[0] == NULL);
  mu_assert("Error utils_actionFixesGap(): partial grounding too restrictive",
            result->act->terms[1] == NULL);
  mu_assert("Error utils_actionFixesGap(), wrong partial grounding.\n",
            strcmp(result->act->terms[2]->name, "obj0") == 0);
  mu_assert("Error utils_actionFixesGap(): partial grounding too restrictive",
            strcmp(result->act->terms[3]->name, "obj1") == 0);
  result = result->next;
  mu_assert("Error utils_actionFixesGap(), wrong partial grounding.\n",
            result->act->terms[0] == NULL);
  mu_assert("Error utils_actionFixesGap(): partial grounding too restrictive",
            strcmp(result->act->terms[1]->name, "obj0") == 0);
  mu_assert("Error utils_actionFixesGap(), wrong partial grounding.\n",
            strcmp(result->act->terms[2]->name, "obj1") == 0);
  mu_assert("Error utils_actionFixesGap(): partial grounding too restrictive",
            result->act->terms[3] == NULL);
  result = result->next;
  mu_assert("Error utils_actionFixesGap(), wrong partial grounding.\n",
            strcmp(result->act->terms[0]->name, "obj0") == 0);
  mu_assert("Error utils_actionFixesGap(): partial grounding too restrictive",
            strcmp(result->act->terms[1]->name, "obj1") == 0);
  mu_assert("Error utils_actionFixesGap(), wrong partial grounding.\n",
            result->act->terms[2] == NULL);
  mu_assert("Error utils_actionFixesGap(): partial grounding too restrictive",
            result->act->terms[3] == NULL);

  utils_free_actionList(resultHead);


  free(gap->literal);
  free(gap);
  libpddl31_problem_free(problem);
  libpddl31_domain_free(domain);
  return 0;
}

static char *
test_actionFixesGap_advanced_forall()
{
  char *domainFilename = "test/c/utils-domain.pddl";
  struct domain *domain = libpddl31_domain_parse(domainFilename);
  char *problemFilename = "test/c/utils-problem0.pddl";
  struct problem *problem = libpddl31_problem_parse(domain, problemFilename);

  // Prepare gap.
  struct gap *gap = malloc(sizeof(*gap));
  gap->literal = malloc(sizeof(*gap->literal));
  gap->position = 0;
  gap->literal->isPos = true;
  // Just take first positive literal from goal.
  gap->literal->atom = &problem->goal->posLiterals[0];
  //libpddl31_atom_print(gap->literal->atom); // DEBUG
  //printf("\n"); // DEBUG

  struct action *action = NULL;
  struct actionList *result = NULL;
  struct actionList *resultHead = NULL;


  /* Nineth action */
  // Prepare action.
  action = actionManag_getAction(domain->actionManag, "a8");

  // Get partial groundings for action, which fix the gap.
  result = utils_actionFixesGap(action, gap);
  //utils_print_actionList(result); // DEBUG
  //printf("\n"); // DEBUG
  mu_assert("Error utils_actionFixesGap(): wrong number of results.\n",
            utils_actionList_length(result) == 1);
  mu_assert("Error utils_actionFixesGap(), wrong partial grounding.\n",
            result->act->terms[0] == NULL);
  mu_assert("Error utils_actionFixesGap(): partial grounding too restrictive",
            result->act->terms[1] == NULL);
  mu_assert("Error utils_actionFixesGap(), wrong partial grounding.\n",
            result->act->terms[2] == NULL);
  mu_assert("Error utils_actionFixesGap(): partial grounding too restrictive",
            result->act->terms[3] == NULL);
  utils_free_actionList(result);

  /* Tenth action */
  // Prepare action.
  action = actionManag_getAction(domain->actionManag, "a9");

  gap->literal->isPos = true;
  // Just take first positive literal from goal.
  gap->literal->atom = &problem->goal->posLiterals[1];
  //libpddl31_atom_print(gap->literal->atom); // DEBUG
  //printf("\n"); // DEBUG

  // Get partial groundings for action, which fix the gap.
  result = utils_actionFixesGap(action, gap);
  resultHead = result;
  //printf("utils_actionList_length(result): %d\n", // DEBUG
  //       utils_actionList_length(result)); // DEBUG
  //utils_print_actionList(result); // DEBUG
  //printf("\n"); // DEBUG

  mu_assert("Error utils_actionFixesGap(): wrong number of results.\n",
            utils_actionList_length(result) == 2);
  mu_assert("Error utils_actionFixesGap(), wrong partial grounding.\n",
            result->act->terms[0] == NULL);
  mu_assert("Error utils_actionFixesGap(): partial grounding too restrictive",
            result->act->terms[1] == NULL);
  mu_assert("Error utils_actionFixesGap(), wrong partial grounding.\n",
            strcmp(result->act->terms[2]->name, "obj0") == 0);
  mu_assert("Error utils_actionFixesGap(): partial grounding too restrictive",
            strcmp(result->act->terms[3]->name, "obj1") == 0);
  result = result->next;
  mu_assert("Error utils_actionFixesGap(), wrong partial grounding.\n",
            strcmp(result->act->terms[0]->name, "obj0") == 0);
  mu_assert("Error utils_actionFixesGap(): partial grounding too restrictive",
            result->act->terms[1] == NULL);
  mu_assert("Error utils_actionFixesGap(), wrong partial grounding.\n",
            result->act->terms[2] == NULL);
  mu_assert("Error utils_actionFixesGap(): partial grounding too restrictive",
            result->act->terms[3] == NULL);

  utils_free_actionList(resultHead);


  free(gap->literal);
  free(gap);
  libpddl31_problem_free(problem);
  libpddl31_domain_free(domain);
  return 0;
}

static char *
test_actionFixesGap_advanced_typed()
{
  char *domainFilename = "test/c/utils-domain.pddl";
  struct domain *domain = libpddl31_domain_parse(domainFilename);
  char *problemFilename = "test/c/utils-problem0.pddl";
  struct problem *problem = libpddl31_problem_parse(domain, problemFilename);

  // Prepare gap.
  struct gap *gap = malloc(sizeof(*gap));
  gap->literal = malloc(sizeof(*gap->literal));
  gap->position = 0;
  gap->literal->isPos = true;
  // Just take positive literal from goal.
  gap->literal->atom = &problem->goal->posLiterals[2];
  //libpddl31_atom_print(gap->literal->atom); // DEBUG
  //printf("\n"); // DEBUG

  struct action *action = NULL;
  struct actionList *result = NULL;
  struct actionList *resultHead = NULL;


  // Prepare action.
  action = actionManag_getAction(domain->actionManag, "a16");


  // Get partial groundings for action, which fix the gap.
  result = utils_actionFixesGap(action, gap);
  resultHead = result;
  //printf("utils_actionList_length(result): %d\n", // DEBUG
  //       utils_actionList_length(result)); // DEBUG
  //utils_print_actionList(result); // DEBUG
  //printf("\n"); // DEBUG
  mu_assert("Error utils_actionFixesGap(): results with typed action\n",
            utils_actionList_length(result) == 2);
  mu_assert("Error utils_actionFixesGap(): results with typed action\n",
            result->act->terms[0] == NULL);
  mu_assert("Error utils_actionFixesGap(): results with typed action\n",
            strcmp(result->act->terms[1]->name, "obj3") == 0);
  mu_assert("Error utils_actionFixesGap(): results with typed action\n",
            result->act->terms[2] == NULL);
  mu_assert("Error utils_actionFixesGap(): results with typed action\n",
            strcmp(result->act->terms[3]->name, "obj2") == 0);
  result = result->next;
  mu_assert("Error utils_actionFixesGap(): results with typed action\n",
            strcmp(result->act->terms[0]->name, "obj2") == 0);
  mu_assert("Error utils_actionFixesGap(): results with typed action\n",
            strcmp(result->act->terms[1]->name, "obj3") == 0);
  mu_assert("Error utils_actionFixesGap(): results with typed action\n",
            result->act->terms[2] == NULL);
  mu_assert("Error utils_actionFixesGap(): results with typed action\n",
            result->act->terms[3] == NULL);

  utils_free_actionList(resultHead);


  free(gap->literal);
  free(gap);
  libpddl31_problem_free(problem);
  libpddl31_domain_free(domain);
  return 0;
}

static char *
test_groundActions_simple()
{
  char *domainFilename = "test/c/utils-domain.pddl";
  struct domain *domain = libpddl31_domain_parse(domainFilename);
  char *problemFilename = "test/c/utils-problem0.pddl";
  struct problem *problem = libpddl31_problem_parse(domain, problemFilename);

  struct action *action = NULL;
  struct actionList *result = NULL;
  struct actionList *resultHead = NULL;

  action = actionManag_getAction(domain->actionManag, "a10");
  struct groundAction *grAct = libpddl31_create_groundAction(action);


  /*** Try with empty grounding ***/

  struct actionList singletonL;
  singletonL.act = grAct;
  singletonL.next = NULL;

  result = utils_groundActions(problem, &singletonL);
  resultHead = result;
  //printf("utils_actionList_length(result): %d\n", // DEBUG
  //       utils_actionList_length(result)); // DEBUG
  //utils_print_actionList(result); // DEBUG
  //printf("\n"); // DEBUG

  // We are working with an action with a single (by types) unconstained 
  // argument. The number of ground actions should be the number of objects
  // in the problem.
  mu_assert("Error grounding empty partial grounding and unconstrained (by"
            " types) action",
            utils_actionList_length(result) == problem->objManag->numOfObjs);
  utils_free_actionList(resultHead);


  /*** Try with fully grounded partial grounding ***/

  grAct->terms[0] = objManag_getObject(problem->objManag, "obj0");

  result = utils_groundActions(problem, &singletonL);
  resultHead = result;
  //printf("utils_actionList_length(result): %d\n", // DEBUG
  //       utils_actionList_length(result)); // DEBUG
  //utils_print_actionList(result); // DEBUG
  //printf("\n"); // DEBUG

  // The reslut must be a single element list with the given mapping.
  mu_assert("Error calling grounder on fully grounded action\n",
            utils_actionList_length(result) == 1);
  mu_assert("Error calling grounder on fully grounded action\n",
            strcmp(result->act->terms[0]->name, "obj0") == 0);

  utils_free_actionList(resultHead);


  libpddl31_free_groundAction(grAct);


  libpddl31_problem_free(problem);
  libpddl31_domain_free(domain);
  return 0;
}

static char *
test_groundActions_basic()
{
  char *domainFilename = "test/c/utils-domain.pddl";
  struct domain *domain = libpddl31_domain_parse(domainFilename);
  char *problemFilename = "test/c/utils-problem0.pddl";
  struct problem *problem = libpddl31_problem_parse(domain, problemFilename);

  struct action *action = NULL;
  struct actionList *result = NULL;
  struct actionList *resultHead = NULL;
  struct groundAction *grAct = NULL;

  action = actionManag_getAction(domain->actionManag, "a11");
  grAct = libpddl31_create_groundAction(action); // empty ground action.

  /*** Action 'a11' got two unground variables. ***/

  struct actionList singletonL;
  singletonL.act = grAct;
  singletonL.next = NULL;

  result = utils_groundActions(problem, &singletonL);
  resultHead = result;
  //printf("utils_actionList_length(result): %d\n", // DEBUG
  //       utils_actionList_length(result)); // DEBUG
  //utils_print_actionList(result); // DEBUG
  //printf("\n"); // DEBUG

  int32_t count = problem->objManag->numOfObjs * problem->objManag->numOfObjs;
  mu_assert("Error grounding two variable action",
            utils_actionList_length(result) == count);
  utils_free_actionList(resultHead);



  /*** One variable out of two is set before grounding. ***/

  grAct->terms[1] = objManag_getObject(problem->objManag, "obj1");

  result = utils_groundActions(problem, &singletonL);
  resultHead = result;
  //printf("utils_actionList_length(result): %d\n", // DEBUG
  //       utils_actionList_length(result)); // DEBUG
  //utils_print_actionList(result); // DEBUG
  //printf("\n"); // DEBUG

  // Exactly one out of two variables to ground will produce a grounding for
  // each object in the problems object manager.
  mu_assert("Error grounding partial grounding and unconstrained (by types)"
            " action",
            utils_actionList_length(result) == problem->objManag->numOfObjs);
  utils_free_actionList(resultHead);



  libpddl31_free_groundAction(grAct);

  libpddl31_problem_free(problem);
  libpddl31_domain_free(domain);
  return 0;
}

static char *
test_groundActions_typeChecks()
{
  char *domainFilename = "test/c/utils-domain.pddl";
  struct domain *domain = libpddl31_domain_parse(domainFilename);
  char *problemFilename = "test/c/utils-problem0.pddl";
  struct problem *problem = libpddl31_problem_parse(domain, problemFilename);

  struct action *action = NULL;
  struct actionList *result = NULL;
  struct actionList *resultHead = NULL;
  struct groundAction *grAct = NULL;

  action = actionManag_getAction(domain->actionManag, "a12");
  grAct = libpddl31_create_groundAction(action); // empty ground action.

  /*** Action 'a11' got two unground variables. ***/

  struct actionList singletonL;
  singletonL.act = grAct;
  singletonL.next = NULL;

  result = utils_groundActions(problem, &singletonL);
  resultHead = result;
  //printf("utils_actionList_length(result): %d\n", // DEBUG
  //       utils_actionList_length(result)); // DEBUG
  //utils_print_actionList(result); // DEBUG
  //printf("\n"); // DEBUG

  mu_assert("Error grounding two variable action with types",
            utils_actionList_length(result) == 3);

  mu_assert("Error utils_actionFixesGap(), wrong partial grounding.\n",
            strcmp(result->act->terms[0]->name, "obj3") == 0);
  mu_assert("Error utils_actionFixesGap(): partial grounding too restrictive",
            strcmp(result->act->terms[1]->name, "obj3") == 0);
  result = result->next;
  mu_assert("Error utils_actionFixesGap(), wrong partial grounding.\n",
            strcmp(result->act->terms[0]->name, "obj2") == 0);
  mu_assert("Error utils_actionFixesGap(): partial grounding too restrictive",
            strcmp(result->act->terms[1]->name, "obj3") == 0);
  result = result->next;
  mu_assert("Error utils_actionFixesGap(), wrong partial grounding.\n",
            strcmp(result->act->terms[0]->name, "const1") == 0);
  mu_assert("Error utils_actionFixesGap(): partial grounding too restrictive",
            strcmp(result->act->terms[1]->name, "obj3") == 0);

  utils_free_actionList(resultHead);
  libpddl31_free_groundAction(grAct);



  action = actionManag_getAction(domain->actionManag, "a13");
  grAct = libpddl31_create_groundAction(action); // empty ground action.
  singletonL.act = grAct;

  result = utils_groundActions(problem, &singletonL);
  resultHead = result;
  //printf("utils_actionList_length(result): %d\n", // DEBUG
  //       utils_actionList_length(result)); // DEBUG
  //utils_print_actionList(result); // DEBUG
  //printf("\n"); // DEBUG

  mu_assert("Error grounding two variable action with types",
            utils_actionList_length(result) == 1);
  utils_free_actionList(resultHead);



  libpddl31_free_groundAction(grAct);

  libpddl31_problem_free(problem);
  libpddl31_domain_free(domain);
  return 0;
}

static char *
test_groundActions_multipleActions()
{
  char *domainFilename = "test/c/utils-domain.pddl";
  struct domain *domain = libpddl31_domain_parse(domainFilename);
  char *problemFilename = "test/c/utils-problem0.pddl";
  struct problem *problem = libpddl31_problem_parse(domain, problemFilename);

  struct action *action = NULL;
  struct actionList *result = NULL;
  struct actionList *resultHead = NULL;
  struct groundAction *grAct = NULL;

  struct actionList listE1;
  struct actionList listE2;
  action = actionManag_getAction(domain->actionManag, "a14");
  listE1.act = libpddl31_create_groundAction(action);
  listE1.next = &listE2;
  action = actionManag_getAction(domain->actionManag, "a15");
  listE2.act = libpddl31_create_groundAction(action);
  listE2.next = NULL;

  result = utils_groundActions(problem, &listE1);
  resultHead = result;
  //printf("utils_actionList_length(result): %d\n", // DEBUG
  //       utils_actionList_length(result)); // DEBUG
  //utils_print_actionList(result); // DEBUG
  //printf("\n"); // DEBUG

  mu_assert("Error grounding two variable action with types",
            utils_actionList_length(result) == 4);

  mu_assert("Error wrong grounding of list of partially grounded actions.\n",
            result->act->action->numOfParams == 2);
  mu_assert("Error wrong grounding of list of partially grounded actions.\n",
            strcmp(result->act->terms[0]->name, "obj3") == 0);
  mu_assert("Error wrong grounding of list of partially grounded actions.\n",
            strcmp(result->act->terms[1]->name, "obj3") == 0);
  result = result->next;
  mu_assert("Error wrong grounding of list of partially grounded actions.\n",
            result->act->action->numOfParams == 1);
  mu_assert("Error wrong grounding of list of partially grounded actions.\n",
            strcmp(result->act->terms[0]->name, "obj3") == 0);
  result = result->next;
  mu_assert("Error wrong grounding of list of partially grounded actions.\n",
            result->act->action->numOfParams == 1);
  mu_assert("Error wrong grounding of list of partially grounded actions.\n",
            strcmp(result->act->terms[0]->name, "obj2") == 0);
  result = result->next;
  mu_assert("Error wrong grounding of list of partially grounded actions.\n",
            result->act->action->numOfParams == 1);
  mu_assert("Error wrong grounding of list of partially grounded actions.\n",
            strcmp(result->act->terms[0]->name, "const1") == 0);

  libpddl31_free_groundAction(listE1.act);
  libpddl31_free_groundAction(listE2.act);

  utils_free_actionList(resultHead);


  libpddl31_problem_free(problem);
  libpddl31_domain_free(domain);
  return 0;
}

static char *
allTests()
{
  // Tests for partialy grounding an action according to a gap.
  mu_run_test(test_actionFixesGap_simple);
  mu_run_test(test_actionFixesGap_advanced_positive);
  mu_run_test(test_actionFixesGap_advanced_negative);
  mu_run_test(test_actionFixesGap_advanced_when);
  mu_run_test(test_actionFixesGap_advanced_forall);
  mu_run_test(test_actionFixesGap_advanced_typed);

  // Tests for grounding a partially grounded set of actions.
  mu_run_test(test_groundActions_simple);
  mu_run_test(test_groundActions_basic);
  mu_run_test(test_groundActions_typeChecks);
  mu_run_test(test_groundActions_multipleActions);

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
