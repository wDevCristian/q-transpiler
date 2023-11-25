
/**
 * @file sintaxer.c
 * @author Gurduza Cristian
 * @date 24.10.2023
 * @ref https://github.com/wDevCristian/q-transpiler "src code:"
 */

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>

#include "lexer.h"
#include "ad.h"
#include "utils.h"
#include "at.h"
#include "gen.h"

/** short version of @code unsigned short int @endcode */
#define USINT unsigned short int;

int iTk = 0;	  // the iterator in tokens
Token *consumed; // the last consumed token

/* Declaration of all functions used in program() */

bool factor(void);
bool exprPrefix(void);
bool exprMul(void);
bool exprAdd(void);
bool exprComp(void);
bool exprAssign(void);
bool exprLogic(void);
bool expr(void);
bool instr(void);
bool funcParam(void);
bool funcParams(void);
bool block(void);
bool defFunc(void);
bool baseType(void);
bool defVar(void);
bool program(void);

/**
 * @brief same as err, but also prints the line of the current token
 * @param[in] *fmt format of the error message @code "%s" @endcode
 * @param[in] ... variable number of error messages
 */
_Noreturn void tkerr(const char *fmt, ...)
{
	fprintf(stderr, "error in line %d: ", tokens[iTk].line);
	va_list va;
	va_start(va, fmt);
	vfprintf(stderr, fmt, va);
	va_end(va);
	fprintf(stderr, "\n");
	exit(EXIT_FAILURE);
}

/**
 * @brief Consume atoms based on their atoms code
 * @see enum atoms
 * @param[in] code code of atom
 */
bool consume(int code)
{
	printf("consume(%s)", ATOMS_CODE_NAME[code]);
	if (tokens[iTk].code == code)
	{
		consumed = &tokens[iTk++];
		printf(" => consumed\n");
		return true;
	}
	printf(" => at line %d: found %s", tokens[iTk].line, ATOMS_CODE_NAME[tokens[iTk].code]);
	if (iTk - 1 >= 0)
	{
		char *details = (tokens[iTk - 1].code == 0) ? ", after %s = %s\n" : ", after %s\n";
		printf(details, ATOMS_CODE_NAME[tokens[iTk - 1].code], tokens[iTk - 1].text);
	}

	return false;
}

/**
 * @brief Starts the syntactic analyser
 * @note Call this function to start Syntactic Analyser
 */
void parse()
{
	iTk = 0;
	program();
}

// ---------------------------------------------------
/* Definition of all Syntactic Rules (SR)/(RS in ro) */
// ---------------------------------------------------

/**
 * @brief program ::= ( defVar | defFunc | block )* FINISH
 */
