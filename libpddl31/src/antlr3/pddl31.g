grammar pddl31;     // Combined grammar. That is, parser and lexer.

options { /* Attetion: options must be placed right after grammar-statementi. */
          /* Otherwise ANTLR3 will not translate it (error(100)). */
    language = C;
}

import pddl31core;

@header {
    #include <stdlib.h>
    #include "pddl31structs.h"

    // Size of lists when initialized
    #define LIST_SIZE_INIT 16
}

// TODO: Fix all memory errors. Use valgrind.

/*** Logic ***/
/* Rules for types */
primitiveType
    :   NAME
    |   'object'
    ;

// Should I support that at all? FastDownward does not support that.
//eitherType 
//    :   '(' 'either' primitiveType+ ')'
//    ;

type
    :   primitiveType
    //|   eitherType
    ;

/* Rules for terms */
term returns [struct term *value]
@init {
    $value = malloc(sizeof(*$value));
}
    : NAME
        {
        $value->type = CONSTANT;
        $value->item.constArgument = 
                                malloc(sizeof(*$value->item.constArgument));
        strncpy($value->item.constArgument->name,
                (char *) $NAME.text->chars,
                NAME_LENGTH_MAX);
        $value->item.constArgument->isTyped = false;
        }
    | variable
        {
        $value->type = VARIABLE;
        $value->item.varArgument = malloc(sizeof(*$value->item.varArgument));
        strncpy($value->item.varArgument->name,
                (char *) $variable.text->chars,
                NAME_LENGTH_MAX);
        $value->item.varArgument->isTyped = false;
        }
    ;

variable
    :   '?' NAME
    ;
	
/* Rules for predicates */
predicate
    :   NAME
    ;

/* Rules for literals */
literal_term returns [struct formula *value]
    : atomicFormula_term
        {
        $value = $atomicFormula_term.value;
        }
    | '(' 'not' atomicFormula_term ')'
        {
        $value = malloc(sizeof(*$value));
        $value->type = NOT;
        $value->item.not_formula.p = $atomicFormula_term.value;
        }
    ;

/* Rules for formulas */
atomicFormulaSkeleton returns [struct predicate *value]
@init {
    pANTLR3_LIST variable_list = antlr3ListNew(LIST_SIZE_INIT);
}
@after {
    variable_list->free(variable_list);
}
    :   '(' predicate typedList_variable[variable_list] ')'
    {
    $value = malloc(sizeof(*$value));
    strncpy($value->name, (char *) $predicate.text->chars, NAME_LENGTH_MAX);
    $value->numOfParameters = variable_list->size(variable_list);
    $value->parameters = malloc(sizeof(*$value->parameters) *
                                $value->numOfParameters);
    for (int i = 0; i < $value->numOfParameters; ++i) {
        //List index starts from 1
        $value->parameters[i] = *(struct variable*)
                               variable_list->get(variable_list, i+1);
    }
    }
    ;

atomicFormula_term returns [struct formula *value]
@init {
    pANTLR3_LIST term_list = antlr3ListNew(LIST_SIZE_INIT);
}
@after {
    term_list->free(term_list);
}
    :   '(' predicate (term {
                            term_list->add(term_list,
                                           $term.value,
                                           &free);
                            }
                      )* ')'
        {
        $value = malloc(sizeof(*$value));
        $value->type = PREDICATE;
        strncpy($value->item.predicate_formula.predicate_name,
                (char *) $predicate.text->chars,
                NAME_LENGTH_MAX);
        $value->item.predicate_formula.numOfArguments =
                                                    term_list->size(term_list);
        $value->item.predicate_formula.arguments =
                     malloc(sizeof(*$value->item.predicate_formula.arguments) *
                     $value->item.predicate_formula.numOfArguments);
        for (int i = 0; i < $value->item.predicate_formula.numOfArguments; ++i){
            //List index starts from 1
            $value->item.predicate_formula.arguments[i] = *(struct term *)
                                                term_list->get(term_list, i+1);
        }
        }
    |   '(' '=' term term ')' // requires :equality
        {
        // TODO
        }
    ;

