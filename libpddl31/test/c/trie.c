#include "minunit.h"
#include "trie.h"

#include "libpddl31.h"

int tests_run = 0;

static char *
test_createAFree()
{
  char *domainFilename = "test/c/trie-domain.pddl";
  struct domain *domain = libpddl31_domain_parse(domainFilename);

  trie_t trie = trie_createEmpty(domain);

  trie_free(trie);
  trie_cleanupSNBuffer();
  libpddl31_domain_free(domain);
  return 0;
}

static char *
test_containsAddDelete()
{
  char *domainFilename = "test/c/trie-domain.pddl";
  struct domain *domain = libpddl31_domain_parse(domainFilename);

  trie_t trie = trie_createEmpty(domain);
  struct atom *atom = malloc(sizeof(*atom));
  atom->pred = predManag_getPred(domain->predManag, "p0");
  // predicate "p0" has only one parameter.
  atom->terms = malloc(sizeof(*atom->terms) * atom->pred->numOfParams);
  atom->terms[0] = objManag_getObject(domain->objManag, "const0");

  //trie_print(trie); // DEBUG
  //printf("\n"); // DEBUG

  mu_assert("Error trie_conatins() returns true on empty trie.",
            ! trie_contains(trie, atom));

  trie_add(trie, atom);

  //trie_print(trie); // DEBUG
  //printf("\n"); // DEBUG

  mu_assert("Error trie_conatins() returns false on added atom.",
            trie_contains(trie, atom));

  trie_remove(trie, atom);

  mu_assert("Error trie_conatins() returns true after removing atom.",
            ! trie_contains(trie, atom));

  trie_add(trie, atom);

  atom->terms[0] = objManag_getObject(domain->objManag, "const1");

  trie_add(trie, atom);

  atom->terms[0] = objManag_getObject(domain->objManag, "const0");

  trie_remove(trie, atom);

  mu_assert("Error trie_conatins().",
            ! trie_contains(trie, atom));

  atom->terms[0] = objManag_getObject(domain->objManag, "const1");

  mu_assert("Error trie_conatins().",
            trie_contains(trie, atom));

  //trie_print(trie); // DEBUG
  //printf("\n"); // DEBUG

  libpddl31_atom_free(atom);
  free(atom);
  trie_free(trie);
  trie_cleanupSNBuffer();
  libpddl31_domain_free(domain);
  return 0;
}

static char *
test_clone()
{
  char *domainFilename = "test/c/trie-domain.pddl";
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

  trie_t trie0 = trie_createEmpty(domain);

  trie_add(trie0, atom0);
  trie_add(trie0, atom1);

  trie_t trie1 = trie_clone(trie0);

  mu_assert("Error trie_clone().",
            trie_contains(trie0, atom0) &&
            trie_contains(trie0, atom1) &&
            trie_contains(trie1, atom0) &&
            trie_contains(trie1, atom1)
           );

  trie_remove(trie0, atom1);

  mu_assert("Error trie_clone().",
            trie_contains(trie0, atom0) &&
            ! trie_contains(trie0, atom1) &&
            trie_contains(trie1, atom0) &&
            trie_contains(trie1, atom1)
           );

  trie_remove(trie1, atom0);

  mu_assert("Error trie_clone().",
            trie_contains(trie0, atom0) &&
            ! trie_contains(trie0, atom1) &&
            ! trie_contains(trie1, atom0) &&
            trie_contains(trie1, atom1)
           );

  trie_remove(trie0, atom0);
  trie_add(trie1, atom0);

  mu_assert("Error trie_clone().",
            ! trie_contains(trie0, atom0) &&
            ! trie_contains(trie0, atom1) &&
            trie_contains(trie1, atom0) &&
            trie_contains(trie1, atom1)
           );

  //trie_print(trie0); // DEBUG
  //printf("\n"); // DEBUG
  //trie_print(trie1); // DEBUG
  //printf("\n"); // DEBUG

  libpddl31_atom_free(atom0);
  free(atom0);
  libpddl31_atom_free(atom1);
  free(atom1);
  trie_free(trie0);
  trie_free(trie1);
  trie_cleanupSNBuffer();
  libpddl31_domain_free(domain);
  return 0;
}

