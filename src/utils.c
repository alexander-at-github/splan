#include <stdlib.h>

#include "utils.h"

void utils_free_literal(struct literal *literal)
{
  free(literal);
}

struct state *utils_copyState(struct state *state)
{
  struct state *result = malloc(sizeof(*result));
  result->numOfFluents = state->numOfFluents;
  size_t size = sizeof(*result->fluents) * result->numOfFluents;

  result->fluents = malloc(size);
  memcpy(result->fluents, state->fluents, size);

  return result;
}

void utils_freeStateShallow(struct state *state)
{
  if (state->fluents != NULL) {
    free(state->fluents);
  }
  free(state);
}


// Returns NULL if this action can not fix the gap. Otherwise it returns a list
// of paritally ground actions, which will fix the gap.
struct actionList *
utils_actionFixesGap(struct action *action, struct gap *gap)
{
  struct actionList *result = NULL;

  struct effect *effect = action->effect;

  struct literal *gapLiteral = gap->literal;

  if (gapLiteral->isPos) { // Gap is true, should be false.
    for (int32_t idxEff = 0; idxEff < effect->numOfElems; ++idxEff) {
      struct effectElem *effE = &effect->elems[idxEff];

      switch (effE->type) {
      case POS_LITERAL: {
        // Nothing to do.
        break;
      }
      case NEG_LITERAL: {
        if (gapLiteral->atom->pred == effE.it->literal-pred) {
          // Chance to fix the gap.
          // TODO
        }
        break;
      }
      case FORALL: {
        // If the effect-part of a forall-effect will fix the gap, then all
        // possible groundings of this action will fix the gap. Thus the
        // function call to utils_actionFixesGap() should just return a single
        // (partially) grounded action with actually no variable set.

        bool canFix = utils_forallFixesGap(effE.it->forall, gap);
        if (canFix) {
          if (result != NULL) {
            // Remove previous partial groundings. See the comment above for
            // the reason.
          }
        }
        break;
      }
      case WHEN: {
        // We can not check the precondition of the when-effect. We just know
        // that the precondition could be met. So just treat the when-effecti
        // as any other effect.
        assert(false);
        // TODO
        break;
      }
      default: {
        assert(false);
        break;
      }
      }
    }
  } else {
    assert( ! gapLiteral->isPos);
  }

  /* while (effect != NULL) { */
  /*   for (int32_t idxEff = 0; idxEff < effect->numOfElems; ++idxEff) { */
  /*     struct effectElem *effE = &effect->elems[idxEff]; */

  /*     switch (effE->type) { */
  /*     case POS_LITERAL: { */
  /*       if ( ! gapLiteral->isPos && // gapLiteral should be fullfilled */
  /*           gapLiteral->atom->pred == effE.it->literal->pred) { */
  /*         // TODO */
  /*       } */
  /*       break; */
  /*     } */
  /*     case NEG_LITERAL: { */
  /*       if (gapLiteral->isPos && // gapLiteral should not be false */
  /*           gapLiteral->atom->pred == effE.it->literal-pred) { */
  /*         // TODO */
  /*       } */
  /*       break; */
  /*     } */
  /*     case FORALL: { */
  /*       // If the effect-part of a forall-effect will fix the gap, then all */
  /*       // possible groundings of this action will fix the gap. Thus the */
  /*       // function call to utils_actionFixesGap() should just return a single */
  /*       // (partially) grounded action with actually no variable set. */
  /*       assert(false); */
  /*       // TODO */
  /*       break; */
  /*     } */
  /*     case WHEN: { */
  /*       // We can not check the precondition of the when-effect. We just know */
  /*       // that the precondition could be met. So just treat the when-effect as */
  /*       // any other effect. */
  /*       assert(false); */
  /*       // TODO */
  /*       break; */
  /*     } */
  /*     default: { */
  /*       assert(false); */
  /*       break; */
  /*     } */
  /*     } */
  /*   } */
  /* } */
}


/*** NEW ***/

struct actionList *
utils_concatActionLists(struct actionList *l1, struct actionList *l2)
{
  if (l2 == NULL) {
    return l1;
  }
  if (l1 == NULL) {
    return l2;
  }
  // TODO
  assert(false);
}

