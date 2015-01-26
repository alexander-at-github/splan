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

// A single linked list of grounded actions.
struct actionList
{
  struct groundAction *act;

  struct actionList *next;

  // A weight for sorting action lists. Often this is not used.
  int32_t weight;

  // A position. This can be used to predefine a position, iat which this
  // element should be placed at in a sequence of elements. Often this is not
  // used.
  int32_t pos;
};

void utils_free_literal(struct literal *literal);
/* struct state *utils_copyState(struct state *state); */
void utils_free_actionList(struct actionList *list);
/* void utils_freeStateShallow(struct state *state); */
/* void utils_freeState(struct state *state); */
struct actionList *utils_actionFixesGap(struct action *action,
                                        struct gap *gap);
void utils_print_actionList(struct actionList *list);
void utils_print_gap(struct gap *gap);
void utils_print_literal(struct literal *literal);

// For debug pusposes only.
int32_t utils_actionList_length(struct actionList *list);

// Completes the grounding of a list of parital grounded actions.
struct actionList *utils_groundActions(struct problem *problem,
                                       struct actionList *partialGrounded);

struct actionList *utils_concatActionLists(struct actionList *l1,
                                           struct actionList *l2);

bool utils_atom_equal(struct atom *a1, struct atom *a2);
// Atom 'a2' has to be affiliated with the ground action 'grAct'.
bool utils_atom_equalWithGrounding( struct atom *a1,
                                    struct atom *a2,
                                    struct groundAction *grAct);
struct atom *utils_atom_clone(struct atom *atom);
struct atom *utils_atom_cloneWithGrounding(struct atom *atom,
                                           struct groundAction *grAct);
void utils_free_gap(struct gap *gap);
struct actionList *utils_cloneActionList(struct actionList *actL);
void utils_print_groundAction(struct groundAction *grAct);
void utils_print_actionListCompact(struct actionList *list);

// @pos: If zero, removes first element.
struct actionList *utils_removeActionFromListAtPosition(struct actionList *,
                                                        int32_t);
// Sets the position variable of a whole list.
void utils_actionList_setPosition(struct actionList *al, int32_t position);

struct actionList *utils_addActionToListAtPosition(struct actionList *head,
                                                   struct groundAction *grAct,
                                                   int32_t position);
struct actionList *utils_addActionToList(struct actionList *head,
                                         struct groundAction *grAct);
void utils_free_actionListShallow(struct actionList *list);
struct actionList *utils_cloneActionListShallow(struct actionList *actL);
struct actionList *utils_actionList_reverse(struct actionList *list);

#endif
