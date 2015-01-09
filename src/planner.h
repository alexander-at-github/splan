#ifndef PLANNER_H
#define PLANNER_H

#include "libpddl31.h"
#include "utils.h"

struct gap *planner_hasGap( state_t initState,
                            struct goal *goal,
                            struct actionList *actions);
struct actionList *planner_getActsToFixGap( struct problem *problem,
                                            struct gap *gap);
struct actionList *planner_solveProblem(struct problem *problem,
                                        int32_t depthLimit);
struct actionList *planner_solveProblem_v2( struct problem *problem,
                                            int32_t depthLimit);

struct literal *planner_satisfies(state_t state, struct goal *goal);
/* void planner_stateRemoveAtom(struct state *state, */
/*                              struct groundAction *grAct, */
/*                              struct atom *atom); */
/* void planner_stateAddAtom(struct state *state, */
/*                           struct groundAction *grAct, */
/*                           struct atom *atom); */
struct actionList *planner_iterativeDeepeningSearch(struct problem *problem);
struct actionList *planner_iterativeDeepeningSearch_v2(struct problem *problem);

// Returns NULL iff the state satisfies the goal.
// Returns the first literal which is not satisfied otherwise.
struct literal *planner_satisfiesPrecond(state_t state,
                                         struct groundAction *grAct);

#endif // PLANNER_H
