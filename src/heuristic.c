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

int
heuristic_estimate(struct probSpace *probSpace, struct actionList *actL)
{
  list_t gaps = aStarPlanner_getAllGaps(probSpace, actL);

  list_t listGapActL = NULL; // Empty list

  // Make a list of gap-set-action-list-compositions.
  for (list_t gapE = gaps; gapE != NULL; gapE = gapE->next) {
    struct gap *gap = (struct gap *)gapE->payload;

    struct gapActL *gapActL = malloc(sizeof(*gapActL));
    gapActL->gaps = list_createElem(gap); // singleton
    gapActL->acts = utils_cloneActionListShallow(ps_getActsToFixGap(probSpace,
                                                                gap->literal));
    listGapActL = list_push(listGapActL, list_createElem(gapActL));
  }

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
        gal1E->next = gal2E->next;
        gal2E->next = NULL;
        list_free(gal2E);

        gal2E = gal2ENext; // Advance loop variable
        continue;
      }
      gal2E = gal2E->next; // Advance loop variable
    }
  }

  int length = list_length(listGapActL);

  list_freeWithPayload(listGapActL, &freeGapActL);

  return length;
}

