// Note: This module is not thread save. We use a global buffer to store
// unused sNodes, in order to keep the number of systemcalls to malloc() and
// free() reasonable.

#include <assert.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include "state.h"

#include "libpddl31.h"

// This state data structure is an adopted trie.
struct st_state {

  // A pointer to the coresponding domain.
  struct domain *domain;

  // Note: 'struct predManag' contains an array of 'struct predicate'. That
  // is, The pointers to all the predicates will be consecutive. We will use
  // that fact here.

  // Array of sNode pointer. The length of the array is
  // domain->predManag->numOfPreds.
  // We will save that value here for later use.
  int32_t numOfChldrn;
  // The first element of the array will represent the first predicate in
  // the domain->predManag's array of consecutive predicates, the second one
  // the second predicate, and so on.

  // Save the pointer to the first element in domain->predManag->preds for
  // pointer arithmetic later on.
  struct predicate *predManagFirst;

  // If the state does not have an atom with a certain predicate, then this
  // pointer will be NULL. Otherwise it will have a child which will represent
  // (possible multiple) the atom (the predicate with its' terms).
  struct sNode **chldrn;
};

struct sNode {
  // A sNode contains an array of term-child pairs. If it does not contain
  // a certain term, it means that this state does not contain the
  // coresponding atom.
  int32_t numOfChldrn;
  // Siyze of array alloced for children.
  int32_t numAlloced;

  // This is an unordered array. TODO: It might be good to do an ordered
  // array.
  struct sNodeArrE *chldrn;

  // A count variable. This variable will be used only on leaf nodes. It is
  // intended to be used to count the number of variable occurences in a
  // planning problem.
  // To include this data field here reduces the cohesion, which is not
  // favourable. The count has actually nothing to do with the state itself.
  // I will just do that, cause it is so much easier.
  int32_t count;
};

// Composition of term and sNode child.
struct sNodeArrE {
  struct term *term; // However, this must be a constant.
  struct sNode *chld;
};



struct sNodeBuffer {
  int32_t numOfSNodes;
  int32_t numAlloced;
  // An array of pointers to unused sNodes.
  struct sNode **store;
};

// A static global variable. It will hold a set of unused sNodes. When a new
// sNode is needed, we will try to retrieve one from here. When a sNode is not
// needed anymore, we will put it here. In order to free the list with all its'
// stored sNodes we need to call state_cleanupSNBuffer().
// Attention: This is not thread save.
static struct sNodeBuffer *sNodeBuffer = NULL;






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
state_initSNBuffer()
{
  if (sNodeBuffer != NULL) {
    return;
  }

  sNodeBuffer = malloc(sizeof(*sNodeBuffer));
  sNodeBuffer->numOfSNodes = 0;
  sNodeBuffer->numAlloced = calcBufferSize(0);
  sNodeBuffer->store = malloc(sizeof(*sNodeBuffer->store) *
                              sNodeBuffer->numAlloced);
}

void
snFreeShallow_aux(struct sNode *sNode)
{
  if (sNode->chldrn != NULL) {
    free(sNode->chldrn);
    sNode->chldrn = NULL;
  }
  free(sNode);
}

void
state_cleanupSNBuffer()
{
  if (sNodeBuffer == NULL) {
    return;
  }
  if (sNodeBuffer->store != NULL) {
    for (int32_t idx = 0; idx < sNodeBuffer->numOfSNodes; ++idx) {
      snFreeShallow_aux(sNodeBuffer->store[idx]);
    }
    free(sNodeBuffer->store);
  }
  free(sNodeBuffer);

  // Important to reset to NULL.
  sNodeBuffer = NULL;
}

state_t
state_createEmpty(struct domain *domain)
{
  // TODO: Check for correctness.
  state_t state = malloc(sizeof(*state));
  state->domain = domain;
  state->numOfChldrn = domain->predManag->numOfPreds;
  //printf("state_createEmpty(): state->numOfChldrn: %d\n", state->numOfChldrn); // DEBUG
  state->predManagFirst = domain->predManag->preds;
  state->chldrn = malloc(sizeof(*state->chldrn) * state->numOfChldrn);
  for (int32_t i = 0; i < state->numOfChldrn; ++i) {
    state->chldrn[i] = NULL;
  }

  state_initSNBuffer();

  return state;
}

