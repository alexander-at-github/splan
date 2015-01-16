// Note: This module is not thread save. We use a global buffer to store
// unused tNodes, in order to keep the number of systemcalls to malloc() and
// free() reasonable.

#include <assert.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include "trie.h"

#include "libpddl31.h"

// A trie data structure. It is mostly used as a state.
struct st_trie {

  // A pointer to the coresponding domain.
  struct domain *domain;

  // Note: 'struct predManag' contains an array of 'struct predicate'. That
  // is, The pointers to all the predicates will be consecutive. We will use
  // that fact here.

  // Array of tNode pointer. The length of the array is
  // domain->predManag->numOfPreds.
  // We will save that value here for later use.
  int32_t numOfChldrn;
  // The first element of the array will represent the first predicate in
  // the domain->predManag's array of consecutive predicates, the second one
  // the second predicate, and so on.

  // Save the pointer to the first element in domain->predManag->preds for
  // pointer arithmetic later on.
  struct predicate *predManagFirst;

  // If the trie does not contain an atom with a certain predicate, then this
  // pointer will be NULL. Otherwise it will have a child which will represent
  // (possible multiple) the atom (the predicate with its' terms).
  struct tNode **chldrn;
};

struct tNode {
  // A tNode contains an array of term-child pairs. If it does not contain
  // a certain term, it means that this trie does not contain the
  // coresponding atom.
  int32_t numOfChldrn;
  // Siyze of array alloced for children.
  int32_t numAlloced;

  // This is an unordered array. TODO: It might be good to do an ordered
  // array.
  struct tNodeArrE *chldrn;

  // A count variable. This variable will be used only on leaf nodes. It is
  // intended to be used to count the number of variable occurences in a
  // planning problem.
  // To include this data field here reduces the cohesion, which is not
  // favourable. The count has actually nothing to do with the trie itself.
  // I will just do that, cause it is so much easier.
  int32_t count;
};

// Composition of term and tNode child.
struct tNodeArrE {
  struct term *term; // However, this must be a constant.
  struct tNode *chld;
};



struct tNodeBuffer {
  int32_t numOfSNodes;
  int32_t numAlloced;
  // An array of pointers to unused tNodes.
  struct tNode **store;
};

// A static global variable. It will hold a set of unused tNodes. When a new
// tNode is needed, we will try to retrieve one from here. When a tNode is not
// needed anymore, we will put it here. In order to free the list with all its'
// stored tNodes we need to call trie_cleanupSNBuffer().
// Attention: This is not thread save.
static struct tNodeBuffer *tNodeBuffer = NULL;




int32_t
calcBufferSize(int32_t oldSize)
{
  int32_t initialBufferSize = 32;
  int32_t growthFactor = 2;
  int32_t newSize = (oldSize < (initialBufferSize / growthFactor)) ?
          initialBufferSize :
          (growthFactor * oldSize);

  //printf("calcBufferSize() %d   ", initialBufferSize / growthFactor);

  /* int32_t newSize; */
  /* if (oldSize < (initialBufferSize / growthFactor)) { */
  /*   newSize = initialBufferSize; */
  /* } else { */
  /*   newSize = growthFactor * oldSize; */
  /* } */

  //printf("calcBufferSize() oldSize: %d newSize: %d\n",
  //       oldSize,
  //       newSize); // DEBUG

  return newSize;
}

void
trie_initSNBuffer()
{
  if (tNodeBuffer != NULL) {
    return;
  }

  tNodeBuffer = malloc(sizeof(*tNodeBuffer));
  tNodeBuffer->numOfSNodes = 0;
  tNodeBuffer->numAlloced = calcBufferSize(0);
  tNodeBuffer->store = malloc(sizeof(*tNodeBuffer->store) *
                              tNodeBuffer->numAlloced);
}

void
snFreeShallow_aux(struct tNode *tNode)
{
  if (tNode->chldrn != NULL) {
    free(tNode->chldrn);
    tNode->chldrn = NULL;
  }
  free(tNode);
}

