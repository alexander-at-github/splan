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
list_t list_removeFirstWOReuse(list_t list);
list_t list_removeFirst(list_t list);
list_t list_remove(list_t list, list_t elem);
list_t list_find(list_t list, listFindFun_t fun, void *payload);


void list_free(list_t list);
void list_freeWithPayload(list_t list, freePayload_t freeFun);
void *list_getFirstPayload(list_t list);

void list_print(list_t list, printF_t fun);

list_t list_cloneShallow(list_t src);

int list_length(list_t list);
list_t list_reverse(list_t list);

void list_cleanupLEBuffer();

list_t list_concat(list_t l1, list_t l2);

typedef bool (*equalFun_t)(void *, void *);

bool list_equal(equalFun_t eqFun, list_t l1, list_t l2);

#endif
