/*

Code generation of logic block: IF, FOR, WHILE
Code generation for KEYWORD based instruction: RETURN, PRINT

*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "enums.h"

codeBlock * ConCode(ExpressionT exp1, ExpressionT exp2, RelopsT op)
{
	codeBlock *c = Assign("");

	if (exp1.reg!=-2)
	{
		if (exp1.snode->Group!=Codeblock)
		{
			struct GetReg reg = GetRegister();
			if (reg.i == exp1.reg)
			{
				reg = GetRegister();
			}
			exp1.snode = AddNode();
			exp1.snode->Group = Codeblock;
			exp1.snode->codeblock = Assign("move $t%i, $t%i\n", reg.i, exp1.reg);
			exp1.reg = reg.i;
		}
	}
	else if (exp2.reg!=-2)
	{
		if (exp2.snode->Group!=Codeblock)
		{
			struct GetReg reg = GetRegister();
			if (reg.i == exp2.reg)
			{
				reg = GetRegister();
			}
			exp2.snode = AddNode();
			exp2.snode->Group = Codeblock;
			exp2.snode->codeblock = Assign("move $t%i, $t%i\n", reg.i, exp2.reg);
			exp2.reg = reg.i;
		}
	}

	ExpressionT exp = ExpOp(exp1, exp2, Sub);
	if (exp.reg == -2)
	{
		return NULL;
	}

	Concat(c, exp.snode->codeblock);
	RemoveNode(exp.snode);

	if (op == Equal)
	{
		Concat(c, Assign("bne $t%i, $zero, L", exp.reg));
		return c;
	}
	else if (op == NEqual)
	{
		Concat(c, Assign("beq $t%i, $zero, L", exp.reg));
		return c;
	}
	else if (op==Leq)
	{
		Concat(c, Assign("bgt $t%i, 0, L", exp.reg));
		return c;
	}
	else if (op==Geq)
	{
		Concat(c, Assign("ble $t%i, 0, L", exp.reg));
		return c;
	}
	else if (op==Le)
	{
		Concat(c, Assign("bgez $t%i, L", exp.reg));
		return c;
	}
	else if (op==Ge)
	{
		Concat(c, Assign("blez $t%i, L", exp.reg));
		return c;
	}
}

// TODO
codeBlock * IFBlock(codeBlock * cond, codeBlock * block_if, codeBlock * block_else)
{
	if (cond == NULL)
	{
		printf("WARNING: Constant condition found. Not implemented. Code will not be added\n");
		free(block_if->code);
		free(block_if);
		free(block_else->code);
		free(block_else);
		for (int i = 0; i < NUM_REG; ++i)
		{
			if (TArray[i].access_level>Level)
			{
				TArray[i].id = 0;
				TArray[i].snode = NULL;
			}
		}
		return Assign("");		
	}
	
	codeBlock * c = Assign("");
// 	if ( strcmp(block_else->code, "") == 0 ) {
	if (block_else == NULL) {
	Concat(c, cond);
	Concat(c, Assign("%lu\n", label_count));
	Concat(c, block_if);
	Concat(c, Assign("L%lu:\n", label_count));
	label_count++;
	} else {
	Concat(c, cond);
	Concat(c, Assign("%lu\n", label_count));
	Concat(c, block_if);
	Concat(c, Assign("b L%lu\n", label_count+1));
	Concat(c, Assign("L%lu:\n", label_count));
	Concat(c, block_else);
	Concat(c, Assign("L%lu:\n", label_count+1));
	label_count+=2;
	Level--;
	}
	Level--;
	return c;
}

codeBlock * WhileBlock(codeBlock * cond, codeBlock * block)
{
	if (cond == NULL)
	{
		printf("WARNING: Constant condition found for loop. Code will not be added\n");
		free(block->code);
		free(block);
		LoopRemove();
		for (int i = 0; i < NUM_REG; ++i)
		{
			if (TArray[i].access_level>Level)
			{
				TArray[i].id = 0;
				TArray[i].snode = NULL;
			}
		}
		return Assign("");
	}

	codeBlock *c = Assign("L%lu:\n", label_count);
	Concat(c, cond);
	Concat(c, Assign("%lu\n", label_count+1));
	Concat(c, block);
	Concat(c, LoopRemove());
	Concat(c, Assign("b L%lu\nL%lu:\n", label_count, label_count+1));
	label_count = label_count + 2;
	Level--;
	return c;
}

codeBlock * BlockClean()
{
	unsigned long int offset = StackTop->offset;
	StackNode * snode = StackTop;

	codeBlock *c = Assign("");

	while(snode->level>Level)
	{
		snode = snode->prev;
		RemoveNode(snode->next);
	}
	offset = offset - snode->offset;
	if (offset)
	{
		Concat(c ,Assign("addi $sp, $sp, %lu\n", offset));
	}
	return c;
}
