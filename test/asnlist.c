#include <stdio.h>

#include "minunit.h"

#include "asnlist.h"
#include "list.h"
#include "probSpace.h"

int tests_run = 0;

static
void
printActionList(void *al)
{
  utils_print_actionListCompact((struct actionList *) al);
}

static char *
test_asnList_createPushRemove()
{
  char *domainFilename = "test/asnlist-domain.pddl";
  struct domain *domain = libpddl31_domain_parse(domainFilename);
  char *problemFilename = "test/asnlist-problem.pddl";
  struct problem *problem = libpddl31_problem_parse(domain, problemFilename);
  struct probSpace *probSpace = ps_init(problem);

  struct groundAction *grAct1 = probSpace->allGrActs->act;
  struct groundAction *grAct2 = probSpace->allGrActs->next->act;
  struct actionList al1;
  struct actionList *actL1 = &al1;
  actL1->act = grAct1;
  actL1->next = NULL;
  struct actionList al2;
  struct actionList *actL2 = &al2;
  actL2->act = grAct2;
  actL2->next = NULL;

  asnList_t asnl = asnList_createEmpty();

  mu_assert("Error asnList_find() on empty list finds something",
            asnList_find(asnl, actL1) == NULL &&
            asnList_find(asnl, actL2) == NULL);

  asnl = asnList_push(asnl, actL1);

  //list_print(asnl->list, &printActionList);

  mu_assert("Error asnList_find() did not find when it should",
            asnList_find(asnl, actL1) != NULL);
  mu_assert("Error asnList_find() did find when it should not",
            asnList_find(asnl, actL2) == NULL);

  asnl = asnList_push(asnl, actL2);

  mu_assert("Error asnList_find() did not find when it should",
            asnList_find(asnl, actL1) != NULL);
  mu_assert("Error asnList_find() did not find when it should",
            asnList_find(asnl, actL2) != NULL);

  //printf("###");
  //utils_print_actionListCompact(asnList_find(asnl, actL1)->payload);
  //printf("\n###");
  //printf("###");
  //utils_print_actionListCompact(asnList_find(asnl, actL2)->payload);
  //printf("\n###");

  asnl = asnList_removeFirst(asnl);

  mu_assert("Error asnList_find() did not find when it should",
            asnList_find(asnl, actL1) != NULL);
  mu_assert("Error asnList_find() did find when it should not",
            asnList_find(asnl, actL2) == NULL);

  asnl = asnList_removeFirst(asnl);

  mu_assert("Error asnList_find() did not find when it should",
            asnList_find(asnl, actL1) == NULL);
  mu_assert("Error asnList_find() did find when it should not",
            asnList_find(asnl, actL2) == NULL);

  asnl = asnList_push(asnl, actL1);

  mu_assert("Error asnList_find() did not find when it should",
            asnList_find(asnl, actL1) != NULL);
  mu_assert("Error asnList_find() did find when it should not",
            asnList_find(asnl, actL2) == NULL);

  // Try to remove non-e3xisting element.
  asnl = asnList_remove(asnl, actL2);

  // Nothing should have changed.
  mu_assert("Error asnList_find() did not find when it should",
            asnList_find(asnl, actL1) != NULL);
  mu_assert("Error asnList_find() did find when it should not",
            asnList_find(asnl, actL2) == NULL);

  asnl = asnList_remove(asnl, actL1);

  mu_assert("Error asnList_find() did not find when it should",
            asnList_find(asnl, actL1) == NULL);
  mu_assert("Error asnList_find() did find when it should not",
            asnList_find(asnl, actL2) == NULL);

  ps_free(probSpace);
  libpddl31_problem_free(problem);
  libpddl31_domain_free(domain);
  return 0;
}

static char *
test_asnList_InsertRemove()
{
  char *domainFilename = "test/asnlist-domain.pddl";
  struct domain *domain = libpddl31_domain_parse(domainFilename);
  char *problemFilename = "test/asnlist-problem.pddl";
  struct problem *problem = libpddl31_problem_parse(domain, problemFilename);
  struct probSpace *probSpace = ps_init(problem);

  ps_free(probSpace);
  libpddl31_problem_free(problem);
  libpddl31_domain_free(domain);

  return 0;
}

static
char *
allTests()
{
  mu_run_test(test_asnList_createPushRemove);

  //trie_cleanupSNBuffer();

  return 0;
}

int
main(int argc, char **argv)
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