bool program()
{
	printf("\n-============ program ===============-\n\n");

	addDomain();
	ILOG("Added new domain.\n");
	addPredefinedFns();
	ILOG("Added predefined funtions.\n");
	// crtCode = &tMain;
	// crtVar = &tBegin;
	// Text_write(&tBegin, "#include \"quick.h\"\n\n");
	// Text_write(&tMain, "\nint main(){\n");

	for (;;)
	{
		if (defVar())
		{
			if (consume(FINISH))
			{
				printf("\n-============ end program ===============-\n\n");
				delDomain();
				// Text_write(&tMain, "return 0;\n}\n");
				// FILE *fis = fopen("1.c", "w");
				// if (!fis)
				// {
				// 	printf("cannot write to file 1.c\n");
				// 	exit(EXIT_FAILURE);
				// }
				// fwrite(tBegin.buf, sizeof(char), tBegin.n, fis);
				// fwrite(tFunctions.buf, sizeof(char), tFunctions.n, fis);
				// fwrite(tMain.buf, sizeof(char), tMain.n, fis);
				// fclose(fis);
				return true;
			}
		}
		else if (defFunc())
		{
			if (consume(FINISH))
			{
				printf("\n-============ end program ===============-\n\n");
				delDomain();
				// Text_write(&tMain, "return 0;\n}\n");
				// FILE *fis = fopen("1.c", "w");
				// if (!fis)
				// {
				// 	printf("cannot write to file 1.c\n");
				// 	exit(EXIT_FAILURE);
				// }
				// fwrite(tBegin.buf, sizeof(char), tBegin.n, fis);
				// fwrite(tFunctions.buf, sizeof(char), tFunctions.n, fis);
				// fwrite(tMain.buf, sizeof(char), tMain.n, fis);
				// fclose(fis);
				return true;
			}
		}
		else if (block())
		{
			if (consume(FINISH))
			{
				printf("\n-============ end program ===============-\n\n");
				delDomain();
				// Text_write(&tMain, "return 0;\n}\n");
				// FILE *fis = fopen("1.c", "w");
				// if (!fis)
				// {
				// 	printf("cannot write to file 1.c\n");
				// 	exit(EXIT_FAILURE);
				// }
				// fwrite(tBegin.buf, sizeof(char), tBegin.n, fis);
				// fwrite(tFunctions.buf, sizeof(char), tFunctions.n, fis);
				// fwrite(tMain.buf, sizeof(char), tMain.n, fis);
				// fclose(fis);
				return true;
			}
		}
		else
			break;
	}
	if (consume(FINISH))
	{
		printf("\n-============ end program ===============-\n\n");
		delDomain();
		// Text_write(&tMain, "return 0;\n}\n");
		// FILE *fis = fopen("1.c", "w");
		// if (!fis)
		// {
		// 	printf("cannot write to file 1.c\n");
		// 	exit(EXIT_FAILURE);
		// }
		// fwrite(tBegin.buf, sizeof(char), tBegin.n, fis);
		// fwrite(tFunctions.buf, sizeof(char), tFunctions.n, fis);
		// fwrite(tMain.buf, sizeof(char), tMain.n, fis);
		// fclose(fis);
		return true;
	}
	else if (strcmp(ATOMS_CODE_NAME[tokens[iTk].code], "ID") == 0)
	{
		tkerr("unexpected token '%s', waiting for 'var', 'function' or instruction block", tokens[iTk].text);
	}
	else
	{
		tkerr("unexpected token '%s', waiting for 'var', 'function' or instruction block", ATOMS_CODE_NAME[tokens[iTk].code]);
	}
}

/**
 * @brief defVar ::= VAR ID COLON baseType SEMICOLON
 */
bool defVar()
{
	printf("\n-============ defVar ===============-\n\n");

	int start = iTk;
	if (consume(VAR))
	{
		if (consume(ID))
		{
			const char *name = consumed->text;
			Symbol *s = searchInCurrentDomain(name);
			if (s)
			{
				ELOG("symbol redefinition: %s\n", name);
				tkerr("symbol redefinition: %s", name);
			}
			s = addSymbol(name, KIND_VAR);
			s->local = crtFn != NULL;
			if (consume(COLON))
			{
				if (baseType())
				{
					s->type = ret.type;
					if (consume(SEMICOLON))
					{
						printf("\n-============ end defVar ===============-\n\n");
						return true;
					}
					else
					{
						printf("iTk = %d\n", iTk);
						tkerr("missing token ';', after data type definition\n");
					}
				}
				else
				{
					printf("iTk = %d\n", iTk);
					tkerr("missing data type definition\n");
				}
			}
			else
			{
				printf("iTk = %d\n", iTk);
				tkerr("missing token ':, after '%s'\n", tokens[iTk].text);
			}
		}
		else
		{
			printf("iTk = %d\n", iTk);
			tkerr("missing id at variable definition/declaration\n");
		}
	}
	else
	{
		// printf("iTk = %d\n", iTk);
		// tkerr("missing token 'var'\n");
	}

	iTk = start;
	printf("\n-============ end defVar ===============-\n\n");
	return false;
}

/**
 * @brief defFunc ::= FUNCTION ID LPAR funcParams? RPAR COLON baseType defVar* block END
 */
