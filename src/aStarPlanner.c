#include <assert.h>
#include <time.h>

#include "asnlist.h"
#include "aStarPlanner.h"
#include "heuristic.h"
#include "trie.h"

#include "planner.h"

// Set the time interval to see number of expanded nodes when running
#define OUTPUT_UPDATE_INTERVAL (CLOCKS_PER_SEC * 10/*sec*/)

static clock_t startTime = 0;
static int timeout = -1; // in clocks

// Here we use the old implementation of actionList, since there are many 
// useful function in utils.c to be used.
typedef struct actionList * aStarNode_t;

static
void
aStarPlanner_applyEffElemPos(trie_t trie,
                struct groundAction *grAct,
                struct effectElem *effElem)
{
  assert (effElem != NULL);
  if (effElem->type != POS_LITERAL) {
    return;
  }
  trie_addGr(trie, effElem->it.literal, grAct);
}

static
void
aStarPlanner_applyEffElemNeg(trie_t trie,
                struct groundAction *grAct,
                struct effectElem *effElem)
{
  assert (effElem != NULL);
  if (effElem->type != NEG_LITERAL) {
    return;
  }
  trie_removeGr(trie, effElem->it.literal, grAct);
}

typedef void (*applyEffElemFun_t)(trie_t,
                                struct groundAction *,
                                struct effectElem *);

static
void
aStarPlanner_applyF(trie_t trie,
               struct groundAction *grAct,
               applyEffElemFun_t applyEffElemFun)
{
  assert (trie != NULL);
  assert (grAct != NULL);

  struct effect *effCurr = grAct->action->effect;

  for (int32_t idx = 0; idx < effCurr->numOfElems; ++idx) {
    struct effectElem *effElem = &effCurr->elems[idx];
    (*applyEffElemFun)(trie, grAct, effElem);
  }
}

static
void
aStarPlanner_applyPos(trie_t trie, struct groundAction *grAct)
{
  aStarPlanner_applyF(trie, grAct, &aStarPlanner_applyEffElemPos);
}

static
void
aStarPlanner_applyNeg(trie_t trie, struct groundAction *grAct)
{
  aStarPlanner_applyF(trie, grAct, &aStarPlanner_applyEffElemNeg);
}

static
void
aStarPlanner_apply(trie_t trie, struct groundAction *grAct)
{
  aStarPlanner_applyPos(trie, grAct);
  aStarPlanner_applyNeg(trie, grAct);
}

// A comparison function for finding locations in lists for inserting
// elements in an ordered manner by their intValue property.
static
int
compIntValueFunAfter(list_t e1, void *fScore_)
{
  if (e1 == NULL || fScore_ == NULL) {
    assert(false);
  }
  int fScore = *(int *) fScore_;

  if (e1->intValue > fScore) {
    return -1;
  }
  assert(e1->intValue <= fScore);

  if (e1->next == NULL) {
    return 0;
  }
  if (e1->next->intValue > fScore) {
    return 0;
  }

  return 1;
}

// A comparison function for finding locations in lists for inserting
// elements in an ordered manner by their intValue property. New elements will
// be inserted before other elements with the same intValue.
static
int
compIntValueFunBefore(list_t e1, void *fScore_)
{
  if (e1 == NULL || fScore_ == NULL) {
    assert(false);
  }
  int fScore = *(int *) fScore_;

  if (e1->intValue >= fScore) {
    return -1;
  }
  assert(e1->intValue < fScore);

  if (e1->next == NULL) {
    return 0;
  }
  if (e1->next->intValue >= fScore) {
    return 0;
  }

  return 1;
}


static
bool
aStarPlanner_aStarNodeEqual(aStarNode_t n1, aStarNode_t n2)
{
  // Note: typedef struct actionList * aStarNode_t;
  if (n1 == NULL && n2 == NULL) {
    return true;
  }
  if (n1 == NULL || n2 == NULL) {
    return false;
  }

  while (n1 != NULL && n2 != NULL) {
    struct groundAction *ga1 = n1->act;
    struct groundAction *ga2 = n2->act;
    if ( ! utils_grAct_equal(ga1, ga2)) {
      return false;
    }
    n1 = n1->next;
    n2 = n2->next;
  }
  if (n1 == NULL && n2 == NULL) {
    return true;
  }
  return false;
}


// A comparison function for finding a sequence of ground actions in a list
// of sequences of ground actions.
static
int
compActList(list_t e1, void *e2)
{
  if (e1 == NULL || e2 == NULL) {
    assert(false);
  }
  aStarNode_t n1 = (aStarNode_t) e1->payload;
  aStarNode_t n2 = (aStarNode_t) e2;

  if ( ! aStarPlanner_aStarNodeEqual(n1, n2)) {
    return 1;
  }

  // Both action lists equal.
  return 0;
}

// Arguments are a list of aStarNode_t and a aStarNode_t in a payload list.
static
list_t
asnl_find(list_t list, aStarNode_t payload)
{
  return list_find(list, &compActList, payload);
}

static
list_t
asnl_insertOrdered(list_t list, void *payload, int fScore)
{
  if (payload == NULL) {
    return list;
  }
  // If list == NULL then it represents the empty list.

  /* aStarNodeList_t head = list; */

  /* aStarNodeList_t beforeCurr = NULL; */
  /* aStarNodeList_t curr = list; */
  /* while (curr != NULL && curr->fScore <= payload->fScore) { */
  /*   beforeCurr = curr; */
  /*   curr = curr->next; */
  /* } */
  /* assert(curr == NULL || curr->fScore > payload->fScore); */

  /* payload->next = curr; */
  /* if (beforeCurr == NULL) { */
  /*   head = payload; */
  /* } else { */
  /*   beforeCurr->next = payload; */
  /* } */

  /* return head; */

  list_t singleton = list_createElem(payload);
  singleton->intValue = fScore;

  //list_t found = list_find(list, &compIntValueFunAfter, &fScore);
  list_t found = list_find(list, &compIntValueFunBefore, &fScore);
  // The new element should now be inserted right after "found".
  if (found == NULL) {
    // Insert new element as head.
    singleton->next = list;
    if (list != NULL) {
      list->prev = singleton;
    }
    return singleton;
  }

  if (found->next != NULL) {
    found->next->prev = singleton;
  }
  singleton->next = found->next;
  found->next = singleton;
  singleton->prev = found;

  return list;
}

