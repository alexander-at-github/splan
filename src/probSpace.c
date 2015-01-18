#include <assert.h>
#include <stdio.h>

#include "probSpace.h"

#include "planner.h"
#include "trie.h"
#include "utils.h"

// Returns true if it did add an atom to the trie, false otherwise.
static
bool
ps_applyEffElem(trie_t trie,
                struct groundAction *grAct,
                struct effectElem *effElem)
{
  assert (effElem != NULL);

  // Return value.
  bool val = false;

  switch(effElem->type) {
  case POS_LITERAL : {
    bool contains =  trie_containsGr(trie, effElem->it.literal, grAct);
    if (!contains) {
      trie_addGr(trie, effElem->it.literal, grAct);
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
// trie and never removes atoms. That is because we want to know all the
// possible atoms in a problems' state.
// This function returns false, if it did not change the trie, and true
// otherwise.
static
bool
ps_apply(trie_t trie, struct groundAction *grAct)
{
  assert (trie != NULL);
  assert (grAct != NULL);

  // Return value.
  bool val = false;

  struct effect *effCurr = grAct->action->effect;

  for (int32_t idx = 0; idx < effCurr->numOfElems; ++idx) {
    struct effectElem *effElem = &effCurr->elems[idx];
    bool locVal = ps_applyEffElem(trie, grAct, effElem);
    if (locVal) {
      val = true;
    }
  }

  return val;
}

void
ps_createIndex(struct probSpace *probSpace)
{
  trie_t trie = probSpace->setFluents;
  for (struct actionList *alE = probSpace->allGrActs;
       alE != NULL;
       alE = alE->next) {

    struct groundAction *grAct = alE->act;
    struct action *action = grAct->action;
    struct effect *effect = action->effect;
    for (int32_t idxE = 0; idxE < effect->numOfElems; ++idxE) {
      struct effectElem *effE = &effect->elems[idxE];
      struct atom *atom = effE->it.literal;
      // TODO: Is that correct?
      if (effE->type == POS_LITERAL) {
        trie_addIndexPos(trie, atom, grAct);
      } else if (effE->type == NEG_LITERAL) {
        trie_addIndexNeg(trie, atom, grAct);
      } else {
        // This code does not support conditional effects.
        assert (false);
      }
    }
  }
}

struct probSpace *
ps_init(struct problem *problem)
{
  struct probSpace *probSpace = malloc(sizeof(*probSpace));
  probSpace->problem = problem;
  // Initialize problem space with initial state of problem.
  probSpace->setFluents = trie_clone(problem->init);
  probSpace->allGrActs = NULL; // Will be set later

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

  // Apply all the action effects to the problem space(which still equals
  // the initial state), until it reaches a fix point, that is, until it
  // does not change anymore.
  bool done = false;
  while (!done) {
    done = true;
    for (struct actionList *actL = allGroundActions;
         actL != NULL;
         actL = actL->next) {

      struct groundAction *grAct = actL->act;

      struct literal *isSatisfied =
                        planner_satisfiesPrecond(probSpace->setFluents, grAct);
      if (isSatisfied == NULL) {
        bool change = ps_apply(probSpace->setFluents, grAct);
        if (change && done) {
          done = false;
        }
      } else {
        utils_free_literal(isSatisfied);
      }
    }
  }

  probSpace->allGrActs = ps_filter(probSpace, allGroundActions);
  // Don't free allGroundActions, because it just has been reduced to
  // probSpace->allGrActs by the previous call. All the unneccessary elements
  // have been freed by that call, too.



  /* Extend the probSpace->setFluents to an index into probSpace->allGrActs. */
  ps_createIndex(probSpace);

  return probSpace;
}

// This function calculates the maximum number of occurrences of a variable as
// described in the paper 'Parameterized Complexity of Optimal Planning: A
// Detailed Map':
// "We say that a variable v ∈ V occurs in a conjunction of literals φ if v
//  or ¬v occurs in φ. Let occ v (φ) be 1 if v occurs in φ and 0 otherwise.
//  The number of occurrences of a variable v ∈ V is defined as
//  Sum_a∈A (occ v (pre(a)) + occ v (eff(a))).i"
int32_t
ps_calcMaxVarOcc(struct probSpace *probSpace)
{
  // The whole function will only work on a copy of the problem space.
  // Later this copy might be used for more things, like an index onto the
  // actions.
  trie_t psClone = trie_clone(probSpace->setFluents);

  // Set occurrance count to zero.
  trie_setCount(psClone, 0);

  for (struct actionList *currLE = probSpace->allGrActs;
       currLE != NULL;
       currLE = currLE->next) {

    struct groundAction *grAct = currLE->act;
    //struct term **gr =grAct->terms;
    struct action *act = grAct->action;
    struct goal *precond = act->precond;
    struct effect *effect = act->effect;

    // Since by the definition of varibale occurance, a variable must
    // be only counted once per precodition. In order to achieve that
    // we will use another set of fluents to accumulate the ground atoms,
    // which we already considered.
    trie_t precondSet = trie_createEmptyFrom(probSpace->setFluents);
    // Add and count all the positive precondition fluents.
    for (int32_t idxPP = 0; idxPP < precond->numOfPos; ++idxPP) {
      struct atom *atom = &precond->posLiterals[idxPP];
      if (trie_containsGr(precondSet, atom, grAct)) {
        // This atom was already considered. Do not count it again.
        continue;
      }
      trie_addGr(precondSet, atom, grAct);
      trie_addGr(psClone, atom, grAct);
      trie_incCountGr(psClone, atom, grAct);
    }

    // Add and count all the negative precondition fluents.
    for (int32_t idxNP = 0; idxNP < precond->numOfNeg; ++idxNP) {
      struct atom *atom = &precond->negLiterals[idxNP];
      if (trie_containsGr(precondSet, atom, grAct)) {
        // This atom was already considered in the precondition. Do not count
        // it again.
        continue;
      }
      trie_addGr(precondSet, atom, grAct);
      trie_addGr(psClone, atom, grAct);
      trie_incCountGr(psClone, atom, grAct);
    }

    // Add and count all the effect fluents.
    //
    // Since by the definition of variable occurrance, a variable must be only
    // counted once per effect. In order to achieve that we will use another
    // set of fluents to accumulate the ground atoms, which we already
    // considered.
    trie_t effectSet = trie_createEmptyFrom(probSpace->setFluents);
    for (int32_t idxE = 0; idxE < effect->numOfElems; ++idxE) {
      struct effectElem *effElem = &effect->elems[idxE];
      if (effElem->type == POS_LITERAL || effElem->type == NEG_LITERAL) {
        struct atom *atom = effElem->it.literal;
        if (trie_containsGr(effectSet, atom, grAct)) {
          // This atomw as already considered.
          continue;
        }
        trie_addGr(effectSet, atom, grAct);
        trie_addGr(psClone, atom, grAct);
        trie_incCountGr(psClone, atom, grAct);
      } else {
        // We do not support conditional effects. However, the the data
        // structures from pddl31structs.h do support conditional effects.
        assert (false);
      }
    }

    trie_free(precondSet);
    trie_free(effectSet);
  }
  // Now all the variables should be counted. We just have to retrieve tha
  // maximum count from the psClone.

  int32_t result = trie_getMaxCount(psClone);

  trie_free(psClone);

  return result;
}

void
ps_free(struct probSpace *probSpace)
{
  trie_free(probSpace->setFluents);
  utils_free_actionList(probSpace->allGrActs);
  free(probSpace);
}

// Checks the ground actions precondition. It only considers positive
// precondtions. Returns true if they are all satisfied by the state, and false
// otherwise.
static
bool
ps_satisfiesPosPrecondAtoms(trie_t state, struct groundAction *grAct)
{
  assert(grAct != NULL);

  struct goal *precond = grAct->action->precond;

  for (int32_t i = 0; i < precond->numOfPos; ++i) {
    struct atom *atom = &precond->posLiterals[i];
    if ( ! trie_containsGr(state, atom, grAct)) {
      return false;
    }
  }

  // Ignoring negative precondtions here on psrpose.

  return true;
}

struct actionList *
ps_filter(struct probSpace *probSpace, struct actionList *actL)
{
  //printf("ps_filter()\n"); // DEBUG
  //utils_print_actionListCompact(actL); // DEBUG
  //printf("\n"); // DEBUG
 
  struct actionList *result = NULL;
  struct actionList *toBeFreed = NULL;

  struct actionList *actLIter = actL;
  struct actionList *actLIterNext = NULL;
  while (actLIter != NULL) {
    actLIterNext = actLIter->next;

    struct groundAction *grAct = actLIter->act;

    bool isSatisfied = ps_satisfiesPosPrecondAtoms(probSpace->setFluents,
                                                   grAct);
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

struct actionList *
ps_getActsToFixGap(struct probSpace *probSpace, struct literal *literal)
{
  if (literal->isPos) {
    return trie_getActsPos(probSpace->setFluents, literal->atom);
  }
  // Else
  return trie_getActsNeg(probSpace->setFluents, literal->atom);
}
