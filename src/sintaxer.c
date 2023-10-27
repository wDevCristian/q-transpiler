
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

/** short version of @code unsigned short int @endcode */
#define USINT unsigned short int;

int iTk = 0;	 // the iterator in tokens
Token *consumed; // the last consumed token

/* Declaration of all functions used in program() */

bool factor();
bool exprPrefix();
bool exprMul();
bool exprAdd();
bool exprComp();
bool exprAssign();
bool exprLogic();
bool expr();
bool instr();
bool funcParam();
bool funcParams();
bool block();
bool defFunc();
bool baseType();
bool defVar();
bool program();

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
	if (iTk - 1 >= 0) {
		char *details = (tokens[iTk - 1].code == 0) ? ", after %s = %s\n" : ", after %s\n";
		printf(details, ATOMS_CODE_NAME[tokens[iTk - 1].code], tokens[iTk - 1].text);
	}

	return false;
}

/**
 * @brief Starts the syntactic analyser
 * @note Call this function to start SA
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

	for (;;)
	{
		if (defVar())
		{
			if (consume(FINISH))
			{
				printf("\n-============ end program ===============-\n\n");
				return true;
			}
		}
		else if (defFunc())
		{
			if (consume(FINISH))
			{
				printf("\n-============ end program ===============-\n\n");
				return true;
			}
		}
		else if (block())
		{
			if (consume(FINISH))
			{
				printf("\n-============ end program ===============-\n\n");
				return true;
			}
		}
		else
			break;
	}
	if (consume(FINISH))
	{
		printf("\n-============ end program ===============-\n\n");
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
			if (consume(COLON))
			{
				if (baseType())
				{
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
							while (defVar())
							{
								if (block())
								{
									if (consume(END))
									{
										printf("\n-============ end defFunc ===============-\n\n");
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
									return true;
								}
								else
								{
									printf("iTk = %d\n", iTk);
									tkerr("missing token 'end'\n");
								}
							} else {
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
		return true;
	}
	else if (consume(TYPE_REAL))
	{
		printf("\n-============ end baseType ===============-\n\n");
		return true;
	}
	else if (consume(TYPE_STR))
	{
		printf("\n-============ end baseType ===============-\n\n");
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
				if (consume(RPAR) != true) {
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
		if (consume(COLON))
		{
			if (baseType())
			{
				printf("\n-============ end funcParam ===============-\n\n");
				return true;
			}
		} else {
			printf("iTk = %d\n", iTk);
			tkerr("missing token ':', after '%s'\n", tokens[iTk - 1].text);
		}
	} else {
		printf("iTk = %d\n", iTk);
		tkerr("missing token 'id' at func param declaration\n");
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
				if (consume(RPAR))
				{
					if (block())
					{
						if (consume(END))
						{
							printf("\n-============ end instr ===============-\n\n");
							return true;
						} else {
							printf("iTk = %d\n", iTk);
							tkerr("missing token 'end', after block\n");
						}
					} else {
						printf("iTk = %d\n", iTk);
						tkerr("missing block of expr in while loop\n");
					}
				} else {
					printf("iTk = %d\n", iTk);
					tkerr("missing token ')', after expr\n");
				}
			} else {
				printf("iTk = %d\n", iTk);
				tkerr("missing expr in while loop\n");
			}
		} else {
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
							} else {
								printf("iTk = %d\n", iTk);
								tkerr("missing block of expr in else branch\n");
							}
						}
						
						if (consume(END))
						{
							printf("\n-============ end instr ===============-\n\n");
							return true;
						}
					} else {
						printf("iTk = %d\n", iTk);
						tkerr("missing block of expr in if statement \n");
					}
				} else {
					printf("iTk = %d\n", iTk);
					tkerr("missing token ')', after expr\n");
				}
			} else {
				printf("iTk = %d\n", iTk);
				tkerr("missing expr in if statement\n");
			}
		} else {
			printf("iTk = %d\n", iTk);
			tkerr("missing token '(', after '%s'\n", ATOMS_CODE_NAME[tokens[iTk - 1].code]);
		}
	}

	if (consume(RETURN))
	{
		if (expr())
		{
			if (consume(SEMICOLON))
			{
				printf("\n-============ end instr ===============-\n\n");
				return true;
			} else {
				printf("iTk = %d\n", iTk);
				tkerr("missing token ';' after expr, received '%s'\n", ATOMS_CODE_NAME[tokens[iTk].code]);
			}
		} else {
			printf("iTk = %d\n", iTk);
			tkerr("missing after 'return' expr\n");
		}
	}

	if (expr())
	{
		if (consume(SEMICOLON)) {
			printf("\n-============ end instr ===============-\n\n");
			return true;
		} else {
			if (consume(COLON)) {
				return false;
			}
			printf("iTk = %d\n", iTk);
			tkerr("missing token ';' after expr, received '%s'\n", ATOMS_CODE_NAME[tokens[iTk].code]);
		}
	}

	if (consume(SEMICOLON)) {
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
		while (consume(AND) || consume(OR))
		{
			if (exprAssign())
			{
			} else {
				printf("iTk = %d\n", iTk);
				tkerr("missing right side operand of 'OR' or 'AND' operator\n");
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
		if (consume(ASSIGN))
		{
			if (exprComp())
			{
				printf("\n-============ end exprAssign ===============-\n\n");
				return true;
			}
		} else {
			iTk--;
		}
	}

	if (consume(ASSIGN)) {
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
			if (exprAdd())
			{
				printf("\n-============ end exprComp ===============-\n\n");
				return true;
			} else {
				printf("iTk = %d\n", iTk);
				tkerr("missing expression after comparison operator\n");
			}
		}

		if (consume(EQUAL)) {
			if (exprAdd())
			{
				printf("\n-============ end exprComp ===============-\n\n");
				return true;
			} else {
				printf("iTk = %d\n", iTk);
				tkerr("missing expression after comparison operator\n");
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
		while (consume(ADD) || consume(SUB))
		{
			if (exprMul())
			{
			} else {
				printf("iTk = %d\n", iTk);
				tkerr("missing right side operand for the 'AND' or 'SUB' operator\n");
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
		while (consume(MUL) || consume(DIV))
		{
			if (exprPrefix())
			{
			} else {
				printf("iTk = %d\n", iTk);
				tkerr("missing right side operand for the 'MUL' or 'DIV' operator\n");
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

	if (consume(SUB) || consume(NOT))
	{
		if (factor())
		{
			printf("\n-============ end exprPrefix ===============-\n\n");
			return true;
		} else {
			printf("iTk = %d\n", iTk);
			tkerr("missing right side operand for the 'SUB' or 'NOT' operator\n");
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
		if (consume(LPAR))
		{
			if (expr())
			{
				while (consume(COMMA))
				{
					if (expr())
					{	
					} else {
						printf("iTk = %d\n", iTk);
						tkerr("missing expr after ','\n");
					}
				}

				if (expr()) {
					printf("iTk = %d\n", iTk - 1);
					tkerr("missing token ','\n");
				}

				if (consume(RPAR)) {
					printf("\n-============ end factor ===============-\n\n");
					return true;	
				} else {
					printf("iTk = %d\n", iTk);
					tkerr("missing token ')', after expr\n");
				}

				// printf("\n-============ end factor ===============-\n\n");
				// return false;
				
			}

			if (consume(RPAR))
			{
				printf("\n-============ end factor ===============-\n\n");
				return true;
			}
		}

		printf("\n-============ end factor ===============-\n\n");
		return true;
	}

	if (consume(INT) || consume(REAL) || consume(STR))
	{
		printf("\n-============ end factor ===============-\n\n");
		return true;
	}

	iTk = start;
	printf("\n-============ end factor ===============-\n\n");
	return false;
}

