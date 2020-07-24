/*
All structs that are not to be used directly will be prefixed with '_'
All new types to be used will be suffixed with T to avoid repeatation
All enum while definition will be preffixed with enum_

*/

#ifndef __ENUMS_H__
#define __ENUMS_H__
#define NUM_REG 10

//--------------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------------

/* ALL ENUM DEFINITIONS */

typedef enum enum_operations{Add, Sub, Mul, Div}OperationsT;

typedef enum enum_relops{Equal, NEqual, Leq, Geq, Le, Ge}RelopsT;

typedef enum enum_keywords{If, Else, Func}KeywordsT;

typedef enum enum_types{Int, Flt}TypesT;

typedef enum enum_tokens{LRbrac, RRbrack, LCbrac, RCbrac, LSbrac, RSbrac}TokensT;

typedef enum enum_groups{Constant, Variable, Codeblock}GroupsT;


//--------------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------------

/* ALL STRUCT DEFINITIONS */

//--------------------------------------------------------------------------------------------------

// This is the only exception to the suffix rule
typedef struct _codeBlock
{
	char *code;
	unsigned long int len;
} codeBlock;

//--------------------------------------------------------------------------------------------------

struct _constant
{
	TypesT Type;
	union{
		long int num;
		double flt;
	};
};

typedef struct _constant ConstantT;

struct _variable
{
	TypesT Type;
	char *Name;
};

typedef struct _variable VariableT;

// To be replaced with argument list later
// Return to be replaced with return list later
struct _function
{
	char *Name;
	VariableT Argument;
	TypesT Return;
};


//--------------------------------------------------------------------------------------------------

// For maintaining variable stack frame
struct _stackNode{
	unsigned long int id;
	unsigned long int offset;
	unsigned long int level;
	// To Define thee type of Node
	GroupsT Group;

	union{
		ConstantT constant;
		VariableT variable;
		codeBlock *codeblock;
	};
	struct _stackNode *prev;
	struct _stackNode *next;
};

typedef struct _stackNode StackNode;


// Level is used to store the load point of a variable
struct _register
{
	unsigned long int id;
	unsigned long int load_level;
	unsigned long int access_level;
	StackNode *snode;
	GroupsT Group;
};

typedef struct _register RegisterT;


//--------------------------------------------------------------------------------------------------

// Node construct for expression evaluation
struct _expression
{
	// For proper register allocation
	long unsigned int depth;

	// Register allocated
	int reg;

	// Pointer to the stackNode with temporary expression
	StackNode * snode;
};

typedef struct _expression ExpressionT;

// Register code struct
struct GetReg
{
	int i;
	codeBlock *c;
};

// Loop Stack Level Manager

struct LoopNode
{
	unsigned long int level;
	unsigned long int offset;
	struct LoopNode * prev;
	unsigned long int spill[NUM_REG];
};

//--------------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------------

/* FUNCTION DEFINITIONS ONLY BELOW */

//--------------------------------------------------------------------------------------------------

/* BEGIN codeman.c */

codeBlock * genCode();
codeBlock * Assign(const char *format, ...);
void Concat(codeBlock *c1, codeBlock *c2);
char *Label(codeBlock *c);


/* END codeman.c */

//--------------------------------------------------------------------------------------------------

/* BEGIN stackframe.c */

void RemoveNode(StackNode * s);
StackNode * AddNode();
StackNode * CheckVar(char * name);

void LoopAdd();
codeBlock * LoopRemove();

/* END stackframe.c */

//--------------------------------------------------------------------------------------------------

/* BEGIN regman.c */

struct GetReg GetRegister();
int CheckVarReg(StackNode *s);

/* END regman.c */

//--------------------------------------------------------------------------------------------------

/* BEGIN blocks.c */

codeBlock * ConCode(ExpressionT exp1, ExpressionT exp2, RelopsT op);
codeBlock * IFBlock(codeBlock * cond, codeBlock * block_if, codeBlock * block_else);
codeBlock * WhileBlock(codeBlock * cond, codeBlock * block);
codeBlock * BlockClean();

/* END blocks.c */

//--------------------------------------------------------------------------------------------------

/* BEGIN expops.c */

ExpressionT ExpOp(ExpressionT exp1, ExpressionT exp2, OperationsT op);
ExpressionT ExpNeg(ExpressionT exp);
ExpressionT DecConst(char * constant, TypesT type);
ExpressionT LoadVar(char * var, int lw);

codeBlock * VarOp(char * var, OperationsT op);

codeBlock * VarAssign(char * var, ExpressionT exp, int mem);

codeBlock * PrintExp(ExpressionT exp);

/* END expops.c */

//--------------------------------------------------------------------------------------------------


/* BEGIN golang.y */
int  yylex(void);
void yyerror (char  *);

/* END golang.y */

//--------------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------------

/* ALL GLOBAL DECLARATIONS BELOW */

// Global for label counter
unsigned long int label_count;

// Global for maintaing level
unsigned long int Level;

// Temporary Register Array
RegisterT TArray[NUM_REG];

// StackTop pointer
StackNode * StackTop;

// Final Code Block
codeBlock * FINAL_CODE;

// Loop Initial node for nested loop handling
struct LoopNode * LoopTop;

// Reg rotator when kickout required
int reg_rot;

#endif /* __ENUMS_H__ */
