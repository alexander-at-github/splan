#include "minunit.h"
#include "state.h"

int tests_run = 0;

static char *
test_createAFree()
{
  char *domainFilename = "test/state-domain.pddl";
  struct domain *domain = libpddl31_domain_parse(domainFilename);

  state_t state = state_createEmpty(domain);

  state_free(state);
  libpddl31_domain_free(domain);
  return 0;
}

static char *
test_containsAddDelete()
{
  char *domainFilename = "test/state-domain.pddl";
  struct domain *domain = libpddl31_domain_parse(domainFilename);

  state_t state = state_createEmpty(domain);
  struct atom *atom = malloc(sizeof(*atom));
  atom->pred = predManag_getPred(domain->predManag, "p0");
  // predicate "p0" has only one parameter.
  atom->terms = malloc(sizeof(*atom->terms) * atom->pred->numOfParams);
  atom->terms[0] = objManag_getObject(domain->objManag, "const0");

  //state_print(state); // DEBUG
  //printf("\n"); // DEBUG

  mu_assert("Error state_conatins() returns true on empty state.",
            ! state_contains(state, atom));

  state_add(state, atom);

  //state_print(state); // DEBUG
  //printf("\n"); // DEBUG

  mu_assert("Error state_conatins() returns false on added atom.",
            state_contains(state, atom));

  state_remove(state, atom);

  mu_assert("Error state_conatins() returns true after removing atom.",
            ! state_contains(state, atom));

  state_add(state, atom);

  atom->terms[0] = objManag_getObject(domain->objManag, "const1");

  state_add(state, atom);

  atom->terms[0] = objManag_getObject(domain->objManag, "const0");

  state_remove(state, atom);

  mu_assert("Error state_conatins().",
            ! state_contains(state, atom));

  atom->terms[0] = objManag_getObject(domain->objManag, "const1");

  mu_assert("Error state_conatins().",
            state_contains(state, atom));

  //state_print(state); // DEBUG
  //printf("\n"); // DEBUG

  libpddl31_atom_free(atom);
  free(atom);
  state_free(state);
  libpddl31_domain_free(domain);
  return 0;
}

static char *
test_clone()
{
  char *domainFilename = "test/state-domain.pddl";
  struct domain *domain = libpddl31_domain_parse(domainFilename);

  struct atom *atom0 = malloc(sizeof(*atom0));
  atom0->pred = predManag_getPred(domain->predManag, "p0");
  // predicate "p0" has only one parameter.
  atom0->terms = malloc(sizeof(*atom0->terms) * atom0->pred->numOfParams);
  atom0->terms[0] = objManag_getObject(domain->objManag, "const0");

  //libpddl31_atom_print(atom0); // DEBUG
  //printf("\n"); // DEBUG

  struct atom *atom1 = malloc(sizeof(*atom1));
  atom1->pred = predManag_getPred(domain->predManag, "p0");
  // predicate "p0" has only one parameter.
  atom1->terms = malloc(sizeof(*atom1->terms) * atom1->pred->numOfParams);
  atom1->terms[0] = objManag_getObject(domain->objManag, "const1");

  state_t state0 = state_createEmpty(domain);

  state_add(state0, atom0);
  state_add(state0, atom1);

  state_t state1 = state_clone(state0);

  mu_assert("Error state_clone().",
            state_contains(state0, atom0) &&
            state_contains(state0, atom1) &&
            state_contains(state1, atom0) &&
            state_contains(state1, atom1)
           );

  state_remove(state0, atom1);

  mu_assert("Error state_clone().",
            state_contains(state0, atom0) &&
            ! state_contains(state0, atom1) &&
            state_contains(state1, atom0) &&
            state_contains(state1, atom1)
           );

  state_remove(state1, atom0);

  mu_assert("Error state_clone().",
            state_contains(state0, atom0) &&
            ! state_contains(state0, atom1) &&
            ! state_contains(state1, atom0) &&
            state_contains(state1, atom1)
           );

  state_remove(state0, atom0);
  state_add(state1, atom0);

  mu_assert("Error state_clone().",
            ! state_contains(state0, atom0) &&
            ! state_contains(state0, atom1) &&
            state_contains(state1, atom0) &&
            state_contains(state1, atom1)
           );

  //state_print(state0); // DEBUG
  //printf("\n"); // DEBUG
  //state_print(state1); // DEBUG
  //printf("\n"); // DEBUG

  libpddl31_atom_free(atom0);
  free(atom0);
  libpddl31_atom_free(atom1);
  free(atom1);
  state_free(state0);
  state_free(state1);
  libpddl31_domain_free(domain);
  return 0;
}

