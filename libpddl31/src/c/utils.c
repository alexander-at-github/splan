#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include "utils.h"
#include "libpddl31.h"

// This is a deep free as you will need it after planner_satisfies() or
// utils_atom_clone().
void utils_free_literal(struct literal *literal)
{
  free(literal->atom->terms);
  free(literal->atom);
  free(literal);
}

/* struct state *utils_copyState(struct state *state) */
/* { */
/*   struct state *result = malloc(sizeof(*result)); */
/*   result->numOfFluents = state->numOfFluents; */
/*   size_t size = sizeof(*result->fluents) * result->numOfFluents; */

/*   result->fluents = malloc(size); */

/*   for (int32_t idxFluent = 0; idxFluent < result->numOfFluents; ++idxFluent) { */
/*     result->fluents[idxFluent].pred  = state->fluents[idxFluent].pred; */
/*     int32_t sizeTerms = sizeof(struct term *) * */
/*                                   result->fluents[idxFluent].pred->numOfParams; */

/*     result->fluents[idxFluent].terms = malloc(sizeTerms); */
/*     memcpy( result->fluents[idxFluent].terms, */
/*             state->fluents[idxFluent].terms, */
/*             sizeTerms); */
/*   } */

/*   return result; */
/* } */

/* void utils_freeState(struct state *state) */
/* { */
/*   if (state->fluents != NULL) { */
/*     // TODO for all fluents do free */
/*     for (int32_t i = 0; i < state->numOfFluents; ++i) { */
/*       libpddl31_atom_free(&state->fluents[i]); */
/*     } */
/*     free(state->fluents); */
/*   } */
/*   free(state); */
/* } */

/* void utils_freeStateShallow(struct state *state) */
/* { */
/*   if (state->fluents != NULL) { */
/*     free(state->fluents); */
/*   } */
/*   free(state); */
/* } */

void
utils_free_actionListShallow(struct actionList *list)
{
  while(list != NULL) {
    struct actionList *next = list->next;
    free(list);
    list = next;
  }
}

void
utils_free_actionList(struct actionList *list)
{
  while(list != NULL) {
    if(list->act != NULL) {
      libpddl31_free_groundAction(list->act);
    }
    struct actionList *next = list->next;
    free(list);
    list = next;
  }
}

// May return NULL on error.
// With position set to 0 it will insert in the front of the list.
// With position set to 1 it will insert after the first element.
struct actionList *
utils_addActionToListAtPosition(struct actionList *head,
                                struct groundAction *grAct,
                                int32_t position)
{
  if (grAct == NULL) {
    return head;
  }
  struct actionList *curr = NULL;
  struct actionList *afterCurr = head;

  int32_t idxAct = 0;
  while (idxAct < position && afterCurr != NULL) {
    curr = afterCurr;
    afterCurr = afterCurr->next;
    idxAct++;
  }
  if (idxAct != position) {
    assert(false);
    // Error.
    return NULL;
  }

  struct actionList *newElem = malloc(sizeof(*newElem));
  newElem->act = grAct;
  newElem->next = afterCurr;
  if (curr == NULL) {
    // We got a new head of the list.
    return newElem;
  }
  curr->next = newElem;
  return head;
}

struct actionList *
utils_addActionToList(struct actionList *head, struct groundAction *grAct)
{
  if (grAct == NULL) {
    // Do not do anything.
    return head;
  }
  struct actionList *newHead = malloc(sizeof(*newHead));
  newHead->act =grAct;
  newHead->next = head;
  return newHead;
}

// Time complexity: O(l1.length)
struct actionList *
utils_concatActionLists(struct actionList *l1, struct actionList *l2)
{
  if (l2 == NULL) {
    return l1;
  }
  if (l1 == NULL) {
    return l2;
  }
  struct actionList *l1Copy = l1;
  while (l1Copy->next != NULL) {
    l1Copy = l1Copy->next;
  }
  l1Copy->next = l2;
  return l1;
}

// Use, when there is chance to fix the gap.
struct groundAction *
utils_tryToFixGap(struct action *action,
                  struct atom *atomToFix, // Atom which might fix the gap
                  struct atom *gapAtom)
{
  struct groundAction *newGrAct = libpddl31_create_groundAction(action);

