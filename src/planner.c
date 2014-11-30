#include <assert.h>

#include "planner.h"

#include "libpddl31.h"
#include "utils.h"

struct actionList *
planner_getActsToFixGap(struct problem *problem, struct gap *gap)
{
  // TODO: Check for correctness.

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

    if (fluent->pred == atom->pred) {

      for ( int32_t idxArgs = 0;
            idxArgs < atom->pred->numOfParams;
            ++idxArgs) {

        if (utils_term_equal(fluent->terms[idxArgs], atom->terms[idxArgs])) {
          return true;
        }
      }
    }
  }
  return false;
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
  //libpddl31_goal_print(goal); // DEBUG
  //printf("\n\n"); //DEBUG

  for (int32_t i = 0; i < goal->numOfPos; ++i) {
    struct atom *atom = &goal->posLiterals[i];
    if ( ! planner_contains(state, atom)) {
      struct literal *returnVal = malloc(sizeof(*returnVal));
      returnVal->atom = atom;
      returnVal->isPos = true;
      return returnVal;
    }
  }
  for (int32_t i = 0; i < goal->numOfNeg; ++i) {
    struct atom *atom = &goal->negLiterals[i];
    if (planner_contains(state, atom)) {
      struct literal *returnVal = malloc(sizeof(*returnVal));
      returnVal->atom = atom;
      returnVal->isPos = false;
      return returnVal;
    }
  }
  // State satisfies goal.
  return NULL;
}

// Applies action to state in place.
// Precondition: The actions precondition must be satisfied by the state.
//               This function does not check that.
//
void
planner_apply(struct state *state, struct groundAction *grAct)
{
  //assert(false);
  //// TODO
  //if (grAct == NULL) {
  //  return;
  //}
  //if (state == NULL) {
  //  assert (false);
  //  // TODO: Check if the parser creates an empty struct state in case of an
  //  // empty state.
  //}

  //struct effect *effCurr = grAct->action->effect;
  //for ( int32_t idxEff = 0;
  //      idxEff < effCurr->numOfElems;
  //      ++idxEff) {

  //  struct effectElem *effElem = &effCurr->elems[idxEff];
  //  // TODO: Continue here
  //}
}

// Returns NULL iff there is no gap. Returns the gap otherwise.
struct gap *
planner_hasGap( struct state *initState,
                struct goal *goal,
                struct actionList *actions)
{
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
    gapLiteral = planner_satisfies(lState, grAct->action->precond);
    if (gapLiteral != NULL) {
      // Precondition is not met.
      result = malloc(sizeof(*result));
      result->literal = gapLiteral;
      result->position = idxAct;
      // Clean up.
      utils_freeStateShallow(lState);
      return result;
    }
    // Precondition is met. Apply the action.
    planner_apply(lState, grAct);
  }

  /* Check if the goal is met after applying all the acitons. */

  gapLiteral = planner_satisfies(lState, goal);
  if (gapLiteral != NULL) {
    result = malloc(sizeof(*result));
    result->literal = gapLiteral;
    result->position = idxAct; // TODO: Is that correct?
  }

  /* Clean up */
  utils_freeStateShallow(lState);
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
  struct actionList *actAccBack = actAcc;

  struct state *initState = problem->init;
  struct goal *goal = problem->goal;
  struct gap *gap = planner_hasGap(initState, goal, actAcc);
  printf("actAcc.length: %d\n", utils_actionList_length(actAcc)); //DEBUG
  printf("gap->position: %d\n", gap->position); // DEBUG
  if (gap == NULL) {
    // Solution found.
    printf("\n\nSOLUTION FOUND\n\n\n"); // DEBUG
    return actAcc;
  }
  if (depth >= depthLimit) { // TODO '<' or '<=' ?
    // Search depth limit.
    return NULL;
  }
  struct actionList *actsToFixGap = planner_getActsToFixGap(problem, gap);
  printf("actsToFixGap.length: %d\n", utils_actionList_length(actsToFixGap)); //DEBUG
  // TODO: Free actsToFixGap somewhere.

  // The order of the following loops can be changed freely. I just choose
  // a abritrarily for now.

  // Iterate over positions. TODO: Is bound of iteration correct? I.e.,
  // gap->pos! I think so.
  // Note: gap->pos >= 1.
  int32_t idxPos = 0;
  struct actionList *curr = NULL;
  struct actionList *afterCurr = actAcc;
  while (idxPos < gap->position) {
  //for ( int32_t idxPos = 0;
  //      idxPos < gap->pos;
  //      idxPos++) {

    // Iterate over actions
    for (struct actionList *pActL = actsToFixGap;
         pActL != NULL;
         pActL = pActL->next) {

      struct actionList tmpActL;
      tmpActL.act = pActL->act;

      // TODO: Add action to action accumlator.
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

      // Recurse with new parital solution.
      struct actionList *result;
      result = planner_solveProblem_aux(problem, actAcc, depthLimit, depth+1);
      // Return the first result
      if (result != NULL) {
        return result;
      }

      // TODO: Remove action from action accumulator for net iteration of
      // loop.
      if (curr == NULL) {
        actAcc = actAccBack;
      } else {
        curr->next = afterCurr;
      }
      //tmpActL.next = NULL;
    }

    // Increment loop variables.
    idxPos++;
    // TODO: Check that.
    if (afterCurr != NULL) {
      curr = afterCurr;
      afterCurr = afterCurr->next;
    } else {
      // This case could only happen in a last iteration.
      assert (idxPos == gap->position);
    }

  }
  // No solution within this depth-limit found.
  return NULL;
}

struct actionList *
planner_solveProblem(struct problem *problem, int32_t depthLimit)
{
  return planner_solveProblem_aux(problem, NULL, depthLimit, 0);
}
