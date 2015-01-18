#ifndef PROBSPACE_H
#define PROBSPACE_H

#include "libpddl31.h"
#include "utils.h"

struct probSpace {
  // The corresponding problem.
  struct problem *problem;
  // Problem Space. That is a state, which is an union of all possible
  // states in the problem instance.
  trie_t setFluents;
  // A set of all possible actions in the problem space.
  struct actionList *allGrActs;
};

struct probSpace *ps_init(struct problem *problem);

void ps_free(struct probSpace *probSpace);

struct actionList *ps_filter(struct probSpace *probSpace,
                             struct actionList *actL);
int32_t ps_calcMaxVarOcc(struct probSpace *probSpace);

struct actionList *ps_getActsToFixGap(struct probSpace *probSpace,
                                      struct literal *literal);
#endif
