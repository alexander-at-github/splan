#ifndef ASTARPLANNER_H
#define ASTARpLANNER_H

#include "libpddl31.h"
#include "list.h"
#include "probSpace.h"
#include "utils.h"

struct actionList *aStarPlanner(struct problem *problem, int timeout);
list_t aStarPlanner_getAllGaps(struct probSpace *probSpace,
                               struct actionList *actions);

#endif