  for ( int32_t idxArgs = 0;
        idxArgs < atomToFix->pred->numOfParams;
        ++idxArgs) {

    struct term *effTerm = atomToFix->terms[idxArgs];
    struct term *gapTerm = gapAtom->terms[idxArgs];

    if (effTerm->isVariable) {
      // Find position of parameter in action. Pointer arithmetic
      int32_t idxActParam = effTerm - action->params;
      if (0 <= idxActParam && idxActParam < action->numOfParams) {
        // Variable points to parameter in action.
        if (newGrAct->terms[idxActParam] != NULL &&
            newGrAct->terms[idxActParam] != gapTerm) {
          // Value was already been set to another constant. That is a
          // problem. The predicate does not fix the gap.
          libpddl31_free_groundAction(newGrAct);
          newGrAct = NULL;
          break;
        }
        // Check Type of action parameter and predicate parameter.
        struct type *predArgType = atomToFix->pred->params[idxArgs].type;
        if ( ! typeSystem_isa(gapTerm->type, effTerm->type) ||
             ! typeSystem_isa(gapTerm->type, predArgType)) {
          // Types do not match. The whole predicate does not fix the gap.
          libpddl31_free_groundAction(newGrAct);
          newGrAct = NULL;
          break;
        }
        // Everything is okay. Do the mapping.
        newGrAct->terms[idxActParam] = gapTerm; // Actually set mapping.
      } else {
        // Variable refers to a forall-variable. Do not map. Just continue.
        // TODO: assert (effect != action->effect);
      }
    } else {
      // A constant in the actions effect. Deal with it.
      if (libpddl31_term_equal(effTerm, gapTerm)) {
        // Okay. Continue with next argument.
        continue;
      } else {
        // The whole predicate does not fix the gap.
        libpddl31_free_groundAction(newGrAct);
        newGrAct = NULL;
        break;
      }
    }
  }

  return newGrAct;
}

// We can not check the precondition of the when-effect. We just know
// that the precondition could be met. So just treat the when-effect
// as any other effect.
struct actionList *
utils_whenFixesGap( struct action *action,
                    struct when *when,
                    struct gap *gap)
{
  struct actionList *result = NULL;

  struct literal *gapLiteral = gap->literal;
  struct atom *gapAtom = gapLiteral->atom;

  if (gapLiteral->isPos) {
    for (int32_t idxWhen = 0; idxWhen < when->numOfPos; ++idxWhen) {
      struct atom *posAtom = &when->posLiterals[idxWhen];
      if (posAtom->pred == gapAtom->pred) {

        // Chance to fix gap.
        struct groundAction *newGrAct = utils_tryToFixGap(action,
                                                          posAtom,
                                                          gapAtom);
        result = utils_addActionToList(result, newGrAct);
      }
    }
  } else {
    assert ( ! gapLiteral->isPos);
    for (int32_t idxWhen = 0; idxWhen < when->numOfNeg; ++idxWhen) {
      struct atom *negAtom = &when->negLiterals[idxWhen];
      if (negAtom->pred == gapAtom->pred) {

        // Chance to fix gap.
        struct groundAction *newGrAct = utils_tryToFixGap(action,
                                                          negAtom,
                                                          gapAtom);
        result = utils_addActionToList(result, newGrAct);
      }
    }
  }

  return result;
}

struct actionList *
utils_actionFixesGap_aux( struct action *action,
                          // Any effect; also forall->effect.
                          struct effect *effect,
                          struct gap *gap)
{
  struct actionList *result = NULL;

  struct literal *gapLiteral = gap->literal;
  struct atom *gapAtom = gapLiteral->atom;

  for (int32_t idxEff = 0; idxEff < effect->numOfElems; ++idxEff) {
    struct effectElem *effE = &effect->elems[idxEff];

    switch (effE->type) {
    case POS_LITERAL: {

      struct atom *posAtom = effE->it.literal;

      if (gapLiteral->isPos &&
          posAtom->pred == gapAtom->pred) {

        // Chance to fix the gap.
        struct groundAction *newGrAct = utils_tryToFixGap(action,
                                                          posAtom,
                                                          gapAtom);
        result = utils_addActionToList(result, newGrAct);
      }
      break;
    }
    case NEG_LITERAL: {

      struct atom *negAtom = effE->it.literal;

      if ( ! gapLiteral->isPos &&
          negAtom->pred == gapAtom->pred) {

        // Chance to fix the gap.
        struct groundAction *newGrAct = utils_tryToFixGap(action,
                                                          negAtom,
                                                          gapAtom);
        result = utils_addActionToList(result, newGrAct);
      }
      break;
    }
    case FORALL: {

      struct effect *effect = effE->it.forall->effect;

      struct actionList *faList = utils_actionFixesGap_aux(action, effect, gap);

      // Adding forall-results to the front of the list.
      result = utils_concatActionLists(faList, result);
      break;
    }
    case WHEN: {

      struct when *when = effE->it.when;

      struct actionList *whenList = utils_whenFixesGap(action, when, gap);

      // Adding forall-results to the front of the list.
      result = utils_concatActionLists(whenList, result);
      break;
    }
    default: {
      assert(false);
      break;
    }
    } // End of switch statement.

  }

