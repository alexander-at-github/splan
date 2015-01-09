
/***  TODO: FIX new state ***/

#include <assert.h>

#include "planner.h"

#include "libpddl31.h"
#include "probUniv.h"
#include "state.h"
#include "utils.h"

struct actionList *
planner_getActsToFixGap(struct problem *problem, struct gap *gap)
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

    // Filter actions according to problem universe.
    newActs = pu_filter(newActs);

    result = utils_concatActionLists(newActs, result);
  }
  return result;
}

/* bool */
/* planner_contains(struct state *state, struct atom *atom) */
/* { */
/*   if (atom == NULL) { */
/*     // That should not happen. */
/*     assert (false); */
/*     // Any state fullfills the nonexisting atom. */
/*     return true; */
/*   } */
/*   if (state == NULL) { */
/*     // An empty state does not contain any fluents anyway. */
/*     return false; */
/*   } */
/*   for ( int32_t idxFluents = 0; */
/*         idxFluents < state->numOfFluents; */
/*         ++idxFluents) { */

/*     struct atom *fluent = &state->fluents[idxFluents]; */

/*     if (utils_atom_equal(fluent, atom)) { */
/*       return true; */
/*     } */

/*   } */
/*   return false; */
/* } */

/* // The 'atom' must be from the action affiliated with 'grAct'. */
/* bool */
/* planner_containsPrecondAtom(struct state *state, */
/*                             struct groundAction *grAct, */
/*                             struct atom *atom) */
/* { */
/*   if (atom == NULL) { */
/*     // That should not happen. */
/*     assert (false); */
/*     // Any state fullfills the nonexisting atom. */
/*     return true; */
/*   } */
/*   if (state == NULL) { */
/*     // An empty state does not contain any fluents anyway. */
/*     return false; */
/*   } */
/*   if (grAct == NULL) { */
/*     // That should not happen */
/*     assert (false); */
/*     // If there is no grounding, then the state does not fullfill the atom. */
/*     return false; */
/*   } */

/*   for ( int32_t idxFluents = 0; */
/*         idxFluents < state->numOfFluents; */
/*         ++idxFluents) { */

/*     struct atom *fluent = &state->fluents[idxFluents]; */
/*     if (utils_atom_equalWithGrounding(fluent, atom, grAct)) { */
/*       return true; */
/*     } */

/*   } */
/*   return false; */
/* } */

// Returns NULL iff the state satisfies the goal.
// Returns the first literal which is not satisfied otherwise.
struct literal *
planner_satisfiesPrecond(state_t state, struct groundAction *grAct)
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
    if ( ! state_containsGr(state, atom, grAct)) {
      struct literal *returnVal = malloc(sizeof(*returnVal));
      returnVal->atom = utils_atom_cloneWithGrounding(atom, grAct);
      returnVal->isPos = true;
      return returnVal;
    }
  }
  for (int32_t i = 0; i < precond->numOfNeg; ++i) {
    struct atom *atom = &precond->negLiterals[i];
    if (state_containsGr(state, atom, grAct)) {
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
planner_satisfies(state_t state, struct goal *goal)
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
    if ( ! state_contains(state, atom)) {
      struct literal *returnVal = malloc(sizeof(*returnVal));
      returnVal->atom = utils_atom_clone(atom);
      returnVal->isPos = true;
      return returnVal;
    }
  }
  for (int32_t i = 0; i < goal->numOfNeg; ++i) {
    struct atom *atom = &goal->negLiterals[i];
    if (state_contains(state, atom)) {
      struct literal *returnVal = malloc(sizeof(*returnVal));
      returnVal->atom = utils_atom_clone(atom);
      returnVal->isPos = false;
      return returnVal;
    }
  }
  // State satisfies goal.
  return NULL;
}

/* void */
/* planner_stateAddAtom( struct state *state, */
/*                       struct groundAction *grAct, */
/*                       struct atom *atom) */
/* { */
/*   if (atom == NULL) { */
/*     return; */
/*   } */

