/*
This contains the code for handling the stack frames
formed due to blocks and function calls.

*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "enums.h"

StackNode * CheckVar(char * name)
{
	StackNode *curr = StackTop;
	while(curr!=NULL){
		if (curr->Group==Variable)
		{
			if(strcmp(name, (curr->variable).Name)==0)
			{
				return curr;
			}
			else
			{
				curr = curr->prev;
			}
		}
		else
		{
			curr = curr->prev;
		}
	}
	return NULL;
}


StackNode * AddNode()
{
	StackNode *s = calloc(1, sizeof(StackNode));
	s->id = StackTop->id +1;
	s->level = Level;
	s->offset = StackTop->offset;
	StackTop->next = s;
	s->prev = StackTop;
	StackTop = s;
	s->next = NULL;
	return s;
}

void RemoveNode(StackNode *s)
{
	if (s==StackTop)
	{
		StackTop = s->prev;
		free(s);
		StackTop->next = NULL;
	}
	else if (s->Group==Codeblock)
	{
		(s->next)->prev = s->prev;
		(s->prev)->next = s->next;
		free(s);
	}
}

// Loop Node Add and Remove
void LoopAdd()
{
	struct LoopNode * t = calloc(1, sizeof(struct LoopNode));
	t->level = Level;
	t->prev = LoopTop;
	for (int i = 0; i < NUM_REG; ++i)
	{
		t->spill[i]=0;
	}
	t->offset = StackTop->offset;
	LoopTop = t;
}

codeBlock * LoopRemove()
{
	struct LoopNode * t = LoopTop->prev;
	codeBlock *c = Assign("");
	for (int i = 0; i < NUM_REG; ++i)
	{
		if (TArray[i].Group == Variable && TArray[i].snode->level < Level && TArray[i].load_level < TArray[i].access_level)
		{
			if (LoopTop->offset +4 > TArray[i].snode->offset)
			Concat(c, Assign("sw $t%i, %lu($sp)\n", i, (LoopTop->offset - TArray[i].snode->offset + 4)));
		}
	}
	for (int i = 0; i < NUM_REG; ++i)
	{
		for (int j = 0; j < NUM_REG; ++j)
		{
			if (TArray[j].Group == Variable && TArray[j].snode->level < Level && TArray[j].load_level < TArray[j].access_level)
			{
				if (LoopTop->spill[i]==(StackTop->offset - TArray[j].snode->offset + 4))
				{
					if (i!=j)
					{
						Concat(c, Assign("move $t%i, $t%i\n", i, j));
						LoopTop->spill[i]=0;
						break;
					}
					LoopTop->spill[i]=0;
					break;
				}
			}
		}
		if (LoopTop->spill[i]>0)
		{
			Concat(c, Assign("lw $t%i, %lu($sp)\n", i, LoopTop->spill[i]));
		}
	}
	free(LoopTop);
	LoopTop = t;
	return c;
}
