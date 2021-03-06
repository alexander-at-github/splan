#ifndef PDDL31STRUCTS_H
#define PDDL31STRUCTS_H

#include <stdbool.h>
#include <stdint.h>

#include "actionManag.h"
#include "typeSystem.h"
#include "predManag.h"
#include "objManag.h"
//#include "state.h"

// Forward declaration for state
struct  st_trie;
typedef struct st_trie * trie_t;

enum requirement
{
    STRIPS
    ,TYPING
    ,NEGATIVE_PRECONDITION
    ,DISJUNKTIVE_PRECONDITION
    ,EQUALITY
    ,EXISTENTIAL_PRECONDITIONS
    ,UNIVERSAL_PRECONDITIONS
    ,QUANTIFIED_PRECONDITIONS
    ,CONDITIONAL_EFFECTS
    ,FLUENTS
    ,NUMERIC_FLUENTS
    ,ADL
    ,DURATIVE_ACTIONS
    ,DURATION_INEQUALITIES
    ,CONTINUOUS_EFFECTS
    ,DERIVED_PREDICATES
    ,TIMED_INITIAL_LITERALS
    ,PREFERENCES
    ,CONSTRAINTS
    ,ACTION_COSTS
    ,NIL // that is, no requirement
};

struct domain
{
    char *name;

    int32_t numOfRequirements;
    enum requirement *requirements;

    // Constants will be treated as objects.
    struct objManag *objManag;

    struct predManag *predManag;

    struct actionManag *actionManag;

    struct typeSystem *typeSystem;
};

struct problem
{
    char *name;

    char *domainName;

    // A reference to the coresponding domain.
    struct domain *domain;

    // A problem will _not_ have its own problem manager. That is only a
    // pointer to the domains' problem manager. Makeing a seperate one
    // (cloneing) introduces a nasty bug (pointers point to these two different
    // locations).
    struct objManag *objManag;

    int32_t numOfRequirements;
    enum requirement *requirements;

    // Objects will be saved in the domains object manager

    // The initial state. We use a trie as a state.
    trie_t init;

    struct goal *goal;
};

// A term is either a constant or a variable. We need terms, cause a
// precondition can include constants and variables.
struct term
{
    // In PDDL a variables name starts with the '?' character.
    // If this structure is a constant, then the name is also its value.
    char *name;
    // true: is variable
    // false: is constant
    bool isVariable;
    struct type *type;
};

// This is a formal predicate. That is, a predicate definition
struct predicate
{
    char *name;
    int32_t numOfParams;
    // We store the parameter just in order to store the types. The names
    // of the parameter are not needed.
    // This terms are all variables.
    struct term *params;
};

/** DEPRECATED **/
// Semantics: A state is a conjunction of fluents.
// Closed world assumption and unique name assumption apply. The state can be
// treated as a set of fluents.
/*
struct state
{
    int32_t numOfFluents;
    struct atom *fluents;
};
*/

/** Goal **/
// Composition of a predicate and terms as arguments.
struct atom
{
    struct predicate *pred;
    // Number of constants is pred_cons->pred->numOfParams
    struct term **terms;
};

// Semantics: conjunction
// A goal could be called a 'partial state'.
struct goal
{
    // positive literals in this conjunction
    int32_t numOfPos;
    struct atom *posLiterals;
    // negative literals in this conjunction
    int32_t numOfNeg;
    struct atom *negLiterals;
};

/*** Action ***/
struct action
{
    char *name;

    int32_t numOfParams;
    // These must be variables though.
    struct term *params;
    
    struct goal *precond;

    struct effect *effect;
};

struct groundAction
{
  struct action *action;
  // action->numOfParams

  // The grounding
  // An array of pointers to constants.
  // It maps the parameter action->params[i] to terms[i] for i in
  // 0..(action->numOfParams - 1).
  struct term **terms;
};

/** Effects **/

enum effElemType
{
    POS_LITERAL
    ,NEG_LITERAL
    ,FORALL
    ,WHEN
};

struct effectElem
{   
    enum effElemType type;
    union {
        // Use variable 'literal' for positive and negative literals
        struct atom *literal;
        struct forall *forall;
        struct when *when;
    } it;
};

// An effect is always a conjunction of either literals, forall-clauses or
// when clauses. It might contain only one element. Then it is strictly speaking
// not a conjunction.
struct effect
{
    int32_t numOfElems;
    // Semantics: conjunction
    struct effectElem *elems;
};

struct forall
{
    // For all ...
    int32_t numOfVars;
    struct term *vars;

    // ... apply the effect
    struct effect *effect;
};

struct when
{
    // If precondition evaluates to true ...
    struct goal *precond;

    // ... apply this conjunction of literals
        // atoms (i.e. positive literals)
    int32_t numOfPos;
    struct atom *posLiterals;
        // negative literals
    int32_t numOfNeg;
    struct atom *negLiterals;
};

#endif //PDDL31STRUCTS_H

