#include <assert.h>
#include <stdlib.h>

#include "aStarPlanner.h"

#include "heuristic.h"

// Composition of a set of gaps and a set of actions which will fix all gaps
// in the before mentioned set.
struct gapActL {
  // Set of gaps
  list_t gaps;
  // Set of actions to fix this gap
  struct actionList *acts;
};

static
void
freeGap(void *gap)
{
  utils_free_gap((struct gap *) gap);
}

static
void
freeGapActL(void *gapActL_)
{
  struct gapActL *gapActL = (struct gapActL *) gapActL_;
  list_freeWithPayload(gapActL->gaps, &freeGap);
  utils_free_actionListShallow(gapActL->acts);
  free(gapActL);
}

static
void
freeActL(void *actL)
{
  utils_free_actionListShallow((struct actionList *) actL);
}

static
void
printGap(void *vGap)
{
  utils_print_gap((struct gap *) vGap);
}

static
struct actionList *
intersect(struct actionList *al1, struct actionList *al2)
{
  struct actionList *result = NULL;

  struct actionList *al1Tmp = NULL;
  struct actionList *al2Tmp = NULL;
  for (al1Tmp = al1; al1Tmp != NULL; al1Tmp = al1Tmp->next) {
    for (al2Tmp = al2; al2Tmp != NULL; al2Tmp = al2Tmp->next) {
      struct groundAction *grAct1 = al1Tmp->act;
      struct groundAction *grAct2 = al2Tmp->act;
      if (utils_grAct_equal(grAct1, grAct2)) {
        // Add ground action to set of intersction.
        // (0 will add to the fornt of the list)
        result = utils_addActionToListAtPosition(result, grAct1, 0);
      }
    }
  }
  return result;
}

static
int
min(int i1, int i2)
{
  return i1 < i2 ? i1 : i2;
}

static
void
printActionList(void *al)
{
  utils_print_actionListCompact(al);
}

static
bool
heuristic_actionListEqual(void *al1, void *al2)
{
  // typedef struct actionList * aStarNode_t;
  return utils_actionListsEqual(al1, al2);
}

// The argument listActL must be a list of struct actionList *.
static
list_t
heuristic_removeDuplicateActL(list_t listActL, bool freePayload)
{
  //printf("### 10: \n");
  //list_print(listActL, &printActionList);
  //printf("\n");

  for (list_t lal1E = listActL; lal1E != NULL; lal1E = lal1E->next) {
    for (list_t lal2E = lal1E->next; lal2E != NULL; /*empty*/) {

      if (utils_actionListsEqual(lal1E->payload, lal2E->payload)) {
        // Remove one of them from the list.
        list_t lal2ENext = lal2E->next;
        assert (lal2E->prev != NULL);
        lal2E->prev->next = lal2E->next;
        if (lal2E->next != NULL) {
          lal2E->next->prev = lal2E->prev;
        }
        lal2E->next = NULL;
        lal2E->prev = NULL;

        if (freePayload) {
          list_freeWithPayload(lal2E, &freeActL);
        } else {
          list_free(lal2E);
        }

        lal2E = lal2ENext; // Loop variable
        continue;
      }
      lal2E = lal2E->next; // Loop variable
    }
  }

  //printf("### 11: \n");
  //list_print(listActL, &printActionList);
  //printf("\n");

  return listActL;
}

// finds smallest number of actions, such that, from each set (= action list)
// at least one action is chosen.
static
int
heuristic_findSmallestNumOfActions(list_t listActL_) // A list of action lists.
{
  // Clone, because we want to remove duplicates.
  list_t listActL = list_cloneShallow(listActL_);
  listActL = heuristic_removeDuplicateActL(listActL, false);
  int result = list_length(listActL);

  //int id = rand();

  //printf("###-2: id: %d length: %d;  ", id, list_length(listActL));
  //list_print(listActL, &printActionList);
  //printf("\n");

  for (list_t lal1E = listActL; lal1E != NULL; lal1E = lal1E->next) {
    for (list_t lal2E = lal1E->next; lal2E != NULL; /*empty*/) {
      struct actionList *al1 = (struct actionList *) lal1E->payload;
      struct actionList *al2 = (struct actionList *) lal2E->payload;

      struct actionList *is = intersect(al1, al2);

      if (is != NULL) {
        /* A subset can be formed. We need to recurse. */

        //printf("###0: id: %d i1: %d i2: %d ", id, i1, i2);
        //printf("length %d; ", list_length(listActL));
        //list_print(listActL, &printActionList);

        //printf("###0.1: id: %d ", id);
        //utils_print_actionListCompact(is);
        //printf("\n");
        //printf("\n");

        //printf("###1: ");
        //list_print(listActL, &printActionList);
        //printf("\n");

        // Remove lal2E from list.
        list_t lal2ENext = lal2E->next;
        assert (lal2E->prev != NULL);
        lal2E->prev->next = lal2E->next;
        if (lal2E->next != NULL) {
          lal2E->next->prev = lal2E->prev;
        }
        // Note: lal2E->next and lal2E->prev are still set.

        // Backup the actionList in lal1E
        void *lal1EPayBack = lal1E->payload; // backup

        // Set lal1E payload to intersecting set.
        lal1E->payload = is;

        // Recurse. Note that lal1E and lal2E are elements of listActL. When we
        // modified them, we modified the list listActL.
        int subResult = heuristic_findSmallestNumOfActions(listActL);

        result = min (result, subResult);

        /* Restore listActL. */
        lal1E->payload = lal1EPayBack;
        // Reinsert lal2E.
        assert (lal2E->prev != NULL);
        lal2E->prev->next = lal2E;
        if (lal2E->next != NULL) {
          lal2E->next->prev = lal2E;
        }

        // Free the intersecting set.
        utils_free_actionListShallow(is);

        //printf("###2: ");
        //list_print(listActL, &printActionList);
        //printf("\n");

        lal2E = lal2ENext; // Advance loop variable.
        continue;
      }
      lal2E = lal2E->next; // Advance loop variable.
    }
  }

  list_free(listActL);
  return result;
}