/* Miscellaneous */
typedList_variable[pANTLR3_LIST list]
@init {
    pANTLR3_LIST local_list = antlr3ListNew(LIST_SIZE_INIT);
}
@after {
    local_list->free(local_list);
}
    :   (variable {
                  struct variable *item = malloc(sizeof(*item));
                  strncpy(item->name,
                          (char *) $variable.text->chars,
                          NAME_LENGTH_MAX);
                  item->isTyped = false;
                  // Do free structs. They must be COPIED into an array later.
                  $list->add($list, item, &free);
                  }
        )*
    |   (variable {
                  struct variable *item = malloc(sizeof(*item));
                  strncpy(item->name,
                          (char *) $variable.text->chars,
                          NAME_LENGTH_MAX);
                  item->isTyped = true;
                  // Save these names in a local list first.
                  // Don't free. 'list' will do that.
                  local_list->add(local_list, item, NULL);
                  }
        )+ '-' type {
                    // Now we know the type of the identifiers.
                    struct type t;
                    strncpy(t.name,
                            (char *) $type.text->chars,
                            NAME_LENGTH_MAX);
                    // ANTLR3_LIST index starts from 1
                    for (int i = 1; i <= local_list->size(local_list); ++i) {
                        // Add type information
                        struct variable *item = local_list->get(local_list, i);
                        item->type = t;
                        // Add names to input-output list
                        // Do free structs. They must be COPIED into an array
                        // later.
                        $list->add($list, item, &free);
                    }
                    }
        typedList_variable[$list] // requires :typing
    ;

// The parameter LIST will be filled by this rule.
typedList_name[pANTLR3_LIST list]
@init {
    pANTLR3_LIST local_list = antlr3ListNew(LIST_SIZE_INIT);
}
@after {
    local_list->free(local_list);
}
    :   (NAME {
              struct constant *item = malloc(sizeof (*item));
              strncpy(item->name, (char *) $NAME.text->chars, NAME_LENGTH_MAX);
              item->isTyped = false;
              // Do free structs. They must be COPIED into an array later.
              $list->add($list, item, &free);
              }
        )*
    |   (NAME {
              struct constant *item = malloc(sizeof (*item));
              strncpy(item->name, (char *) $NAME.text->chars, NAME_LENGTH_MAX);
              item->isTyped = true;
              // Save these names in a local list first.
              // Don't free. 'list' will do that.
              local_list->add(local_list, item, NULL);
              }
        )+ '-' type {
                    // Now we know the type of the identifiers.
                    struct type t;
                    strncpy(t.name,
                            (char *) $type.text->chars,
                            NAME_LENGTH_MAX);
                    // ANTLR3_LIST index starts from 1
                    for (int i = 1; i <= local_list->size(local_list); ++i) {
                        // Add type information
                        struct constant *item = local_list->get(local_list, i);
                        item->type = t;
                        // Add names to input-output list
                        // Do free structs. They must be COPIED into an array
                        // later.
                        $list->add($list, item, &free);
                    }
                    }
        typedList_name[$list] // requires :typing
    ;



/*** Domain ***/
domain returns [struct domain *value]
scope {
    struct domain *item;
}
@init {
    $domain::item = malloc(sizeof (*$domain::item));
    pANTLR3_LIST action_list = antlr3ListNew(LIST_SIZE_INIT);
}
@after {
    action_list->free(action_list);
}
// Don't free the domain struct, cause that's what we are actually
// producing.
    :   '(' 'define' domainName
        requireDef?
        // typesDef?    // requires :typing
        constantsDef?
        predicatesDef?
        // functionsDef?    // requires :fluents
        // constraints?     // requires :constraints ?
        (structureDef {
                      // Do free. Whole structures will be copied later.
                      action_list->add(action_list,
                                       $structureDef.value,
                                       &free);
                      }
        )* ')'
        {
        strncpy($domain::item->name, $domainName.value, NAME_LENGTH_MAX);

        // Copy actions (from structureDef) into domain struct.
        $domain::item->numOfActions = action_list->size(action_list);
        $domain::item->actions = malloc(sizeof(*$domain::item->actions) *
                                        $domain::item->numOfActions);
        for (int i = 0; i < $domain::item->numOfActions; ++i) {
            // antlr list index starts from 1
            $domain::item->actions[i] = *(struct action *)
                                        action_list->get(action_list, i+1);
        }
        $value = $domain::item;
        //printf("Debug: In domain-rule.\n");
        }
    ;

/* Domain name */
domainName returns [char *value]
    :   '(' 'domain' NAME ')'
        {
        $value = (char *) $NAME.text->chars;
        }
    ;

