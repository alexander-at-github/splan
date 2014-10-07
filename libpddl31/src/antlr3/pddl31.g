grammar pddl31;     // Combined grammar. That is, parser and lexer.

options { /* Attetion: options must be placed right after grammar-statementi. */
          /* Otherwise ANTLR3 will not translate it (error(100)). */
    language = C;
}

import pddl31core;

@header {
    #include <assert.h>
    #include <stdbool.h>
    #include <stdlib.h>
    #include <string.h>

    #include <antlr3interfaces.h>
    #include <antlr3string.h>

    #include "pddl31structs.h"
    #include "libpddl31.h"

    // Size of lists when initialized
    #define LIST_SIZE_INIT 16
}
@members {
    /* A helper function. Allocates memory for a string and copies content from
     * src to newly allocated memory.
     * returns destination
     */
    char *string_malloc_copy(pANTLR3_STRING src)
    {
        // antlr3-string->size includes terminating '\0' byte
        char *dest = malloc(sizeof(*dest) * src->size);
        if (dest == NULL) {
            return NULL;
        }
        strncpy(dest, (char *) src->chars, src->size);
        // Set last byte.
        dest[src->size - 1] = '\0';
        return dest;
    }
}

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
        $value->item.constArgument->name = string_malloc_copy($NAME.text);
        $value->item.constArgument->isTyped = false;
        }
    | variable
        {
        $value->type = VARIABLE;
        $value->item.varArgument = malloc(sizeof(*$value->item.varArgument));
        $value->item.varArgument->name = string_malloc_copy($variable.text);
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

literal_name returns [struct formula *value]
    : atomicFormula_name
        {
        $value = $atomicFormula_name.value;
        }
    | '(' 'not' atomicFormula_name ')'
        {
        $value = malloc(sizeof(*$value));
        $value->type = NOT;
        $value->item.not_formula.p = $atomicFormula_name.value;
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
    $value->name = string_malloc_copy($predicate.text);
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
        value->item.predicate_formula.name =string_malloc_copy($predicate.text);
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
    //|   '(' '=' term term ')' // requires :equality
    //    {
    //    // TODO
    //    }
    ;

atomicFormula_name returns [struct formula *value]
@init {
    pANTLR3_LIST name_list = antlr3ListNew(LIST_SIZE_INIT);
}
@after {
    name_list->free(name_list);
}
    :   '(' predicate (NAME {
                            // names are constants (which are terms)
                            struct term *term = malloc(sizeof(*term));
                            term->type = CONSTANT;
                            term->item.constArgument =
                                    malloc(sizeof(*term->item.constArgument));
                            term->item.constArgument->name =
                                                string_malloc_copy($NAME.text);
                            term->item.constArgument->isTyped = false;
                            name_list->add(name_list,
                                           term,
                                           &free);
                            }
                      )* ')'
        {
        $value = malloc(sizeof(*$value));
        $value->type = PREDICATE;
        $value->item.predicate_formula.name =
                                            string_malloc_copy($predicate.text);
        $value->item.predicate_formula.numOfArguments =
                                                    name_list->size(name_list);
        $value->item.predicate_formula.arguments =
                     malloc(sizeof(*$value->item.predicate_formula.arguments) *
                     $value->item.predicate_formula.numOfArguments);
        for (int i = 0; i < $value->item.predicate_formula.numOfArguments; ++i){
            //List index starts from 1
            $value->item.predicate_formula.arguments[i] = *(struct term *)
                                                name_list->get(name_list, i+1);
        }
        }
    //|   '(' '=' NAME NAME ')' // requires :equality
    //    {
    //    // TODO
    //    }
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
                  item->name = string_malloc_copy($variable.text);
                  item->isTyped = false;
                  item->type = NULL;
                  // Do free structs. They must be COPIED into an array later.
                  $list->add($list, item, &free);
                  }
        )*
    /* *** requires typing ***
    |   (variable {
                  struct variable *item = malloc(sizeof(*item));
                  item->name = string_malloc_copy($variable.text);
                  item->isTyped = true;
                  item->type = NULL;
                  // Save these names in a local list first.
                  // Don't free. 'list' will do that.
                  local_list->add(local_list, item, NULL);
                  }
        )+ '-' type {
                    // Now we know the type of the identifiers.
                    // ANTLR3_LIST index starts from 1
                    for (int i = 1; i <= local_list->size(local_list); ++i) {
                        struct type *t = malloc(sizeof(*t));;
                        t->name = string_malloc_copy($type.text);
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
    */
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
              item->name = string_malloc_copy($NAME.text);
              item->isTyped = false;
              item->type = NULL;
              // Do free structs. They must be COPIED into an array later.
              $list->add($list, item, &free);
              }
        )*
    /* *** requires typing ***
    |   (NAME {
              struct constant *item = malloc(sizeof (*item));
              item->name = string_malloc_copy($NAME.text);
              item->isTyped = true;
              // Type will be assigned later.
              item->type = NULL;
              // Save these names in a local list first.
              // Don't free. 'list' will do that.
              local_list->add(local_list, item, NULL);
              }
        )+ '-' type {
                    // Now we know the type of the identifiers.
                    // ANTLR3_LIST index starts from 1
                    for (int i = 1; i <= local_list->size(local_list); ++i) {
                        // Add type information
                        struct constant *item = local_list->get(local_list, i);
                        // Malloc type for every instance of "object",
                        // cause it is much easier to free. Now you can just
                        // free it on any " object". That is not efficient. If
                        // typing is realy needed this should be improved.
                        struct type *t = malloc(sizeof(*t));
                        t->name = string_malloc_copy($type.text);
                        item->type = t;
                        // Add names to input-output list
                        // Do free structs. They must be COPIED into an array
                        // later.
                        $list->add($list, item, &free);
                    }
                    }
        typedList_name[$list] // requires :typing
    */
    ;



