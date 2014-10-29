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

    #include "libpddl31.h"
    #include "pddl31structs.h"
    #include "objManag.h"
    #include "predManag.h"
    #include "actionManag.h"
    #include "typeSystem.h"

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

    struct objManag *global_objManag = NULL;
    /* The type system. We always need that, therefor we make it a global
     * variable.
     * It will be initialized by the domain-rule or problem-rule.
     */
    struct typeSystem *global_typeSystem = NULL;
    struct predManag *global_predManag = NULL;
}

/*** Logic ***/
/* Rules for types */
primitiveType
    :   NAME
    |   'object'
    ;

// Should I support that at all? FastDownward does not support that.
// eitherType 
//    :   '(' 'either' primitiveType+ ')'
//    ;

type
    :   primitiveType
    //|   eitherType
    ;

functionType
    : 'number'  // requires action-costs or numberic-fluents
    ;

/* Rules for terms */
term returns [struct term *value]
@init {
    $value = malloc(sizeof(*$value));
}
    : NAME
        {
        $value->isVariable = false;
        $value->name = string_malloc_copy($NAME.text);
        $value->type = global_typeSystem->root;
        }
    | variable
        {
        $value->isVariable = true;
        $value->name = string_malloc_copy($variable.text);
        $value->type = global_typeSystem->root;
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
// A literal is positive or negated atom
literal_term returns [bool value_pos, struct atom *value]
    : atomicFormula_term
        {
        $value_pos = true;
        $value = $atomicFormula_term.value;
        }
    | '(' 'not' atomicFormula_term ')'
        {
        $value_pos = false;
        $value = $atomicFormula_term.value;
        }
    ;

literal_name returns [bool value_pos, struct atom *value]
    : atomicFormula_name
        {
        $value_pos = true;
        $value = $atomicFormula_name.value;
        }
    | '(' 'not' atomicFormula_name ')'
        {
        $value_pos = false;
        $value = $atomicFormula_name.value;
        }
    ;

/* Rules for predicates */
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
    $value->numOfParams = variable_list->size(variable_list);
    $value->params = malloc(sizeof(*$value->params) *
                                $value->numOfParams);
    for (int i = 0; i < $value->numOfParams; ++i) {
        //List index starts from 1
        $value->params[i] = *(struct term *)
                               variable_list->get(variable_list, i+1);
    }
    }
    ;

atomicFormula_term returns [struct atom *value]
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
        $value->pred = predManag_getPred(global_predManag,
                                         (char *) $predicate.text->chars);
        if ($value->pred == NULL) {
            fprintf(stderr,
                    "error parsing predicate '\%s'\n",
                    $predicate.text->chars);
        }
        if (term_list->size(term_list) != $value->pred->numOfParams) {
            fprintf(stderr,
                  "error parsing predicate '\%s'. Wrong number of arguments\n",
                  $predicate.text->chars);
        }
        $value->term = malloc(sizeof(*$value->term) *
                              $value->pred->numOfParams);
        for (size_t i = 0; i < $value->pred->numOfParams; ++i) {
            $value->term[i] = *(struct term *) term_list->get(term_list, i+1);
        }
        }
    //|   '(' '=' term term ')' // requires :equality
    ;