/* Requirements definitions */
requireDef
@init {
    pANTLR3_LIST req_list = antlr3ListNew(LIST_SIZE_INIT);
}
@after {
    req_list->free(req_list);
}
    :   '(' ':requirements'
            ( requireKey
              {
              // Save this requirement into req_list.
              // Do free here, cause the whole 'enum requirement' will be
              // copied.
              req_list->add(req_list, $requireKey.value, &free);
              }
            )+ ')'
        {
        // Write all collected requirements from req_list to array in 
        // domain struct.
        $domain::item->numOfRequirements = req_list->size(req_list);
        $domain::item->requirements =
                    malloc(sizeof(*$domain::item->requirements) *
                    $domain::item->numOfRequirements);
        for (int i = 0; i < $domain::item->numOfRequirements; ++i) {
            // Index in ANTLR3-List starts at 1!
            $domain::item->requirements[i] =    *(enum requirement *)
                                                req_list->get(req_list, i+1);
        }
        //printf("Debug: in requireDef-rule\n");
        }
    ;

requireKey returns [enum requirement *value]
@init {
    $value = malloc(sizeof(*$value));
}
    : ':strips'
      {
      *$value = STRIPS;
      }
//    | ':typing'
    | ':negative-preconditions'
      {
      *$value = NEGATIVE_PRECONDITION;
      }
//    | ':disjunctive-preconditions'
//    | ':equality'
//    | ':existential-preconditions'
//    | ':universal-preconditions'
//    | ':quantified-preconditions'
//    | ':conditional-effects'
//    | ':fluents'
//    | ':numeric-fluents'
//    | ':adl'
//    | ':durative-actions'
//    | ':durative-inequalities'
//    | ':continuous-effects'
//    | ':derived-predicates'
//    | ':timed-initial-literals'
//    | ':preferences'
//    | ':constraints'
//    | ':action-costs'
    ;

/* Types definitions */

/* Constants definitions */
constantsDef
@init {
    pANTLR3_LIST list = antlr3ListNew(LIST_SIZE_INIT);
}
@after {
    list->free(list);
}
    :   '(' ':constants' typedList_name[list] ')'
        {
        // Write constants into domain struct.
        $domain::item->numOfConstants = list->size(list);
        $domain::item->constants = malloc(sizeof(*$domain::item->constants) *
                                          $domain::item->numOfConstants);
        for(int i = 0; i < $domain::item->numOfConstants; ++i) {
            // Index in ANTLR3-List starts at 1!
            $domain::item->constants[i] = *(struct constant *)
                                          list->get(list, i+1);
        }
        //printf("Debug: in constantsDef\n");
        }
    ;

/* Predicate definitions */
predicatesDef
@init {
    pANTLR3_LIST list = antlr3ListNew(LIST_SIZE_INIT);
}
@after {
    list->free(list);
}
    :   '(' ':predicates' (atomicFormulaSkeleton
                          {
                          // Do free. Whole struct will be copied later.
                          list->add(list, $atomicFormulaSkeleton.value, &free);
                          }
                          )+ ')'
    {
    $domain::item->numOfPredicates = list->size(list);
    $domain::item->predicates = malloc(sizeof(*$domain::item->predicates) *
                                       $domain::item->numOfPredicates);
    for (int i = 0; i < $domain::item->numOfPredicates; ++i) {
        // Index in ANTLR3-List starts at 1!
        $domain::item->predicates[i] = *(struct predicate *)
                                       list->get(list, i+1);
    }
    }
    ;

/* Funtion definitions */
//TODO

/*/ Constraint definitions */
//TODO

/* Structure definitions */
structureDef returns [struct action *value]
    :   actionDef { $value = $actionDef.value; }
    //| durativeActionDef       // requires :durative-actions
    //| derivedDef              // requires :derived-predicates
    ;

actionDef returns [struct action *value]
@init {
    pANTLR3_LIST var_list = antlr3ListNew(LIST_SIZE_INIT);
}
@after {
    var_list->free(var_list);
}
    :   '(' ':action' actionSymbol
        ':parameters' '(' typedList_variable[var_list] ')' 
        actionDefBody ')'
        {
        $value = malloc(sizeof(*$value));
        strncpy($value->name,
                (char *) $actionSymbol.text->chars,
                NAME_LENGTH_MAX);
        $value->numOfParameters = var_list->size(var_list);
        $value->parameters = malloc(sizeof(*$value->parameters) *
                                    $value->numOfParameters);
        for (int i = 0; i < $value->numOfParameters; ++i) {
            // antlr3 list indizes are based on 1
            $value->parameters[i] = *(struct variable *)
                                    var_list->get(var_list, i+1);
        }
        $value->precondition = $actionDefBody.value_precondition;
        $value->effect = $actionDefBody.value_effect;
        }
    ;

actionSymbol
    :   NAME
    ;

actionDefBody returns [struct formula *value_precondition,
                       struct formula *value_effect]
@init {
    $value_precondition = NULL;
    $value_effect = NULL;
}
    :   (':precondition' precond=emptyOr_preconditionGoalDescription
        {
        $value_precondition = $precond.value;
        }
        )?
        (':effect' emptyOr_effect
        {
        $value_effect = $emptyOr_effect.value;
        }
        )?
    ;

