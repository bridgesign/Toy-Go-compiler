/*

Definitions of Operations on expressions
Also includes Definitions for Statements

*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "enums.h"

// Returns a constant Expression
ExpressionT DecConst(char * constant, TypesT type)
{
	ExpressionT exp;
	StackNode * snode = AddNode();
	snode->Group = Constant;
	(snode->constant).Type = type;
	if (type == Int)
	{
		sscanf(constant,"%li",&((snode->constant).num));
	}
	if (type == Flt)
	{
		sscanf(constant,"%lf",&((snode->constant).flt));
	}

	exp.reg = -2;
	exp.snode = snode;
	return exp;
}

/*

Returns variable if it is already in a register
Otherwise returns a codeblock expression with a new stack node
which needs to be removed as soon as evaluated

*/
ExpressionT LoadVar(char * var, int lw)
{
	// Declare expression
	ExpressionT exp;

	// Check if node exists
	StackNode *snode = CheckVar(var);

	// Set snode
	exp.snode = snode;
	// Return if no entry
	if (snode == NULL)
	{
		printf("ERROR: Variable %s referenced before declaration\n", var);
		exp.reg = -1;
		return exp;
	}

	// Checking in Registers
	exp.reg = CheckVarReg(snode);
	if (exp.reg>-1)
	{
		if (TArray[exp.reg].load_level <= Level)
		{
			if (TArray[exp.reg].access_level<Level)
			{
				TArray[exp.reg].access_level = Level;
			}
			return exp;
		}
	}

	// Requesting register and creating code
	struct GetReg reg = GetRegister();
	codeBlock *c;
	if (lw==1)
	{
		c = NULL;
	}
	else
	{
		c = Assign("lw $t%i, %lu($sp)\n", reg.i, (StackTop->offset - snode->offset + 4));
	}

	// Setting up the register state
	// New id needs to be created
	TArray[reg.i].id = snode->id;
	// Pointing to new snode
	TArray[reg.i].snode = snode;
	// Loaded at this level
	TArray[reg.i].load_level = Level;
	// Accessed at this level as load <=access
	TArray[reg.i].access_level = Level;
	// This dummy will tell that it is variable
	TArray[reg.i].Group = Variable;
	
	exp.reg = reg.i;

	// Creating codeblock snode
	snode = AddNode();

	snode->Group = Codeblock;
	if (reg.c==NULL)
	{
		snode->codeblock = c;
	}
	else
	{
		Concat(reg.c, c);
		snode->codeblock = reg.c;
	}
	exp.snode = snode;

	return exp;
}

// TODO
/*

If mem is 0 then its a new allocation else reassignment

*/
// Supporting functions
codeBlock * varReassign(char * var, ExpressionT exp)
{
		ExpressionT texp = LoadVar(var, 1);
		// Changing access level
		TArray[texp.reg].access_level = Level + 1;
		// Constant case
		if (exp.reg == -2)
		{
			codeBlock *c = Assign("");
			if (texp.snode->Group==Codeblock)
			{
				if (TArray[texp.reg].load_level<TArray[texp.reg].load_level < TArray[texp.reg].access_level)
				{
					Concat(c, texp.snode->codeblock);
				}
				else
				{
					free(texp.snode->codeblock->code);
					free(texp.snode->codeblock);
				}
				RemoveNode(texp.snode);
			}
			Concat(c, Assign("li $t%i, %li\n", texp.reg, exp.snode->constant.num));
			RemoveNode(exp.snode);
			return c;
		}
		// Codeblock case
		if (exp.snode->Group==Codeblock)
		{
			if (texp.snode->Group==Codeblock)
			{
				if (TArray[texp.reg].load_level<TArray[texp.reg].load_level < TArray[texp.reg].access_level)
				{
					Concat(exp.snode->codeblock, texp.snode->codeblock);
				}
				else
				{
					free(texp.snode->codeblock->code);
					free(texp.snode->codeblock);				
				}
				RemoveNode(texp.snode);
			}
			if (texp.reg!=exp.reg)
			{
				Concat(exp.snode->codeblock, Assign("move $t%i, $t%i\n", texp.reg, exp.reg));
				if (TArray[texp.reg].load_level < Level && TArray[texp.reg].load_level < TArray[texp.reg].access_level)
				{
					LoopTop->spill[texp.reg] = LoopTop->offset - TArray[texp.reg].snode->offset + 4;
				}
			}
			
			codeBlock *c = Assign("");
			Concat(c, exp.snode->codeblock);

			TArray[exp.reg].id = 0;
			TArray[exp.reg].snode = NULL;
			TArray[exp.reg].Group = Codeblock;
			TArray[exp.reg].load_level = Level;
			TArray[exp.reg].access_level = Level;

			return c;
		}

		// Variable Case
		codeBlock *c = Assign("");
		if (TArray[texp.reg].load_level < Level && TArray[texp.reg].load_level < TArray[texp.reg].access_level)
		{
			Concat(c, Assign("sw $t%i, %lu($sp)\n", texp.reg, StackTop->offset - TArray[texp.reg].snode->offset + 4));
			LoopTop->spill[texp.reg] = LoopTop->offset - TArray[texp.reg].snode->offset + 4;
		}
		if (texp.reg == exp.reg)
		{
			return c;
		}
		Concat(c, Assign("move $t%i, $t%i\n", texp.reg, exp.reg));

		return c;
}