void
trie_cleanupSNBuffer()
{
  if (tNodeBuffer == NULL) {
    return;
  }
  if (tNodeBuffer->store != NULL) {
    for (int32_t idx = 0; idx < tNodeBuffer->numOfSNodes; ++idx) {
      snFreeShallow_aux(tNodeBuffer->store[idx]);
    }
    free(tNodeBuffer->store);
  }
  free(tNodeBuffer);

  // Important to reset to NULL.
  tNodeBuffer = NULL;
}

trie_t
trie_createEmpty(struct domain *domain)
{
  // TODO: Check for correctness.
  trie_t trie = malloc(sizeof(*trie));
  trie->domain = domain;
  trie->numOfChldrn = domain->predManag->numOfPreds;
  //printf("trie_createEmpty(): trie->numOfChldrn: %d\n", trie->numOfChldrn); // DEBUG
  trie->predManagFirst = domain->predManag->preds;
  trie->chldrn = malloc(sizeof(*trie->chldrn) * trie->numOfChldrn);
  for (int32_t i = 0; i < trie->numOfChldrn; ++i) {
    trie->chldrn[i] = NULL;
  }

  trie_initSNBuffer();

  return trie;
}

trie_t
trie_createEmptyFrom(trie_t trie)
{
  return trie_createEmpty(trie->domain);
}

trie_t
trie_createFromLibpddl31(struct domain *domain, pANTLR3_LIST listOfAtoms)
{
  trie_t trie = trie_createEmpty(domain);
  //printf("trie_createFromLibpddl31(): listOfAtoms->size(): %d\n",
  //       listOfAtoms->size(listOfAtoms)); // DEBUG
  //printf("\n"); // DEBUG
  for (int32_t i = 0; i < listOfAtoms->size(listOfAtoms); ++i) {
    // ANTLR3 list index starts from one.
    //libpddl31_atom_print(listOfAtoms->get(listOfAtoms, i+1)); // DEBUG
    //printf("\n"); // DEBUG
    trie_add(trie, listOfAtoms->get(listOfAtoms, i+1));
  }
  //trie_print(trie); // DEBUG
  return trie;
}

void
tNodeBufferAdd_aux(struct tNode *tNode)
{
  //printf("tNodeBufferAdd_aux()\n"); // DEBUG
  if (tNode == NULL) {
    return;
  }

  tNodeBuffer->numOfSNodes ++;
  if (tNodeBuffer->numOfSNodes > tNodeBuffer->numAlloced) {
    //printf( "numOfSNodes: %d numAlloced: %d ",
    //        tNodeBuffer->numOfSNodes,
    //        tNodeBuffer->numAlloced); // DEBUG
    tNodeBuffer->numAlloced = calcBufferSize(tNodeBuffer->numAlloced);
    tNodeBuffer->store = realloc(tNodeBuffer->store,
                      sizeof(*tNodeBuffer->store) * tNodeBuffer->numAlloced);
    //printf( "NEW: numOfSNodes: %d numAlloced: %d\n",
    //        tNodeBuffer->numOfSNodes,
    //        tNodeBuffer->numAlloced); // DEBUG
  }

  tNodeBuffer->store[tNodeBuffer->numOfSNodes - 1] = tNode;
}

struct tNode *
snGetOrCreate_aux()
{
  //static int32_t counter = 0; // DEBUG
  //counter++; // DEBUG
  //printf("snGetOrCreate_aux(): %d\n", counter); // DEBUG

  // Try to retrieve from tNodeBuffer.
  assert (tNodeBuffer != NULL);
  if (tNodeBuffer->numOfSNodes > 0) {
    // Take a tNode from the tNodeBuffer.
    tNodeBuffer->numOfSNodes --;
    struct tNode *reusedSN = tNodeBuffer->store[tNodeBuffer->numOfSNodes];
    tNodeBuffer->store[tNodeBuffer->numOfSNodes] = NULL;
    // Set number of children and count of the tNode. We will not change the
    // other values. Note: There still might be some pointers to other nodes.
    // Don't use them.
    reusedSN->numOfChldrn = 0;
    reusedSN->count = 0;
    return reusedSN;
  }

  // Alloc and initialize new tNode.
  struct tNode *result = malloc(sizeof(*result));
  result->numOfChldrn = 0;
  result->numAlloced = 0;
  result->chldrn = NULL;
  result->count = 0;
  return result;
}

