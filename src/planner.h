#ifndef PLANNER_H
#define PLANNER_H

// This function returns a pointer to the new state. It might change the old
// state (the argument) though. This junction might return NULL on error.
struct formula *planner_apply_effect(   struct formula *stateOld,
                                        struct formula *effect);

#endif // PLANNER_H
