#ifndef PDDL31STRUCTS_H
#define PDDL31STRUCTS_H

#include <stdbool.h>
#include <stdint.h>

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

    int32_t numOfConstants;
    struct constant *constants;

    int32_t numOfPredicates;
    struct predicate *predicates; 

    int32_t numOfActions;
    struct action *actions;

    //int32_t numOfTypes;
    //struct type *types;
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
    
    struct formula *init;

    struct formula *goal;
};

struct predicate
{
    char *name;
    int32_t numOfParameters;
    struct variable *parameters;
};

struct type
{
    char *name;
};

struct constant
{
    char *name;
    bool isTyped;
    struct type *type; // :requirement typing
};

struct action
{
    char *name;

    int32_t numOfParameters;
    struct variable *parameters;
    
    struct formula *precondition;

    struct formula *effect;
};

enum formulaType
{
    PREDICATE 
    ,AND
    ,NOT
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

// TODO: That is the same as struct constant. Do we need both? Time will tell.
struct variable
{
    char *name;
    bool isTyped;
    struct type *type; // :requiremnt typing
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

#endif //PDDL31STRUCTS_H
