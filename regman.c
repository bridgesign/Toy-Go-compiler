/*
Manages the register allocation and code construction for memory loading

*/

#include <stdlib.h>
#include <stdio.h>
#include "enums.h"

struct GetReg GetRegister()
{
	struct GetReg reg;
	reg.i = -1;
	reg.c = NULL;
	int i = 0;
	
	// Returns if reg empty or out-of-scope
	// Returns reg if variable level is out-of-scope
	for (int j = 0; j < NUM_REG; ++j)
	{
		i = (j+reg_rot) % NUM_REG;
		reg.i = i;
		if (TArray[i].snode==NULL)
		{
			reg_rot++;
			reg_rot = reg_rot % NUM_REG;
			return reg;
		}
		else if (TArray[i].snode->id > StackTop->id)
		{
			reg_rot++;
			reg_rot = reg_rot % NUM_REG;
			return reg;
		}
		else if (TArray[i].load_level > Level)
		{
			reg_rot++;
			reg_rot = reg_rot % NUM_REG;
			return reg;
		}
	}

	// Removing a variable which is in same scope
	for (int j = 0; j < NUM_REG; ++j)
	{
		i = (j+reg_rot) % NUM_REG;

		if ((TArray[i].load_level == Level) && (TArray[i].Group == Variable))
		{
			reg.i = i;
			if (TArray[i].load_level < TArray[i].access_level)
				reg.c = Assign("sw $t%i, %lu($sp)\n", i, StackTop->offset - TArray[i].snode->offset + 4);
			reg_rot++;
			reg_rot = reg_rot % NUM_REG;
			return reg;
		}
	}

	// Removing a variable from previous scopes
	for (int j = 0; j < NUM_REG; ++j)
	{
		i = (j + reg_rot) % NUM_REG;
		if (TArray[i].Group == Variable)
		{
			reg.i = i;
			if (LoopTop->level < Level)
			{
				LoopTop->spill[i] = LoopTop->offset - TArray[i].snode->offset + 4;
			}
			if (TArray[i].load_level < TArray[i].access_level)
				reg.c = Assign("sw $t%i, %lu($sp)\n", i, StackTop->offset - TArray[i].snode->offset + 4);
			reg_rot++;
			reg_rot = reg_rot % NUM_REG;
			return reg;

		}
	}
}

int CheckVarReg(StackNode *snode)
{
	for (int j = 0; j < NUM_REG; ++j)
	{
		if (TArray[j].snode == snode)
		{
			return j;
		}
	}
	return -1;
}
