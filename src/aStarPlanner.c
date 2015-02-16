#include <assert.h>

#include "aStarPlanner.h"
#include "list.h"
#include "probSpace.h"
#include "trie.h"
#include "utils.h"


// Here we use the old implementation of actionList, since there are many 
// useful function in utils.c to be used.
typedef struct actionList * aStarNode_t;

static
void
aStarPlanner_applyEffElemPos(trie_t trie,
                struct groundAction *grAct,
                struct effectElem *effElem)
{
  assert (effElem != NULL);
  if (effElem->type != POS_LITERAL) {
    return;
  }
  trie_addGr(trie, effElem->it.literal, grAct);
}

static
void
aStarPlanner_applyEffElemNeg(trie_t trie,
                struct groundAction *grAct,
                struct effectElem *effElem)
{
  assert (effElem != NULL);
  if (effElem->type != NEG_LITERAL) {
    return;
  }
  trie_addGr(trie, effElem->it.literal, grAct);
}

typedef void (*applyEffElemFun_t)(trie_t,
                                struct groundAction *,
                                struct effectElem *);

static
void
aStarPlanner_applyF(trie_t trie,
               struct groundAction *grAct,
               applyEffElemFun_t applyEffElemFun)
{
  assert (trie != NULL);
  assert (grAct != NULL);

  struct effect *effCurr = grAct->action->effect;

  for (int32_t idx = 0; idx < effCurr->numOfElems; ++idx) {
    struct effectElem *effElem = &effCurr->elems[idx];
    (*applyEffElemFun)(trie, grAct, effElem);
  }
}

static
void
aStarPlanner_applyPos(trie_t trie, struct groundAction *grAct)
{
  aStarPlanner_applyF(trie, grAct, &aStarPlanner_applyEffElemPos);
}

static
void
aStarPlanner_applyNeg(trie_t trie, struct groundAction *grAct)
{
  aStarPlanner_applyF(trie, grAct, &aStarPlanner_applyEffElemNeg);
}

static
void
aStarPlanner_apply(trie_t trie, struct groundAction *grAct)
{
  aStarPlanner_applyPos(trie, grAct);
  aStarPlanner_applyNeg(trie, grAct);
}

// A comparison function for finding locations in lists for inserting
// elements in an ordered manner by their intValue property.
static
int
compIntValueFun(list_t e1, void *fScore_)
{
  if (e1 == NULL || fScore_ == NULL) {
    assert(false);
  }
  int fScore = *(int *) fScore_;

  if (e1->intValue > fScore) {
    return -1;
  }
  assert(e1->intValue <= fScore);

  if (e1->next == NULL) {
    return 0;
  }
  if (e1->next->intValue > fScore) {
    return 0;
  }

  return 1;
}