void
snGrowArray_aux(struct tNode *tNode)
{
  //printf("snGrowArray_aux() tNode->numAlloced: %d ", tNode->numAlloced);
  tNode->numAlloced = calcBufferSize(tNode->numAlloced);
  //printf("NEW tNode->numAlloced: %d\n", tNode->numAlloced);
  tNode->chldrn = realloc(tNode->chldrn,
                          sizeof(*tNode->chldrn) * tNode->numAlloced);
}

struct tNode *
snFindOrAdd_aux(struct tNode *tNode, struct term *term)
{
  // Naive method.
  for (int32_t idx = 0; idx < tNode->numOfChldrn; ++idx) {
    struct tNodeArrE *snae = &tNode->chldrn[idx];
    if (libpddl31_term_equal(snae->term, term)) {
      return snae->chld;
    }
  }
  // tNode does not contain term yet. Create it.
  tNode->numOfChldrn ++;

  if (tNode->numOfChldrn > tNode->numAlloced) {
    snGrowArray_aux(tNode);
  }

  struct tNodeArrE *newSNodeArrE = &tNode->chldrn[tNode->numOfChldrn - 1];
  newSNodeArrE->term = term;
  newSNodeArrE->chld = snGetOrCreate_aux();

  return newSNodeArrE->chld;
}

void
trie_addGr(trie_t trie, struct atom *atom, struct groundAction *grAct)
{
  // TODO: Remove that for performance reasons? All the types should be okay.
  /* First check the types before we change anything. */
  for (int32_t idxTerm = 0; idxTerm < atom->pred->numOfParams; ++idxTerm) {
    struct type *predParamType = atom->pred->params[idxTerm].type;
    // Pointer arithmetic for ground action.
    int32_t idxGrounding = atom->terms[idxTerm] - grAct->action->params;
    if (0 <= idxGrounding && idxGrounding < grAct->action->numOfParams) {
      // We are dealing with a grounding.
      if ( ! typeSystem_isa(grAct->terms[idxGrounding]->type, predParamType)) {
        // Type missmatch. Do not apply atom.
        return;
      }
    } else {
      // We are dealing with a constant.
      if ( ! typeSystem_isa(atom->terms[idxTerm]->type, predParamType)) {
        // Type missmatch. Do not apply atom.
        // That should never happen, since it is a constant.
        assert (false);
        return;
      }
    }
  }

  // All types are okay.
  /* Apply the atom. */

  // Pointer arithmetic. See struct st_trie.
  // A pointer into the tries' array of predicates.
  int32_t triePredIdx = atom->pred - trie->predManagFirst;
  if (trie->chldrn[triePredIdx] == NULL) {
    trie->chldrn[triePredIdx] = snGetOrCreate_aux();
  }
  struct tNode *currSN = trie->chldrn[triePredIdx];
  for (int32_t idxTerm = 0; idxTerm < atom->pred->numOfParams; ++idxTerm) {
    struct term *termToAdd;

    // Pointer arithmetic of ground action.
    int32_t idxGrounding = atom->terms[idxTerm] - grAct->action->params;
    if (0 <= idxGrounding && idxGrounding < grAct->action->numOfParams) {
      // We are dealing with a grounding.
      termToAdd = grAct->terms[idxGrounding];
    } else {
      // We are dealing with a constant.
      termToAdd = atom->terms[idxTerm];
    }

    currSN = snFindOrAdd_aux(currSN, termToAdd);
    // TODO: Is that correct?
  }
}

