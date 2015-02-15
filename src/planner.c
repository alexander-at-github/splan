#include <assert.h>
#include <time.h>

#include "planner.h"

#include "libpddl31.h"
#include "probSpace.h"
#include "trie.h"
#include "utils.h"

clock_t startTime = 0;
int32_t timeout = -1;
bool timedout = false;

struct actionList *
planner_getActsToFixGap(struct problem *problem,
                        struct probSpace *probSpace,
                        struct gap *gap)
{
  struct actionManag *actManag = problem->domain->actionManag;

  // We will use the struct actionList again. The ordering does not matter
  // here though. We just want to collect a set here.
  struct actionList *result = NULL;


  for (int32_t idxActManag = 0;
       idxActManag < actManag->numOfActions;
       ++idxActManag) {

    struct action *action = &actManag->actions[idxActManag];

    struct actionList *fix = utils_actionFixesGap(action, gap);
    if (fix == NULL) {
      // This action can not fix the gap.
      continue;
    }

    // Complete grounding of that action and add all of them to the
    // result.
    struct actionList *newActs = utils_groundActions(problem, fix);

    // Clean up.
    utils_free_actionList(fix);

    // Filter actions according to problem space.
    newActs = ps_filter(probSpace, newActs);

    result = utils_concatActionLists(newActs, result);
  }
  return result;
}

// Returns NULL iff the state satisfies the goal.
// Returns the first literal which is not satisfied otherwise.
struct literal *
planner_satisfiesPrecond(trie_t state, struct groundAction *grAct)
{
  if (grAct == NULL) {
    // The precondition of an empty ground action is considered to be
    // satisfied.
    return NULL;
  }
  struct goal *precond = grAct->action->precond;
  //libpddl31_goal_print(precond); // DEBUG
  //printf("\n\n"); //DEBUG

  for (int32_t i = 0; i < precond->numOfPos; ++i) {
    struct atom *atom = &precond->posLiterals[i];
    if ( ! trie_containsGr(state, atom, grAct)) {
      struct literal *returnVal = malloc(sizeof(*returnVal));
      returnVal->atom = utils_atom_cloneWithGrounding(atom, grAct);
      returnVal->isPos = true;
      return returnVal;
    }
  }
  for (int32_t i = 0; i < precond->numOfNeg; ++i) {
    struct atom *atom = &precond->negLiterals[i];
    if (trie_containsGr(state, atom, grAct)) {
      struct literal *returnVal = malloc(sizeof(*returnVal));
      returnVal->atom = utils_atom_cloneWithGrounding(atom, grAct);
      returnVal->isPos = false;
      return returnVal;
    }
  }
  // State satisfies precondition.
  return NULL;
}

// Returns NULL iff the state satisfies the goal.
// Returns the first literal which is not satisfied otherwise.
struct literal *
planner_satisfies(trie_t state, struct goal *goal)
{
  if (goal == NULL) {
    // An empty goal is always satisfied.
    return NULL;
  }
  //printf("planner_satisfies():\n");
  //libpddl31_goal_print(goal); // DEBUG
  //printf("\n\n"); //DEBUG

  for (int32_t i = 0; i < goal->numOfPos; ++i) {
    struct atom *atom = &goal->posLiterals[i];
    if ( ! trie_contains(state, atom)) {
      struct literal *returnVal = malloc(sizeof(*returnVal));
      returnVal->atom = utils_atom_clone(atom);
      returnVal->isPos = true;
      return returnVal;
    }
  }
  for (int32_t i = 0; i < goal->numOfNeg; ++i) {
    struct atom *atom = &goal->negLiterals[i];
    if (trie_contains(state, atom)) {
      struct literal *returnVal = malloc(sizeof(*returnVal));
      returnVal->atom = utils_atom_clone(atom);
      returnVal->isPos = false;
      return returnVal;
    }
  }
  // State satisfies goal.
  return NULL;
}

