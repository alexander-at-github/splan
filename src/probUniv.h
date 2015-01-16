#ifndef PROBUNIV_H
#define PROBUNIV_H

#include "libpddl31.h"

void pu_init(struct problem *problem);
state_t pu_getSingleton();
void pu_cleanup();
struct actionList *pu_filter(struct actionList *actL);
int32_t pu_calcMaxVarOcc();

#endif
