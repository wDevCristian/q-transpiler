#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

#include "lexer.h"
#include "ad.h"
#include "gen.h"

Text tBegin, tMain, tFunctions, tFnHeader;
Text *crtCode;
Text *crtVar;

void Text_write(Text *text, const char *fmt, ...)
{
	va_list va;
	va_start(va, fmt); // "va" is an iterator to the variable list of arguments
	// vsnprintf called with buffer==NULL, or max nb. of chars==0
	// returns the total number of chars, without \0, which will be written
	// if there is a suitable sized buffer
	int n = vsnprintf(NULL, 0, fmt, va);
	// realloc the dynamic buffer to add the new chars
	char *p = (char *)realloc(text->buf, (text->n + n + 1) * sizeof(char));
	if (p == NULL)
	{
		puts("not enough memory");
		exit(EXIT_FAILURE);
	}
	// adds the new chars to the dynamic buffer
	va_start(va, fmt); // resets the iterator in the variable list of arguments
	vsnprintf(p + text->n, n + 1, fmt, va);
	text->buf = p;
	text->n += n;
	va_end(va);
}

void Text_clear(Text *text)
{
	free(text->buf);
	text->buf = NULL;
	text->n = 0;
}

const char *cType(int type)
{
	switch (type)
	{
	case TYPE_INT:
		return "int";
	case TYPE_REAL:
		return "double";
	case TYPE_STR:
		return "str";
	default:
		printf("wrong type: %d\n", type);
		exit(EXIT_FAILURE);
	}
}