// We can not check the precondition of the when-effect. We just know
// that the precondition could be met. So just treat the when-effect
// as any other effect.
struct actionList *
utils_whenFixesGap( struct action *action,
                    struct when *when,
                    struct gap *gap)
{
  struct actionList *result = NULL;

  struct literal *gapLiteral = gap->literal;
  struct atom *gapAtom = gapLiteral->atom;

  for (int32_t idxWhen = 0; idxWhen < when->numOfPos; ++idxWhen) {
    struct atom *posAtom = &when->posLiterals[idxWhen];
    if ( ! gapLiteral->isPos &&
        posAtom->pred == gapAtom->pred) {

      // Chance to fix gap.
      struct groundAction *newGrAct = utils_tryToFixGap(action,
                                                        posAtom,
                                                        gapAtom);
      result = utils_addActionToList(result, newGrACt);
    }
  }
  for (int32_t idxWhen = 0; idxWhen < when->numOfNeg; ++idxWhen) {
    struct atom *negAtom = &when->negLiterals[idxWhen];
    if (gapLiteral->isPos &&
        negAtom->pred == gapAtom->pred) {

      // Chance to fix gap.
      struct groundAction *newGrAct = utils_tryToFixGap(action,
                                                        negAtom,
                                                        gapAtom);
      result = utils_addActionToList(result, newGrACt);
    }
  }

  return result;
}

// Use, when there is chance to fix the gap.
struct groundAction *
utils_tryToFixGap(struct action *action,
                  struct atom *atomToFix, // Atom which might fix the gap
                  struct atom *gapAtom)
{
  struct groundAction *newGrAct = utils_create_groundAction(action);

  for ( int32_t idxArgs = 0;
        idxArgs < atomToFix->pred->numOfParams;
        ++idxArgs) {

    struct term *effTerm = atomToFix->terms[idxArgs];
    struct term *gapTerm = gapAtom->terms[idxArgs];

    if (effTerm->isVariable) {
      // Find position of parameter in action. Pointer arithmetic
      int32_t idxActParam = effTerm - action->params;
      if (0 <= idxActParam && idxActParam < action->numOfParams) {
        // Variable points to parameter in action.
        if (newGrAct->terms[idxActParam] != NULL &&
            newGrAct->terms[idxActParam] != gapTerm) {
          // Value was already been set to another constant. That is a
          // problem. The predicate does not fix the gap.
          utils_free_groundAction(newGrAct);
          newGrAct = NULL;
          break;
        }
        // Do mapping
        newGrAct->terms[idxActParam] = gapTerm; // Actually set mapping.
      } else {
        // Variable refers to a forall-variable. Do not map. Just continue.
        // TODO: assert (effect != action->effect);
      }
    } else {
      // A constant in the actions effect. Deal with it.
      if (term_equal(effTerm, gapTerm)) {
        // Okay. Continue with next argument.
        continue;
      } else {
        // The whole predicate does not fix the gap.
        utils_free_groundAction(newGrAct);
        newGrAct = NULL;
        break;
      }
    }
  }

  retrun newGrAct;
}

struct actionList *
utils_forallFixesGap( struct action *action,
                      struct forall *forall,
                      struct gap *gap)
{
  struct actionList *result = NULL;

  struct literal *gapLiteral = gap->literal;
  struct atom *gapAtom = gapLiteral->atom;
  struct effect *faEff = forall->effect; // forall-effect

  for (int32_t idxEff = 0; idxEff < faEff->numOfElems; ++idxEff) {
    struct effectElem *effE = &faEff->elems[idxEff];

    switch (effE->type) {
    case POS_LITERAL: {

      struct atom *posAtom = effE->it.literal;

      if ( ! gapLiteral->isPos &&
          posAtom->pred == gapAtom->pred) {

        // Chance to fix the gap.
        struct groundAction *newGrAct = utils_tryToFixGap(action,
                                                          posAtom,
                                                          gapAtom);
        result = utils_addActionToList(result, newGrAct);
      }
      break;
    }
    case NEG_LITERAL: {

      struct atom *negAtom = effE->it.literal;

      if (gapLiteral->isPos &&
          negAtom->pred == gapAtpm->pred) {

        // Chance to fix the gap.
        struct groundAction *newGrAct = utils_tryToFixGap(action,
                                                          negAtom,
                                                          gapAtom);
        result = utils_addActionToList(result, newGrAct);
      }
      break;
    }
    case FORALL: {

      struct forall *forall = effE.it->forall;

      struct actionList *forallList = utils_forallFixesGap(action, forall, gap);

      result = utils_concatActionLists(result, forallList);
      break;
    }
    case WHEN: {

      struct when *when = effE.it->when;

      struct actionList *whenList = utils_whenFixesGap(action, when, gap);

      result = utils_concatActionLists(result, whenList);
      break;
    }
    default: {
      assert(false);
      break;
    }
    } // End of switch statement.

  }

