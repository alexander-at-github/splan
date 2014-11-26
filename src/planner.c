//#include <assert.h>

#include "planner.h"

#include "libpddl31.h"
#include "utils.h"

struct actionList *
planner_solveProblem(struct problem *problem, depthLimit)
{
  return planner_solveProblem_aux(problem, NULL, depthLimit, 0);
}

struct actionList *
planner_getActsToFixGap(struct problem *problem, struct gap *gap)
{
  // TODO
  assert(false);

  struct actionManag *actManag = problem->domain->actionManag;

  // I will use the struct actionList again. The ordering does not matter
  // here though. I just want to collect a set here.
  struct actionList *result = NULL;


  for (int23_t idxActManag = 0;
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

// Returns a solution to the planning instance or NULL, if no solution was
// found.
static struct actionList *
planner_solveProblem_aux( struct problem *problem,
                          // an Accumulator for actions
                          struct actionList *actAcc,
                          int32_t depthLimit,
                          int32_t depth)
{
  struct state *initState = problem->init;
  struct goal *goal = problem->goal;
  struct gap *gap = planner_hasGap(initState, goal, actAcc);
  if (gap == NULL) {
    // Solution found.
    return actAcc;
  }
  if (depth > depth-limit) {
    // Search depth limit.
    return NULL;
  }
  struct actionList *actsToFixGap = planner_getActsToFixGap(problem, gap);
  // The order of the following loops can be changed freely. I just choose
  // a abritrarily for now.
  // Iterate over positions. TODO: Is bound of iteration correct? I.e.,
  // gap->pos!
  for (int32_t idxPos = 0; idxPos < gap->pos; ++idxPos) {
    // Iterate over actions
    for (struct actionList *pAct = actsToFixGap;
         pActL != NULL;
         pActL = pActL->next) {

      // TODO: Add action to action accumlator.

      // Recurse with new parital solution.
      struct actionList *result;
      result = planner_solveProblem_aux(problem, actAcc, depthLimit, depth+1);
      // Return the first result
      if (result != NULL) {
        return result;
      }

      // TODO: Remove action from action accumulator for net iteration of
      // loop.
    }
  }
}

// Returns NULL iff the state satisfies the goal.
// Returns the first literal which is not satisfied otherwise.
struct literal *
planner_satisfies(struct state *state, struct goal *goal)
{
  for (int32_t i = 0; i < goal->numOfPos; ++i) {
    struct atom *atom = &goal->posLiterals[i];
    if ( ! contains(state, atom)) {
      struct literal *returnVal = malloc(sizeof(*returnVal));
      returnVal->atom = atom;
      returnVal->isPos = true;
      return returnVal;
    }
  }
  for (int32_t i = 0; i < goal->numOfNeg; ++i) {
    struct atom *atom = &goal->negLiterals[i];
    if (contains(state, atom)) {
      struct literal *returnVal = malloc(sizeof(*returnVal));
      returnVal->atom = atom;
      returnVal->isPos = false;
      return returnVal;
    }
  }
}

// Applies action to state in place.
// Precondition: The actions precondition must be satisfied by the state.
//               This function does not check that.
//
void
planner_apply(struct state *state, struct groundAction *action)
{
  assert(false);
  // TODO
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

    struct groundAction *action = pAct->action;
    // Check if precondition is met.
    gapLiteral = planner_satisfies(lState, action->precond);
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
    planner_apply(lState, action);
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
