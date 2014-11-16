#include <assert.h>

#include "grounder.h"

#include "libpddl31.h"


static void
grounder_freeGrounding(struct grounding *g)
{
  if (g == NULL) {
    return;
  }
  if (g->terms != NULL) {
    free(g->terms);
  }
  free(g);
}

static bool
term_equal(struct term *t1, struct term *t2)
{
  return // t1->isVariable == t2->isVariable && // Not neccessary.
         (t1->name == t2->name || strcmp(t1->name, t2->name) == 0);
         // DO NOT compare types. Not any use of a variable also specifies its
         // type.
         //t1->type == t2->type; // Comparing pointers to types really has to be
                               // enough.
}

static struct grounding *
grounder_copyGrounding(struct grounding *src, size_t numOfTerms)
{
  struct grounding *result = malloc(sizeof(*result));
  if (result == NULL) {
    fprintf(stderr, "Error allocating memory.\n");
    exit(EXIT_FAILURE);
  }
  result->terms = malloc(sizeof(*result->terms) * numOfTerms);
  if (result->terms == NULL) {
    fprintf(stderr, "Error allocating memory.\n");
    exit(EXIT_FAILURE);
  }
  memcpy(result->terms, src->terms, sizeof(result->terms[0]) * numOfTerms);
  return result;
}

// TODO: This is only a dummy for now.
void
grounder_getActions(struct domain *domain, struct state *state)
{
  for (size_t i = 0; i < domain->actionManag->numOfActions; ++i) {
    struct action *action = &domain->actionManag->actions[i];

    struct groundAction grAct;
    grAct.action = action;
    grAct.numOfGrnds = 0;
    grAct.grnds = NULL;

    struct grounding partialGrndng;
    partialGrndng.terms = malloc(sizeof(*partialGrndng.terms) *
                                 action->numOfParams);
    for (size_t j = 0; j < action->numOfParams; ++j) {
      partialGrndng.terms[j] = NULL;
    }

    grounder_groundAction(state, 0, &partialGrndng, &grAct);

    // TODO: Remove groundings, which are inconsistent with negative
    // preconditions.
  }
}

// This function considers only positive preconditions.
// TODO: Add to the function name 'posLiterals'.
void
grounder_groundAction(struct state *state,
                      int32_t idxPrecond, // Index into precondition
                      struct grounding *partialG, // A parial grounding
                      struct groundAction *grAct)
{
  struct action *action = grAct->action;
  struct goal *precond = action->precond;

  // Base case.
  if (idxPrecond >= precond->numOfPos) {

    // Check if the partial grounding is complete.
    for (size_t i = 0; i < action->numOfParams; ++i) {
      struct term *term = partialG->terms[i];
      if (term == NULL) {
        // The partial grounding is not complete. We can not use it.
        return;
      }
    }
    // Grounding is complete. Copy it into the result.
    grAct->numOfGrnds++;
    struct grounding **tmp = realloc(grAct->grnds,
                                     sizeof(*tmp) * grAct->numOfGrnds);
    if (tmp == NULL) {
      fprintf(stderr, "Error allocating memory for grounding.\n");
      exit(EXIT_FAILURE);
    }
    grAct->grnds = tmp;
    grAct->grnds[grAct->numOfGrnds - 1] = grounder_copyGrounding(partialG,
                                                          action->numOfParams);
    return;
  }

  // Precondition current positive literal
  struct atom *pcpl = &precond->posLiterals[idxPrecond];

