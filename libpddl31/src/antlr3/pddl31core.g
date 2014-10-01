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
