#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ad.h"
#include "utils.h"

Ret ret;
Domain *symTable;
Symbol *crtFn;

Domain *addDomain()
{
	ILOG("creates a new domain\n");
	Domain *d = (Domain *)safeAlloc(sizeof(Domain));
	d->parent = symTable;
	d->symbols = NULL;
	symTable = d;
	return d;
}

void delSymbols(Symbol *list);

void delSymbol(Symbol *s)
{
	ILOG("\tdeletes the symbol %s\n", s->name);
	if (s->kind == KIND_FN)
	{
		delSymbols(s->args);
	}
	free(s);
}

void delSymbols(Symbol *list)
{
	for (Symbol *s1 = list, *s2; s1; s1 = s2)
	{
		s2 = s1->next;
		delSymbol(s1);
	}
}

void delDomain()
{
	ILOG("deletes the current domain\n");
	Domain *parent = symTable->parent;
	delSymbols(symTable->symbols);
	free(symTable);
	symTable = parent;
	ILOG("returns to the parent domain\n");
}

Symbol *searchInList(Symbol *list, const char *name)
{
	for (Symbol *s = list; s; s = s->next)
	{
		if (!strcmp(s->name, name))
			return s;
	}
	return NULL;
}

Symbol *searchInCurrentDomain(const char *name)
{
	return searchInList(symTable->symbols, name);
}

Symbol *searchSymbol(const char *name)
{
	for (Domain *d = symTable; d; d = d->parent)
	{
		Symbol *s = searchInList(d->symbols, name);
		if (s)
			return s;
	}
	return NULL;
}

Symbol *createSymbol(const char *name, int kind)
{
	Symbol *s = (Symbol *)safeAlloc(sizeof(Symbol));
	s->name = name;
	s->kind = kind;
	return s;
}

Symbol *addSymbol(const char *name, int kind)
{
	ILOG("\tadds symbol %s\n", name);
	Symbol *s = createSymbol(name, kind);
	s->next = symTable->symbols;
	symTable->symbols = s;
	return s;
}

Symbol *addFnArg(Symbol *fn, const char *argName)
{
	ILOG("\tadds symbol %s as argument\n", argName);
	Symbol *s = createSymbol(argName, KIND_ARG);
	s->next = NULL;
	if (fn->args)
	{
		Symbol *p;
		for (p = fn->args; p->next; p = p->next)
		{
		}
		p->next = s;
	}
	else
	{
		fn->args = s;
	}
	return s;
}