  return result;
}

struct groundAction *
utils_create_groundAction(struct action *action)
{
  struct groundAction *grAct = malloc(sizeof(*grAct));
  grAct->action = action;
  grAct->terms = calloc(sizeof(*grAct->terms) * action->numOfParams);
  return grAct;
}

void
utils_free_groundAction(struct groundAction *grAct)
{
  if (grAct->terms != NULL) {
    free(grAct->terms);
  }
  free(grAct);
}

struct actionList *
utils_addActionToList(struct actionList *head, struct groundAction *grAct)
{
  if (grAct == NULL) {
    // Do not do anything.
    return result;
  }
  struct actionList *newHead = malloc(sizeof(*newHead));
  newHead->act =grAct;
  newHead->next = head;
  return newHead;
}

struct actionList *
utils_actionFixesGap_aux( struct action *action,
                          struct effect *effect,
                          struct gap *gap)
{
  struct actionList *result = NULL;
  struct literal *gapLiteral = gap->literal;

  for (int32_t idxEff = 0; idxEff < effect->numOfElems; ++idxEff) {
    struct effectElem *effE = &effect->elems[idxEff];

    switch (effE->type) {
    case POS_LITERAL: {

      struct atom *posAtom = effE->it.literal;

      if ( ! gap->isPos &&
          posAtom->pred == gap->literal->pred) {

        // Chance to fix the gap.
        struct groundAction *newGrAct = utils_create_groundAction(action);

        for ( int32_t idxArgs = 0;
              idxArgs < posAtom->pred->numOfParams;
              ++idxArgs) {

          struct term *effTerm = posAtom->terms[idxArgs];
          struct term *gapTerm = gapLiteral->terms[idxArgs];

          if (effTerm->isVariable) {
            // Do mapping
            // Find position of parameter in action. Pointer arithmetic.
            int32_t idxActParam = effTerm - action->params;
            assert (0 <= idxActParam && idxActParam < action->numOfParams);
            // Set the mapping from variable to constant. Aka grounding.
            if (newGrAct->terms[idxActParam] != NULL) {
              // Value was already set. That is a problem. The predicate does
              // not fix the gap.
              utils_free_groundAction(newGrAct);
              break;
            }
            newGrAct->terms[idxActParam] = gapTerm; // Actually set the mapping.
          } else {
            // A constant in the actions effect. Deal with it.
            if (term_equal(effTerm, gapTerm)) {
              // Okay. Continue with next argument.
              continue;
            } else {
              // The whole predicate does not fix the gap.
              utils_free_groundAction(newGrAct);
              break;
            }
          }

        }

        // Add the new partially ground action to the list.
        result = utils_addActionToList(result, newGrAct);
      }
      break;
    }
    case NEG_LITERAL: {
      break;
      }
    }
    case FORALL: {
      // If the effect-part of a forall-effect will fix the gap, then all
      // possible groundings of this action will fix the gap. Thus the
      // function call to utils_actionFixesGap() should just return a single
      // (partially) grounded action with actually no variable set.

      bool canFix = utils_forallFixesGap(effE.it->forall, gap);
      if (canFix) {
        if (result != NULL) {
          // Remove previous partial groundings. See the comment above for
          // the reason.
        }
      }
      break;
    }
    case WHEN: {
      // We can not check the precondition of the when-effect. We just know
      // that the precondition could be met. So just treat the when-effecti
      // as any other effect.
      assert(false);
      // TODO
      break;
    }
    default: {
      assert(false);
      break;
    }
    }
  }
}

struct actionList *
utils_actionFixesGap(struct action *action, struct gap *gap)
{
  return utils_actionFixesGap_aux(action, action->effect, gap);
}
