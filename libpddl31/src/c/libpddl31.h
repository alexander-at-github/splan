#ifndef LIBPDDL31_H
#define LIBPDDL31_H

#include <stdlib.h>

struct domain *libpddl31_domain_parse(char *filename);
void libpddl31_domain_free(struct domain *domain);
void libpddl31_domain_print(struct domain *domain);

#endif // LIBPDDL31_H