// Note: typedef void (*freePayload)(void *);
static
void
freeGap(void *gap)
{
  utils_free_gap((struct gap *) gap);
}

list_t
aStarPlanner_getAllGaps(struct probSpace *probSpace, aStarNode_t actions)
{
  /* Note: typedef struct actionList * aStarNode_t; */

  trie_t state = trie_clone(probSpace->problem->init);
  struct goal *goal = probSpace->problem->goal;

  list_t result = NULL;
  struct literal *gapLiteral = NULL;

  /* Check action's precondition and apply the action. */

  // Index for position of gap. Starts with one, because the gap is sayed
  // to be before the index.
  int32_t idxAct = 1;

  for (struct actionList *pAct = actions;
       pAct != NULL;
       pAct = pAct->next, ++idxAct) {

    struct groundAction *grAct = pAct->act;
    // Check if precondition holds.
    struct goal *precond = grAct->action->precond;

    for (int32_t idx = 0; idx < precond->numOfPos; ++idx) {
      struct atom *atom = &precond->posLiterals[idx];
      if ( ! trie_containsGr(state, atom, grAct)) {
        // Positive literal not fulfilled.
        struct gap *gap = malloc(sizeof(*gap));
        gap->position = idxAct;
        gap->literal = malloc(sizeof(*gap->literal));
        gap->literal->atom = utils_atom_cloneWithGrounding(atom, grAct);
        gap->literal->isPos = true;
        result = list_push(result, list_createElem(gap));
      }
    }
    for (int32_t idx = 0; idx < precond->numOfNeg; ++idx) {
      struct atom *atom = &precond->negLiterals[idx];
      if (trie_containsGr(state, atom, grAct)) {
        // Negative literal not fulfilled.
        struct gap *gap = malloc(sizeof(*gap));
        gap->position = idxAct;
        gap->literal = malloc(sizeof(*gap->literal));
        gap->literal->atom = utils_atom_cloneWithGrounding(atom, grAct);
        gap->literal->isPos = false;
        result = list_push(result, list_createElem(gap));
      }
    }
    //printf("getAllGaps(): state before apply: ");
    //trie_print(state);
    //printf("\n");
    aStarPlanner_apply(state, grAct);
    //printf("getAllGaps(): state after apply: ");
    //trie_print(state);
    //printf("\n");
  }

  /* Check if the goal is met after applying all the actions. */

  for (int32_t idx = 0; idx < goal->numOfPos; ++idx) {
    struct atom *atom = &goal->posLiterals[idx];
    if ( ! trie_contains(state, atom)) {
      // Positive literal not fulfilled.
      struct gap *gap = malloc(sizeof(*gap));
      gap->position = idxAct;
      gap->literal = malloc(sizeof(*gap->literal));
      gap->literal->atom = utils_atom_clone(atom);
      gap->literal->isPos = true;
      result = list_push(result, list_createElem(gap));
    }
  }
  for (int32_t idx = 0; idx < goal->numOfNeg; ++idx) {
    struct atom *atom = &goal->negLiterals[idx];
    if (trie_contains(state, atom)) {
      // Negative literal not fulfilled.
      struct gap *gap = malloc(sizeof(*gap));
      gap->position = idxAct;
      gap->literal = malloc(sizeof(*gap->literal));
      gap->literal->atom = utils_atom_clone(atom);
      gap->literal->isPos = false;
      result = list_push(result, list_createElem(gap));
    }
  }

  trie_free(state);

  return result;
}

static
void
printGap(void *vGap)
{
  utils_print_gap((struct gap *) vGap);
}

// This a special return value for the function
// aStarPlanner_estimateCost_v1_fixGaps() in order to differenciate between
// cutoff (depth-limit met) and no solution exists.
struct estimateCost_fixGaps
{
  struct actionList *sol;
  int numActsAdded;
  bool cutoff;
  // not exists solution
  bool nESol;
};

static
struct estimateCost_fixGaps
aStarPlanner_estimateCost_v1_fixGaps(struct probSpace *probSpace,
                                  struct actionList *actL,
                                  list_t gaps_,
                                  int depthLimit,
                                  int depth)

