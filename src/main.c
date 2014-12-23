#include <stdio.h>
#include <unistd.h>

#include "planner.h"
#include "utils.h"

static void
print_usage(char **argv)
{
  fprintf(stderr,
          "Usage: %s -d domain-file-name -p problem-file-name\n",
          argv[0]);
}

int main(int argc, char **argv)
{
  char *domainFilename = NULL;
  char *problemFilename = NULL;

  int opt;
  while ((opt = getopt(argc, argv, "d:p:")) != -1) {
    switch (opt) {
    case 'd':
      domainFilename = optarg;
      break;
    case 'p':
      problemFilename = optarg;
      break;
    default:
      print_usage(argv);
      exit(EXIT_FAILURE);
      break;
    }
  }
  if (domainFilename == NULL || problemFilename == NULL) {
    print_usage(argv);
    exit(EXIT_FAILURE);
  }

  struct domain *domain = libpddl31_domain_parse(domainFilename);
  struct problem *problem = libpddl31_problem_parse(domain, problemFilename);

  struct actionList *result = planner_iterativeDeepeningSearch_v2(problem);
  utils_print_actionList(result);
  printf("\n");

  // Clean up
  utils_free_actionList(result);
  libpddl31_problem_free(problem);
  libpddl31_domain_free(domain);
  return EXIT_SUCCESS;
}