// Atomic formula name is only used by the initial state!
// TODO: Query object manager!
atomicFormula_name returns [struct atom *value]
@init {
    pANTLR3_LIST name_list = antlr3ListNew(LIST_SIZE_INIT);
}
@after {
    name_list->free(name_list);
}
    :   '(' predicate (NAME {
                            char *name = string_malloc_copy($NAME.text);
                            // Do not free here.
                            name_list->add(name_list, name, NULL);
                            }
                      )* ')'
        {
        $value = malloc(sizeof(*$value));
        $value->pred = predManag_getPred(global_predManag,
                                         (char *) $predicate.text->chars);
        if ($value->pred == NULL) {
            fprintf(stderr,
                    "error parsing predicate '\%s'\n",
                    $predicate.text->chars);
        }
        if (name_list->size(name_list) != $value->pred->numOfParams) {
            fprintf(stderr,
                  "error parsing predicate '\%s'. Wrong number of arguments\n",
                  $predicate.text->chars);
        }
        $value->term = malloc(sizeof(*$value->term) *
                              $value->pred->numOfParams);
        for (size_t i = 0; i < $value->pred->numOfParams; ++i) {
            /*
                // TODO TODO TODO !!!
            //$value->term[i].name = name_list->get(name_list, i+1);
            //$value->term[i].isVariable = false;
            //$value->term[i].type = global_typeSystem->root;
            */
            char *name = name_list->get(name_list, i+1);
            $value->term[i] = objManag_getObject(global_objManag, name);
            if ($value->term[i] == NULL) {
                fprintf(stderr,
                        "error parsing object '\%s'. Unkown object\n",
                        name);
            }
        }
        }
    //|   '(' '=' NAME NAME ')' // requires :equality
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
                  struct term *item = malloc(sizeof(*item));
                  item->isVariable = true;
                  item->name = string_malloc_copy($variable.text);
                  item->type = global_typeSystem->root;
                  // Do free structs. They must be COPIED into an array later.
                  $list->add($list, item, &free);
                  }
        )*
    |   (variable {
                  struct term *item = malloc(sizeof(*item));
                  item->isVariable = true;
                  item->name = string_malloc_copy($variable.text);
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
                        struct term *item = local_list->get(local_list, i);
                        item->type = typeSystem_getType(global_typeSystem,
                                                    (char *)$type.text->chars);
                        if (item->type == NULL) {
                            fprintf(stderr,
                                    "type error parsing '\%s - \%s'",
                                    item->name,
                                    $type.text->chars);
                        }
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
              struct term *item = malloc(sizeof (*item));
              item->isVariable = false;
              item->name = string_malloc_copy($NAME.text);
              item->type = global_typeSystem->root;
              // Do free structs. They must be COPIED into an array later.
              $list->add($list, item, &free);
              }
        )*
    |   (NAME {
              struct term *item = malloc(sizeof (*item));
              item->isVariable = false;
              item->name = string_malloc_copy($NAME.text);
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
                        struct term *item = local_list->get(local_list, i);
                        item->type = typeSystem_getType(global_typeSystem,
                                                    (char *)$type.text->chars);
                        if (item->type == NULL) {
                            fprintf(stderr,
                                    "type error parsing '\%s - \%s'",
                                    item->name,
                                    $type.text->chars);
                        }
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
@init {
    $value = malloc(sizeof (*$value));
    // Initialize object manager
    $value->objManag = objManag_create();
    global_objManag = $value->objManag;
    // Initialize type system
    $value->typeSystem = typeSystem_create();
    // Set global pointer to type system
    global_typeSystem = $value->typeSystem;
    // Set default for global predicate manager.
    global_predManag = NULL;

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
        (typesDef[$value->typeSystem])?
        (constantsDef   {
                        objManag_add(   $value->objManag,
                                        $constantsDef.value_num,
                                        $constantsDef.value);
                        free ($constantsDef.value); // TODO: Is it right?
                        }
        )?
        (predicatesDef  {
                        global_predManag = $predicatesDef.value;
                        $value->predManag = global_predManag;        
                        }
        )?
        // Special arrangement to parse action-costs only (and no other
        // functions).
        ( '(' ':functions' '(' 'total-cost' ')' ('-' functionType)? ')' )?
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

        $value->actionManag = actionManag_create(action_list);
        }
    ;

typesDef[struct typeSystem *typeSystem]
    : '(' ':types' typesDefAux[typeSystem] ')'
    ;

// parsing-wise similar to typed list of name
typesDefAux[struct typeSystem *typeSystem]
@init {
    pANTLR3_LIST local_list = antlr3ListNew(LIST_SIZE_INIT);
}
@after {
    local_list->free(local_list);
}
    : (NAME
        {
        bool success = typeSystem_addType(global_typeSystem,
                                          (char *)$NAME.text->chars,
                                          NULL);
        if (!success) {
            fprintf(stderr, "error parsing type '\%s'\n", $NAME.text->chars);
        }
        }
      )*
    | (NAME
        {
        char *name = string_malloc_copy($NAME.text);
        local_list->add(local_list, name, &free);
        }
      )+ '-' type
        {
        for (int i = 0; i < local_list->size(local_list); ++i) {
            // Antlr3 list index starts from 1
            char *typeName = (char *) local_list->get(local_list, i+1);
            bool success = typeSystem_addType(typeSystem,
                                              typeName,
                                              (char *)$type.text->chars);
            if (!success) {
                fprintf(stderr,
                        "error parsing type '\%s - \%s'",
                        typeName,
                        $type.text->chars);
            }
        }
        }
      typesDefAux[typeSystem]
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
    | ':typing'
      {
      *$value = TYPING;
      }
    | ':negative-preconditions'
      {
      *$value = NEGATIVE_PRECONDITION;
      }
//    | ':disjunctive-preconditions'
//    | ':equality'
//    | ':existential-preconditions'
//    | ':universal-preconditions'
//    | ':quantified-preconditions'
    | ':conditional-effects'
      {
      *$value = CONDITIONAL_EFFECTS;
      }
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
    | ':action-costs'
      {
      *$value = ACTION_COSTS;
      }
    ;

/* Types definitions */

/* Constants definitions */
constantsDef returns [int32_t value_num, struct term *value]
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
            $value[i] = *(struct term *) list->get(list, i+1);
        }
        }
    ;

