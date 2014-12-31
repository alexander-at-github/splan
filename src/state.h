#ifndef STATE_H
#define STATE_H

#include "libpddl31.h"
#include "utils.h"

typedef struct st_state * state_t;

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
state_t state_clone(state_t state);
void state_print(state_t state);

#endif // STATE_H