{
  // Important, cause list_removeFirst() will alter the next-pointer of the
  // removed element.
  list_t gaps = list_cloneShallow(gaps_);

  //printf("aStarPlanner_estimateCost_v1_fixGaps() START, depth: %d\n", depth);
  //utils_print_actionListCompact(actL);
  //printf("\n");
  //printf("gaps to fix:\n");
  //list_print(gaps, &printGap);

  struct estimateCost_fixGaps result;
  result.sol = NULL;
  result.numActsAdded = -1;
  result.cutoff = false;
  result.nESol = false;

  if (gaps == NULL) {
    result.sol = utils_cloneActionListShallow(actL);
    result.numActsAdded = depth;
    //printf("aStarPlanner_estimateCost_v1_fixGaps() found result (to estimation)\n");
    return result;
  }

  if (depth >= depthLimit) {
    result.cutoff = true;
    //printf("CUTOFF\n");
    return result;
  }

  /* gap = pop(gaps); */
  struct gap *gap = (struct gap *) gaps->payload;
  gaps = list_removeFirst(gaps);
  //printf("gaps after removing first:\n");
  //list_print(gaps, &printGap);

  struct actionList *actsTFGap = ps_getActsToFixGap(probSpace, gap->literal);
  // Note: Do not free actsTFGap. This list is part of the problem space
  // data structure and will be used again.
  if (actsTFGap == NULL) {
    result.nESol = true;
    return result;
  }

  result.nESol = true;
  result.cutoff = true;
  trie_t lState0 = trie_clone(probSpace->problem->init);
  struct actionList *actL1 = actL;
  for (int idxPos = 0; idxPos < gap->position; ++idxPos) {
    for (struct actionList *currActE = actsTFGap;
         currActE != NULL;
         currActE = currActE->next) {

      actL = utils_addActionToListAtPosition(actL, currActE->act, idxPos);

      trie_t lState1 = trie_clone(lState0);

      list_t lGaps = list_cloneShallow(gaps);

      /* Collect new gaps. */
      struct groundAction *grAct = currActE->act;
      struct goal *precond = grAct->action->precond;
      if (gap->literal->isPos) {
        for (int32_t idx = 0; idx < precond->numOfPos; ++idx) {
          struct atom *atom = &precond->posLiterals[idx];
          if ( ! trie_containsGr(lState1, atom, grAct)) {
            // Positive literal not fulfilled.
            struct gap *gapNew = malloc(sizeof(*gapNew));
            gapNew->position = idxPos + 1; // +1 because idxPos starts with 0
            gapNew->literal = malloc(sizeof(*gapNew->literal));
            gapNew->literal->atom = utils_atom_cloneWithGrounding(atom, grAct);
            gapNew->literal->isPos = true;
            lGaps = list_push(lGaps, list_createElem(gapNew));
          }
        }
      } else {
        assert ( ! gap->literal->isPos);
        for (int32_t idx = 0; idx < precond->numOfNeg; ++idx) {
          struct atom *atom = &precond->negLiterals[idx];
          if (trie_containsGr(lState1, atom, grAct)) {
            // Negative literal not fulfilled.
            struct gap *gapNew = malloc(sizeof(*gapNew));
            gapNew->position = idxPos + 1; // +1 because idxPos starts with 0
            gapNew->literal = malloc(sizeof(*gapNew->literal));
            gapNew->literal->atom = utils_atom_cloneWithGrounding(atom, grAct);
            gapNew->literal->isPos = false;
            lGaps = list_push(lGaps, list_createElem(gapNew));
          }
        }
      }
      // TODO:TODO: Maybe consider also the gaps after the  new action.

      //utils_print_actionListCompact(actL);
      //if (lGaps != NULL) {
      //  printf("lGaps->next: %p\n",
      //         lGaps->next == NULL ? "NULL" : (void *) lGaps->next);
      //}
      //printf("\n### lGaps to fix new:\n");
      //list_print(lGaps, &printGap);
      //printf("\nlength: %d\n", list_length(lGaps));
      //if (lGaps != NULL) {
      //  printf("lGaps->next: %p\n",
      //         lGaps->next == NULL ? "NULL" : (void *) lGaps->next);
      //}

      /* Recursicve call */
      result = aStarPlanner_estimateCost_v1_fixGaps(probSpace,
                                                   actL,
                                                   lGaps,
                                                   depthLimit,
                                                   depth + 1);

      //printf("after recursive call lGaps:\n");
      //list_print(lGaps, &printGap);
      //printf("\nlength: %d\n", list_length(lGaps));
      list_free(lGaps);
      actL = utils_removeActionFromListAtPosition(actL, idxPos);

      trie_free(lState1);
      if (result.sol != NULL) {
        //printf("RETURN SOLUTION\n");
        trie_free(lState0);
        return result;
      }
      if ( ! result.nESol) {
        assert(result.cutoff);
        result.nESol = false;
      }
      if ( ! result.cutoff) {
        assert(result.nESol);
        result.cutoff = false;
      }
    }
    if (actL1 != NULL) {
      // FIXME: Should we only use positive or negative effects according to 
      // the gap ??? FIXME FIXME FIXME
      aStarPlanner_apply(lState0, actL1->act);
      actL1 = actL1->next; // Important!
    } else {
      // Last iteration of the loop.
      assert(idxPos == gap->position - 1);
    }
  }
  trie_free(lState0);

  return result;
}

/*** This function is not addmissible !!! ***/
static
int
aStarPlanner_estimateCost_v1(struct probSpace *probSpace, aStarNode_t node)
{
  //printf("aStarPlanner_estimateCost_v1(): START node: "); // DEBUG
  //utils_print_actionListCompact(node); // DEBUG
  //printf("\n"); // DEBUG

  int maxLength = 0;
  aStarNode_t longestSol = NULL;

  list_t gaps = aStarPlanner_getAllGaps(probSpace, node);
  //list_print(gaps, &printGap); // DEBUG
  //printf("\n"); // DEBUG

  for (list_t gapE = gaps;
       gapE != NULL;
       gapE = gapE->next) {

    struct gap *gap = (struct gap *) gapE->payload;
    struct literal *gapLiteral = gap->literal;

    list_t singletonGap = list_createElem(gap);
    aStarNode_t nodeClone = utils_cloneActionListShallow(node);

    struct actionList *estimateSolution = NULL;
    struct estimateCost_fixGaps esSol;

    int estimateLength = 0;
    for (int depthLimit = 0; depthLimit < INT_MAX; ++depthLimit) {
      //estimateSolution =
      esSol =
          aStarPlanner_estimateCost_v1_fixGaps(probSpace,
                                            nodeClone,
                                            singletonGap,
                                            depthLimit,
                                            0);

      //utils_free_actionListShallow(esSol.sol);

      if (esSol.cutoff) {
        continue;
      }
      if (esSol.nESol) { // No solution exists.
        //estimateLength = INT_MAX;
        //printf("INT_MAX is %d\n", INT_MAX); // DEBUG
        //fflush(stdout); // DEBUG
        maxLength = INT_MAX;
        break;
        //return INT_MAX;
      }

      estimateLength = esSol.numActsAdded;
      break;
    }

    if (estimateLength >= maxLength) {
      maxLength = estimateLength;

      utils_free_actionListShallow(longestSol);
      longestSol = esSol.sol; // Don't free but save for later use.
    } else {
      utils_free_actionListShallow(esSol.sol);
    }
  }

  printf("aStarPlanner_estimateCost_v1(): maxLength %d solution: ", maxLength);
  utils_print_actionListCompact(longestSol);
  printf("\n");
  utils_free_actionListShallow(longestSol);

  //printf("aStarPlanner_estimateCost_v1(): END result: %d\n", maxLength); // DEBUG

  return maxLength;
}

/* static */
/* struct estimateCost_fixGaps */
/* aStarPlanner_estimateCost_v2_fixGaps(struct probSpace *probSpace, */
/*                                   struct actionList *actL, */
/*                                   list_t gaps, */
/*                                   int depthLimit, */
/*                                   int depth) */