bool defFunc()
{
	printf("\n-============ defFunc ===============-\n\n");

	int start = iTk;

	if (consume(FUNCTION))
	{
		if (consume(ID))
		{
			const char *name = consumed->text;
			Symbol *s = searchInCurrentDomain(name);
			if (s)
			{
				ELOG("symbol redefinition: %s", name);
				tkerr("symbol redefinition: %s", name);
			}
			crtFn = addSymbol(name, KIND_FN);
			crtFn->args = NULL;
			addDomain();

			if (consume(LPAR))
			{
				if (funcParams())
				{
				}
				if (consume(RPAR))
				{
					if (consume(COLON))
					{
						if (baseType())
						{
							crtFn->type = ret.type;

							while (defVar())
							{
								if (block())
								{
									if (consume(END))
									{
										printf("\n-============ end defFunc ===============-\n\n");
										delDomain();
										crtFn = NULL;
										return true;
									}
									else
									{
										printf("iTk = %d\n", iTk);
										tkerr("missing token 'end'\n");
									}
								}
							}
							if (block())
							{
								if (consume(END))
								{
									printf("\n-============ end defFunc ===============-\n\n");
									delDomain();
									crtFn = NULL;
									return true;
								}
								else
								{
									printf("iTk = %d\n", iTk);
									tkerr("missing token 'end'\n");
								}
							}
							else
							{
								printf("iTk = %d\n", iTk);
								tkerr("missing block of instruction for function definition\n");
							}
						}
					}
					else
					{
						printf("iTk = %d\n", iTk);
						tkerr("missing token ':', after ')\n");
					}
				}
				else
				{
					printf("iTk = %d\n", iTk);
					tkerr("missing token ')'\n");
				}
			}
			else
			{
				printf("iTk = %d\n", iTk);
				tkerr("missing token '(', after '%s'\n", tokens[iTk - 1].text);
			}
		}
		else
		{
			printf("iTk = %d\n", iTk);
			tkerr("missing function name\n");
		}
	}
	else
	{
		// printf("iTk = %d\n", iTk);
		// tkerr("missing token 'function'\n");
	}

	iTk = start;
	printf("\n-============ end defFunc ===============-\n\n");
	return false;
}

/**
 * @brief block ::= instr+
 */
bool block()
{
	printf("\n-============ block ===============-\n\n");

	int start = iTk;

	if (instr())
	{
	}
	else
	{
		printf("iTk = %d\n", iTk);
		// tkerr("instruction has not been found\n");
		iTk = start;
		printf("\n-============ end block ===============-\n\n");
		return false;
	}

	while (instr())
	{
	}

	printf("\n-============ end block ===============-\n\n");
	return true;
}

/**
 * @brief baseType ::= TYPE_INT | TYPE_REAL | TYPE_STR
 */
bool baseType()
{
	printf("\n-============ baseType ===============-\n\n");

	int start = iTk; // Detailed description after the member
	if (consume(TYPE_INT))
	{
		printf("\n-============ end baseType ===============-\n\n");
		ret.type = TYPE_INT;
		return true;
	}
	else if (consume(TYPE_REAL))
	{
		printf("\n-============ end baseType ===============-\n\n");
		ret.type = TYPE_REAL;
		return true;
	}
	else if (consume(TYPE_STR))
	{
		printf("\n-============ end baseType ===============-\n\n");
		ret.type = TYPE_STR;
		return true;
	}
	else
	{
		iTk = start;
		tkerr("undefined or inexistent type of data\n");
		printf("\n-============ end baseType ===============-\n\n");
		return false;
	}
}

/**
 * @brief funcParams ::= funcParam ( COMMA funcParam )*
 */
bool funcParams()
{
	printf("\n-============ funcParams ===============-\n\n");

	int start = iTk;

	if (funcParam())
	{
		while (true)
		{
			if (consume(COMMA))
			{
				if (funcParam())
				{
					start = iTk;
				}
			}
			else
			{
				if (consume(RPAR) != true)
				{
					printf("iTk = %d\n", iTk - 1);
					tkerr("missing token ',', after '%s'\n", ATOMS_CODE_NAME[tokens[iTk - 2].code]);
				}
				else
				{
					iTk--;
					printf("\n-============ end funcParams ===============-\n\n");
					return true;
				}
			}
		}

		printf("\n-============ end funcParams ===============-\n\n");
		return true;
	}

	iTk = start;
	printf("\n-============ end funcParams ===============-\n\n");
	return false;
}

/**
 * @brief funcParam ::= ID COLON baseType
 */