void
trie_add(trie_t trie, struct atom *atom)
{
  // TODO: Add type checks before you change anything!

  // Pointer arithmetic. See struct st_trie.
  // A pointer into the tries' array of predicates.
  int32_t triePredIdx = atom->pred - trie->predManagFirst;
  if (trie->chldrn[triePredIdx] == NULL) {
    trie->chldrn[triePredIdx] = snGetOrCreate_aux();
  }
  struct tNode *currSN = trie->chldrn[triePredIdx];
  for (int32_t idxTerm = 0; idxTerm < atom->pred->numOfParams; ++idxTerm) {
    struct term *atomTerm = atom->terms[idxTerm];
    currSN = snFindOrAdd_aux(currSN, atomTerm);
    // TODO: Is that correct?
  }
}

struct tNodeArrE *
snNextAe_aux(struct tNode *tNode, struct term *searchTerm)
{
  // Naive method.
  for (int32_t idx = 0; idx < tNode->numOfChldrn; ++idx) {
    struct tNodeArrE *snae = &tNode->chldrn[idx];
    if (libpddl31_term_equal(snae->term, searchTerm)) {
      return snae;
    }
  }
  return NULL;
}

bool
trie_containsGr(trie_t trie, struct atom *atom, struct groundAction *grAct)
{
  // Pointer arithmetic. See struct st_trie.
  // A pointer into the tries' array of predicates.
  int32_t triePredIdx = atom->pred - trie->predManagFirst;
  struct tNode *currSN = trie->chldrn[triePredIdx];
  if (currSN == NULL) {
    return false;
  }

  for (int32_t idxTerm = 0; idxTerm < atom->pred->numOfParams; ++idxTerm) {
    assert (currSN != NULL);
    struct term *atomTerm = NULL;

    // Pointer arithmentic.
    int32_t idxGrounding = atom->terms[idxTerm] - grAct->action->params;
    if (0 <= idxGrounding && idxGrounding < grAct->action->numOfParams) {
      // Action variable
      atomTerm = grAct->terms[idxGrounding];
    } else {
      // Constant in action.
      atomTerm = atom->terms[idxTerm];
    }
    struct tNodeArrE *snae = snNextAe_aux(currSN, atomTerm);
    if (snae == NULL) {
      return false;
    }
    struct tNode *nextSN = snae->chld;
    assert (nextSN != NULL);
    //if (nextSN == NULL) {
    //  return false;
    //}
    currSN = nextSN;
  }
  //printf("currSN->numOfChldrn: %d, currSN->chldrn: %p\n",
  //       currSN->numOfChldrn,
  //       currSN->chldrn); // DEBUG

  // This predicate can not have any further terms as arguments. I.e., it's a
  // leave node.
  assert (currSN->numOfChldrn == 0);

  return true;
}

bool
trie_contains(trie_t trie, struct atom *atom)
{
  // Pointer arithmetic. See struct st_trie.
  // A pointer into the tries' array of predicates.
  int32_t triePredIdx = atom->pred - trie->predManagFirst;
  struct tNode *currSN = trie->chldrn[triePredIdx];
  if (currSN == NULL) {
    return false;
  }

  for (int32_t idxTerm = 0; idxTerm < atom->pred->numOfParams; ++idxTerm) {
    assert (currSN != NULL);
    struct term *atomTerm = atom->terms[idxTerm];
    struct tNodeArrE *snae = snNextAe_aux(currSN, atomTerm);
    if (snae == NULL) {
      return false;
    }
    struct tNode *nextSN = snae->chld;
    assert (nextSN != NULL);
    //if (nextSN == NULL) {
    //  return false;
    //}
    currSN = nextSN;
  }
  //printf("currSN->numOfChldrn: %d, currSN->chldrn: %p\n",
  //       currSN->numOfChldrn,
  //       currSN->chldrn); // DEBUG

  // This predicate can not have any further terms as arguments. I.e., it's a
  // leave node.
  assert (currSN->numOfChldrn == 0);

  return true;
}

