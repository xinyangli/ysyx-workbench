%{
    #include <addrexp.h>
%}
%option noyywrap

%%

0[xX][0-9a-fA-F]+ { yylval = strtoul(yytext, NULL, 16); return HEX_NUMBER; }
[0-9]+            { yylval = strtoul(yytext, NULL, 10); return NUMBER; }
[+\-*/()]         { return *yytext; }
[ \t]             {  }
.                 { printf("Unexpected character: %s\n", yytext); }
%%
