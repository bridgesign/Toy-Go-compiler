/*
This file contains the code for string manipulations required for asm
*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include "enums.h"


codeBlock * genCode()
{
	codeBlock *c;
	c = (codeBlock *)calloc(1, sizeof(codeBlock));
	c->code = (char *)calloc(256, sizeof(char));
	c->len = 0;
	return c;
}

// Returns a codeblock with the given string
codeBlock * Assign(const char *format, ...)
{
	codeBlock *c = genCode();

	va_list args;
	va_start(args, format);
	vsprintf(c->code, format, args);
	va_end(args);
	c->len = strlen(c->code);
	return c;
}


// Concats two codeblocks
void Concat(codeBlock *c1, codeBlock *c2)
{
	if (c2 == NULL)
	{
		return;
	}
	unsigned long int len = c1->len + c2->len+1;
	if(len>c1->len){
		c1->code = (char *)realloc(c1->code, len);
	}
	c1->len = len;
	strcat(c1->code, c2->code);
	free(c2->code);
	free(c2);
}


// Function for adding label to codeblock
// Returns the label string for referencing the same
// in some other place
char *Label(codeBlock *c)
{
	codeBlock *temp;
	char *label = (char *)calloc(12, sizeof(char));
	sprintf(label, "L%lu", label_count);
	
	temp = Assign("L%lu:\n", label_count);
	Concat(c, temp);
	label_count++;
	return label;
}