  return result;
}

struct actionList *
utils_actionFixesGap(struct action *action, struct gap *gap)
{
  return utils_actionFixesGap_aux(action, action->effect, gap);
}

void utils_print_groundAction(struct groundAction *grAct)
{
  if (grAct == NULL) {
    return;
  }
  struct action *action = grAct->action;
  printf("GroundAction:[Action-name:%s,", action->name);
  for (int32_t i = 0; i < action->numOfParams; ++i) {
    char *mapToName = grAct->terms[i] == NULL ? "NULL" : grAct->terms[i]->name;
    printf("Mapping:'%s'->'%s'", action->params[i].name,
                                 mapToName);
    if (i < action->numOfParams - 1) {
      printf(",");
    }
  }
  printf("]");
}

void
utils_print_groundActionCompact(struct groundAction *grAct)
{
  //printf("grAct:(%s", grAct->action->name);
  printf("(%s", grAct->action->name);
  for (int32_t i = 0; i < grAct->action->numOfParams; ++i) {
    char *termName = grAct->terms[i] == NULL ? "NULL" : grAct->terms[i]->name;
    printf(" %s", termName);
  }
  printf(")");
}

void
utils_print_actionListCompact(struct actionList *list)
{
  if (list == NULL) {
    return;
  }

  printf("ActionList:(");
  while (list != NULL) {
    utils_print_groundActionCompact(list->act);
    //printf("w%dp%d", list->weight, list->pos); // print weights
    if (list->next != NULL) {
      printf(" ");
    }
    list = list->next;
  }
  printf(")");
}

void utils_print_actionList(struct actionList *list)
{
  printf("ActionList:[");
  while (list != NULL) {
    utils_print_groundAction(list->act);
    if (list->next != NULL) {
      printf(", ");
    }
    list = list->next;
  }
  printf("]");
}

// For debug purposes only.
int32_t utils_actionList_length(struct actionList *list)
{
  int32_t length = 0;
  while (list != NULL) {
    list = list->next;
    length++;
  }
  return length;
}

struct groundAction *
utils_cloneGroundAction(struct groundAction *src)
{
  struct groundAction *dest = malloc(sizeof(*dest));
  dest->action = src->action;
  int32_t size = sizeof(*dest->terms) * dest->action->numOfParams;
  dest->terms = malloc(size);
  memcpy(dest->terms, src->terms, size);
  return dest;
}

struct actionList *
utils_groundAction_aux(struct problem *problem,
                       struct groundAction *parGrAct,
                       int32_t idxActArg)
{
  struct action *action = parGrAct->action;

  // Compare greater or equal, because indices start from zero.
  if (idxActArg >= action->numOfParams) {
    // Return single element list of full grounding.
    struct actionList *singleton = malloc(sizeof(*singleton));
    singleton->act = utils_cloneGroundAction(parGrAct);
    singleton->next = NULL;
    return singleton;
  }

  if (parGrAct->terms[idxActArg] != NULL) {
    // A mapping for this parameter already exists. Simply proceed with next
    // parameter
    return utils_groundAction_aux(problem, parGrAct, idxActArg + 1);
  }

  struct actionList *result = NULL;

  struct objManag *objManag = problem->objManag;

  // for each object in object manager:
  //  add mapping parGrAct->terms[idxActArg] to object
  //  localresult = utils_groundAction_aux(problem, parGrAct, idxActArg + 1)
  //  result = utils_concatActionLists(localResult, result);
  for (int32_t idxObj = 0; idxObj < objManag->numOfObjs; ++idxObj) {
    struct term *consObj = objManag->objs[idxObj];

    // Check type of action parameter.
    // TODO: Check: I do not think that it makes sense, that we check the
    // type of the predicate parameters, cause we would have to iterate over
    // the whole actions precondition and effects, to find all the predicates
    // where this action-parameter is used.
    // TODO: Or maybe, yes, it makes sense.
    // Maybe do something when parsing the input.
    if ( ! typeSystem_isa(consObj->type, action->params[idxActArg].type)) {
      // Types do not match. Do not map from parameter to this constant. Just
      // continue with next object.
      continue;
    }

    parGrAct->terms[idxActArg] = consObj;
    struct actionList *subResult = utils_groundAction_aux(problem,
                                                          parGrAct,
                                                          idxActArg + 1);
    // I think we don't have to remove here. We could jJust overwrite in next
    // iteration. I still do it for now, in order to not change the input
    // arguments.
    parGrAct->terms[idxActArg] = NULL;

    // Collect the results
    result = utils_concatActionLists(subResult, result);
  }

  return result;
}

