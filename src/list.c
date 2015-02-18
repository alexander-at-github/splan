#include <assert.h>
#include <stdio.h>
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

  list_t second = first->next;
  if (second != NULL) {
    second->prev = NULL;
  }

  // TODO: What to do with the first element? Either free or reuse.
  // This is a memory leak.
  first->next = NULL;

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

list_t
list_cloneShallow(list_t src)
{
  list_t head = NULL;
  list_t curr = NULL;
  while (src != NULL) {
    list_t tmp = list_createElem(src->payload);
    tmp->intValue = src->intValue;

    if (head == NULL) {
      head = tmp;
      curr = head;
    } else {
      curr->next = tmp;
      tmp->prev = curr;
      curr = curr->next;
    }
    src = src->next;
  }
  return head;
}

void
list_free(list_t list)
{
  while (list != NULL) {
    list_t listNext = list->next;
    free(list);
    list = listNext;
  }
}

//typedef void (*printF_t)(void *);

void
list_print(list_t list, printF_t fun)
{
  printf("List:(\n");
  for ( /* empty */; list != NULL; list = list->next) {
    printf("%d_", list->intValue);
    (*fun)(list->payload);
    printf("\n");
  }
  printf(")\n");
}

int
list_length(list_t list)
{
  int length = 0;
  while (list != NULL) {
    length++;
    list = list->next;
  }
  return length;
}
