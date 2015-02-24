#include <assert.h>
#include <stdbool.h>
#include <stdlib.h>

#include "asnlist.h"

static
asnTree_t
asnTreeCreateEmpty_aux(enum treeNodeType type)
{
  asnTree_t treeNode = malloc(sizeof(*treeNode));
  treeNode->numOfChldrn = 0;
  treeNode->numAlloced = 0;
  treeNode->chldrn = NULL;
  treeNode->type = type;
  treeNode->pointerIntoList = NULL;
  return treeNode;
}

asnList_t
asnList_createEmpty()
{
  asnList_t asnl = malloc(sizeof(*asnl));
  asnl->list = NULL;
  asnl->tree = asnTreeCreateEmpty_aux(ACTION);
  return asnl;
}

bool
asnList_isEmpty(asnList_t asnl)
{
  return asnl->list == NULL;
}

static
asnTree_t
asnTreeGetOrCreate_aux(enum treeNodeType type)
{
  // TODO: reuse?
  return asnTreeCreateEmpty_aux(type);
}

static
void
printActionList(void *al)
{
  utils_print_actionListCompact((struct actionList *) al);
}

static
int
asnTreeBinSearch_aux(asnTree_t treeNode, void *pointer)
{
  //struct treeNodeAE *po = NULL;
  int idx = 0;
  int lowR = 0;
  int highR = treeNode->numOfChldrn;
  while (true) {
    // +1 to "round up" at the division by two.
    idx = (lowR + highR + 1) / 2;  //  = lowR + (highR - lowR + 1) / 2;
    if (lowR == highR) {
      // index found.
      break;
    }
    assert (idx > 0);
    int idxMinusOne = idx - 1;
    if (treeNode->chldrn[idxMinusOne].edgeLabel >= pointer) {
      // Move to the left.
      highR = idxMinusOne;
      continue;
    }
    if (idx == highR) {
      // Moving to the left is not neccessary, cause we checked that already
      // in the previous if-statemenet.
      // Since idx points to the right most element, which is not initialized
      // (comparing it would cause an error, too), there is no need to move to
      // the right either.
      // That is, we are done. Position / index found.
      break;
    }
    if (treeNode->chldrn[idx].edgeLabel < pointer) {
      // Move to the right.
      lowR = idx + 1;
      continue;
    }
    // Position found.
    break;
  }
  assert(0 <= idx && idx <= treeNode->numOfChldrn);
  assert(idx == 0 || treeNode->chldrn[idx - 1].edgeLabel < pointer);
  assert(idx == treeNode->numOfChldrn ||
         treeNode->chldrn[idx].edgeLabel >= pointer);
  return idx;
}

static
int
calcBufferSize(int32_t oldSize)
{
  int initialBufferSize = 32;
  int growthFactor = 2;
  int newSize = (oldSize < (initialBufferSize / growthFactor)) ?
          initialBufferSize :
          (growthFactor * oldSize);
  return newSize;
}

// This function edits the tree node in place. It moves the children array
// of the tree node and thus invalidates pointer into this array.
static
void
asnTreeNodeGrowArray_aux(asnTree_t treeNode)
{
  treeNode->numAlloced = calcBufferSize(treeNode->numAlloced);
  treeNode->chldrn = realloc(treeNode->chldrn,
                             sizeof(*treeNode->chldrn) * treeNode->numAlloced);
}

typedef bool (*compFun_t)(void *, void *);

static
bool
actionCompFun(void *a1, void *a2)
{
  return a1 == a2;
}

static
bool
termCompFun(void *t1, void *t2)
{
  return libpddl31_term_equal((struct term *) t1, (struct term *) t2);
}

