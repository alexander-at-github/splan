#ifndef LIST_H
#define LIST_H

#include <stdbool.h>

struct list {
  void *payload;

  struct list *prev;
  struct list *next;

  int intValue;
};

typedef struct list * list_t;

// A function signature for checking list elements.
typedef int (*listFindFun_t)(list_t listElem, void *payload);

// A function to free payloads.
typedef void (*freePayload_t)(void *);

// A function to print payloads in list elements.
typedef void (*printF_t)(void *);

bool list_isEmpty(list_t list);
list_t list_createElem(void *payload);
list_t list_push(list_t list, list_t singleton);
list_t list_removeFirst(list_t list);
list_t list_remove(list_t list, list_t elem);
list_t list_find(list_t list, listFindFun_t fun, void *payload);

void list_freeWithPayload(list_t list, freePayload_t freeFun);
void *list_getFirstPayload(list_t list);

void list_print(list_t list, printF_t fun);

#endif
