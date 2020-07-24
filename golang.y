/* 
Reference: http://dinosaur.compilertools.net/bison/bison_5.html
*/

%{

#include<stdio.h>
#include<stdlib.h>
#include "enums.h"

%}


%union {
    char val[1024];
    TokensT Token; // For managing tokens like '{'
    OperationsT Operation; // For refering to operation like '+'
    RelopsT Relop; // For refering to operations like '=='
    TypesT Types; // For returning types during variable declaration
    KeywordsT Keywords; // For returning Keywords like if for further processing
    ExpressionT Expression;
    codeBlock *code;
}


%token <Token> LRPAREN RRPAREN LCPAREN RCPAREN LSPAREN RSPAREN
%token <Token> SEMICOLON DOUBLEQUOTE
%token <Operation> ADD SUB MUL DIV
%token <Relop> RELOP
%token <Types> TYPE
// %token <Keywords> FUNC IF ELSE PRINT

%token <val> NUM    /* Simple long integer number       */
%token <val> FLT    /* Simple double precision number   */
%token <val> VAR    /* Variable and Function            */


%token <keyword> PACKAGE IMPORT FUNC IF ELSE PRINT WHILE

%type  <Expression> EXPRESSION
%type  <code> input Statement SimpleStmt Assignment IncDecStmt PrintStmt While Condition Block IfStmt

%right EQUAL ASSIGN
%right RELOP
%left SUB ADD
%left MUL DIV //'\%'
%left NEG INCREMENT DECREMENT    /* Negation--unary minus */
 //%right '^'    /* Exponentiation        */

 
 
 
/* Grammar follows */

%%

Program: input { Concat(FINAL_CODE, $1); }
;


NewLine: '\n'
;

OptionalBlankLines:
        | NewLine
;

input:                              { $$ = Assign(""); } 
    | Statement input    { Concat($1, $2); $$ = $1;   }
;

Statement:  Block NewLine                { $$ = $1; }
          | While NewLine                { $$ = $1; }
          | IfStmt NewLine               { $$ = $1; }
          | SimpleStmt OptionalBlankLines   { $$ = $1; }
;

Condition: EXPRESSION RELOP EXPRESSION          { $$ = ConCode($1, $3, $2); }
;

While: WHILE LRPAREN Condition RRPAREN OptionalBlankLines Block     { $$ = WhileBlock($3, $6); }
;

IfStmt: IF LRPAREN Condition RRPAREN OptionalBlankLines Block       { $$ = IFBlock($3, $6, NULL); }
    | IF LRPAREN Condition RRPAREN OptionalBlankLines Block ELSE OptionalBlankLines Block       { $$ = IFBlock($3, $6, $9); }
    | IF LRPAREN Condition RRPAREN OptionalBlankLines Block ELSE IfStmt       { $$ = IFBlock($3, $6, $8); }
;

Block: LCPAREN OptionalBlankLines input RCPAREN { Concat($3, BlockClean()); $$=$3; }
;

SimpleStmt: Assignment Terminator       { $$ = $1; }
          | PrintStmt Terminator        { $$ = $1; }
          | IncDecStmt Terminator       { $$ = $1; }
;

Terminator: NewLine
        | SEMICOLON
;

PrintStmt: PRINT EXPRESSION     { $$ = PrintExp($2); }
;

Assignment:   VAR ASSIGN EXPRESSION     { $$ = VarAssign($1, $3, 0);}
            | VAR EQUAL EXPRESSION      { $$ = VarAssign($1, $3, 1);}
;

IncDecStmt: VAR INCREMENT   { $$ = VarOp($1, Add);   }
          | VAR DECREMENT   { $$ = VarOp($1, Sub);   }
;
        
EXPRESSION:   NUM                               { $$ = DecConst($1, Int);               }
            | FLT                               { $$ = DecConst($1, Flt);               }
            | VAR                               { $$ = LoadVar($1, 0);                  }
            | EXPRESSION ADD EXPRESSION         { $$ = ExpOp($1, $3, Add);              }
            | EXPRESSION SUB EXPRESSION         { $$ = ExpOp($1, $3, Sub);              }
            | EXPRESSION MUL EXPRESSION         { $$ = ExpOp($1, $3, Mul);              }
            | EXPRESSION DIV EXPRESSION         { $$ = ExpOp($1, $3, Div);              }
            | LRPAREN EXPRESSION RRPAREN        { $$ = $2;                              }
;




/* End of grammar */
%%




int main ()
{
  // Init globals
  // Empty string to FINAL_CODE
  FINAL_CODE = Assign(".data\n\n.text\n\n");
  // Defining the global StackTop
  StackTop = (StackNode *)calloc(1,sizeof(StackNode));
  StackTop->id = 0;
  StackTop->offset = 0;
  StackTop->constant.Type = Int;
  StackTop->constant.num = 0;
  StackTop->prev = NULL;
  StackTop->next = NULL;
  StackTop->Group = Constant;
  //
  // Init LoopTop
  LoopAdd();
  //
  // Set label counter to 0
  label_count = 0;
  //
  // Set reg_rot
  reg_rot = 0;
  //
  // Set Level
  Level = 0;
  //
  // Register Array
  for (int i = 0; i < NUM_REG; ++i)
  {
    TArray[i].id = 0;
    TArray[i].snode = NULL;
    TArray[i].load_level = Level;
    TArray[i].access_level = Level;
    TArray[i].Group = Codeblock;
  }

  // Output file pointer
  FILE *fp;

  if (yyparse()==0)
  {
    Concat(FINAL_CODE, Assign("j END\n\nPRINT:\nli $v0,1\nsyscall\nli $v0,11\nli $a0, 10\nsyscall\njr $ra\n\nEND:\nnop"));
    fp=fopen("asmb.asm","w");
    fprintf(fp ,"%s\n", FINAL_CODE->code);
    fclose(fp);
    // printf("%s\n", FINAL_CODE->code);
  }
  else
  {
    printf("Unrecoverable compilation error\n");
  }
}

void yyerror (char *s)  /* Called by yyparse on error */
{
  printf ("%s\n", s);
}


