%{

#include <stdio.h>
#include "enums.h"
#include "golang.tab.h"

%}


floatnum [0-9]+\.[0-9]*|[0-9]*\.[0-9]+
integer [0-9]+
id [a-zA-Z_][a-zA-Z0-9_]*

%%
[ \t] {}

{floatnum} {sscanf(yytext,"%s",yylval.val);return FLT;}
{integer} {sscanf(yytext,"%s",yylval.val);return NUM;}


"package"   {return(PACKAGE);}
"import"    {return(IMPORT);}
"func"      {return(FUNC);}


"{"     {Level++; return (LCPAREN);}
"}"     {Level--; return (RCPAREN);}


"if"        {Level++; return (IF);}
"else"      {Level++; return (ELSE);}
"print"     {return (PRINT);}
"while"     {LoopAdd(); Level++; return (WHILE);}


"("     {return (LRPAREN);}
")"     {return (RRPAREN);}
"["     {return (LSPAREN);}
"]"     {return (RSPAREN);}

";"     {return (SEMICOLON);    }
"\""    {return (DOUBLEQUOTE); }

"=="    {yylval.Relop = Equal; return (RELOP);}
"!="    {yylval.Relop = NEqual; return (RELOP);}
"<="    {yylval.Relop = Leq; return (RELOP);}
"<"    {yylval.Relop = Le; return (RELOP);}
">="    {yylval.Relop = Geq; return (RELOP);}
">"    {yylval.Relop = Ge; return (RELOP);}

"++"    {return (INCREMENT);   }
"--"    {return (DECREMENT);   }

":="    {return (ASSIGN);}
"="     {return (EQUAL);}

"+"     {return (ADD);  }
"-"     {return (SUB);  }
"*"     {return (MUL);  }
"/"     {return (DIV);  }



{id}    {sscanf(yytext,"%s",yylval.val);return VAR;}


\n+|.    {return yytext[0];}
%%