static char *
test_addRemoveWithGrounding()
{
  char *domainFilename = "test/state-domain.pddl";
  struct domain *domain = libpddl31_domain_parse(domainFilename);
  char *problemFilename = "test/state-problem.pddl";
  struct problem *problem = libpddl31_problem_parse(domain, problemFilename);

  struct action *action0 = actionManag_getAction(domain->actionManag,
                                                 "action0");
  struct groundAction *grAct0 = utils_create_groundAction(action0);
  grAct0->terms[0] = objManag_getObject(problem->objManag, "obj0");
  grAct0->terms[1] = objManag_getObject(problem->objManag, "obj1");
  grAct0->terms[2] = objManag_getObject(problem->objManag, "obj2");
  // Same atom as the single effect element of the ground action above.
  struct atom *atom0 = malloc(sizeof(*atom0));
  atom0->pred = predManag_getPred(domain->predManag, "p2");
  // predicate "p0" has only one parameter.
  atom0->terms = malloc(sizeof(*atom0->terms) * atom0->pred->numOfParams);
  atom0->terms[0] = objManag_getObject(problem->objManag, "obj0");
  atom0->terms[1] = objManag_getObject(problem->objManag, "obj1");
  atom0->terms[2] = objManag_getObject(problem->objManag, "obj2");

  struct groundAction *grAct1 = utils_create_groundAction(action0);
  grAct1->terms[0] = objManag_getObject(problem->objManag, "obj0");
  grAct1->terms[1] = objManag_getObject(problem->objManag, "obj1");
  grAct1->terms[2] = objManag_getObject(problem->objManag, "obj3");
  // Same atom as the single effect element of the ground action above.
  struct atom *atom1 = malloc(sizeof(*atom1));
  atom1->pred = predManag_getPred(domain->predManag, "p2");
  // predicate "p0" has only one parameter.
  atom1->terms = malloc(sizeof(*atom1->terms) * atom1->pred->numOfParams);
  atom1->terms[0] = objManag_getObject(problem->objManag, "obj0");
  atom1->terms[1] = objManag_getObject(problem->objManag, "obj1");
  atom1->terms[2] = objManag_getObject(problem->objManag, "obj3");

  struct groundAction *grAct2 = utils_create_groundAction(action0);
  grAct2->terms[0] = objManag_getObject(problem->objManag, "obj0");
  grAct2->terms[1] = objManag_getObject(problem->objManag, "obj3");
  grAct2->terms[2] = objManag_getObject(problem->objManag, "const0");
  // Same atom as the single effect element of the ground action above.
  struct atom *atom2 = malloc(sizeof(*atom2));
  atom2->pred = predManag_getPred(domain->predManag, "p2");
  // predicate "p0" has only one parameter.
  atom2->terms = malloc(sizeof(*atom2->terms) * atom2->pred->numOfParams);
  atom2->terms[0] = objManag_getObject(problem->objManag, "obj0");
  atom2->terms[1] = objManag_getObject(problem->objManag, "obj3");
  atom2->terms[2] = objManag_getObject(problem->objManag, "const0");

  state_t state = state_createEmpty(domain);

  // TODO: continue here.

  state_addGr(state, grAct0->action->effect->elems[0].it.literal, grAct0);
  state_addGr(state, grAct1->action->effect->elems[0].it.literal, grAct1);

  //state_print(state); // DEBUG
  //printf("\n"); // DEBUG

  mu_assert("Error state_addGr()",
            state_contains(state, atom0) && state_contains(state, atom1));

  state_removeGr(state, grAct2->action->effect->elems[0].it.literal, grAct2);
  state_removeGr(state, grAct0->action->effect->elems[0].it.literal, grAct0);

  mu_assert("Error state_removeGr()",
            ! state_contains(state, atom0) &&
            state_contains(state, atom1) &&
            ! state_contains(state, atom2));

  state_addGr(state, grAct1->action->effect->elems[0].it.literal, grAct1);

  mu_assert("Error state_removeGr()",
            ! state_contains(state, atom0) &&
            state_contains(state, atom1) &&
            ! state_contains(state, atom2));

  state_addGr(state, grAct0->action->effect->elems[0].it.literal, grAct0);
  state_addGr(state, grAct2->action->effect->elems[0].it.literal, grAct2);

  mu_assert("Error state_removeGr()",
            state_contains(state, atom0) &&
            state_contains(state, atom1) &&
            state_contains(state, atom2));

  state_removeGr(state, grAct1->action->effect->elems[0].it.literal, grAct1);

  mu_assert("Error state_removeGr()",
            state_contains(state, atom0) &&
            ! state_contains(state, atom1) &&
            state_contains(state, atom2));

  state_removeGr(state, grAct2->action->effect->elems[0].it.literal, grAct2);

  mu_assert("Error state_removeGr()",
            state_contains(state, atom0) &&
            ! state_contains(state, atom1) &&
            ! state_contains(state, atom2));

  state_removeGr(state, grAct0->action->effect->elems[0].it.literal, grAct0);

  mu_assert("Error state_removeGr()",
            ! state_contains(state, atom0) &&
            ! state_contains(state, atom1) &&
            ! state_contains(state, atom2));

  //state_print(state); // DEBUG
  //printf("\n"); // DEBUG

  // Clean up.
  state_free(state);
  utils_free_groundAction(grAct0);
  utils_free_groundAction(grAct1);
  utils_free_groundAction(grAct2);
  libpddl31_atom_free(atom0);
  free(atom0);
  libpddl31_atom_free(atom1);
  free(atom1);
  libpddl31_atom_free(atom2);
  free(atom2);
  libpddl31_problem_free(problem);
  libpddl31_domain_free(domain);
  return 0;
}

static char *
allTests()
{
  mu_run_test(test_createAFree);
  mu_run_test(test_containsAddDelete);
  mu_run_test(test_clone);
  mu_run_test(test_addRemoveWithGrounding);

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