static
asnTree_t
asnTreeFindOrAdd_aux(asnTree_t treeNode,
                     void *pointer,
                     enum treeNodeType type,
                     compFun_t compFun)
{
  assert(treeNode != NULL);
  int aeIdx = asnTreeBinSearch_aux(treeNode, pointer);
  assert (0 <= aeIdx && aeIdx <= treeNode->numOfChldrn);

  // Set the type here.
  if (treeNode->type == UNSET) {
    assert(treeNode->numOfChldrn == 0);
    treeNode->type = type;
  }

  if (aeIdx == treeNode->numOfChldrn) {
    // the variable "ae" points to one element after the array. We will insert
    // it there. Do not dereference the pointer yet.
  } else if ((*compFun)(treeNode->chldrn[aeIdx].edgeLabel, pointer)) {
    // Found
    assert(treeNode->type == type);
    assert(treeNode->chldrn[aeIdx].chld != NULL);
    return treeNode->chldrn[aeIdx].chld;
  }

  treeNode->numOfChldrn++;
  if (treeNode->numOfChldrn > treeNode->numAlloced) {
    asnTreeNodeGrowArray_aux(treeNode);
  }

  // Move elements to the right.
  int nelems = treeNode->numOfChldrn - aeIdx - 1;
  memmove(&treeNode->chldrn[aeIdx + 1], &treeNode->chldrn[aeIdx],
          sizeof(*treeNode->chldrn) * nelems);

  // Write new element in its place.
  treeNode->chldrn[aeIdx].edgeLabel = pointer;
  // I do not know the type of the node yet. The type will be set when
  // asnTreeFindOrAdd_aux() will be called on the chld node. The code to do that
  // is at the beginning of this function.
  treeNode->chldrn[aeIdx].chld = asnTreeGetOrCreate_aux(UNSET);

  assert(treeNode->chldrn[aeIdx].chld != NULL);
  return treeNode->chldrn[aeIdx].chld;
}

static
asnTree_t
asnTreeInsert_aux(asnTree_t tree, list_t singletonList)
{
  aStarNode_t asn = singletonList->payload;


  //printf("asnTreeInsert_aux() ");
  //utils_print_actionListCompact(asn);
  //printf("\n");

  asnTree_t currTreeNode = tree;
  // Note: typedef struct acionList * aStarNode_t;
  for (/* empty */; asn != NULL; asn = asn->next) {
    struct groundAction *grAct = asn->act;
    struct action *act = grAct->action;

    //utils_print_actionListCompact(asn);
    //printf("\n");
    //printf("currTreeNode->type: %d\n", currTreeNode->type);

    // TODO: This MUST be uncommented. Just for debugging purpose it is
    // commented out.
    assert(currTreeNode->type == ACTION ||
           currTreeNode->type == UNSET);

    currTreeNode = asnTreeFindOrAdd_aux(currTreeNode,
                                        act,
                                        ACTION,
                                        &actionCompFun);

    for (int idxTerm = 0; idxTerm < act->numOfParams; ++idxTerm) {
      //printf("currTreeNode->type: %d\n", currTreeNode->type);
      assert(currTreeNode->type == TERM || currTreeNode->type == UNSET);

      currTreeNode = asnTreeFindOrAdd_aux(currTreeNode,
                                          grAct->terms[idxTerm],
                                          TERM,
                                          &termCompFun);
    }
  }
  currTreeNode->pointerIntoList = singletonList;
  return tree;
}

asnList_t
asnList_push(asnList_t asnl, aStarNode_t asn)
{
  list_t singleton = list_createElem(asn);
  asnl->list = list_push(asnl->list, singleton);
  asnl->tree = asnTreeInsert_aux(asnl->tree, singleton);
  return asnl;
}

static
struct treeNodeAE *
asnTreeNext_aux(asnTree_t treeNode, void *pointer, compFun_t compFun)
{
  int idx = asnTreeBinSearch_aux(treeNode, pointer);
  assert (0 <= idx && idx <= treeNode->numOfChldrn);

  //printf("treeNode->numOfChldrn: %d, idx: %d\n", treeNode->numOfChldrn, idx);


  if (idx < treeNode->numOfChldrn) {
    if ((*compFun)(treeNode->chldrn[idx].edgeLabel, pointer)) { // FIXED

      // DEBUG START
      //if (treeNode->chldrn[idx].chld == NULL) {
      //  for (int ii = 0; ii < treeNode->numOfChldrn; ++ii) {
      //    printf("%s ", ((struct term *)treeNode->chldrn[ii].edgeLabel)->name);
      //  }
      //}
      //printf("\n");

      // DEBUG END

      assert(treeNode->chldrn[idx].edgeLabel != NULL);
      assert(treeNode->chldrn[idx].chld != NULL);
      return &treeNode->chldrn[idx];
    }
  }
  return NULL;
}

