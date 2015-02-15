#include <stdlib.h>

#include "list.h"


struct list {
  void *payload;

  struct list *prev;
  struct list *next;

  int intValue;
};

list_t
list_initElem(list_t listElem,
              void *payload,
              list_t prev,
              list_t next,
              int intValue)
{
  listElem->payload = payload;
  listElem->prev = prev;
  listElem->next = next;
  listElem->intValue = intValue;
  return listElem;
}

list_t
list_createElem(void *payload)
{
  list_t elem = malloc(sizeof(*elem));
  return list_initElem(elem, payload, NULL, NULL, -1);
}

list_t
list_push(list_t list, list_t singleton)
{
  singleton->next = list;
  if (list != NULL) {
    list->prev = singleton;
  }
  return singleton;
}

list_t
list_removeFirst(list_t list)
{
  if (list == NULL) {
    return NULL;
  }

  list_t first = list;
  // TODO: What to do with the first element? Either free or reuse.
  // This is a memory leak.
  first->next = NULL;

  list_t second = first->next;
  second->prev = NULL;
  return second;
}

typedef int (*listFindFun_t)(list_t listElem, list_t anotherElem);

list_t
list_find(list_t list, listFindFun_t fun, list_t singleton)
{
  // Singleton can be NULL on purpose.
  if (list == NULL || fun == NULL) {
    return NULL;
  }

  if ((*fun)(list, singleton) < 0) {
    return NULL;
  }

  while (list != NULL && (*fun)(list, singleton) != 0) {
    list = list->next;
  }

  return list;
}

/* list_t */
/* list_insertAfter(list_t list, list_t singleton, listFindFun_t fun) */
/* { */
/*   list_t found = list_find(list, fun); */

/*   if (found == NULL) { */
/*     // Insert as first element. */
/*     singleton->next = list; */
/*     if (list != NULL) { */
/*       list->prev = singleton; */
/*     } */
/*     return singleton; */
/*   } */


/* } */