/*** Domain ***/
domain returns [struct domain *value]
@init {
    $value = malloc(sizeof (*$value));

    bool hasRequirements = false;
    bool hasConstants = false;
    bool hasPredicates = false;

    pANTLR3_LIST action_list = antlr3ListNew(LIST_SIZE_INIT);
}
@after {
    action_list->free(action_list);
}
// Don't free the domain struct, cause that's what we are actually
// producing.
    :   '(' 'define' domainName
        (requireDef     { hasRequirements = true; } )?
        // typesDef?    // requires :typing
        (constantsDef   { hasConstants = true; }    )?
        (predicatesDef  { hasPredicates = true; }   )?
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
        $value->name = string_malloc_copy($domainName.value);

        // Write requrements into struct domain
        if (hasRequirements) {
            $value->numOfRequirements = $requireDef.value_num;
            $value->requirements = $requireDef.value;
        } else {
            $value->numOfRequirements = 0;
            $value->requirements = NULL;
        }

        // Write constants into struct domain
        if (hasConstants) {
            $value->numOfConstants = $constantsDef.value_num;
            $value->constants = $constantsDef.value;
        } else {
            $value->numOfConstants = 0;
            $value->constants = NULL;
        }

        // Write predicates into struct domain
        if (hasPredicates) {
            $value->numOfPredicates = $predicatesDef.value_num;
            $value->predicates = $predicatesDef.value;
        } else {
            $value->numOfPredicates = 0;
            $value->predicates = NULL;
        }

        // Copy actions (from structureDef) into domain struct.
        $value->numOfActions = action_list->size(action_list);
        $value->actions = malloc(sizeof(*$value->actions) *
                                        $value->numOfActions);
        for (int i = 0; i < $value->numOfActions; ++i) {
            // antlr list index starts from 1
            $value->actions[i] = *(struct action *)
                                        action_list->get(action_list, i+1);
        }
        if ($value->numOfActions == 0) {
            $value->actions = NULL;
        }
        }
    ;

