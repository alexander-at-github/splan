#ifndef GROUNDER_H
#define GROUNDER_H

#include "pddl31structs.h"

typedef struct grounder *Grounder;

Grounder grounder_create(struct domain *domain);
struct action **grounder_getActions(Grounder grounder,
                                    struct state *state);

#endif