state_t
state_createEmptyFrom(state_t state)
{
  return state_createEmpty(state->domain);
}

state_t
state_createFromLibpddl31(struct domain *domain, pANTLR3_LIST listOfAtoms)
{
  state_t state = state_createEmpty(domain);
  //printf("state_createFromLibpddl31(): listOfAtoms->size(): %d\n",
  //       listOfAtoms->size(listOfAtoms)); // DEBUG
  //printf("\n"); // DEBUG
  for (int32_t i = 0; i < listOfAtoms->size(listOfAtoms); ++i) {
    // ANTLR3 list index starts from one.
    //libpddl31_atom_print(listOfAtoms->get(listOfAtoms, i+1)); // DEBUG
    //printf("\n"); // DEBUG
    state_add(state, listOfAtoms->get(listOfAtoms, i+1));
  }
  //state_print(state); // DEBUG
  return state;
}

void
sNodeBufferAdd_aux(struct sNode *sNode)
{
  //printf("sNodeBufferAdd_aux()\n"); // DEBUG
  if (sNode == NULL) {
    return;
  }

  sNodeBuffer->numOfSNodes ++;
  if (sNodeBuffer->numOfSNodes > sNodeBuffer->numAlloced) {
    //printf( "numOfSNodes: %d numAlloced: %d ",
    //        sNodeBuffer->numOfSNodes,
    //        sNodeBuffer->numAlloced); // DEBUG
    sNodeBuffer->numAlloced = calcBufferSize(sNodeBuffer->numAlloced);
    sNodeBuffer->store = realloc(sNodeBuffer->store,
                      sizeof(*sNodeBuffer->store) * sNodeBuffer->numAlloced);
    //printf( "NEW: numOfSNodes: %d numAlloced: %d\n",
    //        sNodeBuffer->numOfSNodes,
    //        sNodeBuffer->numAlloced); // DEBUG
  }

  sNodeBuffer->store[sNodeBuffer->numOfSNodes - 1] = sNode;
}

struct sNode *
snGetOrCreate_aux()
{
  //static int32_t counter = 0; // DEBUG
  //counter++; // DEBUG
  //printf("snGetOrCreate_aux(): %d\n", counter); // DEBUG

  // Try to retrieve from sNodeBuffer.
  assert (sNodeBuffer != NULL);
  if (sNodeBuffer->numOfSNodes > 0) {
    // Take a sNode from the sNodeBuffer.
    sNodeBuffer->numOfSNodes --;
    struct sNode *reusedSN = sNodeBuffer->store[sNodeBuffer->numOfSNodes];
    sNodeBuffer->store[sNodeBuffer->numOfSNodes] = NULL;
    // Set number of children and count of the sNode. We will not change the
    // other values. Note: There still might be some pointers to other nodes.
    // Don't use them.
    reusedSN->numOfChldrn = 0;
    reusedSN->count = 0;
    return reusedSN;
  }

  // Alloc and initialize new sNode.
  struct sNode *result = malloc(sizeof(*result));
  result->numOfChldrn = 0;
  result->numAlloced = 0;
  result->chldrn = NULL;
  result->count = 0;
  return result;
}

void
snGrowArray_aux(struct sNode *sNode)
{
  //printf("snGrowArray_aux() sNode->numAlloced: %d ", sNode->numAlloced);
  sNode->numAlloced = calcBufferSize(sNode->numAlloced);
  //printf("NEW sNode->numAlloced: %d\n", sNode->numAlloced);
  sNode->chldrn = realloc(sNode->chldrn,
                          sizeof(*sNode->chldrn) * sNode->numAlloced);
}

struct sNode *
snFindOrAdd_aux(struct sNode *sNode, struct term *term)
{
  // Naive method.
  for (int32_t idx = 0; idx < sNode->numOfChldrn; ++idx) {
    struct sNodeArrE *snae = &sNode->chldrn[idx];
    if (libpddl31_term_equal(snae->term, term)) {
      return snae->chld;
    }
  }
  // sNode does not contain term yet. Create it.
  sNode->numOfChldrn ++;

  if (sNode->numOfChldrn > sNode->numAlloced) {
    snGrowArray_aux(sNode);
  }

  struct sNodeArrE *newSNodeArrE = &sNode->chldrn[sNode->numOfChldrn - 1];
  newSNodeArrE->term = term;
  newSNodeArrE->chld = snGetOrCreate_aux();

  return newSNodeArrE->chld;
}