bool funcParam()
{
	printf("\n-============ funcParam ===============-\n\n");

	int start = iTk;

	if (consume(ID))
	{
		const char *name = consumed->text;
		Symbol *s = searchInCurrentDomain(name);
		if (s)
		{
			ELOG("symbol redefinition: %s", name);
			tkerr("symbol redefinition: %s", name);
		}
		s = addSymbol(name, KIND_ARG);
		Symbol *sFnParam = addFnArg(crtFn, name);

		if (consume(COLON))
		{
			if (baseType())
			{
				printf("\n-============ end funcParam ===============-\n\n");
				s->type = ret.type;
				sFnParam->type = ret.type;
				return true;
			}
		}
		else
		{
			printf("iTk = %d\n", iTk);
			tkerr("missing token ':', after '%s'\n", tokens[iTk - 1].text);
		}
	}
	else
	{
		printf("iTk = %d\n", iTk);
		tkerr("missing 'id' at func. param. declaration\n");
	}

	iTk = start;
	printf("\n-============ end funcParam ===============-\n\n");
	return false;
}

/**
 * instr ::= expr? SEMICOLON
 *		| IF LPAR expr RPAR block ( ELSE block )? END
 *		| RETURN expr SEMICOLON
 *		| WHILE LPAR expr RPAR block END
 */
bool instr()
{
	printf("\n-============ instr ===============-\n\n");

	int start = iTk;

	if (consume(WHILE))
	{
		if (consume(LPAR))
		{
			if (expr())
			{
				if (ret.type == TYPE_STR)
				{
					ELOG("WHILE condition must have TYPE_INT or TYPE_REAL\n");
					tkerr("the while condition must have type int or real");
				}

				if (consume(RPAR))
				{
					if (block())
					{
						if (consume(END))
						{
							printf("\n-============ end instr ===============-\n\n");
							return true;
						}
						else
						{
							printf("iTk = %d\n", iTk);
							tkerr("missing token 'end', after block\n");
						}
					}
					else
					{
						printf("iTk = %d\n", iTk);
						tkerr("missing block of expr in while loop\n");
					}
				}
				else
				{
					printf("iTk = %d\n", iTk);
					tkerr("missing token ')', after expr\n");
				}
			}
			else
			{
				printf("iTk = %d\n", iTk);
				tkerr("missing expr in while loop\n");
			}
		}
		else
		{
			printf("iTk = %d\n", iTk);
			tkerr("missing token '(', after '%s'\n", ATOMS_CODE_NAME[tokens[iTk - 1].code]);
		}
	}

	if (consume(IF))
	{
		if (consume(LPAR))
		{
			if (expr())
			{
				if (ret.type == TYPE_STR)
				{
					ELOG("IF cond myst have TYPE_INT or TYPE_REAL\n");
					tkerr("the if condition must have type int or real");
				}
				if (consume(RPAR))
				{
					if (block())
					{
						if (consume(ELSE))
						{
							if (block())
							{
								if (consume(END))
								{
									printf("\n-============ end instr ===============-\n\n");
									return true;
								}
							}
							else
							{
								printf("iTk = %d\n", iTk);
								tkerr("missing block of expr in else branch\n");
							}
						}
						if (consume(END))
						{
							printf("\n-============ end instr ===============-\n\n");
							return true;
						}
					}
					else
					{
						printf("iTk = %d\n", iTk);
						tkerr("missing block of expr in if statement \n");
					}
				}
				else
				{
					printf("iTk = %d\n", iTk);
					tkerr("missing token ')', after expr\n");
				}
			}
			else
			{
				printf("iTk = %d\n", iTk);
				tkerr("missing expr in if statement\n");
			}
		}
		else
		{
			printf("iTk = %d\n", iTk);
			tkerr("missing token '(', after '%s'\n", ATOMS_CODE_NAME[tokens[iTk - 1].code]);
		}
	}

	if (consume(RETURN))
	{
		if (expr())
		{
			if (!crtFn)
				tkerr("return can be used only in a function");
			if (ret.type != crtFn->type)
				tkerr("the return type must be the same as the function return type");

			if (consume(SEMICOLON))
			{
				printf("\n-============ end instr ===============-\n\n");
				return true;
			}
			else
			{
				printf("iTk = %d\n", iTk);
				tkerr("missing token ';' after expr, received '%s'\n", ATOMS_CODE_NAME[tokens[iTk].code]);
			}
		}
		else
		{
			printf("iTk = %d\n", iTk);
			tkerr("missing after 'return' expr\n");
		}
	}

	if (expr())
	{
		if (consume(SEMICOLON))
		{
			printf("\n-============ end instr ===============-\n\n");
			return true;
		}
		else
		{
			if (consume(COLON))
			{
				return false;
			}
			printf("iTk = %d\n", iTk);
			tkerr("missing token ';' after expr, received '%s'\n", ATOMS_CODE_NAME[tokens[iTk].code]);
		}
	}

	if (consume(SEMICOLON))
	{
		printf("\n-============ end instr ===============-\n\n");
		return true;
	}

	iTk = start;
	printf("\n-============ end instr ===============-\n\n");
	return false;
}