static
list_t
asnTreeFind_aux(asnTree_t asnt, aStarNode_t asn)
{
  asnTree_t currTN = asnt;
  for (/* empty */; asn != NULL; asn = asn->next) {
    struct groundAction *grAct = asn->act;
    struct action *act = grAct->action;
    assert(currTN->type == ACTION ||
           (currTN->type == UNSET && currTN->numOfChldrn == 0));
    struct treeNodeAE *tnae = asnTreeNext_aux(currTN, act, &actionCompFun);
    if (tnae == NULL) {
      return NULL;
    }
    assert(tnae->chld != NULL);
    currTN = tnae->chld;

    for (int idxTerm = 0; idxTerm < act->numOfParams; ++idxTerm) {
      assert(currTN->type == TERM);
      tnae = asnTreeNext_aux(currTN, grAct->terms[idxTerm], &termCompFun);
      if (tnae == NULL) {
        return NULL;
      }
      assert(tnae->chld != NULL);
      currTN = tnae->chld;
    }
  }
  return currTN->pointerIntoList;
}

list_t
asnList_find(asnList_t asnl, aStarNode_t asn)
{
  // asn can be NULL on purpose.
  if (asnl == NULL) {
    return NULL;
  }

  //printf("asnList_find():\n");
  //printf("action list: ");
  //list_print(asnl->list, &printActionList);
  //printf("a-star node: ");
  //utils_print_actionListCompact(asn);
  //printf("\n");

  return asnTreeFind_aux(asnl->tree, asn);
}

void
asnTreePrint(asnTree_t tree)
{
  assert(tree != NULL);
  //if (tree == NULL) {
  //  return;
  //}

  printf("(%d_", tree->numOfChldrn);

  for (int ii = 0; ii < tree->numOfChldrn; ++ii) {
    if (tree->type == ACTION) {
      printf("%s", ((struct action *)tree->chldrn[ii].edgeLabel)->name);
    } else if (tree->type == TERM) {
      printf("%s", ((struct term *)tree->chldrn[ii].edgeLabel)->name);
    } else {
      assert(false && "If the tree has children, "
                      "then it must have a type too.");
    }
    // Recurse
    asnTreePrint(tree->chldrn[ii].chld);
    if (ii < tree->numOfChldrn - 1) {
      printf(" ");
    }
  }

  printf(")");
}

static
void
asnTreeReuse(asnTree_t treeNode)
{
  // TODO: Maybe reuse. For now free.
  free(treeNode);
}

// If idx == -1 then process the action it self. Otherwise process the
// actions parameter at the index.
static
bool
asnTreeRemoveRec_aux(asnTree_t tree, aStarNode_t asn, int idx)
{
  assert(-1 <= idx &&
         (asn == NULL || idx < asn->act->action->numOfParams));

  if (asn == NULL) {
    return true;
  }

  if (tree->numOfChldrn > 0) {
    struct treeNodeAE *tnae = NULL;
    if (idx < 0) {
      tnae = asnTreeNext_aux(tree, asn->act->action, &actionCompFun);
    } else {
      tnae = asnTreeNext_aux(tree, asn->act->terms[idx], &termCompFun);
    }
    if (tnae == NULL) {
      // Does not exist.
      return false;
    }

    bool recResult = false;

    //if (idx < asn->act->action->numOfParams - 1) {
    //  recResult = asnTreeRemoveRec_aux(tnae->chld, asn, idx + 1);
    //} else {
    //  // Proceed to next ground action.
    //  recResult = asnTreeRemoveRec_aux(tnae->chld, asn->next, -1);
    //}
    //printf("idx: %d\n", idx);
    if (idx >= asn->act->action->numOfParams - 1) {
      // Proceed to next ground action.
      recResult = asnTreeRemoveRec_aux(tnae->chld, asn->next, -1);
    } else {
      recResult = asnTreeRemoveRec_aux(tnae->chld, asn, idx + 1);
    }
    if ( ! recResult) {
      return false;
    }
    // Remove child node, if it is not needed anymore.
    if (tnae->chld->numOfChldrn <= 0) {
      asnTreeReuse(tnae->chld);

      tnae->chld = NULL;
      // Remove child reference from this node's array.
      // Pointer arithmetic
      int idxArr = tnae - tree->chldrn;
      memmove(tnae,
              tnae + 1,
               (tree->numOfChldrn - idxArr - 1) * sizeof(*tnae)); // FIXED
      tree->numOfChldrn--;
    }
  } else if (asn != NULL) {
    return false;
  } else {
    // Last tree node found. Remove pointer into list
    tree->pointerIntoList = NULL;
  }
  return true;
}

