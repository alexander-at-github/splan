#include <assert.h>
#include <stdio.h>

#include "probUniv.h"

#include "planner.h"
#include "state.h"
#include "utils.h"

// Problem Universe. That is a state, which is an union of all possible
// states in the problem instance.
static state_t probUniv = NULL;



// Returns true if it did add an atom to the state, false otherwise.
bool
pu_applyEffElem(state_t state,
                struct groundAction *grAct,
                struct effectElem *effElem)
{
  assert (effElem != NULL);

  // Return value.
  bool val = false;

  switch(effElem->type) {
  case POS_LITERAL : {
    bool contains =  state_containsGr(state, effElem->it.literal, grAct);
    if (!contains) {
      state_addGr(state, effElem->it.literal, grAct);
      val = true;
    }
    break;
  }
  case NEG_LITERAL : {
    // Empty on purpose.
    break;
  }
  case FORALL: {
    // TODO
    assert (false);
    break;
  }
  case WHEN: {
    // TODO
    assert (false);
    break;
  }
  default: {
    assert(false);
    break;
  }
  }

  return val;
}

// This function is different from planner_apply(). It only adds atoms to the
// state and never removes atoms. That is because we want to know all the
// possible atoms in a problems' state.
// This function returns false, if it did not change the state, and true
// otherwise.
bool
pu_apply(state_t state, struct groundAction *grAct)
{
  assert (state != NULL);
  assert (grAct != NULL);

  // Return value.
  bool val = false;

  struct effect *effCurr = grAct->action->effect;

  for (int32_t idx = 0; idx < effCurr->numOfElems; ++idx) {
    struct effectElem *effElem = &effCurr->elems[idx];
    bool locVal = pu_applyEffElem(state, grAct, effElem);
    if (locVal) {
      val = true;
    }
  }

  return val;
}

void
pu_init(struct problem *problem)
{
  // Initialize problem universe with initial state of problem.
  probUniv = state_clone(problem->init);

  struct actionManag *actionManag = problem->domain->actionManag;

  struct actionList *allActions = NULL;

  // Add all the ungrounded actions to the allActions list. The actions
  // will be grounded later.
  for (int32_t idx = 0; idx < actionManag->numOfActions; ++idx) {
    struct action *act = &actionManag->actions[idx];
    struct groundAction *grAct = libpddl31_create_groundAction(act);
    struct actionList *grActLE = malloc(sizeof(*grActLE));
    grActLE->act = grAct;
    grActLE->next = allActions;
    allActions = grActLE;
  }

  // Ground the actions.
  struct actionList *allGroundActions = utils_groundActions(problem,
                                                            allActions);
  utils_free_actionList(allActions);

  // Apply all the action effects to the problem universe (which still equals
  // the initial state), until it reaches a fix point, that is, until it
  // does not change anymore.
  bool done = false;
  while (!done) {
    done = true;
    for (struct actionList *actL = allGroundActions;
         actL != NULL;
         actL = actL->next) {

      struct groundAction *grAct = actL->act;

      struct literal *isSatisfied = planner_satisfiesPrecond(probUniv, grAct);
      if (isSatisfied == NULL) {
        bool change = pu_apply(probUniv, grAct);
        if (change && done) {
          done = false;
        }
      } else {
        utils_free_literal(isSatisfied);
      }
    }
  }

  utils_free_actionList(allGroundActions);
}

state_t
pu_getSingleton()
{
  if (probUniv == NULL) {
    assert(false);
    fprintf(stderr, "\nERROR: pu_getSingleton(): not initialized. Please call "
                    "pu_init() first");
  }
  return probUniv;
}

void
pu_cleanup()
{
  state_free(probUniv);
  probUniv = NULL;
}

// Checks the ground actions precondition. It only considers positive
// precondtions. Returns true if they are all satisfied by the state, and false
// otherwise.
bool
pu_satisfiesPosPrecondAtoms(state_t state, struct groundAction *grAct)
{
  assert(grAct != NULL);

  struct goal *precond = grAct->action->precond;

  for (int32_t i = 0; i < precond->numOfPos; ++i) {
    struct atom *atom = &precond->posLiterals[i];
    if ( ! state_containsGr(state, atom, grAct)) {
      return false;
    }
  }

  // Ignoring negative precondtions here on purpose.

  return true;
}

struct actionList *
pu_filter(struct actionList *actL)
{
  //printf("pu_filter()\n"); // DEBUG
  //utils_print_actionListCompact(actL); // DEBUG
  //printf("\n"); // DEBUG
 
  struct actionList *result = NULL;
  struct actionList *toBeFreed = NULL;

  struct actionList *actLIter = actL;
  struct actionList *actLIterNext = NULL;
  while (actLIter != NULL) {
    actLIterNext = actLIter->next;

    struct groundAction *grAct = actLIter->act;

    bool isSatisfied = pu_satisfiesPosPrecondAtoms(probUniv, grAct);
    if (isSatisfied) {
      // Add ground action to result.
      actLIter->next = result;
      result = actLIter;
    } else { // Precondition is not satisfied.
      // Add ground action to to-be-freed-list.
      actLIter->next = toBeFreed;
      toBeFreed = actLIter;
    }

    actLIter = actLIterNext;
  }

  // Free toBeFreed
  utils_free_actionList(toBeFreed);

  //printf("result: ");
  //utils_print_actionListCompact(result); // DEBUG
  //printf("\n"); // DEBUG

  return result;

}
