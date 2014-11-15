#ifndef GROUNDER_H
#define GROUNDER_H

#include "pddl31structs.h"

struct grounding
{
  // An array of pointers to constants.
  struct term **terms;
};

struct groundAction
{
  struct action *action;
  // action->numOfParams

  int32_t numOfGrnds;
  // An array of pointers to groundings.
  struct grounding **grnds;
};


//struct groundAction *
void grounder_groundAction( struct state *state,
                            int32_t idxPrecond,
                            struct grounding *partialG,
                            struct groundAction *grAct);
void grounder_print_groundAction(struct groundAction *grA);

/* struct action **grounder_getActions(Grounder grounder, */
/*                                     struct state *state); */

#endif
