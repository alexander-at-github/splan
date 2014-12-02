#ifndef PLANNER_H
#define PLANNER_H

#include "libpddl31.h"
#include "utils.h"

struct gap *planner_hasGap( struct state *initState,
                            struct goal *goal,
                            struct actionList *actions);
struct actionList *planner_solveProblem(struct problem *problem,
                                        int32_t depthLimit);

#endif // PLANNER_H