// Returns true if it did remove something.
static
bool
asnTreeRemove_aux(asnTree_t tree, aStarNode_t asn)
{
  return asnTreeRemoveRec_aux(tree, asn, -1);

  /* asnTree_t currTree = tree; */
  /* for (/1* empty *1/; asn != NULL; asn = asn->next) { */
  /*   struct groundAction grAct = asn->act; */
  /*   struct action *action = grAct->action; */
  /*   struct treeNodeAE *tnae = asnTreeNext_aux(currTree, action); */
  /*   if (tnae == NULL) { */
  /*     // Not found. */
  /*     return false; */
  /*   } */
  /*   currTree = tnae->chld; */
  /*   for (int idxTerm = 0; idxTerm < action->numOfParams; ++idxTerm) { */
  /*     tnae = asnTreeNext_aux(currTree, grAct->terms[idxTerm]); */
  /*     if (tnae == NULL) { */
  /*       return false; */
  /*     } */
  /*     currTree = tnae->chld; */
  /*     // UNFINISHED: continue. */
  /*   } */
  /* } */
}

void *
asnList_getFirstPayload(asnList_t asnl)
{
  return list_getFirstPayload(asnl->list);
}

asnList_t
asnList_removeFirst(asnList_t asnl)
{
  aStarNode_t asn = list_getFirstPayload(asnl->list);
  asnl->list = list_removeFirst(asnl->list);
  bool rmResult = asnTreeRemove_aux(asnl->tree, asn);
  return asnl;
}

// A comparison function for finding locations in lists for inserting
// elements in an ordered manner by their intValue property.
static
int
compIntValueFunAfter(list_t e1, void *fScore_)
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

// A comparison function for finding locations in lists for inserting
// elements in an ordered manner by their intValue property. New elements will
// be inserted before other elements with the same intValue.
static
int
compIntValueFunBefore(list_t e1, void *fScore_)
{
  if (e1 == NULL || fScore_ == NULL) {
    assert(false);
  }
  int fScore = *(int *) fScore_;

  if (e1->intValue >= fScore) {
    return -1;
  }
  assert(e1->intValue < fScore);

  if (e1->next == NULL) {
    return 0;
  }
  if (e1->next->intValue >= fScore) {
    return 0;
  }

  return 1;
}

static
list_t
asnl_insertOrdered(list_t list, list_t singleton)
{
  assert(singleton != NULL);
  int fScore = singleton->intValue;

  //list_t found = list_find(list, &compIntValueFunAfter, &fScore);
  list_t found = list_find(list, &compIntValueFunBefore, &fScore);
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

asnList_t
asnList_insertOrdered(asnList_t asnl, aStarNode_t asn, int intValue)
{
  if (asn == NULL) {
    return asnl;
  }

  //printf("asnList_insertOrdered().");
  //list_print(asnl->list, &printActionList);
  //utils_print_actionListCompact(asn);
  //printf("\n");
  //printf("\n");

  list_t singleton = list_createElem(asn);
  singleton->intValue = intValue;

  asnl->list = asnl_insertOrdered(asnl->list, singleton);
  //list_print(asnl->list, &printActionList);
  asnl->tree = asnTreeInsert_aux(asnl->tree, singleton);

  return asnl;
}

asnList_t
asnList_remove(asnList_t asnl, aStarNode_t asn)
{
  list_t elem = asnList_find(asnl, asn);
  if (elem == NULL) {
    // Element to remove not found.
    return asnl;
  }

  // Remove from list
  if (elem->prev == NULL) {
    // Elem is the head of the list.
    asnl->list = list_removeFirst(asnl->list);
  } else {
    elem->prev->next = elem->next;
    if (elem->next != NULL) {
      elem->next->prev = elem->prev;
    }
  }

  // Remove from tree
  bool rmResult = asnTreeRemove_aux(asnl->tree, asn);
  //assert(rmResult);

  return asnl;
}

