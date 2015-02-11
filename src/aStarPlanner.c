#include <assert.h>

#include "probSpace.h"
#include "trie.h"


/*** Under Construction. ***/

static
void
planner_applyEffElemPos(trie_t trie,
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
planner_applyEffElemNeg(trie_t trie,
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
planner_applyF(trie_t trie,
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
planner_applyPos(trie_t trie, struct groundAction *grAct)
{
  planner_applyF(trie, grAct, &planner_applyEffElemPos);
}

static
void
planner_applyNeg(trie_t trie, struct groundAction *grAct)
{
  planner_applyF(trie, grAct, &planner_applyEffElemNeg);
}

typedef struct actionList * aStarNode_t;

typedef struct aStarNodeList {
  aStarNode_t node;
  struct aStarNodeList *next;
  // prev? struct aStarNodeList *prev;
} * aStarNodeList_t;

static
aStarNodeList_t
asnl_createElem(aStarNode_t aStarNode)
{
  aStarNodeList_t asnle = malloc(sizeof(*asnle));
  asnle->node = aStarNode;
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

struct actionList *
planner_aStar(struct probSpace *probSpace)
{
  aStarNode_t aStarNode = NULL;    // The empty list of actions.
  aStarNodeList_t frontier = asnl_createElem(aStarNode);
  aStarNodeList_t explored = NULL;    // An empty set.

  while(frontier != NULL) {
    aStarNode_t currN;
    /* currN = pop(frontier) */
    currN = frontier->node;
    frontier = asnl_removeFirst(frontier);

  }

  // No solution.
  return NULL;
}
