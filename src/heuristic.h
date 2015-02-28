#ifndef HEURISTIC_H
#define HEURISTIC_H

#include "probSpace.h"
#include "utils.h"

int heuristic_estimate(struct probSpace *, struct actionList *);
int heuristic_estimate_NOT_admissible(struct probSpace *probSpace,
                                      struct actionList *actL);

#endif
