#include <assert.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include "state.h"

//#include "libpddl31.h"
#include "utils.h"

// This state data structure is an adopted trie.
struct st_state {
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
  const struct predicate *predManagFirst;

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
  state->numOfChldrn = domain->predManag->numOfPreds;
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
  state->numOfChldrn = listOfAtoms->size(listOfAtoms);
  for (int32_t i = 0; i < state->numOfChldrn; ++i) {
    // ANTLR3 list index starts from one.
    state_add(state, listOfAtoms->get(listOfAtoms, i+1));
  }
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
    if (utils_term_equal(snae->term, term)) {
      return snae->chld;
    }
  }
  // sNode does not contain term yet. Create it.
  sNode->numOfChldrn ++;
  sNode->chldrn = realloc(sNode->chldrn,
                          sizeof(*sNode->chldrn) * sNode->numOfChldrn);
  struct sNodeArrE *newSNodeArrE = &sNode->chldrn[sNode->numOfChldrn];
  newSNodeArrE->term = term;
  newSNodeArrE->chld = snCreate_aux();

  return newSNodeArrE->chld;
}

void
state_add(state_t state, struct atom *atom)
{
  // Pointer arithmetic. See struct st_state.
  // A pointer into the states' array of predicates.
  int32_t statePredIdx = atom->pred - state->predManagFirst;
  struct sNode *currSN = state->chldrn[statePredIdx];
  if (currSN == NULL) {
    currSN = snCreate_aux();
  }
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
    if (utils_term_equal(snae->term, searchTerm)) {
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
    struct sNode *nextSN = snae->chld;
    if (nextSN == NULL) {
      return false;
    }
    currSN = nextSN;
  }
  // This predicate can not have any further terms as arguments. I.e., it's a
  // leave node.
  assert (currSN->numOfChldrn == 0 && currSN->chldrn == NULL);

  return true;
}

// A recursive function.
void
snRemove_aux(struct sNode *sNode, struct atom *atom, int32_t depth)
{
  assert (depth < atom->pred->numOfParams);

  if (sNode->numOfChldrn > 0) {
    // Recursive case
    struct sNodeArrE *snaeNext = snNextAe_aux(sNode, atom->terms[depth]);
    snRemove_aux(snaeNext->chld, atom, depth + 1);
    // Remove from this nodes' array.
    // Pointer arithmetic.
    int32_t idxSNodeArr = snaeNext - sNode->chldrn;
    memmove(snaeNext,
            snaeNext + 1,
            (sNode->numOfChldrn - idxSNodeArr - 1) * sizeof(*snaeNext));
    sNode->numOfChldrn --;
    struct sNodeArrE *tmp = realloc(sNode->chldrn, sNode->numOfChldrn);
    if (sNode->numOfChldrn <= 0) {
      // Not really necessary, but nice to have for later assert.
      sNode->chldrn == NULL;
    } else if (tmp == NULL) {
      assert (false);
    }
    sNode->chldrn = tmp;

  }
  // Note: sNode->numOfchldrn could have changed.

  // Base case.
  if (sNode->numOfChldrn <= 0) {
    assert (sNode->chldrn == NULL);
    free(sNode);
  }
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
  snRemove_aux(currSN, atom, 0);
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