/* { */
/*   //printf("aStarPlanner_estimateCost_v2_fixGaps() START, depth: %d\n", depth); */
/*   //utils_print_actionListCompact(actL); */
/*   //printf("\n"); */
/*   //printf("gaps to fix:\n"); */
/*   //list_print(gaps, &printGap); */

/*   struct estimateCost_fixGaps result; */
/*   result.sol = NULL; */
/*   result.numActsAdded = -1; */
/*   result.cutoff = false; */
/*   result.nESol = false; */

/*   if (gaps == NULL) { */
/*     result.sol = utils_cloneActionListShallow(actL); */
/*     result.numActsAdded = depth; */
/*     //printf("aStarPlanner_estimateCost_v2_fixGaps() found result (to estimation)\n"); */
/*     return result; */
/*   } */

/*   if (depth >= depthLimit) { */
/*     result.cutoff = true; */
/*     //printf("CUTOFF\n"); */
/*     return result; */
/*   } */

/*   /1* gap = pop(gaps); *1/ */
/*   struct gap *gap = (struct gap *) gaps->payload; */
/*   gaps = list_removeFirst(gaps); */

/*   struct actionList *actsTFGap = ps_getActsToFixGap(probSpace, gap->literal); */
/*   // Note: Do not free actsTFGap. This list is part of the problem space */
/*   // data structure and will be used again. */
/*   if (actsTFGap == NULL) { */
/*     result.nESol = true; */
/*     return result; */
/*   } */

/*   result.nESol = true; */
/*   result.cutoff = true; */
/*   trie_t lState0 = trie_clone(probSpace->problem->init); */
/*   struct actionList *actL1 = actL; */
/*   for (int idxPos = 0; idxPos < gap->position; ++idxPos) { */
/*     for (struct actionList *currActE = actsTFGap; */
/*          currActE != NULL; */
/*          currActE = currActE->next) { */

/*       actL = utils_addActionToListAtPosition(actL, currActE->act, idxPos); */

/*       trie_t lState1 = trie_clone(lState0); */

/*       list_t lGaps = list_cloneShallow(gaps); */

/*       /1* Collect new gaps. *1/ */
/*       struct groundAction *grAct = currActE->act; */
/*       struct goal *precond = grAct->action->precond; */
/*       if (gap->literal->isPos) { */
/*         for (int32_t idx = 0; idx < precond->numOfPos; ++idx) { */
/*           struct atom *atom = &precond->posLiterals[idx]; */
/*           if ( ! trie_containsGr(lState1, atom, grAct)) { */
/*             // Positive literal not fulfilled. */
/*             struct gap *gapNew = malloc(sizeof(*gapNew)); */
/*             gapNew->position = idxPos + 1; // +1 because idxPos starts with 0 */
/*             gapNew->literal = malloc(sizeof(*gapNew->literal)); */
/*             gapNew->literal->atom = utils_atom_cloneWithGrounding(atom, grAct); */
/*             gapNew->literal->isPos = true; */
/*             lGaps = list_push(lGaps, list_createElem(gapNew)); */
/*           } */
/*         } */
/*       } else { */
/*         assert ( ! gap->literal->isPos); */
/*         for (int32_t idx = 0; idx < precond->numOfNeg; ++idx) { */
/*           struct atom *atom = &precond->negLiterals[idx]; */
/*           if (trie_containsGr(lState1, atom, grAct)) { */
/*             // Negative literal not fulfilled. */
/*             struct gap *gapNew = malloc(sizeof(*gapNew)); */
/*             gapNew->position = idxPos + 1; // +1 because idxPos starts with 0 */
/*             gapNew->literal = malloc(sizeof(*gapNew->literal)); */
/*             gapNew->literal->atom = utils_atom_cloneWithGrounding(atom, grAct); */
/*             gapNew->literal->isPos = false; */
/*             lGaps = list_push(lGaps, list_createElem(gapNew)); */
/*           } */
/*         } */
/*       } */
/*       // TODO: Maybe condsider all gaps! Positive and negative. */
/*       // Maybe consider also the gaps after the  new action. */



/*       //utils_print_actionListCompact(actL); */
/*       //printf("lGaps to fix new:\n"); */
/*       //list_print(lGaps, &printGap); */

/*       /1* Recursicve call *1/ */
/*       result = aStarPlanner_estimateCost_v2_fixGaps(probSpace, */
/*                                                    actL, */
/*                                                    lGaps, */
/*                                                    depthLimit, */
/*                                                    depth + 1); */

/*       list_free(lGaps); */
/*       actL = utils_removeActionFromListAtPosition(actL, idxPos); */

/*       trie_free(lState1); */
/*       if (result.sol != NULL) { */
/*         //printf("RETURN SOLUTION\n"); */
/*         trie_free(lState0); */
/*         return result; */
/*       } */
/*       if ( ! result.nESol) { */
/*         assert(result.cutoff); */
/*         result.nESol = false; */
/*       } */
/*       if ( ! result.cutoff) { */
/*         assert(result.nESol); */
/*         result.cutoff = false; */
/*       } */
/*     } */
/*     if (actL1 != NULL) { */
/*       // FIXME: Should we only use positive or negative effects according to */ 
/*       // the gap ??? FIXME FIXME FIXME */
/*       aStarPlanner_apply(lState0, actL1->act); */
/*       actL1 = actL1->next; // Important! */
/*     } else { */
/*       // Last iteration of the loop. */
/*       assert(idxPos == gap->position - 1); */
/*     } */
/*   } */
/*   trie_free(lState0); */
/*   return result; */
/* } */

static
int
max(int aa, int bb)
{
  return aa > bb ? aa : bb;
}

/*** This function is not admissible!!! ***/
/* static */
/* int */
/* aStarPlanner_estimateCost_v2(struct probSpace *probSpace, aStarNode_t node) */
/* { */
/*   //printf("aStarPlanner_estimateCost_v2(): START node: "); // DEBUG */
/*   //utils_print_actionListCompact(node); // DEBUG */
/*   //printf("\n"); // DEBUG */

/*   /1* int maxLength = 0; *1/ */

