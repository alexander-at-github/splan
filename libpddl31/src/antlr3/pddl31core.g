lexer grammar pddl31core;

@header {

}

NAME	:	LETTER ANYCHAR*
	;
fragment LETTER
	:	'a'..'z' | 'A'..'Z'
	;
fragment ANYCHAR
	:	LETTER | DIGIT | '-' | '_'
	;
fragment NUMBER
	:	DIGIT+ DECIMAL?
	;
fragment DIGIT
	:	'0'..'9'
	;
fragment DECIMAL
	:	'.' DIGIT+
	;

LINE_COMMENT
	:	';' ~('\n'|'\r')* '\r'? '\n' { SKIP(); }
	;
WHITESPACE
	:	(' '|'\t'|'\r'|'\n')+ { SKIP(); }
	;

REQUIRE_KEY
    : ':strips'
//    | ':typing'
    | ':negative-preconditions'
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
