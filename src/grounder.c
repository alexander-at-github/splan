#include "grounder.h"

#include "planner.h"

struct groundAction
{
  struct action *action;
  struct gTerm *root;  // ground term. The root is actually not a mapping.
                       // It just has children.
};

// Ground term. Or grounding for a term. That is just a mapping from a
// variable to a constant or object (in the sense of the PDDL).
struct gTerm
{
  struct term *var;
  struct term *constant;

  int32_t numOfChld;
  struct gTerm **children;
};


void // TODO: add Return value
grounder_getActions(struct domain *domain,
                    struct state *state)
{
  for each action in domain {
    struct groundAction *groundA = malloc(sizeof(*groundA));
    groundA->action = action;
    groundA->root = malloc(sizeof(groundA->root));
    groundA->root->var = NULL;
    groundA->root->constant = NULL;
    groundA->root->numOfChld = 0;
    groundA->root->children = NULL;
    grounder_groundAction(action, state, 0, groundA->root);
  }
}

// Precondition: The actions precondition does not have any literals without
//               free variables.
// TODO: Make sure it also works for negative literals in the precondition.
//       Could do that by filtering the invalid groundings out later.
// TODO: Match types of the grounding.
// TODO: Use a boolean return value, whether the grounding could be completed or
//       not.
//
// For now this function only considers postive literals of the states
// precondition.
//
// Each call of this function is considered only with one atom of the actions'
// precondition. It is a recursive function.
//
// Builds subtree into 'root' data structure passed as argument.
void // TODO: Check return value
grounder_groundAction(struct action *action,
                      struct state *state,
                      int32_t idxPrecond,//Index into predicate of precondition.
                      struct gTerm *gTermRoot)
{
  // We are only considering positive literals in the precondition for now.
  if (idxPrecond > action->precond->numOfPos) {
    // We could increment an counter here in order to know the number
    // of groundings for this action.
    return;
  }
  // Precondtion current positive literal
  struct atom *pcpl = &action->precond->posLiterals[idxPrecond];

  // Save gTermRoot for later use. Note we are only saveing the pointer.
  struct gTerm *gTermBack = gTermRoot;

  assert(free_vars(pcpl)); // Is that still necessary?
  // TODO: What to do if the current positive literal contains free variables
  // and constants??

  // Iterate over the states fluents.
  for (size_t idxFluents = 0; idxFluents < state->numOfFluents; ++idxFluents) {
    struct atom *fluent = state->fluents[idxFluents];
    // Now 'pcpl' and 'fluent' are the two atoms we will work with.

    // If this fluents' predicate is not the predicate we are concerned with
    // just continue with the next fluent.
    if (pcpl->pred != fluent->pred) {
      continue;
    }

    // Iterate over the preconditions' literals' arguments.
    for (size_t idxParam = 0; idxParam < pcpl->pred->numOfParams; idxParam++) {
      if ( ! pcpl->terms[idxParam]->isVariable) {
        // This preconditions' literal contains constants. Check if it matches
        // the states' fluent. If not, we do not have a matching.
        if (term_equal(pcpl->terms[idxParam], fluent->terms[idxParam])) {
          continue;
        } else {
          // The whole fluent does not match! TODO: We might have to delete
          // an previous grounding (of individual variables variables).
          //
          //return; ??
          // I don't think so. Probably more like:
          //continue outer-loop;
        }
      }

      // Now we know pcpl-Terms[idxParam] is a variable

      // Check if the types match.
      if ( ! types_equal( pcpl->terms[idxParam]->type,
                          fluent->terms[idxParam]->type)) {
        // TODO: The whole fluent does not match.
        // continue outer-loop ???
      }

      int32_t idxChld = gTermRoot->numOfChld;
      gTermRoot->numOfChld++;

      // Allocate memory to pointer to new child.
      struct gTerm *tmp = realloc(gTermRoot->children, gTermRoot->numOfChld);
      if (tmp == NULL) {
        fprintf(stderr, "Error allocting memory in grounder_groundAction\n");
        // TODO: What to do? I don't know how to recover from that.
        exit(EXIT_FAILURE);
      }
      gTermRoot->children = tmp;

      // Allocate memory for new child
      gTermRoot->children[idxChld] =
                                malloc(sizeof(*gTermRoot->children[idxchld]));
      struct gTerm *newChild = gTermRoot->children[idxChld];
      if (newChild == NULL) {
        fprintf(stderr, "Error allocting memory in grounder_groundAction\n");
        // TODO: What to do? I don't know how to recover from that.
        exit(EXIT_FAILURE);
      }

      // Initialize new child
      newChild->numOfChld = 0;
      newChild->children = NULL;
      // Write the mapping of variable to constant in this child.
      newChild->var = pcpl->terms[idxParam];
      assert (newChild->var->isVariable);
      newChild->constant = fluent->terms[idxParam];
      assert ( ! newChild->constant->isVariable);

      // Set gTermRoot.
      gTermRoot = newChild;
    } // End of loop over preconditions' literals' arguments

    // Recurse for each grounding of a literal
    grounder_groundAction(action, state, idxPrecond + 1, gTermRoot);

  } // End of loop over the states fluents
}