/*   for (int32_t idx = 0; idx < state->numOfFluents; ++idx) { */
/*     struct atom *fluent = &state->fluents[idx]; */
/*     if (utils_atom_equalWithGrounding(fluent, atom, grAct)) { */
/*       return; */
/*     } */
/*   } */
/*   // Check if the types od the predicates parameter allow the atom to be */
/*   // added. */
/*   // TODO: If we would know that all types (action parameter types and */ 
/*   // predicate parameter types) match after grounding, we would no need that. */
/*   for (int32_t idxArgs = 0; idxArgs < atom->pred->numOfParams; ++idxArgs) { */
/*     struct type *predParamType = atom->pred->params[idxArgs].type; */
/*     // Pointer arithmetic. */
/*     int32_t idxGrounding = atom->terms[idxArgs] - grAct->action->params; */
/*     if (0 <= idxGrounding && idxGrounding < grAct->action->numOfParams) { */
/*       // We are dealing with a grounding. Check type. */
/*       if ( ! typeSystem_isa(grAct->terms[idxGrounding]->type, predParamType)) { */
/*         // Can not apply atom. Type missmatch. */
/*         return; */
/*       } */
/*     } else { */
/*       // We are dealing with a constant. */
/*       if ( ! typeSystem_isa(atom->terms[idxArgs]->type, predParamType)) { */
/*         // Can not apply atom. */
/*         // That should never happen, since it's a constant. */
/*         assert (false); */
/*         return; */
/*       } */
/*     } */
/*   } */

/*   // Add the atom. */
/*   ++state->numOfFluents; */
/*   state->fluents = realloc(state->fluents, */
/*                            sizeof(*state->fluents) * state->numOfFluents); */

/*   //memcpy(&state->fluents[state->numOfFluents - 1], atom, sizeof(*atom)); */
/*   state->fluents[state->numOfFluents - 1].pred = atom->pred; */
/*   int32_t size = sizeof(*state->fluents[state->numOfFluents - 1].terms) * */
/*                  atom->pred->numOfParams; */
/*   state->fluents[state->numOfFluents - 1].terms = malloc(size); */

/*   // Set terms to objects according to mapping. */
/*   for (int32_t idxArgs = 0; idxArgs < atom->pred->numOfParams; ++idxArgs) { */
/*     // Pointer arithmetic. */
/*     int32_t idxGrounding = atom->terms[idxArgs] - grAct->action->params; */
/*     if (0 <= idxGrounding && idxGrounding < grAct->action->numOfParams) { */
/*       // We are dealing with a grounding. */
/*       state->fluents[state->numOfFluents - 1].terms[idxArgs] = */
/*                                                     grAct->terms[idxGrounding]; */
/*     } else { */
/*       // We are dealing with a constant. */
/*       state->fluents[state->numOfFluents - 1].terms[idxArgs] = */
/*                                                           atom->terms[idxArgs]; */
/*     } */
/*   } */
/* } */

/* void */
/* planner_stateRemoveAtom(struct state *state, */
/*                         struct groundAction *grAct, */
/*                         struct atom *atom) */
/* { */
/*   if (atom == NULL) { */
/*     return; */
/*   } */

/*   for (int32_t idx = 0; idx < state->numOfFluents; ++idx) { */
/*     struct atom *fluent = &state->fluents[idx]; */
/*     if (utils_atom_equalWithGrounding(fluent, atom, grAct)) { */
/*       //printf("REMOVING ATOM FROM STATE\n"); // DEBUG */
/*       // Remove atom. */
/*       --state->numOfFluents; */
/*       struct atom *tmp = malloc(sizeof(*tmp) * (state->numOfFluents)); */
/*       // Copy memory before the element to remove. */
/*       memcpy(tmp, state->fluents, sizeof(*tmp) * idx); */
/*       // Copy memory after the element to remove. */
/*       memcpy( &tmp[idx], */
/*               &state->fluents[idx + 1], */
/*               sizeof(*tmp) * (state->numOfFluents - idx)); */

/*       // Clean up removed fluent */
/*       libpddl31_atom_free(&state->fluents[idx]); */
/*       // And old array */
/*       free(state->fluents); */

/*       state->fluents = tmp; */
/*       // Do not return here. I want to remove all of them. */
/*     } */
/*   } */
/* } */