static
bool
aStarPlanner_aStarNodeEqual(aStarNode_t n1, aStarNode_t n2)
{
  // Note: typedef struct actionList * aStarNode_t;
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


// A comparison function for finding a sequence of ground actions in a list
// of sequences of ground actions.
static
int
compActList(list_t e1, void *e2)
{
  if (e1 == NULL || e2 == NULL) {
    assert(false);
  }
  aStarNode_t n1 = (aStarNode_t) e1->payload;
  aStarNode_t n2 = (aStarNode_t) e2;

  if ( ! aStarPlanner_aStarNodeEqual(n1, n2)) {
    return 1;
  }

  // Both action lists equal.
  return 0;
}

// Arguments are a list of aStarNode_t and a aStarNode_t in a payload list.
static
list_t
asnl_find(list_t list, void *payload)
{
  return list_find(list, &compActList, payload);
}

static
list_t
asnl_insertOrdered(list_t list, void *payload, int fScore)
{
  if (payload == NULL) {
    return list;
  }
  // If list == NULL then it represents the empty list.

  /* aStarNodeList_t head = list; */

  /* aStarNodeList_t beforeCurr = NULL; */
  /* aStarNodeList_t curr = list; */
  /* while (curr != NULL && curr->fScore <= payload->fScore) { */
  /*   beforeCurr = curr; */
  /*   curr = curr->next; */
  /* } */
  /* assert(curr == NULL || curr->fScore > payload->fScore); */

  /* payload->next = curr; */
  /* if (beforeCurr == NULL) { */
  /*   head = payload; */
  /* } else { */
  /*   beforeCurr->next = payload; */
  /* } */

  /* return head; */

  list_t singleton = list_createElem(payload);
  singleton->intValue = fScore;

  list_t found = list_find(list, &compIntValueFun, &fScore);
  // The new element should now be inserted right after "found".
  if (found == NULL) {
    // Insert new element as head.
    singleton->next = list;
    if (list != NULL) {
      list->prev = singleton;
    }
    return singleton;
  }

  if (found->next != NULL) {
    found->next->prev = singleton;
  }
  singleton->next = found->next;
  found->next = singleton;
  singleton->prev = found;

  return list;
}

// Note: typedef void (*freePayload)(void *);
void
freeGap(void *gap)
{
  utils_free_gap((struct gap *) gap);
}

static
int
aStarPlanner_calcFScore(aStarNode_t node)
{
  // TODO TODO TODO
  return 0;
}

static
struct actionList *
aStarPlanner_getActions(struct probSpace *probSpace, list_t gaps)
{
  struct actionList *result = NULL;

  while (gaps != NULL) {
    struct gap *gap = (struct gap *) list_getFirstPayload(gaps);
    gaps = list_removeFirst(gaps);

    struct actionList *actsTFGap = ps_getActsToFixGap(probSpace, gap->literal);
    result = utils_concatActionLists(result, actsTFGap);
  }
  return result;
}

static
list_t
aStarPlanner_getAllGaps(struct probSpace *probSpace, aStarNode_t actions)
{
  /* Note: typedef struct actionList * aStarNode_t; */

  trie_t state = trie_clone(probSpace->problem->init);
  struct goal *goal = probSpace->problem->goal;

  list_t result = NULL;
  struct literal *gapLiteral = NULL;

  /* Check action's precondition and apply the action. */

  // Index for position of gap. Starts with one, because the gap is sayed
  // to be before the index.
  int32_t idxAct = 1;

  for (struct actionList *pAct = actions;
       pAct != NULL;
       pAct = pAct->next, ++idxAct) {

    struct groundAction *grAct = pAct->act;
    // Check if precondition holds.
    struct goal *precond = grAct->action->precond;

    for (int32_t idx = 0; idx < precond->numOfPos; ++idx) {
      struct atom *atom = &precond->posLiterals[idx];
      if ( ! trie_containsGr(state, atom, grAct)) {
        // Positive literal not fulfilled.
        struct gap *gap = malloc(sizeof(*gap));
        gap->position = idxAct;
        gap->literal = malloc(sizeof(*gap->literal));
        gap->literal->atom = utils_atom_cloneWithGrounding(atom, grAct);
        gap->literal->isPos = true;
        result = list_push(result, list_createElem(gap));
      }
    }
    for (int32_t idx = 0; idx < precond->numOfNeg; ++idx) {
      struct atom *atom = &precond->negLiterals[idx];
      if (trie_containsGr(state, atom, grAct)) {
        // Negative literal not fulfilled.
        struct gap *gap = malloc(sizeof(*gap));
        gap->position = idxAct;
        gap->literal = malloc(sizeof(*gap->literal));
        gap->literal->atom = utils_atom_cloneWithGrounding(atom, grAct);
        gap->literal->isPos = false;
        result = list_push(result, list_createElem(gap));
      }
    }
    aStarPlanner_apply(state, grAct);
  }

  /* Check if the goal is met after applying all the actions. */

  for (int32_t idx = 0; idx < goal->numOfPos; ++idx) {
    struct atom *atom = &goal->posLiterals[idx];
    if ( ! trie_contains(state, atom)) {
      // Positive literal not fulfilled.
      struct gap *gap = malloc(sizeof(*gap));
      gap->position = idxAct;
      gap->literal = malloc(sizeof(*gap->literal));
      gap->literal->atom = utils_atom_clone(atom);
      gap->literal->isPos = true;
      result = list_push(result, list_createElem(gap));
    }
  }
  for (int32_t idx = 0; idx < goal->numOfNeg; ++idx) {
    struct atom *atom = &goal->negLiterals[idx];
    if (trie_contains(state, atom)) {
      // Negative literal not fulfilled.
      struct gap *gap = malloc(sizeof(*gap));
      gap->position = idxAct;
      gap->literal = malloc(sizeof(*gap->literal));
      gap->literal->atom = utils_atom_clone(atom);
      gap->literal->isPos = false;
      result = list_push(result, list_createElem(gap));
    }
  }

  trie_free(state);

  return result;
}

struct actionList *
aStarPlanner_aStar(struct probSpace *probSpace)
{
  aStarNode_t aStarNode = NULL;    // The empty list of actions.
  list_t frontier = list_createElem(aStarNode);
  list_t explored = NULL;    // An empty set.

  while( ! list_isEmpty(frontier)) {
    list_t currNLE;
    aStarNode_t currN;
    /* Pseudo-Code: currN = pop(frontier) */
    currNLE = frontier;
    currN = (aStarNode_t) currNLE->payload;
    frontier = list_removeFirst(frontier);

    utils_print_actionListCompact(currN); // DEBUG
    printf("\n"); // DEBUG

    list_t gaps = aStarPlanner_getAllGaps(probSpace, currN);
    if (list_isEmpty(gaps)) {
      // Note: typedef struct actionList * aStarNode_t;
      aStarNode_t solution = utils_cloneActionList(currN);
      return solution;
    }

    explored = list_push(explored, currNLE);

    //list_t actions = aStarPlanner_getActions(gaps);

    for (list_t gapE = gaps;
         gaps != NULL;
         gaps = gaps->next) {

      struct gap *gap = (struct gap *) gaps->payload;
      //utils_print_gap(gap); // DEBUG
      //printf("\n"); // DEBUG
      struct actionList *actions = ps_getActsToFixGap(probSpace, gap->literal);
      //utils_print_actionListCompact(actions); // DEBUG
      //printf("\n"); // DEBUG

      for (struct actionList *currAct = actions;
           currAct != NULL;
           currAct = currAct->next) {

        struct groundAction *grAct = currAct->act;
        for (int idxPos = 0;
             idxPos < gap->position; // Note: gap->position > 0
             idxPos++) {

          aStarNode_t chld = utils_cloneActionListShallow(currN);
          //printf("chld: "); // DEBUG
          //utils_print_actionListCompact(chld); // DEBUG
          //printf("\n"); // DEBUG
          chld = utils_addActionToListAtPosition(chld, grAct, idxPos);
          //printf("chld: "); // DEBUG
          //utils_print_actionListCompact(chld); // DEBUG
          //printf("\n"); // DEBUG

          /* If explored contains chld. */
          if (asnl_find(explored, chld) != NULL) {
            utils_free_actionListShallow(chld);
            continue;
          }
          // Calculate f-score
          int fScore = aStarPlanner_calcFScore(chld);

          /* If frontier contains chld. */
          list_t frontierElem = asnl_find(frontier, chld);
          if (frontierElem != NULL) {
            // If new score is lower, then replace node in frontier, otherwise
            // free chld.
            if (frontierElem->intValue > fScore) {
              aStarNode_t frontierElemPayload =
                                          (aStarNode_t) frontierElem->payload;
              frontier = list_remove(frontier, frontierElem);
              frontier = asnl_insertOrdered(frontier, chld, fScore);
              utils_free_actionListShallow(frontierElemPayload);
            } else {
              utils_free_actionListShallow(chld);
            }
            continue;
          }

          /* Add node to frontier. */
          frontier = asnl_insertOrdered(frontier, chld, fScore);
        }
      }
    }

    list_freeWithPayload(gaps, &freeGap);
  }

  // No solution exists.
  return NULL;
}

struct actionList *
aStarPlanner(struct problem *problem)
{
  struct probSpace *probSpace = ps_init(problem);
  struct actionList *solution = aStarPlanner_aStar(probSpace);
  ps_free(probSpace);
  trie_cleanupSNBuffer();
  return solution;
}