/**
 * @brief expr ::= exprLogic
 */
bool expr()
{
	printf("\n-============ expr ===============-\n\n");

	int start = iTk;

	if (exprLogic())
	{
		printf("\n-============ end expr ===============-\n\n");
		return true;
	}

	iTk = start;
	printf("\n-============ end expr ===============-\n\n");
	return false;
}

/**
 * @brief exprLogic ::= exprAssign ( ( AND | OR ) exprAssign )*
 */
bool exprLogic()
{
	printf("\n-============ exprLogic ===============-\n\n");

	int start = iTk;

	if (exprAssign())
	{
		while (true)
		{
			if (consume(AND))
			{
				Ret leftType = ret;
				if (leftType.type == TYPE_STR)
					tkerr("the left operand of && cannot be of type str");
				ILOG("[AT] left operand has a valid data type '%s'\n", ATOMS_CODE_NAME[leftType.type]);
				if (exprAssign())
				{
					if (ret.type == TYPE_STR)
						tkerr("the right operand of && cannot be of type str");
					setRet(TYPE_INT, false);
					ILOG("[AT] right operand has a valid data type '%s'\n", ATOMS_CODE_NAME[leftType.type]);
				}
				else
				{
					printf("iTk = %d\n", iTk);
					tkerr("missing expression after '&&' operator\n");
				}
			}

			if (consume(OR))
			{
				Ret leftType = ret;
				if (leftType.type == TYPE_STR)
					tkerr("the left operand of || cannot be of type str");
				ILOG("[AT] left operand has a valid data type '%s'\n", ATOMS_CODE_NAME[leftType.type]);
				if (exprAssign())
				{
					if (ret.type == TYPE_STR)
						tkerr("the right operand of || cannot be of type str");
					ILOG("[AT] right operand has a valid data type '%s'\n", ATOMS_CODE_NAME[leftType.type]);
					setRet(TYPE_INT, false);
				}
				else
				{
					printf("iTk = %d\n", iTk);
					tkerr("missing expression after '||' operator\n");
				}
			}

			if (!consume(OR) && !consume(AND))
			{
				break;
			}
		}
		printf("\n-============ end ===============-\n\n");
		return true;
	}

	iTk = start;
	printf("\n-============ end exprLogic ===============-\n\n");
	return false;
}

/**
 * @brief exprAssign ::= ( ID ASSIGN )? exprComp
 */
bool exprAssign()
{
	printf("\n-============ exprAssign ===============-\n\n");

	int start = iTk;

	if (consume(ID))
	{
		const char *name = consumed->text;
		ILOG("[AT] added %s id\n", name);
		if (consume(ASSIGN))
		{
			if (exprComp())
			{
				Symbol *s = searchSymbol(name);
				if (!s)
					tkerr("undefined symbol: %s", name);
				if (s->kind == KIND_FN)
					tkerr("a function (%s) cannot be used as a destination for assignment ", name);
				if (s->type != ret.type)
					tkerr("the source and destination for assignment must have the same type");
				ret.lval = false;
				ILOG("[AT] found valid symbol %s\n", name);
				printf("\n-============ end exprAssign ===============-\n\n");
				return true;
			}
		}
		else
		{
			iTk--;
		}
	}

	if (consume(ASSIGN))
	{
		printf("iTk = %d\n", iTk);
		tkerr("missing id in front of '='\n");
	}

	if (exprComp())
	{
		printf("\n-============ end exprAssign ===============-\n\n");
		return true;
	}

	iTk = start;
	printf("\n-============ end exprAssign ===============-\n\n");
	return false;
}

/**
 * @brief exprComp ::= exprAdd ( ( LESS | EQUAL ) exprAdd )?
 */
