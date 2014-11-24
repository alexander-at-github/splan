#ifndef UTILS_H
#define UTILS_H

#include "libpddl31.h"

// A gap is a literal combined with a position in a sequence of actions.
// For details see Theorem 5 of Paper 'Parameterized Complexity of Optimal
// Planning'.
struct gap {
  struct literal *literal;
  // The position of the gap in the sequence of actions.
  int32_t position;
};

struct literal
{
  // For a negated literal 'isPos' will be false. Otherwise 'isPos' will be
  // true.
  bool isPos;
  struct atom *atom;
};

struct groundAction
{
  struct action *action;
  // action->numOfParams

  // The grounding
  // An array of pointers to constants.
  // It maps the parameter action->params[i] to terms[i] for i in
  // 0..(action->numOfParams - 1).
  struct term **terms;
};

// A single linked list of grounded actions.
struct actionList
{
  struct groundAction *act;

  struct actionList *next;
};

void utils_free_literal(struct literal *literal);
struct state *utils_copyState(struct state *state);
void utils_free_actionList(struct actionList *list);
void utils_freeStateShallow(struct state *state);
struct actionList *utils_actionFixesGap(struct action *action,
                                        struct gap *gap);
void utils_print_actionList(struct actionList *list);
// For debug pusposes only.
int32_t utils_actionList_length(struct actionList *list);

#endif