static char *
test_addRemoveWithGrounding()
{
  char *domainFilename = "test/c/trie-domain.pddl";
  struct domain *domain = libpddl31_domain_parse(domainFilename);
  char *problemFilename = "test/c/trie-problem.pddl";
  struct problem *problem = libpddl31_problem_parse(domain, problemFilename);

  struct action *action0 = actionManag_getAction(domain->actionManag,
                                                 "action0");
  struct groundAction *grAct0 = libpddl31_create_groundAction(action0);
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

  struct groundAction *grAct1 = libpddl31_create_groundAction(action0);
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

  struct groundAction *grAct2 = libpddl31_create_groundAction(action0);
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

  trie_t trie = trie_createEmpty(domain);

  trie_addGr(trie, grAct0->action->effect->elems[0].it.literal, grAct0);
  trie_addGr(trie, grAct1->action->effect->elems[0].it.literal, grAct1);

  //trie_print(trie); // DEBUG
  //printf("\n"); // DEBUG

  mu_assert("Error trie_addGr()",
            // Check with ordinary atom.
            trie_contains(trie, atom0) &&
            trie_contains(trie, atom1) &&
            ! trie_contains(trie, atom2) &&

            // And also check with ground atom.
            trie_containsGr(trie,
                             grAct0->action->effect->elems[0].it.literal,
                             grAct0) &&
            trie_containsGr(trie,
                             grAct1->action->effect->elems[0].it.literal,
                             grAct1) && 
          ! trie_containsGr(trie,
                             grAct2->action->effect->elems[0].it.literal,
                             grAct2)
            );

  trie_removeGr(trie, grAct2->action->effect->elems[0].it.literal, grAct2);
  trie_removeGr(trie, grAct0->action->effect->elems[0].it.literal, grAct0);

  mu_assert("Error trie_removeGr()",
             ! trie_containsGr(trie,
                                grAct0->action->effect->elems[0].it.literal,
                                grAct0) &&
               trie_containsGr(trie,
                                grAct1->action->effect->elems[0].it.literal,
                                grAct1) &&
             ! trie_containsGr(trie,
                                grAct2->action->effect->elems[0].it.literal,
                                grAct2)
             );

  trie_addGr(trie, grAct1->action->effect->elems[0].it.literal, grAct1);

  mu_assert("Error trie_removeGr()",
             ! trie_containsGr(trie,
                                grAct0->action->effect->elems[0].it.literal,
                                grAct0) &&
               trie_containsGr(trie,
                                grAct1->action->effect->elems[0].it.literal,
                                grAct1) &&
             ! trie_containsGr(trie,
                                grAct2->action->effect->elems[0].it.literal,
                                grAct2)
            );

  trie_addGr(trie, grAct0->action->effect->elems[0].it.literal, grAct0);
  trie_addGr(trie, grAct2->action->effect->elems[0].it.literal, grAct2);

  mu_assert("Error trie_removeGr()",
               trie_containsGr(trie,
                                grAct0->action->effect->elems[0].it.literal,
                                grAct0) &&
               trie_containsGr(trie,
                                grAct1->action->effect->elems[0].it.literal,
                                grAct1) &&
               trie_containsGr(trie,
                                grAct2->action->effect->elems[0].it.literal,
                                grAct2)
            );

  trie_removeGr(trie, grAct1->action->effect->elems[0].it.literal, grAct1);

  mu_assert("Error trie_removeGr()",
               trie_containsGr(trie,
                                grAct0->action->effect->elems[0].it.literal,
                                grAct0) &&
             ! trie_containsGr(trie,
                                grAct1->action->effect->elems[0].it.literal,
                                grAct1) &&
               trie_containsGr(trie,
                                grAct2->action->effect->elems[0].it.literal,
                                grAct2)
            );

  trie_removeGr(trie, grAct2->action->effect->elems[0].it.literal, grAct2);

  mu_assert("Error trie_removeGr()",
               trie_containsGr(trie,
                                grAct0->action->effect->elems[0].it.literal,
                                grAct0) &&
             ! trie_containsGr(trie,
                                grAct1->action->effect->elems[0].it.literal,
                                grAct1) &&
             ! trie_containsGr(trie,
                                grAct2->action->effect->elems[0].it.literal,
                                grAct2)
            );

  trie_removeGr(trie, grAct0->action->effect->elems[0].it.literal, grAct0);

  mu_assert("Error trie_removeGr()",
             ! trie_containsGr(trie,
                                grAct0->action->effect->elems[0].it.literal,
                                grAct0) &&
             ! trie_containsGr(trie,
                                grAct1->action->effect->elems[0].it.literal,
                                grAct1) &&
             ! trie_containsGr(trie,
                                grAct2->action->effect->elems[0].it.literal,
                                grAct2)
            );

  //trie_print(trie); // DEBUG
  //printf("\n"); // DEBUG

  // Clean up.
  trie_free(trie);
  trie_cleanupSNBuffer();
  libpddl31_free_groundAction(grAct0);
  libpddl31_free_groundAction(grAct1);
  libpddl31_free_groundAction(grAct2);
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