void
planner_applyEffElem( state_t state,
                      struct groundAction *grAct,
                      struct effectElem *effElem)
{
  if (effElem == NULL)  {
    return;
  }
  switch(effElem->type) {
  case POS_LITERAL : {
    state_addGr(state, effElem->it.literal, grAct);
    break;
  }
  case NEG_LITERAL: {
    state_removeGr(state, effElem->it.literal, grAct);
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
planner_apply(state_t state, struct groundAction *grAct)
{
  if (grAct == NULL) {
    return;
  }
  if (state == NULL) {
    // Parser creates an empty state_t in case of an empty state.
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
planner_hasGap( state_t initState,
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
  state_t lState = state_clone(initState);
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
      state_free(lState);
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
  state_free(lState);
  return result; // Returns NULL when neccessary.
}

// Returns a solution to the planning instance or NULL, if no solution was
// found.
static struct actionList *
planner_solveProblem_aux( struct problem *problem,
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

  state_t initState = problem->init;
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

  struct actionList *actsToFixGap = planner_getActsToFixGap(problem, gap);
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
      result = planner_solveProblem_aux(problem, actAcc, depthLimit, depth+1);
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
planner_solveProblem(struct problem *problem, int32_t depthLimit)
{
  return planner_solveProblem_aux(problem, NULL, depthLimit, 0);
}

struct actionList *
planner_iterativeDeepeningSearch(struct problem *problem)
{
  pu_init(problem);
  printf("Problem Universe: "); // DEBUG
  state_print(pu_getSingleton()); // DEBUG
  printf("\n"); // DEBUG

  for (int32_t depth = 1; depth < INT32_MAX; ++depth) {
    printf("\n### depth search with depth %d\n\n", depth); // DEBUG
    struct actionList *solution = planner_solveProblem(problem, depth);
    if (solution != NULL) {

      pu_cleanup();
      state_cleanupSNBuffer();
      return solution;
    }
  }

  pu_cleanup();
  state_cleanupSNBuffer();
  return NULL;
}

// This function will edit the weights of the list in place.
// It produces weights between 0 and INT32_MAX.
// If an actions' precondition is fulfilled by the state, then its weight will
// be zero. For every atom of the precondition, which is not fulfilled, its
// weight will be incremented by one.
void planner_actionList_calculateWeights(struct actionList *actL,
                                         state_t state)
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
      if ( ! state_containsGr(state, atom, grAct)) {
        // Actually increasing the weight, when a precondition is not met.
        weight++;
      }
    }
    for (int32_t idxPrecond = 0; idxPrecond < precond->numOfNeg; ++idxPrecond) {
      struct atom *atom = &precond->negLiterals[idxPrecond];
      if (state_containsGr(state, atom, grAct)) {
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
planner_sortActsAccToState(struct actionList *actL, state_t state)
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

  state_t initState = problem->init;
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

  struct actionList *actsToFixGap = planner_getActsToFixGap(problem, gap);
  //printf("actsToFixGap.length: %d\n",
  //       utils_actionList_length(actsToFixGap)); //DEBUG
  //utils_print_actionList(actsToFixGap); // DEBUG
  //printf("\n"); // DEBUG
  if (actsToFixGap == NULL) {
    // There are no actions to fix the gap.
    utils_free_gap(gap);
    return NULL;
  }

  // Copy the state, so we can modify it freely.
  state_t lState = state_clone(initState);

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
                                           actAcc,
                                           depthLimit,
                                           depth+1);
      // Return the first solution.
      if (result != NULL) {
        //printf("RETURN RESULT"); // DEBUG

        // Clean up before return.
        state_free(lState);
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
  state_free(lState);
  utils_free_gap(gap);
  utils_free_actionList(actsToFixGap);
  return NULL;
}

struct actionList *
planner_solveProblem_v2(struct problem *problem, int32_t depthLimit)
{
  return planner_solveProblem_aux_v2(problem, NULL, depthLimit, 0);
}

struct actionList *
planner_iterativeDeepeningSearch_v2(struct problem *problem)
{
  pu_init(problem);
  printf("Problem Universe: "); // DEBUG
  state_print(pu_getSingleton()); // DEBUG
  printf("\n"); // DEBUG

  for (int32_t depth = 1; depth < INT32_MAX; ++depth) {
    printf("\n### depth search with depth %d\n\n", depth); // DEBUG
    struct actionList *solution = planner_solveProblem_v2(problem, depth);
    if (solution != NULL) {

      pu_cleanup();
      state_cleanupSNBuffer();

      return solution;
    }
  }

  pu_cleanup();
  state_cleanupSNBuffer();
  return NULL;
}
