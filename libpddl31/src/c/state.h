#ifndef STATE_H
#define STATE_H

#include "pddl31structs.h"

typedef struct st_state * state_t;

//// Forward declarations of pddl31 structs
//struct domain;
//struct atom;
//struct groundAction;

state_t state_createEmptyFrom(state_t state);
state_t state_createEmpty(struct domain *domain);
state_t state_createFromLibpddl31(struct domain *domain,
                                  pANTLR3_LIST listOfAtoms);
void state_add(state_t state, struct atom *atom);
bool state_contains(state_t state, struct atom *atom);
state_t state_remove(state_t state, struct atom *atom);
void state_free(state_t state);

// The ground action needs to be the owner of the atom.
void state_addGr( state_t state,
                  struct atom *atom,
                  struct groundAction *grAct);
// The ground action needs to be the owner of the atom.
state_t state_removeGr( state_t state,
                        struct atom *atom,
                        struct groundAction *grAct);
bool state_containsGr(state_t state,
                      struct atom *atom,
                      struct groundAction *grAct);
state_t state_clone(state_t state);
void state_print(state_t state);
void state_cleanupSNBuffer();
bool state_incCountGr(state_t state,
                      struct atom *atom,
                      struct groundAction *grAct);
int32_t state_getMaxCount(state_t state);
void state_setCount(state_t state, int32_t num);

#endif // STATE_H
