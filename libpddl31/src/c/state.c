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
  // int32_t numOfAlloced; // TODO really?

  // This is an unordered array. TODO: It might be good to do an ordered
  // array.
  struct sNodeArrE *chldrn;
};

// Composition of term and sNode child.
struct sNodeArrE {
  struct term *term; // However, this must be a constant.
  struct sNode *chld;
};

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
  return state;
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

struct sNode *
snCreate_aux()
{
  struct sNode *result = malloc(sizeof(*result));
  result->numOfChldrn = 0;
  result->chldrn = NULL;
  return result;
}

struct sNode *
snFindOrCreate_aux(struct sNode *sNode, struct term *term)
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
  sNode->chldrn = realloc(sNode->chldrn,
                          sizeof(*sNode->chldrn) * sNode->numOfChldrn);
  struct sNodeArrE *newSNodeArrE = &sNode->chldrn[sNode->numOfChldrn - 1];
  newSNodeArrE->term = term;
  newSNodeArrE->chld = snCreate_aux();

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
    state->chldrn[statePredIdx] = snCreate_aux();
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

    currSN = snFindOrCreate_aux(currSN, termToAdd);
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
    state->chldrn[statePredIdx] = snCreate_aux();
  }
  struct sNode *currSN = state->chldrn[statePredIdx];
  for (int32_t idxTerm = 0; idxTerm < atom->pred->numOfParams; ++idxTerm) {
    struct term *atomTerm = atom->terms[idxTerm];
    currSN = snFindOrCreate_aux(currSN, atomTerm);
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
  assert (currSN->numOfChldrn == 0 && currSN->chldrn == NULL);

  return true;
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

    // Free child node, if it is not needed anymore.
    // We do that here, so we can also free the child node after the first
    // call of this recursive funtion (after the call snRemove_aux(sNode,atom,0)
    if (snaeNext->chld->numOfChldrn <= 0) {
      assert (snaeNext->chld->chldrn == NULL);
      free(snaeNext->chld);
      snaeNext->chld = NULL;

      // Remove child reference from this nodes' array.
      // Pointer arithmetic.
      int32_t idxSNodeArr = snaeNext - sNode->chldrn;
      memmove(snaeNext,
              snaeNext + 1,
              (sNode->numOfChldrn - idxSNodeArr - 1) * sizeof(*snaeNext));
      sNode->numOfChldrn --;
      struct sNodeArrE *tmp = realloc(sNode->chldrn,
                                      sizeof(*tmp) * sNode->numOfChldrn);
      if (sNode->numOfChldrn <= 0) {
        // Not really necessary, but nice to have for later assert.
        sNode->chldrn == NULL;
      } else if (tmp == NULL) {
        assert (false);
      }
      sNode->chldrn = tmp;
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
    assert (currSN->chldrn == NULL);
    free(currSN);
    state->chldrn[statePredIdx] = NULL;
  }
  return state;
}

void snFree_aux(struct sNode *sNode)
{
  if (sNode->numOfChldrn > 0) {
    for (int32_t idx = 0; idx < sNode->numOfChldrn; ++idx) {
      struct sNodeArrE *snae = &sNode->chldrn[idx];
      snFree_aux(snae->chld);
      // No need to free snae. It is a array of struct sNodeArrE.
    }
    free(sNode->chldrn);
  } else {
    assert (sNode->chldrn == NULL);
  }
  free(sNode);
}

void
state_free(state_t state)
{
  if (state == NULL) {
    return;
  }
  for ( int32_t idxStChldrn = 0;
        idxStChldrn < state->numOfChldrn;
        ++idxStChldrn) {

    struct sNode *sn = state->chldrn[idxStChldrn];
    if (sn == NULL) {
      continue;
    }
    snFree_aux(sn);
  }
  free(state->chldrn);
  free(state);
}

struct sNode *
snClone_aux(struct sNode *sn)
{
  struct sNode *result = malloc(sizeof(*result));
  result->numOfChldrn = sn->numOfChldrn;

  // malloc with a size of zero will still return a pointer other than NULL
  // and it should still be freed later. In order to avoid that we will not
  // allocate if sn->numOfchldrn equals zero.
  if (result->numOfChldrn == 0) {
    result->chldrn = NULL;
    return result;
  }

  result->chldrn = malloc(sizeof(*result->chldrn) * result->numOfChldrn);
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
  printf("state_print(): state->numOfChldrn: %d\n", state->numOfChldrn); // DEBUG
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