// This function frees a tNode and its' children.
void snFreeRec_aux(struct tNode *tNode)
{
  //static int32_t counter = 0; // DEBUG
  //counter++; // DEBUG
  //printf("snFreeRec_aux(): %d\n", counter); // DEBUG

  for (int32_t idx = 0; idx < tNode->numOfChldrn; ++idx) {
    struct tNodeArrE *snae = &tNode->chldrn[idx];
    snFreeRec_aux(snae->chld);
    // No need to free snae. It is a array of struct tNodeArrE.
  }
  snFreeShallow_aux(tNode);


  /* if (tNode->numOfChldrn > 0) { */
  /*   for (int32_t idx = 0; idx < tNode->numOfChldrn; ++idx) { */
  /*     struct tNodeArrE *snae = &tNode->chldrn[idx]; */
  /*     snFreeRec_aux(snae->chld); */
  /*     // No need to free snae. It is a array of struct tNodeArrE. */
  /*   } */
  /*   free(tNode->chldrn); */
  /* } else { */
  /*   assert (tNode->chldrn == NULL); */
  /* } */
  /* free(tNode); */
}

// A recursive function.
// Returns true if it did remove something.
bool
snRemove_aux(struct tNode *tNode, struct atom *atom, int32_t depth)
{
  if (depth > atom->pred->numOfParams) {
    // This is a sever error.
    assert (false);
    fprintf(stderr, "ERROR in trie_remove()\n");
    return false;
  }

  if (tNode->numOfChldrn > 0) {
    assert (depth < atom->pred->numOfParams);

    struct tNodeArrE *snaeNext = snNextAe_aux(tNode, atom->terms[depth]);
    if (snaeNext == NULL) {
      // Term does not exist. I.e., this atom is not in this trie.
      return false;
    }

    bool recursiveResult = snRemove_aux(snaeNext->chld, atom, depth + 1);
    if ( ! recursiveResult) {
      return false;
    }

    // Remove child node, if it is not needed anymore.
    // We do that here, so we can also remove the child node after the first
    // call of this recursive funtion (after the call
    // snRemove_aux(tNode,atom,0).
    // We do not free the node, but move it to the tNodeBuffer for reuse.
    if (snaeNext->chld->numOfChldrn <= 0) {

      // Move the tNode into the tNodeBuffer for later reuse. Do not free it.
      tNodeBufferAdd_aux(snaeNext->chld);

      snaeNext->chld = NULL;

      // Remove child reference from this nodes' array.
      // Pointer arithmetic.
      int32_t idxSNodeArr = snaeNext - tNode->chldrn;
      memmove(snaeNext,
              snaeNext + 1,
              (tNode->numOfChldrn - idxSNodeArr - 1) * sizeof(*snaeNext));
      tNode->numOfChldrn --;
    }

  } else {
    assert (depth == atom->pred->numOfParams);
  }

  // The atom (the atoms' term) has been removed indeed.
  return true;
}

trie_t
trie_removeGr(trie_t trie, struct atom *atom, struct groundAction *grAct)
{
  // TODO: improve that.

  // Just allocating a ground atom.
  struct atom *groundAtom = malloc(sizeof(*groundAtom));
  groundAtom->pred = atom->pred;
  groundAtom->terms = malloc( sizeof(*groundAtom->terms) *
                                groundAtom->pred->numOfParams);

  for (int32_t idx = 0; idx < atom->pred->numOfParams; ++idx) {
    int32_t idxGrounding = atom->terms[idx] - grAct->action->params;
    if (0 <= idxGrounding && idxGrounding < grAct->action->numOfParams) {
      // A grounding.
      groundAtom->terms[idx] = grAct->terms[idxGrounding];
    } else {
      // A constant.
      groundAtom->terms[idx] = atom->terms[idx];
    }
  }

  trie_t result = trie_remove(trie, groundAtom);

  libpddl31_atom_free(groundAtom);
  free(groundAtom);

  return result;
}

trie_t
trie_remove(trie_t trie, struct atom *atom)
{
  // Pointer arithmetic. See struct st_trie.
  // A pointer into the tries' array of predicates.
  int32_t triePredIdx = atom->pred - trie->predManagFirst;
  struct tNode *currSN = trie->chldrn[triePredIdx];
  if (currSN == NULL) {
    // Nothing to do.
    return trie;
  }
  (void) snRemove_aux(currSN, atom, 0);
  // Free tNode if it is not needed anymore.
  if (currSN->numOfChldrn <= 0) {

    // Move the tNode into the tNodeBuffer for later reuse. Do not free it.
    tNodeBufferAdd_aux(currSN);

    trie->chldrn[triePredIdx] = NULL;
  }
  return trie;
}