/* Predicate definitions */
predicatesDef returns [struct predManag *value]
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
    $value = predManag_create(list);
    }
    ;

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
        $value->numOfParams = var_list->size(var_list);
        $value->params = malloc(sizeof(*$value->params) *
                                $value->numOfParams);
        for (int i = 0; i < $value->numOfParams; ++i) {
            // antlr3 list indizes are based on 1
            $value->params[i] = *(struct term*)var_list->get(var_list, i+1);
        }
        $value->precond = $actionDefBody.value_precondition;
        $value->effect = $actionDefBody.value_effect;
        }
    ;

actionSymbol
    :   NAME
    ;

actionDefBody returns [struct goal *value_precondition,
                       struct effect *value_effect]
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

emptyOr_preconditionGoalDescription returns [struct goal *value]
    :   '(' ')'
        {
        $value = NULL;
        }
    |   preconditionGoalDescription
        {
        $value = $preconditionGoalDescription.value;
        }
    ;

emptyOr_effect returns [struct effect *value]
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
preconditionGoalDescription returns [struct goal *value]
    : goalDescription
        {
        $value = $goalDescription.value;
        }
    ;

//preferencesGoalDescription
//    :   goalDescription
//    //| '(' 'preference' preferenceName? goalDescription ')'  // requires :preferences
//    ;

//preferenceName
//    :   NAME
//    ;

goalDescription returns [struct goal *value]
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
    : literal_term // requires :negative-preconditions
        {
        $value = malloc(sizeof(*$value));

        bool pos = $literal_term.value_pos;
        $value->numOfPos = pos ? 1 : 0;
        $value->numOfNeg = pos ? 0 : 1;
        $value->posLiterals = pos ? $literal_term.value : NULL;
        $value->negLiterals = pos ? NULL : $literal_term.value;
        }
    | '(' 'and' (gd=goalDescription {
                                    and_list->add(and_list,
                                                  $gd.value,
                                                  &free);
                                    }
    
                )* ')'
        {
        $value = malloc(sizeof(*$value));
        // Count positive and negative literals in goalDescriptions
        $value->numOfPos = 0;
        $value->numOfNeg = 0;
        for (size_t i = 0; i < and_list->size(and_list); ++i) {
            struct goal *subGoal = and_list->get(and_list, i+1);
            $value->numOfPos += subGoal->numOfPos;
            $value->numOfNeg += subGoal->numOfNeg;
        }
        $value->posLiterals = malloc(sizeof(*$value->posLiterals) *
                                     $value->numOfPos);
        $value->negLiterals = malloc(sizeof(*$value->negLiterals) *
                                     $value->numOfNeg);
        size_t posIndex = 0;
        size_t negIndex = 0;
        for (size_t i = 0; i < and_list->size(and_list); ++i) {
            struct goal *subGoal = and_list->get(and_list, i+1);
            for (size_t j = 0; j < subGoal->numOfPos; ++j) {
                $value->posLiterals[posIndex] = subGoal->posLiterals[j];
                posIndex++;
            }
            for (size_t j = 0; j < subGoal->numOfNeg; ++j) {
                $value->negLiterals[negIndex] = subGoal->negLiterals[j];
                negIndex++;
            }
            // Important to free the arrays the sub-goal allocated.
            if (subGoal->posLiterals != NULL) {
                free(subGoal->posLiterals);
            }
            if (subGoal->negLiterals != NULL) {
                free(subGoal->negLiterals);
            }
        }
        assert(posIndex == $value->numOfPos && negIndex == $value->numOfNeg);
        }
    //| '(' 'forall' '(' typedList_variable ')' goalDescription ')' // requires :universal-preconditions
    //| '(' 'or' goalDescription* ')' // requires :disjunctive-preconditions
    //| '(' 'not' goalDescription* ')' // requires :disjunctive-preconditions
    //| '(' 'imply' goalDescription goalDescription ')' // requires :disjunctive-preconditions
    //| '(' 'exists' '(' typedList_variable ')' goalDescription ')' // requires :existential-preconditions
    //| fComp // requires :atomic-fluents
    ;