bool exprComp()
{
	printf("\n-============ exprComp ===============-\n\n");

	int start = iTk;

	if (exprAdd())
	{
		if (consume(LESS))
		{
			Ret leftType = ret;
			if (exprAdd())
			{
				if (leftType.type != ret.type)
					tkerr("different types for the operands of <");
				setRet(TYPE_INT, false); // the result of comparation is int 0 or 1
				printf("\n-============ end exprComp ===============-\n\n");
				return true;
			}
			else
			{
				printf("iTk = %d\n", iTk);
				tkerr("missing expression after '<' operator\n");
			}
		}

		if (consume(EQUAL))
		{
			Ret leftType = ret;
			if (exprAdd())
			{
				if (leftType.type != ret.type)
					tkerr("different types for the operands of ==");
				setRet(TYPE_INT, false); // the result of comparation is int 0 or 1
				printf("\n-============ end exprComp ===============-\n\n");
				return true;
			}
			else
			{
				printf("iTk = %d\n", iTk);
				tkerr("missing expression after '==' operator\n");
			}
		}

		printf("\n-============ end exprComp ===============-\n\n");
		return true;
	}

	iTk = start;
	printf("\n-============ end exprComp ===============-\n\n");
	return false;
}

/**
 * @brief exprAdd ::= exprMul ( ( ADD | SUB ) exprMul )*
 */
bool exprAdd()
{
	printf("\n-============ exprAdd ===============-\n\n");

	int start = iTk;

	if (exprMul())
	{
		while (true)
		{

			if (consume(ADD))
			{
				Ret leftType = ret;
				if (leftType.type == TYPE_STR)
					tkerr("the operands of + or - cannot be of type str");
				if (exprMul())
				{
					if (leftType.type != ret.type)
						tkerr("different types for the operands of +");
					ret.lval = false;
				}
				else
				{
					printf("iTk = %d\n", iTk);
					tkerr("missing right side operand for the '+' operator\n");
				}
			}

			if (consume(SUB))
			{
				Ret leftType = ret;
				if (leftType.type == TYPE_STR)
					tkerr("the operands of + or - cannot be of type str");
				if (exprMul())
				{
					if (leftType.type != ret.type)
						tkerr("different types for the operands of -");
					ret.lval = false;
				}
				else
				{
					printf("iTk = %d\n", iTk);
					tkerr("missing right side operand for the '-' operator\n");
				}
			}

			if (!consume(ADD) && !consume(SUB))
			{
				break;
			}
		}

		printf("\n-============ end exprAdd ===============-\n\n");
		return true;
	}

	iTk = start;
	printf("\n-============ end exprAdd ===============-\n\n");
	return false;
}

/**
 * @brief exprMul ::= exprPrefix ( ( MUL | DIV ) exprPrefix )*
 */
bool exprMul()
{
	printf("\n-============ exprMul ===============-\n\n");

	int start = iTk;

	if (exprPrefix())
	{
		while (true)
		{
			if (consume(MUL))
			{
				Ret leftType = ret;
				if (leftType.type == TYPE_STR)
					tkerr("the operands of * or / cannot be of type str");

				if (exprPrefix())
				{
					if (leftType.type != ret.type)
						tkerr("different types for the operands of * or /");
					ret.lval = false;
				}
				else
				{
					printf("iTk = %d\n", iTk);
					tkerr("missing right side operand for the '*' operator\n");
				}
			}

			if (consume(DIV))
			{
				Ret leftType = ret;
				if (leftType.type == TYPE_STR)
					tkerr("the operands of * or / cannot be of type str");

				if (exprPrefix())
				{
					if (leftType.type != ret.type)
						tkerr("different types for the operands of * or /");
					ret.lval = false;
				}
				else
				{
					printf("iTk = %d\n", iTk);
					tkerr("missing right side operand for the '/' operator\n");
				}
			}

			if (!consume(MUL) && !consume(DIV))
			{
				break;
			}
		}

		printf("\n-============ end exprMul ===============-\n\n");
		return true;
	}

	iTk = start;
	printf("\n-============ end exprMul ===============-\n\n");
	return false;
}

/**
 * @brief exprPrefix ::= (SUB | NOT)? factor
 */
