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
          " [-l plan-length-guess] [-t timeout-in-seconds] [-d | -a]\n",
          argv[0]);
}

int main(int argc, char **argv)
{
  char *domainFilename = NULL;
  char *problemFilename = NULL;
  int32_t planLengthGuess = 1;
  int32_t timeout = -1; // in seconds
  bool iterativeDeepeningSearch = false;
  bool aStarSearch = false;

  int opt;
  while ((opt = getopt(argc, argv, "d:p:l:t:ia")) != -1) {
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
    case 'i':
      iterativeDeepeningSearch = true;
      break;
    case 'a':
      aStarSearch = true;
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
  if ( ! iterativeDeepeningSearch && ! aStarSearch) {
    printf("No algorithm selected: Use either -i for iterative deepening "
           "depth-first search or -a for A-Star Search.\n");
    return EXIT_FAILURE;
  }

  struct domain *domain = libpddl31_domain_parse(domainFilename);
  if (domain == NULL) {
    return EXIT_FAILURE;
  }
  struct problem *problem = libpddl31_problem_parse(domain, problemFilename);

  if (aStarSearch) {
    struct actionList *result = aStarPlanner(problem, timeout);

    if (result != NULL) {
      printf("SOLUTION FOUND\n");
      utils_print_actionListCompact(result);
      printf("\n");
      utils_free_actionList(result);
    }
  }
  if (iterativeDeepeningSearch) {
    struct actionList *result = planner_iterativeDeepeningSearch_v3(problem,
                                                               planLengthGuess,
                                                               timeout);
    if (result != NULL) {
      utils_print_actionListCompact(result);
      printf("\n");
      utils_free_actionList(result);
    }
  }

  // Clean up
  libpddl31_problem_free(problem);
  libpddl31_domain_free(domain);
  return EXIT_SUCCESS;
}