effect returns [struct effect *value]
@init {
        pANTLR3_LIST and_list = antlr3ListNew(LIST_SIZE_INIT);
}
@after {
        and_list->free(and_list);
}
    : '(' 'and' (cEffect {
                         if ($cEffect.value != NULL) {
                             and_list->add(and_list, $cEffect.value, &free);
                         }
                         }
                  )* ')'
        {
        $value = malloc(sizeof(*$value));
        $value->numOfElems = and_list->size(and_list);
        $value->elems = malloc(sizeof(*$value->elems) * $value->numOfElems);
        for (int i = 0; i < $value->numOfElems; ++i) {
            // Antlr3 list index starts from 1.
            $value->elems[i] = *(struct effectElem *)
                                            and_list->get(and_list, i+1);
        }
        }
    | cEffect
        {
        $value = malloc(sizeof(*$value));
        $value->numOfElems = 1;
        $value->elems = $cEffect.value;
        }
    ;

// Attention: May return NULL.
cEffect returns [struct effectElem *value]
@init {
    pANTLR3_LIST var_list = antlr3ListNew(LIST_SIZE_INIT);
}
@after {
    var_list->free(var_list);
}
    : '(' 'forall' '(' typedList_variable[var_list] ')' effect ')'
        {
        $value = malloc(sizeof(*$value));
        $value->type = FORALL;
        $value->it.forall = malloc(sizeof(*$value->it.forall));
        $value->it.forall->numOfVars = var_list->size(var_list);
        $value->it.forall->vars = malloc(sizeof(*$value->it.forall->vars) *
                                         $value->it.forall->numOfVars);
        for (int i = 0; i < $value->it.forall->numOfVars; ++i) {
            $value->it.forall->vars[i] = *(struct term *)
                                                var_list->get(var_list, i+1);
        }
        $value->it.forall->effect = $effect.value;
        }
    | condEffectPre
        {
        $value = $condEffectPre.value;
        }
    | pEffect
        {
        if ($pEffect.value == NULL) {
            $value = NULL;
        } else {
            $value = malloc(sizeof(*$value));
            $value->type = $pEffect.value_pos ? POS_LITERAL : NEG_LITERAL;
            $value->it.literal = $pEffect.value;
        }
        }
    ;

// May return NULL
condEffectPre returns [struct effectElem *value]
@init {
    pANTLR3_LIST posAtoms = antlr3ListNew(LIST_SIZE_INIT);
    pANTLR3_LIST negAtoms = antlr3ListNew(LIST_SIZE_INIT);
}
@after {
    posAtoms->free(posAtoms);
    negAtoms->free(negAtoms);
}
    : '(' 'when' goalDescription condEffect[posAtoms, negAtoms] ')'
        {
        if (posAtoms->size(posAtoms) == 0 && negAtoms->size(negAtoms) == 0) {
            $value = NULL;
        } else {
            $value = malloc(sizeof(*$value));
            $value->type = WHEN;
            struct when *when = $value->it.when;
            when->precond = $goalDescription.value;
            when->numOfPos = posAtoms->size(posAtoms);
            when->numOfNeg = negAtoms->size(negAtoms);
            when->posLiterals = malloc(sizeof(*when->posLiterals) *
                                         when->numOfPos);
            when->negLiterals = malloc(sizeof(*when->negLiterals) *
                                         when->numOfNeg);
            for (int i = 0; i < when->numOfPos; ++i) {
                when->posLiterals[i] = *(struct atom *)
                                                posAtoms->get(posAtoms, i+1);
            }
            for (int i = 0; i < when->numOfNeg; ++i) {
                when->negLiterals[i] = *(struct atom *)
                                                negAtoms->get(negAtoms, i+1);
            }
        }
        }
    ;

// This rule fills the lists passed as arguments.
condEffect[pANTLR3_LIST posAtoms, pANTLR3_LIST negAtoms]
    : '(' 'and' (pEffect
                        {
                        if ($pEffect.value == NULL) {
                            // Do not do anything
                        } else {
                            if ($pEffect.value_pos) {
                                // Do free?
                                $posAtoms->add($posAtoms,
                                               $pEffect.value,
                                               &free);
                            } else {
                                assert (!$pEffect.value_pos);
                                // Do free?
                                $negAtoms->add($negAtoms,
                                               $pEffect.value,
                                               &free);
                            }
                        }
                        }
        
                        )* ')'
    | pEffect
        {
        if ($pEffect.value == NULL) {
            // Do not do anything
        } else {
            if ($pEffect.value_pos) {
                // Do free?
                $posAtoms->add($posAtoms,
                               $pEffect.value,
                               &free);
            } else {
                assert (!$pEffect.value_pos);
                // Do free?
                $negAtoms->add($negAtoms,
                               $pEffect.value,
                               &free);
            }
        }
        }
    ;

