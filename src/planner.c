#include <assert.h>

#include "planner.h"

#include "libpddl31.h"
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

    result = utils_concatActionLists(newActs, result);
  }
  return result;
}

bool
planner_contains(struct state *state, struct atom *atom)
{
  if (atom == NULL) {
    // That should not happen.
    assert (false);
    // Any state fullfills the nonexisting atom.
    return true;
  }
  if (state == NULL) {
    // An empty state does not contain any fluents anyway.
    return false;
  }
  for ( int32_t idxFluents = 0;
        idxFluents < state->numOfFluents;
        ++idxFluents) {

    struct atom *fluent = &state->fluents[idxFluents];

    if (utils_atom_equal(fluent, atom)) {
      return true;
    }

  }
  return false;
}

// The 'atom' must be from the action affiliated with 'grAct'.
bool
planner_containsPrecondAtom(struct state *state,
                            struct groundAction *grAct,
                            struct atom *atom)
{
  if (atom == NULL) {
    // That should not happen.
    assert (false);
    // Any state fullfills the nonexisting atom.
    return true;
  }
  if (state == NULL) {
    // An empty state does not contain any fluents anyway.
    return false;
  }
  if (grAct == NULL) {
    // That should not happen
    assert (false);
    // If there is no grounding, then the state does not fullfill the atom.
    return false;
  }

  for ( int32_t idxFluents = 0;
        idxFluents < state->numOfFluents;
        ++idxFluents) {

    struct atom *fluent = &state->fluents[idxFluents];
    if (utils_atom_equalWithGrounding(fluent, atom, grAct)) {
      return true;
    }

  }
  return false;
}

