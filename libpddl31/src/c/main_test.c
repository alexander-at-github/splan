#include <stdio.h>
#include "libpddl31.h"
#include "pddl31structs.h"

int
main (int agrc, char **argv)
{
    char *domainFilename =
                        "../test_instances/openstacks-strips/p01-domain.pddl";
                        //"test_instances/blocks_domain_altered.pddl";
    struct domain *domain = libpddl31_domain_parse(domainFilename);
    libpddl31_domain_print(domain);

    char *problemFilename =
                        "../test_instances/openstacks-strips/p01.pddl";
                        //"test_instances/probBLOCKS-4-0_altered.pddl";
    struct problem *problem = libpddl31_problem_parse(problemFilename);
    libpddl31_problem_print(problem);

    printf("\n");
    bool isMember = libpddl31_problem_is_member_of_domain(problem, domain);
    printf("problem is member of domain: %s\n", isMember ? "true" : "false");
    printf("\n");

    libpddl31_problem_free(problem);
    libpddl31_domain_free(domain);
}