// Attention: May return NULL.
pEffect returns [bool value_pos, struct atom *value]
    : '(' 'not' atomicFormula_term ')'
        {
        $value_pos = false;
        $value = $atomicFormula_term.value;
        }
    | atomicFormula_term
        {
        $value_pos = true;
        $value = $atomicFormula_term.value;
        }
    // Special arrangement to parse action-costs
    | '(' 'increase' '(' 'total-cost' ')' NUMBER ')'
        {
        $value = NULL; // we have to check that in the caller.
        $value_pos = true;
        }
    //| '(' assignOp fHead fExp ')' // requires :numeric-fluents
    //| '(' 'assign' function_term term ')' //requires :object-fluents
    //| '(' 'assign' function_term 'undefined' ')' //requires :object-fluents
    ;

///*** Problem ***/
problem[struct domain *domain] returns [struct problem *value]
@init {
    $value = malloc(sizeof(*value));

    bool hasRequirements = false;
    bool hasObjects = false;

    global_objManag = $domain->objManag;
    // Set global pointer to type system
    global_typeSystem = $domain->typeSystem;
    // Set default for global predicate manager.
    global_predManag = $domain->predManag;
}
    : '(' 'define' '(' 'problem' pName=NAME ')'
            '(' ':domain' dName=NAME ')'
            (requireDef         { hasRequirements = true; } )?
            (od=objectDeclaration  {
                                // Adding the objects to the domains
                                // object manager
                                objManag_add(   $domain->objManag,
                                                $od.value_num,
                                                $od.value);
                                }
            
            )?
            init
            goal
            //constraints?  // requires :constraints
            // action-costs only allow this special objective
            ('(' ':metric' 'minimize' '(' 'total-cost' ')' ')')?
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

      // Write initial state to struct problem
      $value->init = $init.value;

      // Write goal to struct problem
      $value->goal = $goal.value;
      }
    ;

objectDeclaration returns [int32_t value_num, struct term *value]
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
              $value[i] = *(struct term *) obj_list->get(obj_list, i+1);
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
                          // Check for NULL, because action-cost
                          // initializations return NULL.
                          if ($initEl.value != NULL) {
                              init_list->add(init_list, $initEl.value, &free);
                          }
                          }
                  )* ')'
        {
        $value = malloc(sizeof(*$value));

        $value->numOfFluents = init_list->size(init_list);
        if ($value->numOfFluents == 0) {
            $value->fluents = NULL;
        } else {
            $value->fluents = malloc(   sizeof(*$value->fluents) *
                                        $value->numOfFluents);
            for (int i = 0; i < $value->numOfFluents; ++i) {
                $value->fluents[i] = *(struct atom *)
                                     init_list->get(init_list, i+1);
            }
        }
        }
    ;

/*** Closed world assumption applies. ***/
// Negative literals will be ignored and only positive literals
// will be included in the state
// I have to parse negative literals just because of pddl 3.1
// specification.
initEl returns [struct atom *value]
    : literal_name
      {
      // Negative literals will be ignored, because of CWA.
      if (! $literal_name.value_pos) {
          $value = NULL;
          // Should we free that?
          libpddl31_atom_free($literal_name.value);
      } else {
          $value = $literal_name.value;
      }
      }
      // Special arrangement for action-costs. According to the pddl 3.1
      // specification total-cost must be initialized to 0.
      // I will just ignore that since the algorithm will just ignore action
      // costs.
    | '(' '=' '(' 'total-cost' ')' '0' ')'
      {
      $value = NULL;
      }
    //| '(' 'at' <number> <literal(name)> ')' // requires :timed−initial−literals 
    //| '(' '=' <basic-function-term> <number> ')' // requires :numeric-fluents 
    //| '(' '=' <basic-function-term> <name> ')' // requires :object-fluents
    ;

goal returns [struct goal *value]
    : '(' ':goal' preconditionGoalDescription ')'
      {
      $value = $preconditionGoalDescription.value;
      }
    ;