void tNodeBufferAddRec_aux(struct tNode *tNode)
{
  for (int32_t idx = 0; idx < tNode->numOfChldrn; ++idx) {
    tNodeBufferAddRec_aux(tNode->chldrn[idx].chld);
  }
  tNodeBufferAdd_aux(tNode);
}

void
trie_empty(trie_t trie)
{
  for (int32_t idxSt = 0;
       idxSt < trie->numOfChldrn;
       ++idxSt) {

    if (trie->chldrn[idxSt] == NULL) {
      continue;
    }
    if (tNodeBuffer != NULL) {
      // Move all the tNodes into the tNodeBuffer.
      tNodeBufferAddRec_aux(trie->chldrn[idxSt]);
    } else {
      snFreeRec_aux(trie->chldrn[idxSt]);
    }
    trie->chldrn[idxSt] = NULL;
  }
}

void
trie_free(trie_t trie)
{
  if (trie == NULL) {
    return;
  }

  trie_empty(trie);

  free(trie->chldrn);
  free(trie);
}

struct tNode *
snClone_aux(struct tNode *sn)
{
  //static int32_t counter = 0; // DEBUG
  //counter++; // DEBUG
  //printf("snClone_aux(): %d\n", counter); // DEBUG

  struct tNode *result = snGetOrCreate_aux();
  result->numOfChldrn = sn->numOfChldrn;
  while (result->numOfChldrn > result->numAlloced) {
    snGrowArray_aux(result);
  }

  assert (result->numOfChldrn <= result->numAlloced);
  //printf("result->numOfChldrn: %d, result->chldrn: %p\n",
  //      result->numOfChldrn,
  //      result->chldrn); // DEBUG
  for (int32_t idx = 0; idx < sn->numOfChldrn; ++idx) {
    // Just copy the term ...
    result->chldrn[idx].term = sn->chldrn[idx].term;
    // ... and clone the child node.
    result->chldrn[idx].chld = snClone_aux(sn->chldrn[idx].chld);
  }
  return result;
}

trie_t
trie_clone(trie_t trie)
{
  if (trie == NULL) {
    return NULL;
  }

  trie_t result = trie_createEmpty(trie->domain);
  for (int32_t idx = 0; idx < trie->numOfChldrn; ++idx) {
    if (trie->chldrn[idx] == NULL) {
      continue;
    }
    result->chldrn[idx] = snClone_aux(trie->chldrn[idx]);
  }
  return result;
}

void
trie_print_aux(char *nameAcc, int32_t nameAccLen, struct tNode *tNode)
{
  assert (tNode != NULL);
  if (tNode->numOfChldrn == 0) {
    printf(" (%s)", nameAcc);
    return;
  }

  for (int32_t idx = 0; idx < tNode->numOfChldrn; ++idx) {
    struct tNodeArrE *snae = &tNode->chldrn[idx];

    int32_t snaeTermNameLen = strlen(snae->term->name);
    // Adding +1 for the delimiter
    int32_t nameAccLenNew = nameAccLen + 1 + snaeTermNameLen;

    char *nameAccNew = malloc(sizeof(*nameAccNew) *
                          nameAccLenNew + 1); // +1 for terminating '\0'.

    strncpy(nameAccNew, nameAcc, nameAccLen);
    strncpy(nameAccNew + nameAccLen, " ", 1); // Use space as seperator.
    strncpy(nameAccNew + nameAccLen + 1, snae->term->name, snaeTermNameLen);
    nameAccNew[nameAccLenNew] = '\0';

    trie_print_aux(nameAccNew, nameAccLenNew, snae->chld);

    free(nameAccNew);
  }
}

