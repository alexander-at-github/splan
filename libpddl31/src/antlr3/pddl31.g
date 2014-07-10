grammar pddl31;     // Combined grammar. That is, parser and lexer.
options { /* Attetion: options must be placed right after grammar-statementi. */
          /* Otherwise ANTLR3 will not translate it (error(100)). */
    output = AST;
    language = C;
    // If generating an AST (output=AST; option) or specifying a tree
    // walker then also add the following line.
    ASTLabelType=pANTLR3_BASE_TREE;
}
import pddl31core;



/* Logic */
// Rules for types
primitiveType
    : NAME
    | 'object'
    ;
eitherType
    : '(' 'either' primitiveType+ ')'
    ;
type
    : eitherType
    | primitiveType
    ;

// Rules for terms
term
    : NAME
    | variable
    ;
variable
    : '?' NAME
    ;
	
// Rules for predicates
predicate
    :   NAME
    ;

// Rules for literals
literal_term
    : atomicFormula_term
    | '(' 'not' atomicFormula_term ')'
    ;

// Rules for formulas
atomicFormulaSkeleton
    : '(' predicate typedList_variable ')'
    ;
atomicFormula_term
    : '(' predicate term* ')'
    | '(' '=' term term ')' // requires :equality
    ;

// Miscellaneous
typedList_variable
    : variable*
    | variable+ '-' type typedList_variable
    ;

typedList_name
    : NAME*
    | NAME+ '-' type typedList_name
    ;



/* Domain */
domain
    : '(' 'define' domainName
        requireDef?
        // typesDef?    // requires :typing
        constantsDef?
        predicatesDef?
        // functionsDef?    // requires :fluents
       //  constraints?     // requires :constraints ?
        structureDef* ')'
    ;

// Domain name
domainName
    : '(' 'domain' NAME ')'
    ;

// Requirements definitions
requireDef
    : '(' ':requirements' REQUIRE_KEY+ ')'
    ;

// Types definitions
//TODO

// Constants definitions
constantsDef
    : '(' ':constants' typedList_name ')'
    ;

// Predicate definitions
predicatesDef
    : '(' ':predicates' atomicFormulaSkeleton+ ')'
    ;

// Funtion definitions
//TODO

// Constraint definitions
//TODO

// Structure definitions
structureDef
    : actionDef
    //| durativeActionDef       // requires :durative-actions
    //| derivedDef              // requires :derived-predicates
    ;
actionDef
    : '(' ':action' actionSymbol
        ':parameters' '(' typedList_variable ')' 
        actionDefBody
        ')'
    ;
actionSymbol
    : NAME
    ;
actionDefBody
    : (':precondition' emptyOr_preconditionGoalDescription)?
        (':effect' emptyOr_effect)?
    ;
emptyOr_preconditionGoalDescription
    : '(' ')'
    | preconditionGoalDescription
    ;
emptyOr_effect
    : '(' ')'
    | effect
    ;
//preconditionGoalDescription
//    : preferencesGoalDescription
//    | '(' 'and' preconditionGoalDescription* ')'
//    // requires :universal−preconditions
//    | '(' 'forall' '(' typedList_variable ')' preconditionGoalDescription ')'
//    ;
//preconditionGoalDescription
//    : preferencesGoalDescription
//    | preconditionGoalDescriptionAnd
//    | preconditionGoalDescriptionForall
//    ;
//preconditionGoalDescriptionAnd
//    : '(' 'and' preconditionGoalDescription* preconditionGoalDescriptionForall
//        preconditionGoalDescription* ')'
//    ;
//preconditionGoalDescriptionForall
//    : '(' 'forall' '(' typedList_variable ')' preconditionGoalDescription ')'
//    ;
/* cause we don't support :preferences */
preconditionGoalDescription
    : goalDescription
    ;

preferencesGoalDescription
    : goalDescription
    //| '(' 'preference' preferenceName? goalDescription ')'  // requires :preferences
    ;
preferenceName
    : NAME
    ;
goalDescription
    /*
    : atomicFormula_term
    | literal_term // requires :negative-preconditions
    */
    //: atomicFormula_term
    // The following line includes atomicFormula_term
    : literal_term // requires :negative-preconditions
    | '(' 'and' goalDescription* ')'
    | '(' 'forall' '(' typedList_variable ')' goalDescription ')' // requires :universal-preconditions
    //| '(' 'or' goalDescription* ')' // requires :disjunctive-preconditions
    //| '(' 'not' goalDescription* ')' // requires :disjunctive-preconditions
    //| '(' 'imply' goalDescription goalDescription ')' // requires :disjunctive-preconditions
    //| '(' 'exists' '(' typedList_variable ')' goalDescription ')' // requires :existential-preconditions
    //| fComp // requires :atomic-fluents
    ;
effect
    : '(' 'and' cEffect* ')'
    | cEffect
    ;
cEffect
    : '(' 'forall' '(' typedList_variable ')' effect ')' // requires :conditional-effects
    | '(' 'when' goalDescription condEffect ')' // requires :conditional-effects
    | pEffect
    ;
condEffect
    : '(' 'and' pEffect* ')'
    | pEffect
    ;
pEffect
    : '(' 'not' atomicFormula_term ')'
    | atomicFormula_term
    //| '(' assignOp fHead fExp ')' // requires :numeric-fluents
    //| '(' 'assign' function_term term ')' //requires :object-fluents
    //| '(' 'assign' function_term 'undefined' ')' //requires :object-fluents
    ;
