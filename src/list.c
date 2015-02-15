#include <assert.h>
#include <stdlib.h>

#include "list.h"


bool
list_isEmpty(list_t list)
{
  return list == NULL;
}

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

void *
list_getFirstPayload(list_t list)
{
  if (list == NULL) {
    return NULL;
  }

  return list->payload;
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
  if (second != NULL) {
    second->prev = NULL;
  }
  return second;
}

//typedef int (*listFindFun_t)(list_t listElem, void *payload);

list_t
list_find(list_t list, listFindFun_t fun, void *payload)
{
  // payload can be NULL on purpose.
  if (list == NULL || fun == NULL) {
    return NULL;
  }

  if ((*fun)(list, payload) < 0) {
    return NULL;
  }

  while (list != NULL && (*fun)(list, payload) != 0) {
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

list_t
list_remove(list_t list, list_t elem)
{
  if (list == NULL || elem == NULL) {
    return list;
  }

  if (list == elem) {
    // Remove first element.
    return list_removeFirst(list);
  }

  assert(elem->prev != NULL);
  elem->prev->next = elem->next;
  if (elem->next != NULL) {
    elem->next->prev = elem->prev;
  }

  // TODO: What to do with the element? Free or reuse.
  elem->next = NULL;
  elem->prev = NULL;

  return list;
}

typedef void (*freePayload_t)(void *);

void
list_freeWithPayload(list_t list, freePayload_t freeFun)
{
  while (list != NULL) {
    list_t listNext = list->next;
    // TODO: Maybe reuse the list elements.
    (*freeFun)(list->payload);
    free(list);
    list = listNext;
  }
}