/* Domain name */
domainName returns [pANTLR3_STRING value]
    :   '(' 'domain' NAME ')'
        {
        $value = $NAME.text;
        }
    ;

/* Requirements definitions */
requireDef returns [int32_t value_num,
                    enum requirement *value]
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
        $value_num = req_list->size(req_list);
        $value = malloc(sizeof(*$value) * $value_num);
        for (int i = 0; i < $value_num; ++i) {
            // Index in antlr3 list starts at 1
            $value[i] = *(enum requirement *) req_list->get(req_list, i+1);
        }
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
//    | ':typing' // TODO or not todo?
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
constantsDef returns [int32_t value_num, struct constant *value]
@init {
    pANTLR3_LIST list = antlr3ListNew(LIST_SIZE_INIT);
}
@after {
    list->free(list);
}
    :   '(' ':constants' typedList_name[list] ')'
        {
        $value_num = list->size(list);
        $value = malloc(sizeof(*$value) * $value_num);
        for (int i = 0; i < $value_num; ++i) {
            // antlr3 lists start at 1
            $value[i] = *(struct constant *) list->get(list, i+1);
        }
        }
    ;

/* Predicate definitions */
predicatesDef returns [int32_t value_num, struct predicate *value]
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
    $value_num = list->size(list);
    $value = malloc(sizeof(*$value) * $value_num);
    for (int i = 0; i < $value_num; ++i) {
        // Index in ANTLR3-List starts at 1!
        $value[i] = *(struct predicate *) list->get(list, i+1);
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
        $value->name = string_malloc_copy($actionSymbol.text);
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
        $value = $effect.value;
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
    : atomicFormula_merm
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
                                                  &free);
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
        if ($value->item.and_formula.numOfParameters == 0) {
            $value->item.and_formula.p = NULL;
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
    : '(' 'and' (cEffect {
                         and_list->add(and_list, $cEffect.value, &free);
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

/*** Problem ***/
problem returns [struct problem *value]
@init {
    $value = malloc(sizeof(*value));

    bool hasRequirements = false;
    bool hasObjects = false;
}
    : '(' 'define' '(' 'problem' pName=NAME ')'
            '(' ':domain' dName=NAME ')'
            (requireDef         { hasRequirements = true; } )?
            (objectDeclaration  { hasObjects = true; }      )?
            init
            goal
            //constraints?  // requires :constraints
            //metricSpec?   // requires :numeric-fluents
            //lengthSpec?   // deprecated since PDDL 2.1
      ')'
      {
      // Write name into struct problem
      $value->name = string_malloc_copy($pName.text);

      // Write coresponding domain name into struct problem
      $value->domainName = string_malloc_copy($dName.text);

      // Write requirements into struct problem.
      if (hasRequirements) {
          $value->numOfRequirements = $requireDef.value_num;
          $value->requirements = $requireDef.value;
      } else {
          $value->numOfRequirements = 0;
          $value->requirements = NULL;
      }

      // Write objects into struct problem.
      if (hasObjects) {
          $value->numOfObjects = $objectDeclaration.value_num;
          $value->objects = $objectDeclaration.value;
      } else {
          $value->numOfObjects = 0;
          $value->objects = NULL;
      }

      // Write initial state to struct problem
      $value->init = $init.value;

      // Write goal to struct problem
      $value->goal = $goal.value;
      }
    ;

objectDeclaration returns [int32_t value_num, struct constant *value]
@init {
    pANTLR3_LIST obj_list = antlr3ListNew(LIST_SIZE_INIT);
}
@after {
    obj_list->free(obj_list);
}
    : '(' ':objects' typedList_name[obj_list] ')'
      {
      $value_num = obj_list->size(obj_list);
      if ($value_num == 0) {
        $value = NULL;
      } else {
          $value = malloc (sizeof(*$value) * $value_num);
          for (int i = 0; i < $value_num; ++i) {
              // antlr3 list index starts from 1
              $value[i] = *(struct constant *) obj_list->get(obj_list, i+1);
          }
      }
      }
    ;

init returns [struct state *value]
@init {
    pANTLR3_LIST init_list = antlr3ListNew(LIST_SIZE_INIT);
}
@after {
    init_list->free(init_list);
}
    : '(' ':init' (initEl {
                          init_list->add(init_list, $initEl.value, &free);
                          }
                  )* ')'
        {
        $value = malloc(sizeof(*$value));
        /*** Closed world assumption applies. ***/
        // Negative literals will be ignored and only positive literals
        // will be included in the state
        // I have to parse negative literals just because of pddl 3.1
        // specification.

        pANTLR3_LIST fluent_list = antlr3ListNew(LIST_SIZE_INIT);
        for (int i = 0; i < init_list->size(init_list); ++i) {
            // antlr3 list index starts from 1
            struct formula *f =
                            (struct formula *) init_list->get(init_list, i+1);
            switch (f->type) {
            case PREDICATE: {
                // Do not use free here, cause init_list->free() will free the
                // elements.
                fluent_list->add(fluent_list, f, NULL);
                break;
            }
            case AND: {
                // init should be a set of fluents. ANDs are not allowed.
                assert(false);
                // Free unused structures immediately
                libpddl31_formula_free_rec(f);
                break;
            }
            case NOT: {
                // We just ignore NOT-formulas, because of closed world
                // assumption.

                // Free unused structures immediately
                libpddl31_formula_free_rec(f);
                break;
            }
            default: {
                assert(false);
                break;
            }
            }
        }
        // Now all positive literals are in the fluent_list.
        $value->numOfFluents = fluent_list->size(fluent_list);
        $value->fluents = malloc(   sizeof(*$value->fluents) *
                                    $value->numOfFluents);
        for (int i = 0; i < $value->numOfFluents; ++i) {
            struct formula *form =   (struct formula *)
                                     fluent_list->get(fluent_list, i+1);
            assert (form->type == PREDICATE);
 
            struct fluent fluent;
            fluent.name = form->item.predicate_formula.name;
            fluent.numOfArguments = form->item.predicate_formula.numOfArguments;
            fluent.arguments = malloc(  sizeof(*fluent.arguments) *
                                        fluent.numOfArguments);
            for (int j = 0; j < fluent.numOfArguments; ++j) {
                if (form->item.predicate_formula.arguments[j].type == VARIABLE){
                    assert (false && "ungrounded predicate in initial state");
                    libpddl31_term_free(
                                    &form->item.predicate_formula.arguments[j]);
                    continue;
                }
                assert (form->item.predicate_formula.arguments[j].type ==
                                                                    CONSTANT);
                // Copy constant arguments into fluent
                fluent.arguments[j] =
                  *form->item.predicate_formula.arguments[j].item.constArgument;
                // Tricky free() calls!
                free(form->item.predicate_formula.arguments[j].
                                                            item.constArgument);
            }
            free(form->item.predicate_formula.arguments);
            
            // Copy fluent into state
            memcpy(&$value->fluents[i], &fluent, sizeof(fluent));
        }
        fluent_list->free(fluent_list);
        }

    ;

initEl returns [struct formula *value]
    : literal_name
      {
      $value = $literal_name.value;
      }
    //| '(' 'at' <number> <literal(name)> ')' // requires :timed−initial−literals 
    //| '(' '=' <basic-function-term> <number> ')' // requires :numeric-fluents 
    //| '(' '=' <basic-function-term> <name> ')' // requires :object-fluents
    ;

goal returns [struct formula *value]
    : '(' ':goal' preconditionGoalDescription ')'
      {
      $value = $preconditionGoalDescription.value;
      }
    ;
