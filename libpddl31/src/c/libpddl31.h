#ifndef LIBPDDL31_H
#define LIBPDDL31_H

#include <stdbool.h>
#include <stdlib.h>

#include "pddl31structs.h"

struct domain *libpddl31_domain_parse(char *filename);
// Attention: struct problem must be free'd before domain!
void libpddl31_domain_free(struct domain *domain);
void libpddl31_domain_print(struct domain *domain);

struct problem *libpddl31_problem_parse(struct domain *domain, char *filename);
// Attention: struct problem must be free'd before domain!
void libpddl31_problem_free(struct problem *problem);
//void libpddl31_problem_print(struct problem *problem);
//
///* Checks if a problem is member of a domain */
//bool libpddl31_problem_is_member_of_domain( struct problem *problem,
//                                            struct domain *domain);

void libpddl31_atom_free(struct atom *atom);
void libpddl31_term_free(struct term *term);
void libpddl31_action_free(struct action *action);

void libpddl31_term_print(struct term *term);
void libpddl31_predicate_print(struct predicate *pred);
void libpddl31_action_print(struct action *action);
void libpddl31_goal_print(struct goal *goal);

#endif // LIBPDDL31_H
