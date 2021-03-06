﻿#ifndef PLANNER_H
#define PLANNER_H

#include "libpddl31.h"
#include "probSpace.h"
#include "utils.h"

struct gap *planner_hasGap( trie_t initState,
                            struct goal *goal,
                            struct actionList *actions);
struct actionList *planner_getActsToFixGap( struct problem *problem,
                                            struct probSpace *probSpace,
                                            struct gap *gap);
struct actionList *planner_solveProblem(struct problem *problem,
                                        struct probSpace *probSpace,
                                        int32_t depthLimit);
struct actionList *planner_solveProblem_v2( struct problem *problem,
                                            struct probSpace *probSpace,
                                            int32_t depthLimit);

struct literal *planner_satisfies(trie_t state, struct goal *goal);
/* void planner_stateRemoveAtom(struct state *state, */
/*                              struct groundAction *grAct, */
/*                              struct atom *atom); */
/* void planner_stateAddAtom(struct state *state, */
/*                           struct groundAction *grAct, */
/*                           struct atom *atom); */
struct actionList *planner_iterativeDeepeningSearch(struct problem *problem);
struct actionList *planner_iterativeDeepeningSearch_v2(struct problem *problem);
struct actionList *planner_iterativeDeepeningSearch_v3(struct problem *problem, int32_t planLengthguess, int32_t timeout); // timeout in seconds

// Returns NULL iff the state satisfies the goal.
// Returns the first literal which is not satisfied otherwise.
struct literal *planner_satisfiesPrecond(trie_t state,
                                         struct groundAction *grAct);

#endif // PLANNER_H