/*   list_t gaps = aStarPlanner_getAllGaps(probSpace, node); */
/*   //list_print(gaps, &printGap); // DEBUG */
/*   //printf(" list_length: %d\n", list_length(gaps)); // DEBUG */

/*   // Devide the gaps into positive and negative gaps. */
/*   list_t gapsPos = NULL; */
/*   list_t gapsNeg = NULL; */
/*   // Attention: The loop variable will be advanced by the call to */
/*   // list_removeFirst(). */
/*   while (gaps != NULL) { */
/*   //for (list_t ll = gaps; ll != NULL; ll = ll->next) { */
/*     //list_print(gaps, &printGap); // DEBUG */
/*     struct gap *gg = (struct gap *) gaps->payload; */
/*     gaps = list_removeFirst(gaps); // Advances loop variable "gaps". */
/*     // The call to list_removeFirst might free the memory for the list */
/*     // container. So we should not reuse it and use list_createElem() again */
/*     // later. */
/*     if (gg->literal->isPos) { */
/*       gapsPos = list_push(gapsPos, list_createElem(gg)); */
/*     } else { */
/*       assert( ! gg->literal->isPos); */
/*       gapsNeg = list_push(gapsNeg, list_createElem(gg)); */
/*     } */
/*   } */
/*   assert(gaps == NULL); */
/*   //printf("gapsPos: "); // DEBUG */
/*   //list_print(gapsPos, &printGap); // DEBUG */
/*   //printf("\n"); // DEBUG */
/*   //printf("gapsNeg: "); // DEBUG */
/*   //list_print(gapsNeg, &printGap); // DEBUG */
/*   //printf("\n"); // DEBUG */

/*   // Make a list which holds gapsPos (a list with all the positive gaps) and */
/*   // another list with gapsNeg (a list with all negative gaps). This list */
/*   // is a list of lists of gaps. */
/*   list_t lstLstGps = list_createElem(gapsPos); */
/*   lstLstGps = list_push(lstLstGps, list_createElem(gapsNeg)); */
/*   assert(list_length(lstLstGps) == 2); */

/*   int result = 0; */
/*   aStarNode_t longestSol = NULL; */

/*   for (list_t llgTmp = lstLstGps; llgTmp != NULL; llgTmp = llgTmp->next) { */
/*     list_t gapsTmp = (list_t) llgTmp->payload; */
/*     struct estimateCost_fixGaps esSol; */
/*     for (int depthLimit = 0; depthLimit < INT_MAX; ++depthLimit) { */
/*       esSol = aStarPlanner_estimateCost_v2_fixGaps(probSpace, */
/*                                                    node, */
/*                                                    gapsTmp, */
/*                                                    depthLimit, */
/*                                                    0); */
/*       //utils_free_actionListShallow(esSol.sol); */
/*       if (esSol.cutoff) { */
/*         continue; */
/*       } */
/*       if (esSol.nESol) { */
/*         result = INT_MAX; */
/*         break; */
/*       } */


/*       if (esSol.numActsAdded >= result) { */
/*         utils_free_actionListShallow(longestSol); */
/*         longestSol = esSol.sol; // Don't free but save for later use. */
/*       } else { */
/*         utils_free_actionListShallow(esSol.sol); */
/*       } */

/*       //printf("esSol.numActsAdded %d\n", esSol.numActsAdded); */
/*       result = max(result, esSol.numActsAdded); */
/*       break; */
/*     } */
/*   } */
/*   list_freeWithPayload(gapsPos, &freeGap); */
/*   list_freeWithPayload(gapsNeg, &freeGap); */

/*   printf("aStarPlanner_estimateCost_v2(): maxLength %d solution: ", result); */
/*   utils_print_actionListCompact(longestSol); */
/*   printf("\n"); */
/*   utils_free_actionListShallow(longestSol); */


/*   //printf("aStarPlanner_estimateCost_v2(): END result: %d\n", result); // DEBUG */
/*   return result; */
/* } */

// TODO: aStarPlanner_estimateCost_v3(): Solve the problem fully, but only
// consider negative or positive effects.

static
struct literal *
aStarPlanner_satisfiesPosPrecond(trie_t state, struct groundAction *grAct)
{
  if (grAct == NULL) {
    return NULL;
  }

  struct goal *precond = grAct->action->precond;

  for (int i = 0; i < precond->numOfPos; ++i) {
    struct atom *atom = &precond->posLiterals[i];
    if ( ! trie_containsGr(state, atom, grAct)) {
      struct literal *returnVal = malloc(sizeof(*returnVal));
      returnVal->atom = utils_atom_cloneWithGrounding(atom, grAct);
      returnVal->isPos = true;
      return returnVal;
    }
  }
  return NULL;
}

static
struct literal *
aStarPlanner_satisfiesNegPrecond(trie_t state, struct groundAction *grAct)
{
  if (grAct == NULL) {
    return NULL;
  }

  struct goal *precond = grAct->action->precond;

  for (int i = 0; i < precond->numOfNeg; ++i) {
    struct atom *atom = &precond->negLiterals[i];
    if (trie_containsGr(state, atom, grAct)) {
      struct literal *returnVal = malloc(sizeof(*returnVal));
      returnVal->atom = utils_atom_cloneWithGrounding(atom, grAct);
      returnVal->isPos = false;
      return returnVal;
    }
  }
  return NULL;
}

static
struct literal *
aStarPlanner_satisfiesPos(trie_t state, struct goal *goal)
{
  if (goal == NULL) {
    // An empty goal is always satisfied.
    return NULL;
  }

  for (int32_t i = 0; i < goal->numOfPos; ++i) {
    struct atom *atom = &goal->posLiterals[i];
    if ( ! trie_contains(state, atom)) {
      struct literal *returnVal = malloc(sizeof(*returnVal));
      returnVal->atom = utils_atom_clone(atom);
      returnVal->isPos = true;
      return returnVal;
    }
  }
  return NULL;
}

