#ifndef ASNLIST_H
#define ASNLIST_H

#include "list.h"
#include "pddl31structs.h"
#include "utils.h"

/* This module extends a list (list.h) of A-Star nodes (which are action lists)
 * to a list with an index in order to find elements in the list fast.
 * This collection behaves like a set. Each element will only be once
 * referenced by the index.
 * */

// In the A-Star algorithm a node is an action list.
typedef struct actionList * aStarNode_t;

/* A tree which will be an index into the list. */
typedef struct treeNode * asnTree_t;

enum treeNodeType
{
  UNSET,
  ACTION,
  TERM
};

struct treeNode {
  // The type specifies of what type the edge labels of edges from this
  // node to its children are.
  enum treeNodeType type;

  int numOfChldrn;
  int numAlloced;
  // Array of labeled edges to children.
  struct treeNodeAE *chldrn;

  list_t pointerIntoList;
};

// Tree node array element. It is composition of either an action pointer and
// a child pointer or a term pointer and a child pointer.
struct treeNodeAE {
  void *edgeLabel;
  //union {
  //  struct action *act;
  //  struct term * term;
  //} edgeLabel;

  struct treeNode *chld;
};

/* An a-star node list is composed of a list of a-star nodes and an index
 * (which is a tree) into this list.
 * */
typedef struct asnList * asnList_t;

struct asnList {
  list_t list;
  asnTree_t tree;
};

// A comparison function for A-Star nodes.
typedef int (*elTreeCompFun)(aStarNode_t, aStarNode_t);

void asnList_cleanup();
asnList_t asnList_createEmpty();
bool asnList_isEmpty(asnList_t list);
asnList_t asnList_push(asnList_t asnl, aStarNode_t asn);
asnList_t asnList_remove(asnList_t asnl, aStarNode_t asn);
asnList_t asnList_removeFirst(asnList_t asnl);
void *asnList_getFirstPayload(asnList_t asnl);
asnList_t asnList_insertOrdered(asnList_t asnl,
                                aStarNode_t asn,
                                int intValue);
list_t asnList_find(asnList_t asnl, aStarNode_t asn);

void asnTreePrint(asnTree_t tree);

#endif
