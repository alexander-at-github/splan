#include <assert.h>

#include "probSpace.h"
#include "trie.h"



typedef struct actionList * aStarNode_t;

typedef struct aStarNodeList {
  aStarNode_t node;
  struct aStarNodeList *next;
  // prev? struct aStarNodeList *prev;
  int32_t fScore;
} * aStarNodeList_t;




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

static
aStarNodeList_t
asnl_createElem(aStarNode_t aStarNode)
{
  aStarNodeList_t asnle = malloc(sizeof(*asnle));
  asnle->node = aStarNode;
  asnle->fScore = -1;
  return asnle;
}

static
aStarNodeList_t
asnl_removeFirst(aStarNodeList_t *asnl)
{
  if (asnl == NULL) {
    // Should be correct -right?
    return NULL;
  }

  aStarNodeList_t first = asnl;
  aStarNodeList_t second = asnl->next;

  // TODO: Deal with first element. Either free it or reuse it!
  // For now just unset next pointer.
  first->next = NULL;

  return second;
}

static
int32_t
asnl_indexOf(aStarNodeList_t list, aStarNode_t node)
{
  // TODO
  if contains:
    return index;

  return -1;
}

// TODO: Maybe change to return pointer to the element.
static
int32_t
asnl_contains(aStarNodeList_t list, aStarNode_t node)
{
  return asnl_indexOf(list, node);
}

static
aStarNodeList_t
asnl_push(aStarNodeList_t list, aStarNodeList_t elem)
{
  elem->next = list;
  return elem;
}

// A comparison function for finding locations in lists for inserting
// elements in an ordered manner by their intValue property.
static
int
compIntValueFun(list_t e1, list_t another e2)
{
  if (e1 == NULL || e2 == NULL) {
    assert(false);
  }

  if (e1->intValue > e2->intValue) {
    return -1;
  }
  assert(e1->intValue <= e2->intValue);

  if (e1->next == NULL) {
    return 0;
  }
  if (e1->next->intValue > e2->intValue) {
    return 0;
  }

  return 1;
}

static
aStarNodeList_t
asnl_insertOrdered(aStarNodeList_t list, aStarNodeList_t elem)
{
  if (elem == NULL) {
    return;
  }
  // If list == NULL then it represents the empty list.

  /* aStarNodeList_t head = list; */

  /* aStarNodeList_t beforeCurr = NULL; */
  /* aStarNodeList_t curr = list; */
  /* while (curr != NULL && curr->fScore <= elem->fScore) { */
  /*   beforeCurr = curr; */
  /*   curr = curr->next; */
  /* } */
  /* assert(curr == NULL || curr->fScore > elem->fScore); */

  /* elem->next = curr; */
  /* if (beforeCurr == NULL) { */
  /*   head = elem; */
  /* } else { */
  /*   beforeCurr->next = elem; */
  /* } */

  /* return head; */

  list_t found = list_find(list, &compIntValueFun, elem)
  // The new element should now be inserted right after "found".
  if (found == NULL) {
    // Insert new element as head.
    elem->next = list;
    if (list != NULL) {
      list->prev = elem;
    }
    return elem;
  }

  if (found->next != NULL) {
    found->next->prev = elem;
  }
  elem->next = found->next;
  found->next = elem;
  elem->prev = found;

  return list;
}



struct actionList *
aStarPlanner_aStar(struct probSpace *probSpace)
{
  aStarNode_t aStarNode = NULL;    // The empty list of actions.
  aStarNodeList_t frontier = asnl_createElem(aStarNode);
  aStarNodeList_t explored = NULL;    // An empty set.

  while(frontier != NULL) {
    aStarNodeList_t currNLE;
    aStarNode_t currN;
    /* Pseudo-Code: currN = pop(frontier) */
    currNLE = frontier
    currN = currNLE->node;
    frontier = asnl_removeFirst(frontier);

    gaps = aStarPlanner_getAllGaps(probSpace, currN);
    if (gaps == NULL) {
      // Note: typedef struct actionList * aStarNode_t;
      aStarNode_t solution = utils_cloneActionList(currN);
      return solution;
    }

    explored = asnl_push(explored, currNLE);

    actions = get(gaps);
    for action in actions {
      aStarNode_t chld = utils_cloneActionListShallow(currN);
      chld = utils_addActionToListAtPosition(chld, gap, gap->position);

      /* If explored contains chld. */
      if (asnl_contains(explored, chld) != -1) {
        utils_free_actionListShallow(chld);
        continue;
      }
      /* If frontier contains chld. */
      int32_t idxFrontier = asnl_contains(frontier, chld);
      if (idxFrontier != -1) {
        // TODO: If new score is lower, then replace node in frontier.
        continue;
      }

      /* Add node to frontier. */
      frontier = asnl_insertOrdered(frontier, chld);
    }

  }

  // No solution.
  return NULL;
}

typedef struct gapList {
  struct gap *gap;
  struct gapList *next;
} * gapList_t;

static
gapList_t
aStarPlanner_getAllGaps(struct probSpace *probSpace, aStarNode_t actions)
{
  /* Note: typedef struct actionList * aStarNode_t; */

  trie_t state = trie_clone(probSpace->problem->init);
  struct goal *goal = probSpace->problem->goal;

  gapList_t result = NULL;
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
      struct atom *atom = &precond->posLiteral[idx];
      if ( ! trie_containsGr(state, atom, grAct)) {
        // TODO: Positive literal not fulfilled.
      }
    }
    for (int32_t idx = 0; idx < precond->numOfNeg; ++idx) {
      struct atom *atom = &precond->negLiteral[idx];
      if (trie_containsGr(state, atom, grAct)) {
        // TODO: Negative literal not fulfilled.
      }
    }
    aStarPlanner_apply(state, grAct);
    }
  }

  /* Check if the goal is met after applying all the actions. */

  for (int32_t idx = 0; idx < goal->numOfPos; ++idx) {
    struct atom *atom = &goal->posLiteral[idx];
    if ( ! trie_containsGr(state, atom, grAct)) {
      // TODO: Positive literal not fulfilled.
    }
  }
  for (int32_t idx = 0; idx < precond->numOfNeg; ++idx) {
    struct atom *atom = &goal->negLiteral[idx];
    if (trie_containsGr(state, atom, grAct)) {
      // TODO: Negative literal not fulfilled.
    }
  }

  return result;
}