void
state_addGr(state_t state, struct atom *atom, struct groundAction *grAct)
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

  // Pointer arithmetic. See struct st_state.
  // A pointer into the states' array of predicates.
  int32_t statePredIdx = atom->pred - state->predManagFirst;
  if (state->chldrn[statePredIdx] == NULL) {
    state->chldrn[statePredIdx] = snGetOrCreate_aux();
  }
  struct sNode *currSN = state->chldrn[statePredIdx];
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
state_add(state_t state, struct atom *atom)
{
  // TODO: Add type checks before you change anything!

  // Pointer arithmetic. See struct st_state.
  // A pointer into the states' array of predicates.
  int32_t statePredIdx = atom->pred - state->predManagFirst;
  if (state->chldrn[statePredIdx] == NULL) {
    state->chldrn[statePredIdx] = snGetOrCreate_aux();
  }
  struct sNode *currSN = state->chldrn[statePredIdx];
  for (int32_t idxTerm = 0; idxTerm < atom->pred->numOfParams; ++idxTerm) {
    struct term *atomTerm = atom->terms[idxTerm];
    currSN = snFindOrAdd_aux(currSN, atomTerm);
    // TODO: Is that correct?
  }
}

struct sNodeArrE *
snNextAe_aux(struct sNode *sNode, struct term *searchTerm)
{
  // Naive method.
  for (int32_t idx = 0; idx < sNode->numOfChldrn; ++idx) {
    struct sNodeArrE *snae = &sNode->chldrn[idx];
    if (libpddl31_term_equal(snae->term, searchTerm)) {
      return snae;
    }
  }
  return NULL;
}

