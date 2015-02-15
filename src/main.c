#include <stdio.h>
#include <unistd.h>

#include "aStarPlanner.h"
#include "planner.h"
#include "utils.h"

static void
print_usage(char **argv)
{
  fprintf(stderr,
          "Usage: %s -d domain-file-name -p problem-file-name"
          " [-l plan-length-guess] [-t timeout-in-seconds]\n",
          argv[0]);
}

int main(int argc, char **argv)
{
  char *domainFilename = NULL;
  char *problemFilename = NULL;
  int32_t planLengthGuess = 1;
  int32_t timeout = -1; // in seconds

  int opt;
  while ((opt = getopt(argc, argv, "d:p:l:t:")) != -1) {
    switch (opt) {
    case 'd':
      domainFilename = optarg;
      break;
    case 'p':
      problemFilename = optarg;
      break;
    case 'l':
      planLengthGuess = (int32_t) strtol(optarg, NULL, 10);
      break;
    case 't':
      timeout = (int32_t) strtol(optarg, NULL, 10);
      //printf("timeout set to %d seconds.\n", timeout);
      break;
    default:
      print_usage(argv);
      exit(EXIT_FAILURE);
      break;
    }
  }
  if (domainFilename == NULL || problemFilename == NULL) {
    print_usage(argv);
    return EXIT_FAILURE;
  }

  struct domain *domain = libpddl31_domain_parse(domainFilename);
  if (domain == NULL) {
    return EXIT_FAILURE;
  }
  struct problem *problem = libpddl31_problem_parse(domain, problemFilename);

//  struct actionList *result = planner_iterativeDeepeningSearch_v3(problem,
//                                                               planLengthGuess,
//                                                               timeout);

  struct actionList *result = aStarPlanner(problem);

  printf("SOLUTION FOUND\n");
  utils_print_actionListCompact(result);
  printf("\n");

  // Clean up
  utils_free_actionList(result);
  libpddl31_problem_free(problem);
  libpddl31_domain_free(domain);
  return EXIT_SUCCESS;
}