codeBlock * varAlloc(char * var, ExpressionT exp)
{
		StackNode * snode = exp.snode;
		// Case when exp is constant
		if (exp.reg == -2)
		{
			struct GetReg reg = GetRegister();
			// Register Allocated
			TArray[reg.i].id = snode->id;
			TArray[reg.i].load_level = Level;
			TArray[reg.i].access_level = Level+1;
			TArray[reg.i].snode = snode;
			TArray[reg.i].Group = Variable;

			// Changing snode to make variable node
			snode->offset = snode->offset + 4;
			codeBlock *c = Assign("addi $sp, $sp, -4\nli $t%i, %li\n", reg.i, (snode->constant).num);
			snode->Group = Variable;
			snode->level = Level;
			(snode->variable).Type = Int;
			(snode->variable).Name = (char *)calloc(1024, sizeof(char));
			strcpy((snode->variable).Name, var);

			// Returning if no overflow
			if (reg.c == NULL)
			{
				return c;
			}

			// Concat overflow code
			Concat(reg.c, c);
			return reg.c;
		}
		// Case when RHS is variable: Two cases exp points to variable node or codeblock node
		// Codeblock case
		if (snode->Group==Codeblock)
		{
			codeBlock *c = Assign("addi $sp, $sp, -4\n");
			Concat(snode->codeblock, c);
			codeBlock *ret = Assign("");
			Concat(ret, snode->codeblock);

			TArray[exp.reg].snode = snode;
			TArray[exp.reg].Group = Variable;
			TArray[exp.reg].load_level = Level;
			TArray[exp.reg].access_level = Level+1;

			snode->offset = snode->offset+4;
			snode->Group = Variable;
			snode->level = Level;
			(snode->variable).Type = Int;
			(snode->variable).Name = (char *)calloc(1024, sizeof(char));
			strcpy((snode->variable).Name, var);

			return ret;
		}
		// Variable case
		codeBlock *c = Assign("sw $t%i, %lu($sp)\n", exp.reg, StackTop->offset - TArray[exp.reg].snode->offset + 4);
		if (TArray[exp.reg].load_level < Level && TArray[exp.reg].load_level < TArray[exp.reg].access_level)
		{
			LoopTop->spill[exp.reg] = LoopTop->offset - TArray[exp.reg].snode->offset + 4;
		}
		snode = AddNode();
		snode->offset = snode->offset + 4;
		snode->Group = Variable;
		snode->level = Level;
		(snode->variable).Type = Int;
		(snode->variable).Name = (char *)calloc(1024, sizeof(char));
		strcpy((snode->variable).Name, var);

		TArray[exp.reg].snode = snode;
		TArray[exp.reg].load_level = Level;
		TArray[exp.reg].access_level = Level+1;

		Concat(c, Assign("addi $sp, $sp, -4\n"));
		return c;
}

codeBlock * VarAssign(char * var, ExpressionT exp, int mem)
{
	// Check if node exists
	StackNode *snode = CheckVar(var);
	if (snode==NULL)
	{
		if (mem==1)
		{
			printf("ERROR: Memory not allocated for Variable %s\n", var);
			return NULL;
		}

		return varAlloc(var, exp);
	}

	if (mem==1)
	{
		return varReassign(var, exp);
	}
	if (snode->level==Level)
	{
		printf("ERROR: Memory already allocated for Variable %s\n", var);
		return NULL;
	}
	return varAlloc(var, exp);
}

