#ifndef PDDL31STRUCTS_H
#define PDDL31STRUCTS_H

#include <stdbool.h>
#include <stdint.h>

#include "types.h"

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

    int32_t numOfCons;
    struct constant *cons;

    int32_t numOfPreds;
    struct predicate *predes; 

    int32_t numOfActions;
    struct action *actions;

    struct typeSystem *typeSystem;
};

struct problem
{
    char *name;

    char *domainName;

    int32_t numOfRequirements;
    enum requirement *requirements;

    int32_t numOfObjects;
    // objects are constants
    struct constant *objects;
    
    struct state *init;

    struct goal *goal;
};

struct constant
{
    char *name;
    struct type *type;
};

// struct variable is the same as struct constant. I want to differentiate that
// though.
struct variable
{
    char *name;
    struct type *type;
};

struct predicate
{
    char *name;
    int32_t numOfParams;
};

// Basically a ground predicate or atom.
struct fluent
{
    struct predicate *pred;
    // The number of arguments is in fluent->predicate->numOfParams
    struct constant *args;
};

// Semantics: A state is a conjunction of fluents.
// Closed world assumption and unique name assumption apply. The state can be
// treated as a set of fluents.
struct state
{
    int32_t numOfFluents;
    struct fluent *fluents;
};

/** Goal **/
// Composition of a predicate and constants of an goal, for example
struct pred_cons
{
    struct predicate *pred;
    // Number of constants is pred_cons->pred->numOfParams
    struct constant *cons;
};

// Semantics: conjunction
struct goal
{
    // positive literals in this conjunction
    int32_t numOfPos;
    struct pred_cons *posLiterals;
    // negative literals in this conjunction
    int32_t numOfNeg;
    struct pred_cons *negLiterals;
};

/*** Action ***/
struct action
{
    char *name;

    int32_t numOfParams;
    struct variable *params;
    
    struct precond *precond;

    struct effect *effect;
};

// Composition of a predicate and variables of an action, for example.
struct pred_var
{
    struct predicate *pred;
    // Number of variables is pred_var->predicate->numOfParams
    struct variable *var;
};

/** Precondition **/

// Semantics: A conjunction of literals (positive or negated atoms)
struct precond
{
    // positive literals in this conjunction
    int32_t numOfPos;
    struct pred_var *posLiterals;
    // negative literals in this conjunction
    int32_t numOfNeg;
    struct pred_var *negLiterals;
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
        struct pred_var *literal;
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
    struct effectElem elems;
};

struct forall
{
    // For all ...
    int32_t numOfVars;
    struct variable *vars;

    // ... apply the effect
    struct effect *effect;
};

struct when
{
    // If precondition evaluates to true ...
    struct precond *precond;

    // ... apply this conjunction of literals
        // atoms (i.e. positive literals)
    int32_t numOfPos;
    struct pred_var *posLiterals;
        // negative literals
    int32_t numOfNeg;
    struct pred_var *negLiterals;
};

/*
enum formulaType
{
    PREDICATE 
    ,AND
    ,NOT
    ,EMPTY  // TODO: do we need that? TODO: delete
};

struct formula
{
    enum formulaType type;
    union {
        struct {
            char *name;
            int32_t numOfArguments;
            struct term *arguments; // Arguments are terms
        } predicate_formula;
        struct {
            int32_t numOfParameters;
            struct formula *p;
        } and_formula;
        struct {
            struct formula *p;
        } not_formula;
    } item;
};


enum termType
{
    CONSTANT
    ,VARIABLE
};

struct term
{
    enum termType type;
    union {
        struct constant *constArgument;
        struct variable *varArgument;
    } item;
};
*/

#endif //PDDL31STRUCTS_H
