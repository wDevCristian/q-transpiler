#include <stddef.h>

#include "lexer.h"
#include "ad.h"

// adds in ST a function with an argument
// the argument has the type argType and the function returns the type retType
Symbol *addFn1Arg(const char *fnName, int argType, int retType)
{
    Symbol *fn = addSymbol(fnName, KIND_FN);
    fn->type = retType;
    fn->args = NULL;
    Symbol *arg = addFnArg(fn, "arg");
    arg->type = argType;
    return fn;
}

void addPredefinedFns()
{
    addFn1Arg("puti", TYPE_INT, TYPE_INT);
    addFn1Arg("putr", TYPE_REAL, TYPE_REAL);
    addFn1Arg("puts", TYPE_STR, TYPE_STR);
}

void setRet(int type, bool lval)
{
    ret.type = type;
    ret.lval = lval;
}
