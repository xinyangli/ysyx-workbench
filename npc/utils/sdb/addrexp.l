%{
    #include <types.h>
    #include <stdbool.h>
    #include "addrexp.h"
    static bool success = false;
    void yyerror(word_t *result, const char *err);
    word_t reg_str2val(const char *name, bool*);
%}
%option noyywrap

%%

0[xX][0-9a-fA-F]+ { yylval = strtoul(yytext, NULL, 16); return HEX_NUMBER; }
[0-9]+            { yylval = strtoul(yytext, NULL, 10); return NUMBER; }
$[asgprt$][0-9pa][0-9]?    {
    yylval = reg_str2val(yytext + 1, &success);
    if(!success) {
        yyerror(NULL, "Failed to convert reg to value");
        return YYerror;
    }
    return REGISTER;
}
[+\-*/<=()]         { return *yytext; }
[ \t]             {  }
.                 { printf("Unexpected character: %s\n", yytext); return YYerror; }
%%