// Returns NULL iff the state satisfies the goal.
// Returns the first literal which is not satisfied otherwise.
struct literal *
planner_satisfiesPrecond(struct state *state, struct groundAction *grAct)
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
    if ( ! planner_containsPrecondAtom(state, grAct, atom)) {
      struct literal *returnVal = malloc(sizeof(*returnVal));
      returnVal->atom = utils_atom_cloneWithGrounding(atom, grAct);
      returnVal->isPos = true;
      return returnVal;
    }
  }
  for (int32_t i = 0; i < precond->numOfNeg; ++i) {
    struct atom *atom = &precond->negLiterals[i];
    if (planner_containsPrecondAtom(state, grAct, atom)) {
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
planner_satisfies(struct state *state, struct goal *goal)
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
    if ( ! planner_contains(state, atom)) {
      struct literal *returnVal = malloc(sizeof(*returnVal));
      returnVal->atom = utils_atom_clone(atom);
      returnVal->isPos = true;
      return returnVal;
    }
  }
  for (int32_t i = 0; i < goal->numOfNeg; ++i) {
    struct atom *atom = &goal->negLiterals[i];
    if (planner_contains(state, atom)) {
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
planner_stateAddAtom( struct state *state,
                      struct groundAction *grAct,
                      struct atom *atom)
{
  if (atom == NULL) {
    return;
  }

  for (int32_t idx = 0; idx < state->numOfFluents; ++idx) {
    struct atom *fluent = &state->fluents[idx];
    if (utils_atom_equalWithGrounding(fluent, atom, grAct)) {
      return;
    }
  }
  // Check if the types od the predicates parameter allow the atom to be
  // added.
  // TODO: If we would know that all types (action parameter types and 
  // predicate parameter types) match after grounding, we would no need that.
  for (int32_t idxArgs = 0; idxArgs < atom->pred->numOfParams; ++idxArgs) {
    struct type *predParamType = atom->pred->params[idxArgs].type;
    // Pointer arithmetic.
    int32_t idxGrounding = atom->terms[idxArgs] - grAct->action->params;
    if (0 <= idxGrounding && idxGrounding < grAct->action->numOfParams) {
      // We are dealing with a grounding. Check type.
      if ( ! typeSystem_isa(grAct->terms[idxGrounding]->type, predParamType)) {
        // Can not apply atom. Type missmatch.
        return;
      }
    } else {
      // We are dealing with a constant.
      if ( ! typeSystem_isa(atom->terms[idxArgs]->type, predParamType)) {
        // Can not apply atom.
        // That should never happen, since it's a constant.
        assert (false);
        return;
      }
    }
  }

  // Add the atom.
  ++state->numOfFluents;
  state->fluents = realloc(state->fluents,
                           sizeof(*state->fluents) * state->numOfFluents);

  //memcpy(&state->fluents[state->numOfFluents - 1], atom, sizeof(*atom));
  state->fluents[state->numOfFluents - 1].pred = atom->pred;
  int32_t size = sizeof(*state->fluents[state->numOfFluents - 1].terms) *
                 atom->pred->numOfParams;
  state->fluents[state->numOfFluents - 1].terms = malloc(size);

  // Set terms to objects according to mapping.
  for (int32_t idxArgs = 0; idxArgs < atom->pred->numOfParams; ++idxArgs) {
    // Pointer arithmetic.
    int32_t idxGrounding = atom->terms[idxArgs] - grAct->action->params;
    if (0 <= idxGrounding && idxGrounding < grAct->action->numOfParams) {
      // We are dealing with a grounding.
      state->fluents[state->numOfFluents - 1].terms[idxArgs] =
                                                    grAct->terms[idxGrounding];
    } else {
      // We are dealing with a constant.
      state->fluents[state->numOfFluents - 1].terms[idxArgs] =
                                                          atom->terms[idxArgs];
    }
  }
}

void
planner_stateRemoveAtom(struct state *state,
                        struct groundAction *grAct,
                        struct atom *atom)
{
  if (atom == NULL) {
    return;
  }

  for (int32_t idx = 0; idx < state->numOfFluents; ++idx) {
    struct atom *fluent = &state->fluents[idx];
    if (utils_atom_equalWithGrounding(fluent, atom, grAct)) {
      //printf("REMOVING ATOM FROM STATE\n"); // DEBUG
      // Remove atom.
      --state->numOfFluents;
      struct atom *tmp = malloc(sizeof(*tmp) * (state->numOfFluents));
      // Copy memory before the element to remove.
      memcpy(tmp, state->fluents, sizeof(*tmp) * idx);
      // Copy memory after the element to remove.
      memcpy( &tmp[idx],
              &state->fluents[idx + 1],
              sizeof(*tmp) * (state->numOfFluents - idx));

      // Clean up removed fluent
      libpddl31_atom_free(&state->fluents[idx]);
      // And old array
      free(state->fluents);

      state->fluents = tmp;
      // Do not return here. I want to remove all of them.
    }
  }
}

void
planner_applyEffElem( struct state *state,
                      struct groundAction *grAct,
                      struct effectElem *effElem)
{
  if (effElem == NULL)  {
    return;
  }
  switch(effElem->type) {
  case POS_LITERAL : {
    planner_stateAddAtom(state, grAct, effElem->it.literal);
    break;
  }
  case NEG_LITERAL: {
    planner_stateRemoveAtom(state, grAct, effElem->it.literal);
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
planner_apply(struct state *state, struct groundAction *grAct)
{
  if (grAct == NULL) {
    return;
  }
  if (state == NULL) {
    // Parser creates an empty struct state in case of an empty state.
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
planner_hasGap( struct state *initState,
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
  struct state *lState = utils_copyState(initState);
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
      utils_freeState(lState);
      return result;
    }
    // Precondition is met. Apply the action.
    planner_apply(lState, grAct);
    printf("action applied:   "); // DEBUG
    utils_print_groundAction(grAct); // DEBUG
    printf("\n"); // DEBUG
    libpddl31_state_print(lState); // DEBUG
    printf("\n"); // DEBUG
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
  utils_freeState(lState);
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

  utils_print_actionListCompact(actAcc); // DEBUG
  printf("\n"); // DEBUG

  struct actionList *actAccBack = actAcc;

  struct state *initState = problem->init;
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
    printf("dl*");
    utils_free_gap(gap);
    return NULL;
  }

  printf("gap:   "); // DEBUG
  utils_print_gap(gap); // DEBUG
  printf("\n"); // DEBUG

  struct actionList *actsToFixGap = planner_getActsToFixGap(problem, gap);
  printf("actsToFixGap.length: %d\n",
         utils_actionList_length(actsToFixGap)); //DEBUG
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
  for (int32_t depth = 1; depth < INT32_MAX; ++depth) {
    printf("\n### depth search with depth %d\n\n", depth); // DEBUG
    struct actionList *solution = planner_solveProblem(problem, depth);
    if (solution != NULL) {
      return solution;
    }
  }
  return NULL;
}

// This function will edit the weights of the list in place.
// It produces weights between INT32_MIN and 0.
// If an actions' precondition is fulfilled by the state, then its weight will
// be zero. For every atom of the precondition, which is not fulfilled, its
// weight will be reduced by one.
void planner_actionList_calculateWeights(struct actionList *actL,
                                         struct state *state)
{
  // Iterate over actions.
  for (/* empty */; actL != NULL; actL = actL->next) {
    int32_t weight = 0;

    struct groundAction grAct = actL->act;
    struct action *action = grAct->action;
    struct goal *precond = action->precond;

    // Iterate over precondition atoms.
    for (int32_t idxPrecond = 0; idxPrecond < precond->numOfPos; ++idxPrecond) {
      struct atom *atom = &precond->posLiterals[idxPrecond];
      if ( ! planner_containsPrecondAtom(state, grAct, atom)) {
        // Actually decreasing the weight, when a precondition is not met.
        weight--;
      }
    }
    for (int32_t idxPrecond = 0; idxPrecond < precond->numOfNeg; ++idxPrecond) {
      struct atom *atom = &precond->negLiterals[idxPrecond];
      if (planner_containsPrecondAtom(state, grAct, atom)) {
        // Actually decreasing the weight, when a precondition is not met.
        weight--;
      }
    }

    // Write weight to actual action-list-element.
    actL->weight = weight;
  }
}

// This quicksort is stable.
struct actionList *
planner_quicksort(struct actionList *actL)
{
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

  // List for elements before pivot. New elements will be inserted in the
  // front. So I will add the pivot element already here.
  struct actionList *before = pivot;
  struct actionList *after = NULL; // List for elements after the pivot.

  // Iterator over the input list.
  while(actL != NULL) {
    // Note: Including the current element in the result and iterating to the
    // next element of the input list needs careful processing.
    struct actionList *actLNext = actL->next;
    if (actL->weight < pivot->weight) {
      struct actionList *beforeOld = before;
      before = actL;
      before->next = beforeOld;
    } else {
      assert(actL->weight >= pivot->weight);
      struct actionList *afterOld = after;
      after = actL;
      after->next = afterOld;
    }
    actL = actLNext;
  }

  // Recursion.
  before = planner_quicksort(before);
  after = planner_quicksort(after);

  // The pivot element must still be the last element of the before-list,
  // because the before list only holds elements with smaller weight than the
  // pivot element.
  assert(pivot->next == NULL);

  // Put lists together.
  pivot->next = after;
  struct actionList *result = before;

  return result;
}

struct actionList *
planner_sortActsAccToState(struct actionList *actL, struct state *state)
{
  // Calculate weights for sorting.
  planner_actionList_calculateWeights(actL, state);
  return planner_quicksort(actL);
}