bool exprPrefix()
{
	printf("\n-============ exprPrefix ===============-\n\n");

	int start = iTk;

	if (consume(SUB))
	{
		if (factor())
		{
			if (ret.type == TYPE_STR)
				tkerr("the expression of unary - must be of type int or real");
			ret.lval = false;
			printf("\n-============ end exprPrefix ===============-\n\n");
			return true;
		}
		else
		{
			printf("iTk = %d\n", iTk);
			tkerr("missing right side operand for the 'SUB'  operator\n");
		}
	}

	if (consume(NOT))
	{
		if (factor())
		{
			if (ret.type == TYPE_STR)
				tkerr("the expression of ! must be of type int or real");
			setRet(TYPE_INT, false);
			printf("\n-============ end exprPrefix ===============-\n\n");
			return true;
		}
		else
		{
			printf("iTk = %d\n", iTk);
			tkerr("missing right side operand for the 'NOT'  operator\n");
		}
	}

	if (factor())
	{
		printf("\n-============ end exprPrefix ===============-\n\n");
		return true;
	}

	iTk = start;
	printf("\n-============ end exprPrefix ===============-\n\n");
	return false;
}

/**
 * factor ::= INT
| REAL
| STR
| LPAR expr RPAR
| ID ( LPAR ( expr ( COMMA expr )* )? RPAR )?

ID
ID LPAR RPAR
ID LPAR EXPR RPAR
ID LPAR EXPR (COMMA EXPR)* RPAR
*/

// ! TODO: implement types analyzer for factor function

bool factor()
{
	printf("\n-============ factor ===============-\n\n");

	int start = iTk;

	if (consume(LPAR))
	{
		if (expr())
		{
			if (consume(RPAR))
			{
				printf("\n-============ end factor ===============-\n\n");
				return true;
			}
		}
	}

	if (consume(ID))
	{
		Symbol *s = searchSymbol(consumed->text);
		if (!s)
			tkerr("undefined symbol: %s", consumed->text);

		if (consume(LPAR))
		{
			if (s->kind != KIND_FN)
				tkerr("%s cannot be called, because it is not a function", s->name);
			Symbol *argDef = s->args;

			if (expr())
			{
				if (!argDef)
					tkerr("the function %s is called with too many arguments", s->name);
				if (argDef->type != ret.type)
					tkerr("the argument type at function %s call is different from the one given at its definition", s->name);
				argDef = argDef->next;

				while (consume(COMMA))
				{
					if (expr())
					{
						if (!argDef)
							tkerr("the function %s is called with too many arguments", s->name);
						if (argDef->type != ret.type)
							tkerr("the argument type at function %s call is different from the one given at its definition", s->name);
						argDef = argDef->next;
					}
					else
					{
						printf("iTk = %d\n", iTk);
						tkerr("missing expr after ','\n");
					}
				}

				if (expr())
				{
					printf("iTk = %d\n", iTk - 1);
					tkerr("missing token ','\n");
				}

				if (consume(RPAR))
				{
					if (argDef)
						tkerr("the function %s is called with too few arguments", s->name);
					setRet(s->type, false);
					printf("\n-============ end factor ===============-\n\n");
					return true;
				}
				else
				{
					if (s->kind == KIND_FN)
						tkerr("the function %s can only be called", s->name);
					setRet(s->type, true);

					printf("iTk = %d\n", iTk);
					tkerr("missing token ')', after expr\n");
				}
			}

			if (consume(RPAR))
			{
				if (argDef)
					tkerr("the function %s is called with too few arguments", s->name);
				setRet(s->type, false);
				printf("\n-============ end factor ===============-\n\n");
				return true;
			}
			else
			{
				if (s->kind == KIND_FN)
					tkerr("the function %s can only be called", s->name);
				setRet(s->type, true);
			}
		}

		printf("\n-============ end factor ===============-\n\n");
		return true;
	}

	if (consume(INT))
	{
		setRet(TYPE_INT, false);
		ILOG("[AT] assign int '%d' as a right operand.\n", tokens[iTk].i);
		printf("\n-============ end factor ===============-\n\n");
		return true;
	}

	if (consume(REAL))
	{
		setRet(TYPE_REAL, false);
		ILOG("[AT] assign real '%f' as a right operand.\n", tokens[iTk].r);
		printf("\n-============ end factor ===============-\n\n");
		return true;
	}

	if (consume(STR))
	{
		setRet(TYPE_STR, false);
		ILOG("[AT] assign str '%s' as a right operand.\n", tokens[iTk].text);
		printf("\n-============ end factor ===============-\n\n");
		return true;
	}

	iTk = start;
	printf("\n-============ end factor ===============-\n\n");
	return false;
}
