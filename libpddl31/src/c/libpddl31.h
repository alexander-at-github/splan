#ifndef LIBPDDL31_H
#define LIBPDDL31_H

#include <stdbool.h>
#include <stdlib.h>

#include "pddl31structs.h"

struct domain *libpddl31_domain_parse(char *filename);
// Attention: struct problem must be free'd before domain!
void libpddl31_domain_free(struct domain *domain);
void libpddl31_domain_print(struct domain *domain);

// TODO: Does this call actually alter the domains' object manager?
// It should not.
struct problem *libpddl31_problem_parse(struct domain *domain, char *filename);
// Attention: struct problem must be free'd before domain!
void libpddl31_problem_free(struct problem *problem);
void libpddl31_problem_print(struct problem *problem);

void libpddl31_atom_free(struct atom *atom);
void libpddl31_term_free(struct term *term);
void libpddl31_action_free(struct action *action);

void libpddl31_atom_print(struct atom *atom);
void libpddl31_term_print(struct term *term);
void libpddl31_predicate_print(struct predicate *pred);
void libpddl31_action_print(struct action *action);
void libpddl31_goal_print(struct goal *goal);

//void libpddl31_state_print(struct state *state);
//void libpddl31_free_state(struct state *state);

struct term *libpddl31_term_clone(struct term *src);


// Creates (allocates) a ground action with all the groundings set to NULL.
struct groundAction *libpddl31_create_groundAction(struct action *action);
void libpddl31_free_groundAction(struct groundAction *grAct);

bool libpddl31_term_equal(struct term *t1, struct term *t2);

#endif // LIBPDDL31_H
