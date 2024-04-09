%code requires {
    #include <types.h>
    #include <stdio.h>
    #include <stdlib.h>
    extern int yylex(void);
}
%{
    #include <types.h>
    #include <stdio.h>
    #include <stdlib.h>
    void yyerror(word_t *result, const char *err) {
      fprintf(stderr, "%s", err);
    }
    int pmem_read(int raddr);
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
          fprintf(stderr, "Error: divide by zero at" FMT_WORD " / " FMT_WORD "\n", $1, $3);
          YYABORT;
        };
        $$ = $1 / $3;
      }
    | '-' number { $$ = -$2; }
    | '*' expression { $$ = pmem_read($2); }
    | '(' expression ')' { $$ = $2; }

number
    : REGISTER
    | NUMBER
    | HEX_NUMBER

%%