ExpressionT ExpOp(ExpressionT exp1, ExpressionT exp2, OperationsT op)
{
	ExpressionT exp;
	codeBlock *c;
	if (exp1.reg ==-2 && exp2.reg==-2)
	{
		if(op==Add)
			exp1.snode->constant.num = (exp1.snode->constant).num + (exp2.snode->constant).num;
		else if(op==Sub)
			(exp1.snode->constant).num = (exp1.snode->constant).num - (exp2.snode->constant).num;
		else if(op==Mul)
			(exp1.snode->constant).num = (exp1.snode->constant).num * (exp2.snode->constant).num;
		else
			(exp1.snode->constant).num = (exp1.snode->constant).num / (exp2.snode->constant).num;
		RemoveNode(exp2.snode);
		return exp1;
	} // Swapping exp1 and exp2 so that if constant is there, it is in exp2
	else if (exp1.reg==-2)
	{
		exp = exp1;
		exp1 = exp2;
		exp2 = exp;
		exp.depth = 1;
	}

	if (exp2.reg==-2)
	{
		if(op==Add)
			c = Assign("addi $t%i, $t%i, %li\n", exp1.reg, exp1.reg, (exp2.snode->constant).num);
		else if(op==Sub)
		{
			c = Assign("addi $t%i, $t%i, %li\n", exp1.reg, exp1.reg, -(exp2.snode->constant).num);
			if (exp.depth==1)
			{
				Concat(c, Assign("sub $t%i, $zero, $t%i\n", exp1.reg, exp1.reg));
			}
		}
		else if(op==Mul)
			c = Assign("mul $t%i, $t%i, %li\n", exp1.reg, exp1.reg, (exp2.snode->constant).num);
		else
			c = Assign("div $t%i, $t%i, %li\n", exp1.reg, exp1.reg, (exp2.snode->constant).num);

		if ((exp1.snode)->Group == Codeblock)
		{
			Concat((exp1.snode)->codeblock, c);
			TArray[exp1.reg].Group = Codeblock;
			TArray[exp1.reg].snode = exp1.snode;
			RemoveNode(exp2.snode);
			return exp1;
		}
		else
		{
			if (LoopTop->level < Level && TArray[exp1.reg].load_level<Level && TArray[exp1.reg].load_level < TArray[exp1.reg].access_level)
			{
				LoopTop->spill[exp1.reg] = LoopTop->offset - TArray[exp1.reg].snode->offset + 4;
			}
			(exp2.snode)->codeblock = Assign("sw $t%i, %lu($sp)\n", exp1.reg, StackTop->offset - exp1.snode->offset + 4);
			Concat((exp2.snode)->codeblock, c);
			exp2.reg = exp1.reg;
			TArray[exp2.reg].Group = Codeblock;
			TArray[exp2.reg].snode = exp2.snode;
			(exp2.snode)->Group = Codeblock;

			return exp2;
		}
	}

	// When both are having register allocated
	if(op==Add)
		c = Assign("add $t%i, $t%i, $t%i\n", exp1.reg, exp1.reg, exp2.reg);
	else if(op==Sub)
		c = Assign("sub $t%i, $t%i, $t%i\n", exp1.reg, exp1.reg, exp2.reg);
	else if(op==Mul)
		c = Assign("mul $t%i, $t%i, $t%i\n", exp1.reg, exp1.reg, exp2.reg);
	else
		c = Assign("div $t%i, $t%i, $t%i\n", exp1.reg, exp1.reg, exp2.reg);

	if ((exp1.snode)->Group == Variable)
	{
		exp.snode = AddNode();
		(exp.snode)->Group = Codeblock;
		if (LoopTop->level < Level && TArray[exp1.reg].load_level<Level && TArray[exp1.reg].load_level < TArray[exp1.reg].access_level)
		{
			LoopTop->spill[exp1.reg] = LoopTop->offset - TArray[exp1.reg].snode->offset + 4;
		}
		(exp.snode)->codeblock = Assign("sw $t%i, %lu($sp)\n", exp1.reg, (StackTop->offset - (exp1.snode)->offset + 4));
		exp.reg = exp1.reg;

		TArray[exp.reg].Group = Codeblock;
		TArray[exp.reg].snode = exp.snode;

		if (exp2.snode->Group==Codeblock)
		{
			Concat(exp.snode->codeblock, exp2.snode->codeblock);
			RemoveNode(exp2.snode);
		}
		Concat((exp.snode)->codeblock, c);
		return exp;
	}
	else
	{
		TArray[exp1.reg].Group = Codeblock;
		TArray[exp1.reg].snode = exp1.snode;
		if (exp2.snode->Group==Codeblock)
		{
			Concat(exp1.snode->codeblock, exp2.snode->codeblock);
			RemoveNode(exp2.snode);
		}
		Concat((exp1.snode)->codeblock, c);
		return exp1;
	}
}


// TODO
codeBlock * VarOp(char * var, OperationsT op)
{
	return Assign("");
}

// TODO
ExpressionT ExpNeg(ExpressionT exp)
{
	return exp;
}

codeBlock * PrintExp(ExpressionT exp)
{
	codeBlock *c;
	// Constant case
	if (exp.reg==-2)
	{
		c = Assign("\nli $a0, %li\njal PRINT\n", exp.snode->constant.num);
		RemoveNode(exp.snode);
		return c;
	}
	// Codeblock case
	if (exp.snode->Group==Codeblock)
	{
		Concat(exp.snode->codeblock, Assign("\nmove $a0, $t%i\njal PRINT\n", exp.reg));
		c = Assign("");
		Concat(c, exp.snode->codeblock);
		RemoveNode(exp.snode);
		return c;
	}
	// Variable case
	return Assign("\nmove $a0, $t%i\njal PRINT\n", exp.reg);
}