static bool
term_equal(struct term *t1, struct term *t2)
{
  return t1->isVariable == t2->isVariable &&
         (t1->name == t2->name || strcmp(t1->name, t2->name) == 0) &&
         t1->type == t2->type; // Comparing pointers to types really has to be
                               // enough.
}

// Returns true iff the atom contains at least one variable.
bool
free_vars(struct atom *atom)
{
  for (size_t i = 0; i < atom->pred->numOfParams; ++i) {
    if (atom->term[i]->isVariable()) {
      return true;
    }
  }
  return false;
}














// OLD:

struct grounder {
  struct domain *domain;
};

Grounder
grounder_create(struct domain *domain)
{
  struct grounder *grounder = malloc(sizeof(*grounder));
  grounder->domain = domain;
  return grounder;
}






struct grounding {
  struct action *action;
  // A set of arguments. This is associates values to the parameter of the
  // action. These terms (! term->isVariable) must hold.
  struct term **arguments;

  // It is a linked list
  struct grounding *previous;
};

static struct grounding *
grounder_groundAction(struct action *action,
                      struct state *state,
                      struct grounding *acc)
{
  // CONTINUE
  for each positive-literal in action->precond
    if free_vars?(positive-literal)
      for each atom in state
        if pred(positive-literal) == pred(atom)
          if acc == NULL then allocate memory and set to zero
          for each param (with index i) in positiv-literal find
              parameter in struct action (with index n)
            acc->term[n] = atom.getParam(i)

          // ...
          // TODO: Does that work?
          for i in 0..positive-literal.numOfParams
          acc->term
          // Do the grounding
          struct term *var = atom->
    else
      // Check if positive-literal is fullfilled by state as usual
    end

  for each negative-literal in action-precond
    if free_vars?(negative-literal)
    else
    end

    bool satisfied = false
    for each atom in state
      if positive-literal == atom
        satisfied = true
      else if free_vars(positive-literal) &&
              pred(positive-literal) == pred(atom)

}

struct grounding *
grounder_getActions(Grounder grounder,
                    struct state *state,
                    int32_t numInAcc,
                    struct grounding *acc)
{
  for each action in grounder->domain->actionManag do
    grounder_groundAction(action, state, NULL);
  
}

/* struct action ** */
/* grounder_getActions(Grounder grounder, struct state *state); */
/* { */
/*   struct actionManag *am = grounder->domain->actionManag; */
/*   // Go over all the actions */
/*   for (size_t i = 0; i < am->numOfActions; ++i) { */
/*     struct action *action = &am->actions[i]; */

/*     // Check if action is applicable. Action preconditions are struct goal. */
/*     bool applicable = true; */
/*     for each positive-literal in action->precond { */
/*       bool satisfied = false */
/*       for each atom in state { */
/*         if positive-literal == atom { */
/*           satisfied = true */
/*           break */
/*         } */
/*       } */
/*       if satisfied == false { */
/*         // TODO: If positive literal has free variables, then bind free */
/*         // variable to all possible objects from the state. */
/*         applicable = false; */
/*       } */
/*     } */
/*     for each negative-literal in action->precond { */
/*       for each atom in state { */
/*         if negative-literal == atom { */
/*           applicable = false */
/*         } */
/*       } */
/*     } */

/*     if (action_isApplicable(action, state)) { */
/*       // TODO: Ground action and save it */
/*     } */
/*   } */
/* } */

static bool
action_isApplicable(struct action *action, struct state *state)
{
  // Should we use a different function here?
  // Checking for goal in a problem only uses grounded predicates.
  // Whereas preconditions in actions can contain variables.
  return planner_goalMet(action->precond, state);
}
