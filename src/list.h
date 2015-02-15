#ifndef LIST_H
#define LIST_H

#include <stdbool.h>

typedef struct list * list_t;

// A function signature for checking list elements.
typedef int (*listFindFun_t)(list_t listElem, list_t anotherElem);

list_t list_find(list_t list, listFindFun_t fun, list_t singleton);

#endif