static
struct literal *
aStarPlanner_satisfiesNeg(trie_t state, struct goal *goal)
{
  if (goal == NULL) {
    // An empty goal is always satisfied.
    return NULL;
  }

  for (int32_t i = 0; i < goal->numOfNeg; ++i) {
    struct atom *atom = &goal->negLiterals[i];
    if (trie_contains(state, atom)) {
      struct literal *returnVal = malloc(sizeof(*returnVal));
      returnVal->atom = utils_atom_clone(atom);
      returnVal->isPos = false;
      return returnVal;
    }
  }
  return NULL;
}

static
struct gap *
aStarPlanner_getFirstPosGap(struct probSpace *probSpace, aStarNode_t actL)
{
  trie_t lState = trie_clone(probSpace->problem->init);
  struct goal *goal = probSpace->problem->goal;

  struct gap *result = NULL;
  struct literal *gapLiteral = NULL;

  // Index for position of gap. Starts with one, because the gap is sayed
  // to be before the index.
  int idxAct = 1;

  for (struct actionList *actLP = actL;
       actLP != NULL;
       actLP = actLP->next, ++idxAct) {

    struct groundAction *grAct = actLP->act;
    gapLiteral = aStarPlanner_satisfiesPosPrecond(lState, grAct);
    if (gapLiteral != NULL) {
      // Positive precondition is not met.
      result = malloc(sizeof(*result));
      result->literal = gapLiteral;
      result->position = idxAct;
      // Clean up
      trie_free(lState);
      return result;
    }
    aStarPlanner_applyPos(lState, grAct);
  }

  // After applying all the actions check if the goal is met.
  gapLiteral = aStarPlanner_satisfiesPos(lState, goal);
  if (gapLiteral != NULL) {
    result = malloc(sizeof(*result));
    result->literal = gapLiteral;
    result->position = idxAct;
  }
  trie_free(lState);
  return result;
}

static
struct gap *
aStarPlanner_getFirstNegGap(struct probSpace *probSpace, aStarNode_t actL)
{
  trie_t lState = trie_clone(probSpace->problem->init);
  struct goal *goal = probSpace->problem->goal;

  struct gap *result = NULL;
  struct literal *gapLiteral = NULL;

  // Index for position of gap. Starts with one, because the gap is sayed
  // to be before the index.
  int idxAct = 1;

  for (struct actionList *actLP = actL;
       actLP != NULL;
       actLP = actLP->next, ++idxAct) {

    struct groundAction *grAct = actLP->act;
    gapLiteral = aStarPlanner_satisfiesNegPrecond(lState, grAct);
    if (gapLiteral != NULL) {
      // Negative precondition is not met.
      result = malloc(sizeof(*result));
      result->literal = gapLiteral;
      result->position = idxAct;
      // Clean up
      trie_free(lState);
      return result;
    }
    aStarPlanner_applyNeg(lState, grAct);
  }

  // After applying all the actions check if the goal is met.
  gapLiteral = aStarPlanner_satisfiesNeg(lState, goal);
  if (gapLiteral != NULL) {
    result = malloc(sizeof(*result));
    result->literal = gapLiteral;
    result->position = idxAct;
  }
  trie_free(lState);
  return result;
}

static struct estimateCost_fixGaps
aStarPlanner_estimateCost_v3_fixGaps(struct probSpace *probSpace,
                                     struct actionList *actL,
                                     int depthLimit,
                                     int depth,
                                     bool positive) // when true fixes only
                                                    // positive gaps. When
                                                    // false fixes only
                                                    // negative gaps.
{
  //printf("aStarPlanner_estimateCost_v3_fixGaps() START, "
  //       "%s, depth: %d\n",
  //       positive ? "positive" : "negative",
  //       depth);
  //utils_print_actionListCompact(actL);
  //printf("\n");

  struct estimateCost_fixGaps result;
  result.sol = NULL;
  result.numActsAdded = -1;
  result.cutoff = false;
  result.nESol = false;

  clock_t endTime = clock();
  if (timeout >= 0) {
    if ((endTime - startTime) > timeout) {
      // It is a timeout. We just say 'no solution exists' in order to
      // terminate the function.
      // Without that the timeout does not work.
      result.nESol = true;
      return result;
    }
  }

  struct gap *gap = NULL;
  if (positive) {
    gap = aStarPlanner_getFirstPosGap(probSpace, actL);
  } else {
    gap = aStarPlanner_getFirstNegGap(probSpace, actL);
  }
  if (gap == NULL) {
    //printf("aStarPlanner_estimateCost_v3_fixGaps() END\n");
    //printf("\n");
    result.sol = utils_cloneActionListShallow(actL);
    result.numActsAdded = depth;
    return result;
  }
  //printf("aStarPlanner_estimateCost_v3_fixGaps(): gap: ");
  //utils_print_gap(gap);
  //printf("\n");

  if (depth >= depthLimit) {
    //printf("aStarPlanner_estimateCost_v3_fixGaps() END\n");
    //printf("\n");
    utils_free_gap(gap);
    result.cutoff = true;
    return result;
  }

  struct actionList *actsTFGap = ps_getActsToFixGap(probSpace, gap->literal);
  // Note: Do not free actsTFGap. This list is part of the problem space
  // data structure and will be used again.
  if (actsTFGap == NULL) {
    //printf("aStarPlanner_estimateCost_v3_fixGaps() END\n");
    //printf("\n");
    result.nESol = true;
    return result;
  }

  result.nESol = true;
  result.cutoff = true;
  for (int idxPos = 0; idxPos < gap->position; ++idxPos) {
    for (struct actionList *currActE = actsTFGap;
         currActE != NULL;
         currActE = currActE->next) {

      actL = utils_addActionToListAtPosition(actL, currActE->act, idxPos);

      result = aStarPlanner_estimateCost_v3_fixGaps(probSpace,
                                                    actL,
                                                    depthLimit,
                                                    depth + 1,
                                                    positive);
      actL = utils_removeActionFromListAtPosition(actL, idxPos);
      if (result.sol != NULL) {
        //printf("aStarPlanner_estimateCost_v3_fixGaps() END\n");
        //printf("\n");
        utils_free_gap(gap);
        return result;
      }
      if ( ! result.nESol) {
        assert(result.cutoff);
        result.nESol = false;
      }
      if ( ! result.cutoff) {
        assert(result.nESol);
        result.cutoff = false;
      }
    }
  }
  utils_free_gap(gap); // Gap is not needed anymore.
  //printf("aStarPlanner_estimateCost_v3_fixGaps() END\n");
  //printf("\n");
  return result;
}

