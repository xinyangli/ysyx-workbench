%{
    #include <stdio.h>
    #include <stdlib.h>
    #include <stdint.h>
    extern int yylex(void);
    void yyerror(uint32_t *result, const char *err) {
        fprintf(stderr, "Error: %s\n", err);
    }
%}

%token NUMBER HEX_NUMBER
%start input
%define api.value.type { uint32_t }
%parse-param { uint32_t *result }
%left '-' '+'
%left '*' '/'

%%
input
    : expression { *result = $1; }
    ;

expression
    : number { $$ = $1; }
    | expression '+' expression { $$ = $1 + $3; }
    | expression '-' expression { $$ = $1 - $3; }
    | expression '*' expression { $$ = $1 * $3; } 
    | expression '/' expression { if($3 == 0) { perror("Devide by 0\n"); YYABORT; }; $$ = $1 / $3;}
    | '-' number { $$ = -$2; }
    | '(' expression ')' { $$ = $2; }

number
    : NUMBER 
    | HEX_NUMBER 

%%