// Returns an estimate solution cost. If there does not exist an solution,
// then it returns -1.
int
heuristic_estimate(struct probSpace *probSpace, struct actionList *actL)
{
  list_t gaps = aStarPlanner_getAllGaps(probSpace, actL);

  //printf("## START heuristic_estimate()\n");
  //printf("number of gaps: %d\n", list_length(gaps));
  //list_print(gaps, &printGap);
  //printf("actL: ");
  //utils_print_actionListCompact(actL);
  //printf("\n\n");

  // A list of action lists.
  list_t listActL = NULL; // Empty list
  for (list_t gapE = gaps; gapE != NULL; gapE = gapE->next) {
    struct gap *gap = (struct gap *)gapE->payload;

    struct actionList *actsToFixGap = ps_getActsToFixGap(probSpace,
                                                         gap->literal);
    if (actsToFixGap == NULL) {
      // There is no way to fix the gap.
      // Cleanup
      list_freeWithPayload(gaps, &freeGap);
      // Free without payload since the payloads are the action lists from the
      // problem space. They will be needed there again.
      list_free(listActL);
      //printf("result: -1\n");
      //printf("## END heuristic_estimate()\n");
      return -1;
    }

    listActL = list_push(listActL, list_createElem(actsToFixGap));
  }
  // We don't need the gaps itself anymore.
  list_freeWithPayload(gaps, &freeGap);

  // Removeing duplicates will save us a lot of work.
  listActL = heuristic_removeDuplicateActL(listActL, false);

  // Explanation: Each gap needs to be fixed. To fix a gap we just use an
  // action which will do so. The problem space will give us this set of
  // actions. Now we need to find the smallest number of actions, such that
  // each gap can be fixed.
  int result = heuristic_findSmallestNumOfActions(listActL);

  // Free without payload since the payloads are the action lists from the
  // problem space. They will be needed there again.
  list_free(listActL);

  //printf("result: %d\n", result);
  //printf("## END heuristic_estimate()\n");

  assert (result >= 0);
  return result;
}


// Returns an estimate solution cost. If there does not exist an solution,
// then it returns -1.
// Just a greedy search. Not admissible.
int
heuristic_estimate_NOT_admissible(struct probSpace *probSpace, struct actionList *actL)
{
  list_t gaps = aStarPlanner_getAllGaps(probSpace, actL);

  //printf("## START heuristic_estimate_NOT_admissible()\n");
  //printf("number of gaps: %d\n", list_length(gaps));
  //list_print(gaps, &printGap);

  list_t listGapActL = NULL; // Empty list

  // Make a list of gap-set-action-list-compositions.
  for (list_t gapE = gaps; gapE != NULL; gapE = gapE->next) {
    struct gap *gap = (struct gap *)gapE->payload;

    struct actionList *actsToFixGap = ps_getActsToFixGap(probSpace,
                                                         gap->literal);
    if (actsToFixGap == NULL) {
      // There is no way to fix the gap.
      // Cleanup
      if (gapE->prev != NULL) {
        // Split the gaps-list to free it properly.
        gapE->prev->next = NULL;
        list_free(gaps);
      }
      list_freeWithPayload(gapE, &freeGap);
      list_freeWithPayload(listGapActL, &freeGapActL);
      return -1;
    }

    struct gapActL *gapActL = malloc(sizeof(*gapActL));
    gapActL->gaps = list_createElem(gap); // singleton

    gapActL->acts = utils_cloneActionListShallow(actsToFixGap);

    listGapActL = list_push(listGapActL, list_createElem(gapActL));
  }
  list_free(gaps); // Free shallow, cause the gaps itself are still used.

  for (list_t gal1E = listGapActL; gal1E != NULL; gal1E = gal1E->next) {
    for (list_t gal2E = gal1E->next; gal2E != NULL; /* empty */) {
      struct gapActL *gal1 = gal1E->payload;
      struct gapActL *gal2 = gal2E->payload;

      struct actionList *intersectSet = intersect(gal1->acts, gal2->acts);

      if (intersectSet != NULL) {
        gal1->gaps = list_concat(gal1->gaps, gal2->gaps);

        utils_free_actionListShallow(gal1->acts);
        utils_free_actionListShallow(gal2->acts);

        gal1->acts = intersectSet;

        // Remove gal2E from list
        list_t gal2ENext = gal2E->next;
        assert(gal2E->prev != NULL);
        gal2E->prev->next = gal2E->next;
        if (gal2E->next != NULL) {
          gal2E->next->prev = gal2E->prev;
        }
        gal2E->next = NULL;
        gal2E->prev = NULL;

        //list_freeWithPayload(gal2E, &freeGapActL); // Error
        free(gal2E->payload);
        list_free(gal2E);
        gal2E = NULL;

        gal2E = gal2ENext; // Advance loop variable
        continue;
      }
      gal2E = gal2E->next; // Advance loop variable
    }
  }

  int length = list_length(listGapActL);

  list_freeWithPayload(listGapActL, &freeGapActL);

  //printf("result: %d\n", length);
  //printf("## END heuristic_estimate_NOT_admissible()\n");

  assert(length >= 0);
  return length;
}