emptyOr_preconditionGoalDescription returns [struct formula *value]
    :   '(' ')'
        {
        $value = NULL;
        }
    |   preconditionGoalDescription
        {
        $value = $preconditionGoalDescription.value;
        }
    ;

emptyOr_effect returns [struct formula *value]
    :   '(' ')'
        {
        $value = NULL;
        }
    |   effect
        {
        $value = $effect.value; // TODO: What? I think it is done.
        }
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
preconditionGoalDescription returns [struct formula *value]
    : goalDescription
        {
        $value = $goalDescription.value;
        }
    ;

preferencesGoalDescription
    :   goalDescription
    //| '(' 'preference' preferenceName? goalDescription ')'  // requires :preferences
    ;

preferenceName
    :   NAME
    ;

goalDescription returns [struct formula *value]
@init {
    pANTLR3_LIST and_list = antlr3ListNew(LIST_SIZE_INIT);
}
@after {
    and_list->free(and_list);
}
    /*
    : atomicFormula_term
    | literal_term // requires :negative-preconditions
    */
    //: atomicFormula_term
    // The following line includes atomicFormula_term
    : literal_term // requires :negative-preconditions TODO
        {
        $value = $literal_term.value;
        }
    | '(' 'and' (gd=goalDescription {
                                    and_list->add(and_list,
                                                  $gd.value,
                                                  NULL);
                                    }
    
                )* ')'
        {
        $value = malloc(sizeof(*$value));
        $value->type = AND;
        $value->item.and_formula.numOfParameters = and_list->size(and_list);
        $value->item.and_formula.p =malloc(sizeof(*$value->item.and_formula.p) *
                                      $value->item.and_formula.numOfParameters);
        for (int i = 0; i < $value->item.and_formula.numOfParameters; ++i) {
            // antlr3 list index starts from 1.
            $value->item.and_formula.p[i] = *(struct formula *)
                                            and_list->get(and_list, i+1);
        }
        }
    //| '(' 'forall' '(' typedList_variable ')' goalDescription ')' // requires :universal-preconditions
    //| '(' 'or' goalDescription* ')' // requires :disjunctive-preconditions
    //| '(' 'not' goalDescription* ')' // requires :disjunctive-preconditions
    //| '(' 'imply' goalDescription goalDescription ')' // requires :disjunctive-preconditions
    //| '(' 'exists' '(' typedList_variable ')' goalDescription ')' // requires :existential-preconditions
    //| fComp // requires :atomic-fluents
    ;

effect returns [struct formula *value]
@init {
        pANTLR3_LIST and_list = antlr3ListNew(LIST_SIZE_INIT);
}
@after {
        and_list->free(and_list);
}
    : '(' 'and' (cEffect  {
                            and_list->add(and_list, $cEffect.value, NULL);
                            }
                  )* ')'
        {
        $value = malloc(sizeof(*$value));
        $value->type = AND;
        $value->item.and_formula.numOfParameters = and_list->size(and_list);
        $value->item.and_formula.p =malloc(sizeof(*$value->item.and_formula.p) *
                                      $value->item.and_formula.numOfParameters);
        for (int i = 0; i < $value->item.and_formula.numOfParameters; ++i) {
            // Antlr3 list index starts from 1.
            $value->item.and_formula.p[i] = *(struct formula *)
                                            and_list->get(and_list, i+1);
        }
        }
    | cEffect
        {
        $value = $cEffect.value;
        }
    ;

cEffect returns [struct formula *value]
    //: '(' 'forall' '(' typedList_variable ')' effect ')' // requires :conditional-effects
    //| '(' 'when' goalDescription condEffect ')' // requires :conditional-effects
    //|
    : pEffect
        {
        $value = $pEffect.value;
        }
    ;

condEffect
    : '(' 'and' pEffect* ')'
    | pEffect
    ;

pEffect returns [struct formula *value]
// TODO: check: That is the same as a literal_term - right?
    : '(' 'not' atomicFormula_term ')'
        {
        $value = malloc(sizeof(*$value));
        $value->type = NOT;
        $value->item.not_formula.p = $atomicFormula_term.value;
        }
    | atomicFormula_term
        {
        $value = $atomicFormula_term.value;
        }
    //| '(' assignOp fHead fExp ')' // requires :numeric-fluents
    //| '(' 'assign' function_term term ')' //requires :object-fluents
    //| '(' 'assign' function_term 'undefined' ')' //requires :object-fluents
    ;
