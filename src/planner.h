#ifndef PLANNER_H
#define PLANNER_H

#include "libpddl31.h"

struct actionList *planner_solveProblem(struct problem *problem,
                                        int32_t depthLimit);

#endif // PLANNER_H
