#ifndef LIBPDDL31_H
#define LIBPDDL31_H

#include <stdbool.h>
#include <stdlib.h>

#include "pddl31structs.h"

struct domain *libpddl31_domain_parse(char *filename);
void libpddl31_domain_free(struct domain *domain);
void libpddl31_domain_print(struct domain *domain);

struct problem *libpddl31_problem_parse(char *filename);
void libpddl31_problem_free(struct problem *problem);
void libpddl31_problem_print(struct problem *problem);

/* Checks if a problem is member of a domain */
bool libpddl31_problem_is_member_of_domain( struct problem *problem,
                                            struct domain *domain);

void libpddl31_formula_free_rec(struct formula *formula);
void libpddl31_formula_print(struct formula *formula);

void libpddl31_term_free(struct term *term);

#endif // LIBPDDL31_H