void
planner_applyEffElem( trie_t state,
                      struct groundAction *grAct,
                      struct effectElem *effElem)
{
  if (effElem == NULL)  {
    return;
  }
  switch(effElem->type) {
  case POS_LITERAL : {
    trie_addGr(state, effElem->it.literal, grAct);
    break;
  }
  case NEG_LITERAL: {
    trie_removeGr(state, effElem->it.literal, grAct);
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
}

// Applies action to state in place.
// Precondition: The actions precondition must be satisfied by the state.
//               This function does not check that.
//
void
planner_apply(trie_t state, struct groundAction *grAct)
{
  if (grAct == NULL) {
    return;
  }
  if (state == NULL) {
    // Parser creates an empty trie_t in case of an empty state.
    assert (false);
  }

  struct effect *effCurr = grAct->action->effect;
  for ( int32_t idxEff = 0;
        idxEff < effCurr->numOfElems;
        ++idxEff) {

    struct effectElem *effElem = &effCurr->elems[idxEff];
    planner_applyEffElem(state, grAct, effElem);
  }
}

// Returns NULL iff there is no gap. Returns the gap otherwise.
struct gap *
planner_hasGap( trie_t initState,
                struct goal *goal,
                struct actionList *actions)
{
  //printf("\n\nplanner_hasGap():\n"); // DEBUG
  //libpddl31_state_print(initState); // DEBUG
  //printf("\n"); // DEBUG
  //libpddl31_goal_print(goal); // DEBUG
  //printf("\n\t"); // DEBUG
  //utils_print_actionList(actions); // DEBUG
  //printf("\n\n"); // DEBUG

  struct gap *result = NULL;
  // Copy the state, so we can modify it freely.
  trie_t lState = trie_clone(initState);
  struct literal *gapLiteral = NULL;

  /* Check actions' precondition and apply the action. */

  // Index for position of gap. Starts with one, because the gap is sayed
  // to be before the index.
  int32_t idxAct = 1;

  // Iterate over list of actions.
  for (struct actionList *pAct = actions;
       pAct != NULL;
       pAct = pAct->next, ++idxAct) { // Also increase index for gap

    struct groundAction *grAct = pAct->act;
    // Check if precondition is met.
    gapLiteral = planner_satisfiesPrecond(lState, grAct);
    if (gapLiteral != NULL) {
      // Precondition is not met.
      result = malloc(sizeof(*result));
      result->literal = gapLiteral;
      result->position = idxAct;
      // Clean up.
      trie_free(lState);
      return result;
    }
    // Precondition is met. Apply the action.
    planner_apply(lState, grAct);
    //printf("action applied:   "); // DEBUG
    //utils_print_groundAction(grAct); // DEBUG
    //printf("\n"); // DEBUG
    //libpddl31_state_print(lState); // DEBUG
    //printf("\n"); // DEBUG
    //libpddl31_state_print(lState); // DEBUG
    //printf("\n"); // DEBUG
  }

  /* Check if the goal is met after applying all the acitons. */

  gapLiteral = planner_satisfies(lState, goal);
  if (gapLiteral != NULL) {
    result = malloc(sizeof(*result));
    result->literal = gapLiteral;
    result->position = idxAct;
  }

  /* Clean up */
  trie_free(lState);
  return result; // Returns NULL when neccessary.
}

// Returns a solution to the planning instance or NULL, if no solution was
// found.
static struct actionList *
planner_solveProblem_aux( struct problem *problem,
                          struct probSpace *probSpace,
                          // an Accumulator for actions
                          struct actionList *actAcc,
                          int32_t depthLimit,
                          int32_t depth)
{
  //printf("\n>>>new iteration\n");
  //utils_print_actionList(actAcc); // DEBUG
  //printf("\n");

  //utils_print_actionListCompact(actAcc); // DEBUG
  //printf("\n"); // DEBUG

  struct actionList *actAccBack = actAcc;

  trie_t initState = problem->init;
  struct goal *goal = problem->goal;
  struct gap *gap = planner_hasGap(initState, goal, actAcc);
  if (gap == NULL) {
    // Solution found.
    //printf("\n\nSOLUTION FOUND\n\n\n"); // DEBUG
    // The parameter 'actAcc' points to elements of the stack. These addresses
    // will be invalid when problem is solved. Thus, we will clone it.
    struct actionList *solution = utils_cloneActionList(actAcc);
    return solution;
  }

  if (depth >= depthLimit) {
    // Depth limit.
    //printf("dl*");
    utils_free_gap(gap);
    return NULL;
  }

  //printf("gap:   "); // DEBUG
  //utils_print_gap(gap); // DEBUG
  //printf("\n"); // DEBUG

  struct actionList *actsToFixGap = planner_getActsToFixGap(problem,
                                                            probSpace,
                                                            gap);
  //printf("actsToFixGap.length: %d\n",
  //       utils_actionList_length(actsToFixGap)); //DEBUG
  //utils_print_actionList(actsToFixGap); // DEBUG
  //printf("\n"); // DEBUG

  // The order of the following loops can be changed freely. I just choose
  // a abritrarily for now.

  // Iterate over positions.
  // Note: gap->pos >= 1.
  int32_t idxPos = 0;
  struct actionList *curr = NULL;
  struct actionList *afterCurr = actAcc;
  while (idxPos < gap->position) {

    // Action list element.
    // Note: The action itself (tmpActL->act) will be set later (in the inner
    // loop).
    struct actionList tmpActL;

    // Add action to action accumlator.
    // I will incorporate the mechanics to add and remove grounded actions
    // to the potential solution here, cause it can be more efficient than
    // using an external function (which would have to iterate over the
    // list of ground actions every time we want to add or remove elements.
    tmpActL.next = afterCurr;
    if (curr == NULL) {
      actAcc = &tmpActL;
    } else {
      curr->next = &tmpActL;
    }

    // Iterate over actions
    for (struct actionList *pActL = actsToFixGap;
         pActL != NULL;
         pActL = pActL->next) {

      tmpActL.act = pActL->act;

      // Recurse with new parital solution.
      struct actionList *result;
      result = planner_solveProblem_aux(problem,
                                        probSpace,
                                        actAcc,
                                        depthLimit,
                                        depth+1);
      // Return the first result
      if (result != NULL) {
        //printf("RETURN RESULT"); // DEBUG

        // Clean up before return.
        utils_free_gap(gap);
        utils_free_actionList(actsToFixGap);
        return result;
      }

    }

    // Remove action from action accumulator for net iteration of loop.
    if (curr == NULL) {
      actAcc = actAccBack;
    } else {
      curr->next = afterCurr;
    }


    // Increment loop variables.
    idxPos++;
    if (afterCurr != NULL) {
      curr = afterCurr;
      afterCurr = afterCurr->next;
    } else {
      // This case could only happen in the last iteration.
      assert (idxPos == gap->position);
    }

  }
  // No solution within this depth-limit found.
  // Clean up
  //printf("Cleanup in planner_solveProblem_aux()\n\n");
  utils_free_gap(gap);
  utils_free_actionList(actsToFixGap);
  return NULL;
}

struct actionList *
planner_solveProblem(struct problem *problem,
                     struct probSpace *probSpace,
                     int32_t depthLimit)
{
  return planner_solveProblem_aux(problem,
                                  probSpace,
                                  NULL,
                                  depthLimit,
                                  0);
}

struct actionList *
planner_iterativeDeepeningSearch(struct problem *problem)
{
  struct probSpace *probSpace = ps_init(problem);

  printf("Problem Space: "); // DEBUG
  trie_print(probSpace->setFluents); // DEBUG
  printf("\n"); // DEBUG

  for (int32_t depth = 1; depth < INT32_MAX; ++depth) {
    printf("\n### depth search with depth %d\n\n", depth); // DEBUG
    struct actionList *solution = planner_solveProblem(problem,
                                                       probSpace,
                                                       depth);
    if (solution != NULL) {

      ps_free(probSpace);
      trie_cleanupSNBuffer();
      return solution;
    }
  }

  ps_free(probSpace);
  trie_cleanupSNBuffer();
  return NULL;
}

uint32_t
planner_actionList_calculateWeights_aux_v3(trie_t state,
                                           struct groundAction *grAct,
                                           struct goal *precond)
{
  uint32_t weight = 0;
  // Iterate over precondition atoms.
  for (int32_t idxPrecond = 0; idxPrecond < precond->numOfPos; ++idxPrecond) {
    struct atom *atom = &precond->posLiterals[idxPrecond];
    if ( ! trie_containsGr(state, atom, grAct)) {
      // Actually increasing the weight, when a precondition is not met.
      weight++;
    }
  }
  for (int32_t idxPrecond = 0; idxPrecond < precond->numOfNeg; ++idxPrecond) {
    struct atom *atom = &precond->negLiterals[idxPrecond];
    if (trie_containsGr(state, atom, grAct)) {
      // Actually increasing the weight, when a precondition is not met.
      weight++;
    }
  }
  return weight;
}

// Sets the weight on all actions in actL.
// @nextGrAct is the action in the solution which will be after the ones
//   considered in actL.
void
planner_actionList_calculateWeights_v3(struct actionList *actL,
                                       trie_t state,
                                       struct groundAction *nextGrAct)
{
  //if (nextGrAct == NULL) {
  //  return planner_actionList_calculateWeights(actL, state);
  //}

  for (/* empty */; actL != NULL; actL = actL->next) {

    struct groundAction *grAct = actL->act;
    struct action *action = grAct->action;
    struct goal *precond = action->precond;

    // Consider weight before action in actL.
    uint32_t weight =
                    planner_actionList_calculateWeights_aux_v3(state,
                                                               grAct,
                                                               precond);
    if (nextGrAct != NULL) {
      trie_t stateCopy = trie_clone(state);
      planner_apply(stateCopy, actL->act);
      // Consider weight after action in actL.
      weight += planner_actionList_calculateWeights_aux_v3(stateCopy,
                                                    nextGrAct,
                                                    nextGrAct->action->precond);
      trie_free(stateCopy);
    }

    // Write weight to actual action-list-element.
    actL->weight = weight > INT32_MAX ? INT32_MAX : (int32_t) weight;
  }
}

// This function will edit the weights of the list in place.
// It produces weights between 0 and INT32_MAX.
// If an actions' precondition is fulfilled by the state, then its weight will
// be zero. For every atom of the precondition, which is not fulfilled, its
// weight will be incremented by one.
void
planner_actionList_calculateWeights(struct actionList *actL, trie_t state)
{
  //printf("CALL: planner_actionList_calculateWeights()\n"); // DEBUG

  // Iterate over actions.
  for (/* empty */; actL != NULL; actL = actL->next) {
    uint32_t weight = 0;

    struct groundAction *grAct = actL->act;
    struct action *action = grAct->action;
    struct goal *precond = action->precond;

    // Iterate over precondition atoms.
    for (int32_t idxPrecond = 0; idxPrecond < precond->numOfPos; ++idxPrecond) {
      struct atom *atom = &precond->posLiterals[idxPrecond];
      if ( ! trie_containsGr(state, atom, grAct)) {
        // Actually increasing the weight, when a precondition is not met.
        weight++;
      }
    }
    for (int32_t idxPrecond = 0; idxPrecond < precond->numOfNeg; ++idxPrecond) {
      struct atom *atom = &precond->negLiterals[idxPrecond];
      if (trie_containsGr(state, atom, grAct)) {
        // Actually increasing the weight, when a precondition is not met.
        weight++;
      }
    }

    // Write weight to actual action-list-element.
    actL->weight = weight > INT32_MAX ? INT32_MAX : (int32_t) weight;
    //printf("set action weight to %d\n", weight); // DEBUG
  }
}

// This quicksort should be stable.
struct actionList *
planner_quicksort(struct actionList *actL)
{
  //printf("CALL planner_quicksort()\n");
  //printf("actL length: %d\n", utils_actionList_length(actL));

  /* Base case */
  if (actL == NULL || actL->next == NULL) { // That is, length <= 1
    return actL;
  }
  // List is at least two elements long.

  /* Recursive case */

  // Just take the first element as pivot element.
  struct actionList *pivot = actL;
  // Recal: List is at least two elements long.
  // Remove pivot element from the input list.
  actL = actL->next;
  pivot->next = NULL;
  //printf("pivot->weight: %d\n", pivot->weight);
  //printf("remaining actL length: %d\n", utils_actionList_length(actL));

  // List for elements before/less than pivot. New elements will go to the
  // front.
  struct actionList *before = NULL;
  // List for elements equal to the pivot element. New elements will go to the
  // back in order to make the sort stable. (We choose the first element as
  // pivot element, and add equal elements to the back of the equals list).
  struct actionList *equalFirst = pivot;
  struct actionList *equalLast = equalFirst;
  // List for elements after/greater than pivot. New elements will go to the
  // front.
  struct actionList *after = NULL; // List for elements after the pivot.

  // Note: We are doing fat partitioning. I.e., There are three lists for
  // elements less than, equal to and greater than the pivot element. The lists
  // names in this implementation are 'before', 'equal', 'after'.
  // At all lists we add to the front.

  // Iterator over the input list.
  while(actL != NULL) {
    // Note: Including the current element in the result and iterating to the
    // next element of the input list needs careful processing.
    struct actionList *actLNext = actL->next;
    if (actL->weight < pivot->weight) {
      // New elements will be added to the front of the before-list
      struct actionList *beforeOld = before;
      before = actL;
      before->next = beforeOld;
    } else if (actL->weight > pivot->weight) {
      // New elements will be added to the front of the after-list
      struct actionList *afterOld = after;
      after = actL;
      after->next = afterOld;
    } else {
      assert (actL->weight == pivot->weight);
      // New elements will be added to the end of the equal-list
      equalLast->next = actL;
      equalLast = equalLast->next;
    }
    actL = actLNext;
  }

  //printf("before length: %d\n", utils_actionList_length(before));
  //printf("after length: %d\n", utils_actionList_length(after));
  // Recursion.
  before = planner_quicksort(before);
  after = planner_quicksort(after);

  // Put lists together.
  struct actionList *result = NULL;
  // Connecting the before-list with the equal-list, if necessary.
  if (before != NULL) {
    result = before;
    struct actionList *beforeLast = before;
    while (beforeLast->next != NULL) {
      beforeLast = beforeLast->next;
    }
    beforeLast->next = equalFirst;
  } else {
    assert (equalFirst != NULL && equalFirst == pivot); // Just clarification
    result = equalFirst;
  }
  // Connecting the equal-list with the after-list.
  equalLast->next = after;

  return result;
}

struct actionList *
planner_sortActsAccToState(struct actionList *actL, trie_t state)
{
  // Calculate weights for sorting.
  planner_actionList_calculateWeights(actL, state);
  actL = planner_quicksort(actL);
  // utils_print_actionListCompact(actL); // DEBUG
  // printf("\n"); // DEBUG
  return actL;
}


// Returns a solution to the planning instance or NULL, if no solution was
// found.
static struct actionList *
planner_solveProblem_aux_v2(struct problem *problem,
                            struct probSpace *probSpace,
                            // an Accumulator for actions
                            struct actionList *actAcc,
                            int32_t depthLimit,
                            int32_t depth)
{
  //printf("*");
  //printf("\n>>>planner_solveProblem_aux_v2()   ");
  //utils_print_actionList(actAcc); // DEBUG
  //printf("\n");

  //utils_print_actionListCompact(actAcc); // DEBUG
  //printf("\n"); // DEBUG

  struct actionList *actAccBack = actAcc;

  trie_t initState = problem->init;
  struct goal *goal = problem->goal;
  struct gap *gap = planner_hasGap(initState, goal, actAcc);
  if (gap == NULL) {
    // Solution found.
    //printf("\n\nSOLUTION FOUND\n\n\n"); // DEBUG
    // The parameter 'actAcc' points to elements of the stack. These addresses
    // will be invalid when problem is solved. Thus, we will clone it.
    struct actionList *solution = utils_cloneActionList(actAcc);
    return solution;
  }

  if (depth >= depthLimit) {
    // Depth limit.
    //printf("dl*"); // DEBUG
    utils_free_gap(gap);
    return NULL;
  }

  //printf("gap:   "); // DEBUG
  //utils_print_gap(gap); // DEBUG
  //printf("\n"); // DEBUG

  struct actionList *actsToFixGap = planner_getActsToFixGap(problem,
                                                            probSpace,
                                                            gap);
  //if ( depth == 4) {
  //  printf("actsToFixGap.length: %d\n",
  //         utils_actionList_length(actsToFixGap)); //DEBUG
  //}
  //utils_print_actionList(actsToFixGap); // DEBUG
  //printf("\n"); // DEBUG
  if (actsToFixGap == NULL) {
    // There are no actions to fix the gap.
    utils_free_gap(gap);
    return NULL;
  }

  // Copy the state, so we can modify it freely.
  trie_t lState = trie_clone(initState);

  // The order of the following loops can be changed freely. I just choose
  // a abritrarily for now.

  // Iterate over positions.
  // Note: gap->pos >= 1.
  int32_t idxPos = 0;
  struct actionList *curr = NULL;
  struct actionList *afterCurr = actAcc;
  // TODO: I think it will be better to iterate from back to front, i.e., from
  // a high to low index. CAUTION: That does not work with updating the state.
  while (idxPos < gap->position) {

    // Action list element.
    // Note: The action itself (tmpActL->act) will be set later (in the inner
    // loop).
    struct actionList tmpActL;

    // Add action to action accumlator.
    // I will incorporate the mechanics to add and remove grounded actions
    // to the potential solution here, cause it can be more efficient than
    // using an external function (which would have to iterate over the
    // list of ground actions every time we want to add or remove elements.
    tmpActL.next = afterCurr;
    if (curr == NULL) {
      actAcc = &tmpActL;
    } else {
      curr->next = &tmpActL;
    }

    // Sort actions according to local state.
    //printf("SORTING actsToFixGap\n"); // DEBUG
    actsToFixGap = planner_sortActsAccToState(actsToFixGap, lState);
    //printf("actsToFixGap.length: %d\n",
    //       utils_actionList_length(actsToFixGap)); //DEBUG
    //utils_print_actionListCompact(actsToFixGap);
    //printf("\n"); // DEBUG
    //exit(EXIT_FAILURE);

    // Iterate over actions
    for (struct actionList *pActL = actsToFixGap;
         pActL != NULL;
         pActL = pActL->next) {

      tmpActL.act = pActL->act;

      // Recurse with new parital solution.
      struct actionList *result;
      result = planner_solveProblem_aux_v2(problem,
                                           probSpace,
                                           actAcc,
                                           depthLimit,
                                           depth+1);
      // Return the first solution.
      if (result != NULL) {
        //printf("RETURN RESULT"); // DEBUG

        // Clean up before return.
        trie_free(lState);
        utils_free_gap(gap);
        utils_free_actionList(actsToFixGap);
        return result;
      }

    }

    // Remove action from action accumulator for next iteration of loop.
    if (curr == NULL) {
      actAcc = actAccBack;
    } else {
      curr->next = afterCurr;
    }


    // Increment loop variables.
    idxPos++;
    if (afterCurr != NULL) {
      curr = afterCurr;
      afterCurr = afterCurr->next;

      // Apply action to local state.
      // TODO: Is the location of this call right?
      planner_apply(lState, curr->act); // TODO: Is that (curr->act) right?

    } else {
      // This case could only happen in the last iteration.
      assert (idxPos == gap->position);
    }
  }
  // No solution within this depth-limit found.
  // Clean up
  //printf("Cleanup in planner_solveProblem_aux()\n\n");
  trie_free(lState);
  utils_free_gap(gap);
  utils_free_actionList(actsToFixGap);
  return NULL;
}

struct actionList *
planner_solveProblem_v2(struct problem *problem,
                        struct probSpace *probSpace,
                        int32_t depthLimit)
{
  return planner_solveProblem_aux_v2(problem,
                                     probSpace,
                                     NULL,
                                     depthLimit,
                                     0);
}

struct actionList *
planner_iterativeDeepeningSearch_v2(struct problem *problem)
{
  struct probSpace *probSpace = ps_init(problem);

  printf("Problem Space: "); // DEBUG
  trie_print(probSpace->setFluents); // DEBUG
  printf("\n"); // DEBUG
  printf("variable occurrence: %d\n", ps_calcMaxVarOcc(probSpace));

  for (int32_t depth = 1; depth < INT32_MAX; ++depth) {
    printf("\n### depth search with depth %d\n\n", depth); // DEBUG
    struct actionList *solution = planner_solveProblem_v2(problem,
                                                          probSpace,
                                                          depth);
    if (solution != NULL) {

      ps_free(probSpace);
      trie_cleanupSNBuffer();

      return solution;
    }
  }

  ps_free(probSpace);
  trie_cleanupSNBuffer();
  return NULL;
}

// Count the number of calls to this function
static int64_t planner_solveProblem_aux_v3_callCount = 0;

struct actionList *
planner_solveProblem_aux_v3(struct probSpace *probSpace,
                            struct actionList *actAcc,
                            int32_t depthLimit,
                            int32_t depth)
{
  if (timeout >= 0) {
    clock_t endTime = clock();
    if ((endTime - startTime) > timeout) {
      timedout = true;
      printf("TIMEOUT.\n");
      return NULL;
    }
  }

  planner_solveProblem_aux_v3_callCount ++;

  //if (depth == 17) {
  //  printf("\nplanner_solveProblem_aux_v3(): actAcc length: %d\n",
  //         utils_actionList_length(actAcc));
  //  utils_print_actionListCompact(actAcc);
  //  printf("\n");
  //}

  struct problem *problem = probSpace->problem;
  trie_t initState = problem->init;
  struct goal *goal = problem->goal;

  struct gap *gap = planner_hasGap(initState, goal, actAcc);
  if (gap == NULL) {
    // Solution found.
    printf("SOLUTION FOUND\n"); // DEBUG
    //utils_print_actionListCompact(actAcc); // DEBUG
    //printf("\n"); // DEBUG
    struct actionList *solution = utils_cloneActionList(actAcc);
    return solution;
  }

  if (depth >= depthLimit) {
    utils_free_gap(gap);
    return NULL;
  }
  //utils_print_gap(gap); // DEBUG
  //printf("gap->position: %d\n", gap->position); // DEBUG

  struct actionList *actsToFixGap = ps_getActsToFixGap(probSpace, gap->literal);
  //printf("actstofixgap length: %d\n", utils_actionList_length(actsToFixGap));

  if (actsToFixGap == NULL) {
    // There is no way (action) to fix the gap.
    utils_free_gap(gap);
    return NULL;
  }

  /*** First: Prepare a list of actions (with coresponding positions) ***/

  trie_t lState = trie_clone(initState);

  // Accumulate actions with their coresponding position.
  struct actionList *actsToFixGapWithPos = NULL;

  // Note:
  assert (gap->position >= 1);
  int32_t idxPos = 0;
  struct actionList *curr = NULL;
  struct actionList *afterCurr = actAcc;
  while (idxPos < gap->position) {
    struct actionList *actsToFixGapCopy =
                                    utils_cloneActionListShallow(actsToFixGap);
    // Calculate weight at this position.
    //planner_actionList_calculateWeights(actsToFixGapCopy, lState);
    planner_actionList_calculateWeights_v3(actsToFixGapCopy,
                                           lState,
                                           afterCurr == NULL ? NULL : afterCurr->act);

    // Set their position.
    utils_actionList_setPosition(actsToFixGapCopy, idxPos);
    // Add them to the set of actions with theid positions.
    actsToFixGapWithPos = utils_concatActionLists(actsToFixGapCopy,
                                                  actsToFixGapWithPos);
    // Do not free actsToFixGapCopy. All the actions are accumulated in
    // actsToFixGapWithPos.

    // Advance loop variables.
    idxPos++;
    if (afterCurr != NULL) {
      curr = afterCurr;
      afterCurr = afterCurr->next;

      // Apply action to local state.
      planner_apply(lState, curr->act);
    } else {
      // This case should only happen in the last iteration.
      assert (idxPos == gap->position);
    }
  }
  utils_free_gap(gap);
  trie_free(lState);
  // ATTENTION: Do not free actsToFixGap. This list is part of the trie/index
  // and will be needed again. It will be freed witht he trie.

  // Note: weights in actsToFixGapWithPos are already set.

  //printf("actsToFixGapWithPos length: %d\n",
  //       utils_actionList_length(actsToFixGapWithPos));

  /*** Second: Sort that list according to their weights. ***/

  // Note: Do not reverse the order here. It might be better to have the actions
  // with high positions first.
  //utils_print_actionListCompact(actsToFixGapWithPos); // DEBUG
  //printf("\n"); // DEBUG

  // Sort the list of actions (with their positions) now.
  actsToFixGapWithPos = planner_quicksort(actsToFixGapWithPos);

  //utils_print_actionListCompact(actsToFixGapWithPos); // DEBUG
  //printf("\n"); // DEBUG

  /*** Third: Apply that actions ***/

  struct actionList *solution = NULL;
  for (struct actionList *currAle = actsToFixGapWithPos;
       currAle != NULL;
       currAle = currAle->next) {

    actAcc = utils_addActionToListAtPosition( actAcc,
                                              currAle->act,
                                              currAle->pos);

    solution = planner_solveProblem_aux_v3(probSpace,
                                           actAcc,
                                           depthLimit,
                                           depth + 1);

    actAcc = utils_removeActionFromListAtPosition(actAcc, currAle->pos);

    // Return the first solution.
    if (solution != NULL) {
      break;
    }

    // Not strictly neccessary.
    if (timedout) {
      break;
    }
  }

  utils_free_actionListShallow(actsToFixGapWithPos);
  // May return NULL, if there was no solution found within this depth-limit.
  return solution;
}

struct actionList *
planner_solveProblem_v3(struct probSpace *probSpace, int32_t depthLimit)
{
  return planner_solveProblem_aux_v3(probSpace, NULL, depthLimit, 0);
}

struct actionList *
planner_iterativeDeepeningSearch_v3(struct problem *problem,
                                    int32_t planLengthGuess,
                                    int32_t timeout_) // in seconds
{
  if (timeout_ >= 0) {
    startTime = clock();
    timeout = timeout_ * CLOCKS_PER_SEC; // in clocks
    //printf("timeout set to %d circles\n", timeout);
  }

  struct probSpace *probSpace = ps_init(problem);

  printf("all ground actions in problem space:\n"); // DEBUG
  utils_print_actionListCompact(probSpace->allGrActs); // DEBUG
  printf("\n"); // DEBUG
  printf("number of ground actions in problem space: %d",
         utils_actionList_length(probSpace->allGrActs));
  printf("\n");

  printf("Problem Space: "); // DEBUG
  trie_print(probSpace->setFluents); // DEBUG
  printf("\n"); // DEBUG
  printf("variable occurrence: %d\n", ps_calcMaxVarOcc(probSpace));

  struct actionList *solution = NULL;
  for (int32_t depth = planLengthGuess; depth < INT32_MAX; ++depth) {
    printf("\n### depth search with depth %d\n", depth); // DEBUG
    solution = planner_solveProblem_v3(probSpace, depth);
    printf("planner_solveProblem_aux_v3_callCount, i.e., nodes expanded: %d\n",
            planner_solveProblem_aux_v3_callCount);
    printf("\n");

    if (solution != NULL || timedout) {
      break;
    }
  }

  ps_free(probSpace);
  trie_cleanupSNBuffer();

  return solution;
}







/*** Under Construction. ***/

/* static */
/* void */
/* planner_applyEffElemPos(trie_t trie, */
/*                 struct groundAction *grAct, */
/*                 struct effectElem *effElem) */
/* { */
/*   assert (effElem != NULL); */
/*   if (effElem->type != POS_LITERAL) { */
/*     return; */
/*   } */
/*   trie_addGr(trie, effElem->it.literal, grAct); */
/* } */

/* static */
/* void */
/* planner_applyEffElemNeg(trie_t trie, */
/*                 struct groundAction *grAct, */
/*                 struct effectElem *effElem) */
/* { */
/*   assert (effElem != NULL); */
/*   if (effElem->type != NEG_LITERAL) { */
/*     return; */
/*   } */
/*   trie_addGr(trie, effElem->it.literal, grAct); */
/* } */

/* typedef void (*applyEffElemFun)(trie_t, */
/*                                 struct groundAction *, */
/*                                 struct effectElem) ; */

/* static */
/* void */
/* planner_applyF(trie_t trie, struct groundAction *grAct, applyEffElemFun) */
/* { */
/*   assert (trie != NULL); */
/*   assert (grAct != NULL); */

/*   struct effect *effCurr = grAct->action->effect; */

/*   for (int32_t idx = 0; idx < effCurr->numOfElems; ++idx) { */
/*     struct effectElem *effElem = &effCurr->elems[idx]; */
/*     (*applyEffElemFun)(trie, grAct, effElem); */
/*   } */
/* } */

/* static */
/* void */
/* planner_applyPos(trie_t trie, struct groundAction *grAct) */
/* { */
/*   plannerApplyF(trie, grAct, &planner_applyEffElemPos); */
/* } */

/* static */
/* void */
/* planner_applyNeg(trie_t trie, struct groundAction *grAct) */
/* { */
/*   plannerApplyF(trie, grAct, &planner_applyEffElemNeg); */
/* } */

/* typedef struct actionList * aStarNode_t; */

/* typedef struct aStarNodeList { */
/*   aStarNode_t node; */
/*   struct aStarNodeList *next; */
/*   // prev? struct aStarNodeList *prev; */
/* } * aStarNodeList_t; */

/* static */
/* aStarNodeList_t */
/* asnl_createElem(aStarNode_t aStarNode) */
/* { */
/*   aStarNodeList_t asnle = malloc(sizeof(*asnle)); */
/*   asnle->node = aStarNode; */
/*   return asnle; */
/* } */

/* struct actionList * */
/* planner_aStar(struct probSpace *probSpace) */
/* { */
/*   aStarNode_t aStarNode = NULL;    // The empty list of actions. */
/*   aStarNodeList_t frontier = asnl_createElem(aStarNode); */
/*   aStarNodeList_t explored = NULL;    // An empty set. */

/*   while(frontier != NULL) { */
/*     aStarNode_t currN; */
/*     /1* currN = pop(frontier) *1/ */
/*     currN = frontier->node; */
/*     frontier = asnl_removeFirst(frontier); */

/*   } */

/*   // No solution. */
/*   return NULL; */
/* } */