static
int
aStarPlanner_estimateCost_v3(struct probSpace *probSpace, aStarNode_t node)
{
  //printf("aStarPlanner_estimateCost_v3(): START node: "); // DEBUG
  //utils_print_actionListCompact(node); // DEBUG
  //printf("\n"); // DEBUG

  int result = 0;
  struct estimateCost_fixGaps esSol;

  aStarNode_t longestSol = NULL;

  // Positive gaps, preconditions and effects
  for (int depthLimit = 0; depthLimit < INT_MAX; ++depthLimit) {
    // true means: Positive gaps,preconditions and effects.
    esSol = aStarPlanner_estimateCost_v3_fixGaps(probSpace,
                                                 node,
                                                 depthLimit,
                                                 0,
                                                 true);
    //utils_free_actionListShallow(esSol.sol);
    if (esSol.cutoff) {
      //printf("aStarPlanner_estimateCost_v3() CUTOFF\n");
      continue;
    }
    if (esSol.nESol) {
      //printf("aStarPlanner_estimateCost_v3() NESOL\n");
      result = INT_MAX;
      break;
    }

    if (esSol.numActsAdded >= result) {
      utils_free_actionListShallow(longestSol);
      longestSol = esSol.sol;
    } else {
      utils_free_actionListShallow(esSol.sol);
    }

    //printf("aStarPlanner_estimateCost_v3() SOL length: %d\n",
    //       esSol.numActsAdded);
    result = max(result, esSol.numActsAdded);
    break;
  }

  // Negative gaps, preconditions and effects
  for (int depthLimit = 0; depthLimit < INT_MAX; ++depthLimit) {
    // false means: Negative gaps,preconditions and effects.
    esSol = aStarPlanner_estimateCost_v3_fixGaps(probSpace,
                                                 node,
                                                 depthLimit,
                                                 0,
                                                 false);
    //utils_free_actionListShallow(esSol.sol);
    if (esSol.cutoff) {
      //printf("aStarPlanner_estimateCost_v3() CUTOFF\n");
      continue;
    }
    if (esSol.nESol) {
      //printf("aStarPlanner_estimateCost_v3() NESOL\n");
      result = INT_MAX;
      break;
    }

    if (esSol.numActsAdded > result) {
      utils_free_actionListShallow(longestSol);
      longestSol = esSol.sol;
    } else {
      utils_free_actionListShallow(esSol.sol);
    }

    //printf("aStarPlanner_estimateCost_v3() SOL length: %d\n",
    //       esSol.numActsAdded);
    result = max(result, esSol.numActsAdded);
    break;
  }

  printf("aStarPlanner_estimateCost_v3(): maxLength %d solution: ", result);
  utils_print_actionListCompact(longestSol);
  printf("\n");
  utils_free_actionListShallow(longestSol);

  //printf("aStarPlanner_estimateCost_v3(): END result: %d\n", result); // DEBUG
  return result;
}

/* static */
/* int */
/* aStarPlanner_calcFScore(struct probSpace *probSpace, aStarNode_t node) */
/* { */
/*   // f(n) = g(n) + h(n) */
/*   // TODO: Check: Should we use gScore here? */
/*   int gScore = utils_actionList_length(node); */
/*   int hScore = aStarPlanner_estimateCost_v3(probSpace, node); */
/*   assert(gScore >= 0); */
/*   assert(hScore >= 0); */
/*   int fScore = gScore + hScore; */
/*   if (fScore < 0) { // In case of wrap around */
/*     return INT_MAX; */
/*   } */
/*   return fScore; */
/* } */

/* static */
/* struct actionList * */
/* aStarPlanner_getActions(struct probSpace *probSpace, list_t gaps) */
/* { */
/*   struct actionList *result = NULL; */

/*   while (gaps != NULL) { */

/*     struct gap *gap = (struct gap *) list_getFirstPayload(gaps); */
/*     gaps = list_removeFirst(gaps); */

/*     struct actionList *actsTFGap = ps_getActsToFixGap(probSpace, gap->literal); */
/*     result = utils_concatActionLists(result, actsTFGap); */
/*   } */
/*   return result; */
/* } */

static
void
printActionList(void *al)
{
  utils_print_actionListCompact((struct actionList *) al);
}

