lexer grammar pddl31core;

@header {

}

// Rules with the keyword 'fragment' can only be used in lexer rules.

NAME	:	LETTER ANYCHAR*
	;
fragment LETTER
	:	'a'..'z' | 'A'..'Z'
	;
fragment ANYCHAR
	:	LETTER | DIGIT | '-' | '_'
	;
NUMBER
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