void
trie_print(trie_t trie)
{
  //printf("trie_print(): trie->numOfChldrn: %d\n", trie->numOfChldrn); // DEBUG
  printf("(trie");
  for (int32_t idx = 0; idx < trie->numOfChldrn; ++idx) {
    struct tNode *tNode = trie->chldrn[idx];
    if (tNode == NULL) {
      continue;
    }
    // Pointer arithmetic.
    struct predicate *pred = trie->predManagFirst + idx;
    trie_print_aux(pred->name, strlen(pred->name), tNode);
  }
  printf(")");
}

void
trie_setCountRec_aux(struct tNode *tNode, int32_t num)
{
  tNode->count = num;
  for (int32_t idxC = 0; idxC < tNode->numOfChldrn; ++idxC) {
    trie_setCountRec_aux(tNode->chldrn[idxC].chld, num);
  }
}

void
trie_setCount(trie_t trie, int32_t num)
{
  for (int32_t idxP = 0; idxP < trie->numOfChldrn; ++idxP) {
    struct tNode *sn = trie->chldrn[idxP];
    if (sn == NULL) {
      continue;
    }
    trie_setCountRec_aux(sn, num);
  }
}

// Returns true if it did increase count. Returns false otherwise (when the
// atom is not in the set of fluents).
bool
trie_incCountGr(trie_t trie, struct atom *atom, struct groundAction *grAct)
{
  //printf("trie_incCountGr(): "); // DEBUG
  //libpddl31_atom_print(atom); // DEBUG
  //printf("%s %s\n", grAct->terms[0]->name, grAct->terms[1]->name); // DEBUG


  // Pointer arithmetic. See struct st_trie.
  // A pointer into the tries' array of predicates.
  int32_t triePredIdx = atom->pred - trie->predManagFirst;
  if (trie->chldrn[triePredIdx] == NULL) {
    return false;
  }
  struct tNode *currSN = trie->chldrn[triePredIdx];
  for (int32_t idxTerm = 0; idxTerm < atom->pred->numOfParams; ++idxTerm) {
      struct term *atomTerm;

      // Pointer arithmetic of ground action.
      int32_t idxGrounding = atom->terms[idxTerm] - grAct->action->params;
      if (0 <= idxGrounding && idxGrounding < grAct->action->numOfParams) {
        // We are dealing with a grounding.
        atomTerm = grAct->terms[idxGrounding];
      } else {
        // We are dealing with a constant.
        atomTerm = atom->terms[idxTerm];
    }

    struct tNodeArrE *snae = snNextAe_aux(currSN, atomTerm);
    if (snae == NULL) {
      return false;
    }
    struct tNode *nextSN = snae->chld;
    assert (nextSN != NULL);
    currSN = nextSN;
  }

  // This predicate can not have any further terms as arguments. I.e., it's a
  // leave node.
  assert (currSN->numOfChldrn == 0);

  // Leaf node found. The purpose of this function is to increment the leaf
  // nodes counter.
  currSN->count++;

  return true;
}

int32_t
trie_getMaxCountRec_aux(struct tNode *tNode)
{
  if (tNode->numOfChldrn == 0) {
    // It is ia leaf node.
    return tNode->count;
  }
  int32_t max = 0;
  for (int32_t idxC = 0; idxC < tNode->numOfChldrn; ++idxC) {
    int32_t subTreeMax = trie_getMaxCountRec_aux(tNode->chldrn[idxC].chld);
    if (subTreeMax > max) {
      //printf("trie_getMaxCountRec_aux() new max: %d, term: %s\n",
      //       subTreeMax,
      //       tNode->chldrn[idxC].term->name); // DEBUG
      max = subTreeMax;
    }
  }
  return max;
}

int32_t
trie_getMaxCount(trie_t trie)
{
  int32_t max = 0;

  for (int32_t idxP = 0; idxP < trie->numOfChldrn; ++idxP) {
    struct tNode *currSN = trie->chldrn[idxP];
    if (currSN == NULL) {
      continue;
    }
    int32_t subTreeMax = trie_getMaxCountRec_aux(currSN);
    if (subTreeMax > max) {
      max = subTreeMax;
    }
  }

  return max;
}