struct actionList *
utils_groundActions(struct problem *problem,
                    struct actionList *partialGroundedList)
{
  struct actionList *result = NULL;

  // Iterate over the partial grounded actions
  for (struct actionList *parGrActE = partialGroundedList;
       parGrActE != NULL;
       parGrActE = parGrActE->next) {

    struct groundAction *parGrAction = parGrActE->act;
    struct actionList *fullyGrActs = utils_groundAction_aux(problem,
                                                            parGrAction,
                                                            0);
    result = utils_concatActionLists(fullyGrActs, result);
  }

  return result;
}

bool
utils_grAct_equal(struct groundAction *ga1, struct groundAction *ga2)
{
  if (ga1 == ga2) { // Includes (ga1 == NULL && ga2 == NULL)
    return true;
  }
  if (ga1 == NULL || ga2 == NULL) {
    return false;
  }

  //printf("\n### a1: %s, a2: %s\n\n\n", ga1->action->name, ga2->action->name); // DE BUG
  //fflush(stdout); // DEBUG

  if (ga1->action != ga2->action) {
      return false;
  }
  assert (ga1->action->numOfParams == ga2->action->numOfParams);
  for (int idx = 0; idx < ga1->action->numOfParams; ++idx) {
    if ( ! libpddl31_term_equal(ga1->terms[idx], ga2->terms[idx])) {
      return false;
    }
  }
  return true;
}

bool
utils_atom_equal(struct atom *a1, struct atom *a2)
{
  if (a1 == NULL && a2 == NULL) {
    return true;
  }
  if (a1 == NULL || a2 == NULL) {
    return false;
  }

  if (a1->pred != a2->pred) {
    return false;
  }

  for (int32_t idxArgs = 0; idxArgs < a1->pred->numOfParams; ++idxArgs) {
    if ( ! libpddl31_term_equal(a1->terms[idxArgs], a2->terms[idxArgs])) {
      return false;
    }
  }

  return true;
}

// Atom 'a2' has to be affiliated with the ground action 'grAct'.
bool
utils_atom_equalWithGrounding(struct atom *a1,
                              struct atom *a2,
                              struct groundAction *grAct)
{
  if (a1 == NULL && a2 == NULL) {
    return true;
  }
  if (a1 == NULL || a2 == NULL) {
    return false;
  }

  if (a1->pred != a2->pred) {
    return false;
  }
  //printf("\n\npred-name: %s\n", a2->pred->name);

  for (int32_t idxArgs = 0; idxArgs < a1->pred->numOfParams; ++idxArgs) {
    // Pointer arithmetic.
    int32_t idxGrounding = a2->terms[idxArgs] - grAct->action->params;

    if (0 <= idxGrounding && idxGrounding < grAct->action->numOfParams) {
      if ( ! libpddl31_term_equal(a1->terms[idxArgs], grAct->terms[idxGrounding])) {
        return false;
      }
    } else {
      // This is a constant. Just compare it.
      assert ( ! a2->terms[idxArgs]->isVariable);
      if ( ! libpddl31_term_equal(a1->terms[idxArgs], a2->terms[idxArgs])) {
        return false;
      }
    }
  }

  return true;
}

void
utils_print_gap(struct gap *gap)
{
  printf("Gap:[position:%d,", gap->position);
  utils_print_literal(gap->literal);
  printf("]");
}

void
utils_print_literal(struct literal *literal)
{
  if ( ! literal->isPos) {
    printf("NOT ");
  }
  libpddl31_atom_print(literal->atom);
}

struct atom *utils_atom_clone(struct atom *atom)
{
  struct atom *result = malloc(sizeof(*result));
  result->pred = atom->pred;
  int32_t size = sizeof(*result->terms) * result->pred->numOfParams;
  result->terms = malloc(size);
  memcpy(result->terms, atom->terms, size);
  return result;
}

