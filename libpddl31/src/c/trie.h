#ifndef TRIE_H
#define TRIE_H

#include "pddl31structs.h"

#include "utils.h"

typedef struct st_trie * trie_t;

trie_t trie_createEmptyFrom(trie_t trie);
trie_t trie_createEmpty(struct domain *domain);
trie_t trie_createFromLibpddl31(struct domain *domain,
                                  pANTLR3_LIST listOfAtoms);
void trie_add(trie_t trie, struct atom *atom);
bool trie_contains(trie_t trie, struct atom *atom);
trie_t trie_remove(trie_t trie, struct atom *atom);
void trie_free(trie_t trie);

// The ground action needs to be the owner of the atom.
void trie_addGr( trie_t trie,
                  struct atom *atom,
                  struct groundAction *grAct);
// The ground action needs to be the owner of the atom.
trie_t trie_removeGr( trie_t trie,
                        struct atom *atom,
                        struct groundAction *grAct);
bool trie_containsGr(trie_t trie,
                      struct atom *atom,
                      struct groundAction *grAct);
trie_t trie_clone(trie_t trie);
void trie_print(trie_t trie);
void trie_cleanupSNBuffer();
bool trie_incCountGr(trie_t trie,
                      struct atom *atom,
                      struct groundAction *grAct);
int32_t trie_getMaxCount(trie_t trie);
void trie_setCount(trie_t trie, int32_t num);

void trie_addIndexPos(trie_t, struct atom *, struct groundAction *);
void trie_addIndexNeg(trie_t, struct atom *, struct groundAction *);;

struct actionList *trie_getActsPos(trie_t trie, struct atom *atom);
struct actionList *trie_getActsNeg(trie_t trie, struct atom *atom);

#endif // TRIE_H
