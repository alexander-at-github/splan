#include <stdlib.h>

#include "utils.h"

void utils_free_literal(struct literal *literal)
{
  free(literal);
}

struct state *utils_copyState(struct state *state)
{
  struct state *result = malloc(sizeof(*result));
  result->numOfFluents = state->numOfFluents;
  size_t size = sizeof(*result->fluents) * result->numOfFluents;

  result->fluents = malloc(size);
  memcpy(result->fluents, state->fluents, size);

  return result;
}

void utils_freeStateShallow(struct state *state)
{
  if (state->fluents != NULL) {
    free(state->fluents);
  }
  free(state);
}
