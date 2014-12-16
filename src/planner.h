#ifndef PLANNER_H
#define PLANNER_H

#include "libpddl31.h"
#include "utils.h"

struct gap *planner_hasGap( struct state *initState,
                            struct goal *goal,
                            struct actionList *actions);
struct actionList *planner_getActsToFixGap( struct problem *problem,
                                            struct gap *gap);
struct actionList *planner_solveProblem(struct problem *problem,
                                        int32_t depthLimit);

struct literal *planner_satisfies(struct state *state, struct goal *goal);
void planner_stateRemoveAtom(struct state *state,
                             struct groundAction *grAct,
                             struct atom *atom);
void planner_stateAddAtom(struct state *state,
                          struct groundAction *grAct,
                          struct atom *atom);
struct actionList *planner_iterativeDeepeningSearch(struct problem *problem);

#endif // PLANNER_H