  // A working copy of the partial grounding. This is actually only needed
  // to allocate the memory. The copying must take place later in each iteration
  // of the loop anyway.
  struct grounding *partialGTmp = grounder_copyGrounding(partialG,
                                                          action->numOfParams);
  for (size_t idxFlnts = 0; idxFlnts < state->numOfFluents; ++idxFlnts) {
    struct atom *crrFlnt = &state->fluents[idxFlnts];

    if (crrFlnt->pred != pcpl->pred) {
      // preconditions' current positive literals' predicate does not match
      // current fluents' predicate. Nothing to do here.
      continue;
    }

    // Write working copy of partial grounding.
    memcpy( partialGTmp->terms,
            partialG->terms,
            sizeof(partialGTmp->terms[0]) * action->numOfParams); // TODO: sane?

    bool continueOuter = false;
    for (size_t idxParam = 0; idxParam < pcpl->pred->numOfParams; ++idxParam) {
      struct term *pcplTerm = pcpl->terms[idxParam];
      struct term *flntTerm = crrFlnt->terms[idxParam];

      if ( ! pcplTerm->isVariable) {
        // This precondition contains constants. Check if the constant matches
        // the state. If no, there is no matching.
        if (term_equal(pcplTerm, flntTerm)) {
          // Two constants match.
          continue; // Continue with next parameter.
        } else {
          // TODO: sane?
          continueOuter = true;
          break;
        }
      }

      assert(pcplTerm->isVariable);

      // TODO: Compare to type of predicate definition!!!
      // Check type.
      if ( ! typeSystem_isa(flntTerm->type, pcplTerm->type)) {
        // TODO: sane ?
        continueOuter = true;
        break;
      }
      // Types match

      // libpddl31_term_print(pcplTerm); // DEBUG
      // printf("\n"); // DEBUG

      // Find position of parameter in action.
      size_t idxActParam = 0;
      for (idxActParam = 0; idxActParam < action->numOfParams; ++idxActParam) {
        struct term *actParam = &action->params[idxActParam];

        //printf("idxActParam:%d ", idxActParam); // DEBUG
        //libpddl31_term_print(actParam); // DEBUG
        //printf("\n"); // DEBUG

        if (term_equal(actParam, pcplTerm)) {
          // Position found
          break;
        }
      }
      if (idxActParam >= action->numOfParams) {
        fprintf(stderr,
                "Error in action '%s'. Variable '%s' in precondition does"
                " not match any parameter.",
                action->name,
                pcplTerm->name);
        exit(EXIT_FAILURE);
      }
      // idxActParam is set now.

      // Check if there exists a mapping for this parameter already.
      if (partialGTmp->terms[idxActParam] != NULL) {
        // Grounding already exists.
        // Check if it does not conflict
        if (term_equal(partialGTmp->terms[idxActParam], flntTerm)) {
          // Grounding already set. Nothing more to be done.
          break; // TODO: Check this line for correctness.
        } else {
          // Conflicting grounding.
          // TODO: sane?
          continueOuter = true;
          break;
        }
      }

      // Create mapping from variable to constant in grounding.
      assert(partialGTmp->terms[idxActParam] == NULL);
      partialGTmp->terms[idxActParam] = flntTerm;

      /* // Maybe outer loop needs to be factorized to a recursive function. */
      /* // Or we need to make a backup of the partial grounding before we start */
      /* // the outer loop? */

    } // End of loop over parameters of term in precondition and state
    if (continueOuter) {
      continue;
    }

    // Precondition and state matched in some way.
    // Recurse
    grounder_groundAction(state, idxPrecond + 1, partialGTmp, grAct);

  } // End of loop over fluents in state

  grounder_freeGrounding(partialGTmp);

}

void
grounder_print_grounding(struct grounding *gr, int32_t numOfTerms)
{
  printf("Grounding:[");
  for (size_t i = 0; i < numOfTerms; ++i) {
    printf("%s", gr->terms[i]->name);
    if (i < numOfTerms - 1) {
      printf(",");
    }
  }
  printf("]");
}

void
grounder_print_groundAction(struct groundAction *grA)
{
  printf("GroundAction:[action-name:%s,numOfGrnds:%d," /* TODO */,
         grA->action->name,
         grA->numOfGrnds);

  for (size_t i = 0; i < grA->numOfGrnds; ++i) {
    grounder_print_grounding(grA->grnds[i], grA->action->numOfParams);
    if (i < grA->numOfGrnds - 1) {
      printf(",");
    }
  }
  printf("]");
}
