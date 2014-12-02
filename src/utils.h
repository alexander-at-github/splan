#ifndef UTILS_H
#define UTILS_H

#include "libpddl31.h"

// A gap is a literal combined with a position in a sequence of actions.
// For details see Theorem 5 of Paper 'Parameterized Complexity of Optimal
// Planning'.
struct gap {
  struct literal *literal;
  // The position of the gap in the sequence of actions. The gap is said to be
  // before 'position'. Thus 'position' >= 1.
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
void utils_print_gap(struct gap *gap);
void utils_print_literal(struct literal *literal);

// For debug pusposes only.
int32_t utils_actionList_length(struct actionList *list);

// Creates (allocates) a ground action with all the groundings set to NULL.
struct groundAction *utils_create_groundAction(struct action *action);

// Completes the grounding of a list of parital grounded actions.
struct actionList *utils_groundActions(struct problem *problem,
                                       struct actionList *partialGrounded);

void utils_free_groundAction(struct groundAction *grAct);

struct actionList *utils_concatActionLists(struct actionList *l1,
                                           struct actionList *l2);

bool utils_term_equal(struct term *t1, struct term *t2);
bool utils_atom_equal(struct atom *a1, struct atom *a2);
// Atom 'a2' has to be affiliated with the ground action 'grAct'.
bool utils_atom_equalWithGrounding( struct atom *a1,
                                    struct atom *a2,
                                    struct groundAction *grAct);
struct atom *utils_atom_cloneWithGrounding(struct atom *atom,
                                           struct groundAction *grAct);

#endif