struct actionList *
aStarPlanner_aStar(struct probSpace *probSpace)
{
  aStarNode_t aStarNode = NULL;    // The empty list of actions.
  //list_t frontier = list_createElem(aStarNode);
  asnList_t frontier = asnList_push(asnList_createEmpty(), aStarNode);
  //list_t explored = NULL;    // An empty set.
  asnList_t explored = asnList_createEmpty();

  int bestHScore = INT_MAX;

  clock_t timeTmp = clock();
  while( ! asnList_isEmpty(frontier)) {

    clock_t endTime = clock();
    if (timeout >= 0) {
      if ((endTime - startTime) > timeout) {
        printf("nodes expanded: %d\n", list_length(explored->list));
        printf("TIMEOUT.\n");
        break;
      }
    }
    if ((endTime - timeTmp) > OUTPUT_UPDATE_INTERVAL) {
      printf("nodes expanded: %d\n", list_length(explored->list));
      timeTmp = endTime;
    }


    //printf("\nNEW ITERATION. frontier at beginning of loop:\n");
    //list_print(frontier->list, &printActionList); // DEBUG
    ////asnTreePrint(frontier->tree);
    //printf("\n");

    aStarNode_t currN;
    /* Pseudo-Code: currN = pop(frontier) */
    currN = (aStarNode_t) asnList_getFirstPayload(frontier);
    frontier = asnList_removeFirst(frontier);

    //printf("\nfrontier after removeFirst:\n");
    //list_print(frontier, &printActionList); // DEBUG

    //printf("current node (action list):\n");
    //utils_print_actionListCompact(currN); // DEBUG
    //printf("\n"); // DEBUG

    //list_t gaps = aStarPlanner_getAllGaps(probSpace, currN);
    struct gap *gap = planner_hasGap(probSpace->problem->init,
                                     probSpace->problem->goal,
                                     currN);
    // We will put this single gap in a (singleton) list, just to not change
    // the rest of the code.
    list_t gaps = NULL;
    if (gap != NULL) {
      gaps = list_createElem(gap);
    }

    if (list_isEmpty(gaps)) {
      // Note: typedef struct actionList * aStarNode_t;
      aStarNode_t solution = utils_cloneActionList(currN);
      printf("nodes expanded: %d\n", list_length(explored->list));
      return solution;
    }

    //explored = list_push(explored, list_createElem(currN));
    explored = asnList_push(explored, currN);
    //printf("explored length: %d\n", list_length(explored));
    
    // Experimental. This ordering might be better.
    //gaps = list_reverse(gaps);

    for (list_t gapE = gaps;
         gaps != NULL;
         gaps = gaps->next) {

      struct gap *gap = (struct gap *) gaps->payload;
      //utils_print_gap(gap); // DEBUG
      //printf("\n"); // DEBUG
      struct actionList *actions = ps_getActsToFixGap(probSpace, gap->literal);
      //utils_print_actionListCompact(actions); // DEBUG
      //printf("\n"); // DEBUG

      for (struct actionList *currAct = actions;
           currAct != NULL;
           currAct = currAct->next) {

        struct groundAction *grAct = currAct->act;
        for (int idxPos = 0;
             idxPos < gap->position; // Note: gap->position > 0
             idxPos++) {

          aStarNode_t chld = utils_cloneActionListShallow(currN);
          //printf("chld: "); // DEBUG
          //utils_print_actionListCompact(chld); // DEBUG
          //printf("\n"); // DEBUG
          chld = utils_addActionToListAtPosition(chld, grAct, idxPos);
          //printf("chld: "); // DEBUG
          //utils_print_actionListCompact(chld); // DEBUG
          //printf("\n"); // DEBUG

          /* If explored contains chld. */
          //if (asnl_find(explored, chld) != NULL) {
          if (asnList_find(explored, chld) != NULL) {
            utils_free_actionListShallow(chld);
            continue;
          }

          ///* EXPERIMENTAL START */
          //printf("node: "); // DEBUG
          //utils_print_actionListCompact(chld); // DEBUG
          //printf("\n"); // DEBUG

          //// Attention: only v3 is admissible!
          ////int hScore1 = aStarPlanner_estimateCost_v1(probSpace, chld);
          ////int hScore2 = aStarPlanner_estimateCost_v2(probSpace, chld);
          ////int hScore3 = aStarPlanner_estimateCost_v3(probSpace, chld);
          //int hScore4 = heuristic_estimate(probSpace, chld);

          ////printf("hScores: %d\t%d\t%d\n", hScore1, hScore3, hScore4);

          ///* EXPERIMENTAL END */

          // Calculate f-score
          int hScore = heuristic_estimate(probSpace, chld); //hScore4;
          if (hScore < 0) {
            // No solution to this chld exists
            // Either add it to explored-list or free it.
            explored = asnList_push(explored, chld);
            //utils_free_actionListShallow(chld);
            continue;
          }

          int gScore = utils_actionList_length(chld);
          int fScore = gScore + hScore;

          if (hScore < bestHScore) {
            bestHScore = hScore;
            printf("new best h-score %d. nodes expanded: %d\n",
                   bestHScore,
                   list_length(explored->list));
          }

          /* If frontier contains chld. */
          //list_t frontierElem = asnl_find(frontier, chld);
          list_t frontierElem = asnList_find(frontier, chld);
          // DEbuG START
          //printf("### frontierElem: ");
          //if (frontierElem == NULL) {
          //  printf("NULL");
          //} else {
          //  utils_print_actionListCompact(frontierElem->payload);
          //  //list_print(frontierElem, &printActionList); // DEBUG
          //}
          //printf("\n###\n");
          // DEbuG END
          if (frontierElem != NULL) {
            // If new score is lower, then replace node in frontier, otherwise
            // free chld.
            if (frontierElem->intValue > fScore) {
              aStarNode_t frontierElemPayload =
                                          (aStarNode_t) frontierElem->payload;
              //frontier = list_remove(frontier, frontierElem);
              frontier = asnList_remove(frontier, frontierElemPayload);
              // Can I insert it before elements with the same fScore? Yes.
              //frontier = asnl_insertOrdered(frontier, chld, fScore);
              frontier = asnList_insertOrdered(frontier, chld, fScore);
              utils_free_actionListShallow(frontierElemPayload);
            } else {
              utils_free_actionListShallow(chld);
            }
            continue;
          }

          /* Add node to frontier. */
          //printf("frontier before insertOrdered:\n"); // DEBUG
          //list_print(frontier, &printActionList); // DEBUG
          //frontier = asnl_insertOrdered(frontier, chld, fScore);
          frontier = asnList_insertOrdered(frontier, chld, fScore);
          //printf("frontier after insertOrdered:\n"); // DEBUG
          //list_print(frontier, &printActionList); // DEBUG
        }
      }
    }

    list_freeWithPayload(gaps, &freeGap);
  }

  // No solution exists.
  return NULL;
}

struct actionList *
aStarPlanner(struct problem *problem, int timeout_)
{
  if (timeout_ > 0) {
    startTime = clock();
    timeout = timeout_ *CLOCKS_PER_SEC; // in clocks
  }

  struct probSpace *probSpace = ps_init(problem);

  printf("all ground actions in problem space:\n"); // DEBUG
  utils_print_actionListCompact(probSpace->allGrActs); // DEBUG
  printf("\n"); // DEBUG
  printf("number of ground actions in problem space: %d",
         utils_actionList_length(probSpace->allGrActs));
  printf("\n");

  printf("Problem Space: "); // DEBUG
  trie_print(probSpace->setFluents); // DEBUG
  printf("\n"); // DEBUG
  printf("variable occurrence: %d\n", ps_calcMaxVarOcc(probSpace));

  struct actionList *solution = aStarPlanner_aStar(probSpace);
  ps_free(probSpace);
  trie_cleanupSNBuffer();
  asnList_cleanup();
  list_cleanupLEBuffer();
  return solution;
}
