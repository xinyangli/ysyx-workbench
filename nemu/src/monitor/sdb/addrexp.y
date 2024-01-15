%code requires {
    #include <common.h>
    #include <stdio.h>
    #include <stdlib.h>
    extern int yylex(void);
}
%{
    #include <common.h>
    #include <stdio.h>
    #include <stdlib.h>
    void yyerror(word_t *result, const char *err) {
      fprintf(stderr, "Error: %s\n", err);
    }
%}

%token NUMBER HEX_NUMBER
%token REGISTER
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
    | register { $$ = $1; }
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
    | '(' expression ')' { $$ = $2; }

number
    : NUMBER 
    | HEX_NUMBER 

register
    : REGISTER

%%