// 'atom' needs to be an atom of the ground action.
struct atom *
utils_atom_cloneWithGrounding(struct atom *atom,
                              struct groundAction *grAct)
{
  struct atom *result = malloc(sizeof(*result));
  result->pred = atom->pred;
  int32_t size = sizeof(*result->terms) * result->pred->numOfParams;
  result->terms = malloc(size);
  // Copy ground terms into atom.
  for (int32_t idxArgs = 0; idxArgs < atom->pred->numOfParams; ++idxArgs) {
    // Pointer arithmetic
    int32_t idxGrounding = atom->terms[idxArgs] - grAct->action->params;

    //libpddl31_term_print(atom->terms[idxArgs]); // DEBUG
    //printf("\n"); // DEBUG

    if (0 <= idxGrounding && idxGrounding < grAct->action->numOfParams) {
      result->terms[idxArgs] = grAct->terms[idxGrounding];
    } else {
      // If this term is a constant, the index will be out of the actions'
      // parameter array. Then we just use the pointer to the constant
      // itself.
      result->terms[idxArgs] = atom->terms[idxArgs];
    }
  }
  return result;
}

void
utils_free_gap(struct gap *gap)
{
  if (gap != NULL) {
    free(gap->literal->atom->terms);
    free(gap->literal->atom);
    free(gap->literal);
    free(gap);
  }
}

struct actionList *
utils_cloneActionListShallow(struct actionList *actL)
{
  struct actionList *actLBack = actL;

  struct actionList *head = NULL;
  struct actionList *curr = NULL;
  struct actionList *prev = NULL;
  for (/* empty */; actL != NULL; actL = actL->next) {
    prev = curr;
    curr = malloc(sizeof(*curr));
    if (prev == NULL) {
      // First element of the list. Save pointer to the head.
      head = curr;
    } else {
      // Set the next-pointer of the previous element.
      prev->next = curr;
    }
    // Set the ground action.
    curr->act = actL->act;

    curr->weight = actL->weight;
    curr->pos= actL->pos;
  }

  // Set next->pointer on last element to NULL
  if (curr != NULL) {
    curr->next = NULL;
  } else {
    // Note: The list to clone is empty.
    assert (utils_actionList_length(actLBack) == 0);
  }

  return head;
}

struct actionList *
utils_cloneActionList(struct actionList *actL)
{
  struct actionList *actLBack = actL;

  struct actionList *head = NULL;
  struct actionList *curr = NULL;
  struct actionList *prev = NULL;
  for (/* empty */; actL != NULL; actL = actL->next) {
    prev = curr;
    curr = malloc(sizeof(*curr));
    if (prev == NULL) {
      // First element of the list. Save pointer to the head.
      head = curr;
    } else {
      // Set the next-pointer of the previous element.
      prev->next = curr;
    }
    // Clone and set the ground action.
    curr->act = utils_cloneGroundAction(actL->act);
  }

  // Set next->pointer on last element to NULL
  if (curr != NULL) {
    curr->next = NULL;
  } else {
    // Note: The list to clone is empty.
    assert (utils_actionList_length(actLBack) == 0);
  }

  return head;
}

// @pos: If zero, removes first element.
struct actionList *
utils_removeActionFromListAtPosition(struct actionList *al, int32_t pos)
{
  struct actionList *head = al;

  if (pos == 0) {
    // Removing first element of list.
    if (al != NULL) {
      head = al->next;
      free(al);
    }
    return head;
  }

  int32_t idx = 0;
  struct actionList *before = NULL;
  while (al != NULL && idx < pos) {
    before = al;
    al = al->next;
    idx++;
  }

  if (al == NULL) {
    return head;
  }

  assert (before != NULL);
  before->next = al->next;
  free(al);
  return head;
}

void
utils_actionList_setPosition(struct actionList *al, int32_t position)
{
  while (al != NULL) {
    al->pos = position;
    al = al->next;
  }
}

struct actionList *
utils_actionList_reverse(struct actionList *list)
{
  struct actionList *newHead = NULL;

  while (list != NULL) {
    struct actionList *listNextOld = list->next;
    list->next = newHead;
    newHead = list;

    list = listNextOld;
  }

  return newHead;
}

bool
utils_actionListsEqual(struct actionList *n1, struct actionList *n2)
{
  if (n1 == NULL && n2 == NULL) {
    return true;
  }
  if (n1 == NULL || n2 == NULL) {
    return false;
  }

  while (n1 != NULL && n2 != NULL) {
    struct groundAction *ga1 = n1->act;
    struct groundAction *ga2 = n2->act;
    if ( ! utils_grAct_equal(ga1, ga2)) {
      return false;
    }
    n1 = n1->next;
    n2 = n2->next;
  }
  if (n1 == NULL && n2 == NULL) {
    return true;
  }
  return false;
}
