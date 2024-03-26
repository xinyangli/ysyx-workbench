%code requires {
    #include <common.h>
    #include <memory/vaddr.h>
    #include <stdio.h>
    #include <stdlib.h>
    extern int yylex(void);
}
%{
    #include <common.h>
    #include <utils.h>
    #include <isa.h>
    #include <stdio.h>
    #include <stdlib.h>
    void yyerror(word_t *result, const char *err) {
      Error("%s", err);
    }
%}

%token NUMBER HEX_NUMBER
%token REGISTER
%locations
%start input
%define api.value.type { word_t }
%parse-param { uint32_t *result }
%left '-' '+'
%left '*' '/'

%%
input
    : expression { *result = $1; }
    ;

expression
    : number { $$ = $1; }
    | expression '>' '=' expression { $$ = ($1 >= $4); }
    | expression '<' '=' expression { $$ = ($1 <= $4); }
    | expression '=' '=' expression { $$ = ($1 == $4); }
    | expression '!' '=' expression { $$ = ($1 == $4); }
    | expression '>' expression { $$ = ($1 > $3); }
    | expression '<' expression { $$ = ($1 < $3); }
    | expression '+' expression { $$ = $1 + $3; }
    | expression '-' expression { $$ = $1 - $3; }
    | expression '*' expression { $$ = $1 * $3; } 
    | expression '/' expression {
        if($3 == 0) {
          fprintf(stderr, "Error: divide by zero at %u / %u\n", $1, $3);
          YYABORT;
        };
        $$ = $1 / $3;
      }
    | '-' number { $$ = -$2; }
    | '*' expression { $$ = vaddr_read($2, WORD_BYTES); }
    | '(' expression ')' { $$ = $2; }

number
    : REGISTER
    | NUMBER 
    | HEX_NUMBER 

%%