bool
state_containsGr(state_t state, struct atom *atom, struct groundAction *grAct)
{
  // Pointer arithmetic. See struct st_state.
  // A pointer into the states' array of predicates.
  int32_t statePredIdx = atom->pred - state->predManagFirst;
  struct sNode *currSN = state->chldrn[statePredIdx];
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
    struct sNodeArrE *snae = snNextAe_aux(currSN, atomTerm);
    if (snae == NULL) {
      return false;
    }
    struct sNode *nextSN = snae->chld;
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
state_contains(state_t state, struct atom *atom)
{
  // Pointer arithmetic. See struct st_state.
  // A pointer into the states' array of predicates.
  int32_t statePredIdx = atom->pred - state->predManagFirst;
  struct sNode *currSN = state->chldrn[statePredIdx];
  if (currSN == NULL) {
    return false;
  }

  for (int32_t idxTerm = 0; idxTerm < atom->pred->numOfParams; ++idxTerm) {
    assert (currSN != NULL);
    struct term *atomTerm = atom->terms[idxTerm];
    struct sNodeArrE *snae = snNextAe_aux(currSN, atomTerm);
    if (snae == NULL) {
      return false;
    }
    struct sNode *nextSN = snae->chld;
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

// This function frees a sNode and its' children.
void snFreeRec_aux(struct sNode *sNode)
{
  //static int32_t counter = 0; // DEBUG
  //counter++; // DEBUG
  //printf("snFreeRec_aux(): %d\n", counter); // DEBUG

  for (int32_t idx = 0; idx < sNode->numOfChldrn; ++idx) {
    struct sNodeArrE *snae = &sNode->chldrn[idx];
    snFreeRec_aux(snae->chld);
    // No need to free snae. It is a array of struct sNodeArrE.
  }
  snFreeShallow_aux(sNode);


  /* if (sNode->numOfChldrn > 0) { */
  /*   for (int32_t idx = 0; idx < sNode->numOfChldrn; ++idx) { */
  /*     struct sNodeArrE *snae = &sNode->chldrn[idx]; */
  /*     snFreeRec_aux(snae->chld); */
  /*     // No need to free snae. It is a array of struct sNodeArrE. */
  /*   } */
  /*   free(sNode->chldrn); */
  /* } else { */
  /*   assert (sNode->chldrn == NULL); */
  /* } */
  /* free(sNode); */
}

// A recursive function.
// Returns true if it did remove something.
bool
snRemove_aux(struct sNode *sNode, struct atom *atom, int32_t depth)
{
  if (depth > atom->pred->numOfParams) {
    // This is a sever error.
    assert (false);
    fprintf(stderr, "ERROR in state_remove()\n");
    return false;
  }

  if (sNode->numOfChldrn > 0) {
    assert (depth < atom->pred->numOfParams);

    struct sNodeArrE *snaeNext = snNextAe_aux(sNode, atom->terms[depth]);
    if (snaeNext == NULL) {
      // Term does not exist. I.e., this atom is not in this state.
      return false;
    }

    bool recursiveResult = snRemove_aux(snaeNext->chld, atom, depth + 1);
    if ( ! recursiveResult) {
      return false;
    }

    // Remove child node, if it is not needed anymore.
    // We do that here, so we can also remove the child node after the first
    // call of this recursive funtion (after the call
    // snRemove_aux(sNode,atom,0).
    // We do not free the node, but move it to the sNodeBuffer for reuse.
    if (snaeNext->chld->numOfChldrn <= 0) {

      // Move the sNode into the sNodeBuffer for later reuse. Do not free it.
      sNodeBufferAdd_aux(snaeNext->chld);

      snaeNext->chld = NULL;

      // Remove child reference from this nodes' array.
      // Pointer arithmetic.
      int32_t idxSNodeArr = snaeNext - sNode->chldrn;
      memmove(snaeNext,
              snaeNext + 1,
              (sNode->numOfChldrn - idxSNodeArr - 1) * sizeof(*snaeNext));
      sNode->numOfChldrn --;
    }

  } else {
    assert (depth == atom->pred->numOfParams);
  }

  // The atom (the atoms' term) has been removed indeed.
  return true;
}

state_t
state_removeGr(state_t state, struct atom *atom, struct groundAction *grAct)
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

  state_t result = state_remove(state, groundAtom);

  libpddl31_atom_free(groundAtom);
  free(groundAtom);

  return result;
}

state_t
state_remove(state_t state, struct atom *atom)
{
  // Pointer arithmetic. See struct st_state.
  // A pointer into the states' array of predicates.
  int32_t statePredIdx = atom->pred - state->predManagFirst;
  struct sNode *currSN = state->chldrn[statePredIdx];
  if (currSN == NULL) {
    // Nothing to do.
    return state;
  }
  (void) snRemove_aux(currSN, atom, 0);
  // Free sNode if it is not needed anymore.
  if (currSN->numOfChldrn <= 0) {

    // Move the sNode into the sNodeBuffer for later reuse. Do not free it.
    sNodeBufferAdd_aux(currSN);

    state->chldrn[statePredIdx] = NULL;
  }
  return state;
}

void sNodeBufferAddRec_aux(struct sNode *sNode)
{
  for (int32_t idx = 0; idx < sNode->numOfChldrn; ++idx) {
    sNodeBufferAddRec_aux(sNode->chldrn[idx].chld);
  }
  sNodeBufferAdd_aux(sNode);
}

void
state_empty(state_t state)
{
  for (int32_t idxSt = 0;
       idxSt < state->numOfChldrn;
       ++idxSt) {

    if (state->chldrn[idxSt] == NULL) {
      continue;
    }
    if (sNodeBuffer != NULL) {
      // Move all the sNodes into the sNodeBuffer.
      sNodeBufferAddRec_aux(state->chldrn[idxSt]);
    } else {
      snFreeRec_aux(state->chldrn[idxSt]);
    }
    state->chldrn[idxSt] = NULL;
  }
}

void
state_free(state_t state)
{
  if (state == NULL) {
    return;
  }

  state_empty(state);

  free(state->chldrn);
  free(state);
}

struct sNode *
snClone_aux(struct sNode *sn)
{
  //static int32_t counter = 0; // DEBUG
  //counter++; // DEBUG
  //printf("snClone_aux(): %d\n", counter); // DEBUG

  struct sNode *result = snGetOrCreate_aux();
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

state_t
state_clone(state_t state)
{
  if (state == NULL) {
    return NULL;
  }

  state_t result = state_createEmpty(state->domain);
  for (int32_t idx = 0; idx < state->numOfChldrn; ++idx) {
    if (state->chldrn[idx] == NULL) {
      continue;
    }
    result->chldrn[idx] = snClone_aux(state->chldrn[idx]);
  }
  return result;
}

void
state_print_aux(char *nameAcc, int32_t nameAccLen, struct sNode *sNode)
{
  assert (sNode != NULL);
  if (sNode->numOfChldrn == 0) {
    printf(" (%s)", nameAcc);
    return;
  }

  for (int32_t idx = 0; idx < sNode->numOfChldrn; ++idx) {
    struct sNodeArrE *snae = &sNode->chldrn[idx];

    int32_t snaeTermNameLen = strlen(snae->term->name);
    // Adding +1 for the delimiter
    int32_t nameAccLenNew = nameAccLen + 1 + snaeTermNameLen;

    char *nameAccNew = malloc(sizeof(*nameAccNew) *
                          nameAccLenNew + 1); // +1 for terminating '\0'.

    strncpy(nameAccNew, nameAcc, nameAccLen);
    strncpy(nameAccNew + nameAccLen, " ", 1); // Use space as seperator.
    strncpy(nameAccNew + nameAccLen + 1, snae->term->name, snaeTermNameLen);
    nameAccNew[nameAccLenNew] = '\0';

    state_print_aux(nameAccNew, nameAccLenNew, snae->chld);

    free(nameAccNew);
  }
}

void
state_print(state_t state)
{
  //printf("state_print(): state->numOfChldrn: %d\n", state->numOfChldrn); // DEBUG
  printf("(State");
  for (int32_t idx = 0; idx < state->numOfChldrn; ++idx) {
    struct sNode *sNode = state->chldrn[idx];
    if (sNode == NULL) {
      continue;
    }
    // Pointer arithmetic.
    struct predicate *pred = state->predManagFirst + idx;
    state_print_aux(pred->name, strlen(pred->name), sNode);
  }
  printf(")");
}

void
state_setCountRec_aux(struct sNode *sNode, int32_t num)
{
  sNode->count = num;
  for (int32_t idxC = 0; idxC < sNode->numOfChldrn; ++idxC) {
    state_setCountRec_aux(sNode->chldrn[idxC].chld, num);
  }
}

void
state_setCount(state_t state, int32_t num)
{
  for (int32_t idxP = 0; idxP < state->numOfChldrn; ++idxP) {
    struct sNode *sn = state->chldrn[idxP];
    if (sn == NULL) {
      continue;
    }
    state_setCountRec_aux(sn, num);
  }
}

// Returns true if it did increase count. Returns false otherwise (when the
// atom is not in the set of fluents).
bool
state_incCountGr(state_t state, struct atom *atom, struct groundAction *grAct)
{
  //printf("state_incCountGr(): "); // DEBUG
  //libpddl31_atom_print(atom); // DEBUG
  //printf("%s %s\n", grAct->terms[0]->name, grAct->terms[1]->name); // DEBUG


  // Pointer arithmetic. See struct st_state.
  // A pointer into the states' array of predicates.
  int32_t statePredIdx = atom->pred - state->predManagFirst;
  if (state->chldrn[statePredIdx] == NULL) {
    return false;
  }
  struct sNode *currSN = state->chldrn[statePredIdx];
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

    struct sNodeArrE *snae = snNextAe_aux(currSN, atomTerm);
    if (snae == NULL) {
      return false;
    }
    struct sNode *nextSN = snae->chld;
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
state_getMaxCountRec_aux(struct sNode *sNode)
{
  if (sNode->numOfChldrn == 0) {
    // It is ia leaf node.
    return sNode->count;
  }
  int32_t max = 0;
  for (int32_t idxC = 0; idxC < sNode->numOfChldrn; ++idxC) {
    int32_t subTreeMax = state_getMaxCountRec_aux(sNode->chldrn[idxC].chld);
    if (subTreeMax > max) {
      //printf("state_getMaxCountRec_aux() new max: %d, term: %s\n",
      //       subTreeMax,
      //       sNode->chldrn[idxC].term->name); // DEBUG
      max = subTreeMax;
    }
  }
  return max;
}

int32_t
state_getMaxCount(state_t state)
{
  int32_t max = 0;

  for (int32_t idxP = 0; idxP < state->numOfChldrn; ++idxP) {
    struct sNode *currSN = state->chldrn[idxP];
    if (currSN == NULL) {
      continue;
    }
    int32_t subTreeMax = state_getMaxCountRec_aux(currSN);
    if (subTreeMax > max) {
      max = subTreeMax;
    }
  }

  return max;
}
