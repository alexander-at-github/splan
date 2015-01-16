#ifndef PROBSPACE_H
#define PROBSPACE_H

#include "libpddl31.h"

void ps_init(struct problem *problem);
trie_t ps_getSingleton();
void ps_cleanup();
struct actionList *ps_filter(struct actionList *actL);
int32_t ps_calcMaxVarOcc();

#endif
