
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
	printf(" => found %s\n", ATOMS_CODE_NAME[tokens[iTk].code]);
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
	for (;;)
	{
		if (defVar())
		{
			if (consume(FINISH))
			{
				return true;
			}
		}
		// else if (defFunc())
		// {
		// }
		// else if (block())
		// {
		// }
		else
			break;
	}
	if (consume(FINISH))
	{
		return true;
	}
	else
		tkerr("syntax error");
	return false;
}

/**
 * @brief defVar ::= VAR ID COLON baseType SEMICOLON
 */
bool defVar()
{
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
						return true;
					}
					else
					{
						printf("iTk = %d\n", iTk);
						tkerr("missing token ';'\n");
					}
				}
				else
				{
					printf("iTk = %d\n", iTk);
					tkerr("missing base type\n");
				}
			}
			else
			{
				printf("iTk = %d\n", iTk);
				tkerr("missing token ':'\n");
			}
		}
		else
		{
			printf("iTk = %d\n", iTk);
			tkerr("missing id\n");
		}
	}
	else
	{
		printf("iTk = %d\n", iTk);
		tkerr("missing token 'var'\n");
		// TODO: find a way to write about the first token error on defVar, defFunc and block 
	}

	iTk = start;
	return false;
}

/**
 * @brief defFunc ::= FUNCTION ID LPAR funcParams? RPAR COLON baseType defVar* block END
*/
bool defFunc() {
	int start = iTk; 

	if (consume(FUNCTION)) {
		if (consume(ID)) {
			if (consume(LPAR)) {
				if (funcParams()) {

				}
				if (consume(RPAR)) {
					if (consume(COLON)) {
						if (baseType()) {
							while (true) {
								if (defVar()) {

								}
							}
							if (block()) {
								if (consume(END)) {
									return true;
								}
							}
						}
 					}
				}
			}
		}
	}

	iTk = start; 
	return false; 
}

/**
 * @brief block ::= instr+
*/
bool block() {
	int start = iTk; 

	do
	{
		if (instr()) {

		}
	} while (true);
	
	
	
	iTk = start; 
	return false; 
}

/**
 * @brief baseType ::= TYPE_INT | TYPE_REAL | TYPE_STR
 */
bool baseType()
{
	int start = iTk; // Detailed description after the member
	if (consume(TYPE_INT))
	{
		return true;
	}
	else if (consume(TYPE_REAL))
	{
		return true;
	}
	else if (consume(TYPE_STR))
	{
		return true;
	}
	else
	{
		iTk = start;
		tkerr("inexistent type of data\n");
		return false;
	}
}


/**
 * @brief funcParams ::= funcParam ( COMMA funcParam )*
*/
bool funcParams() {
	int start = iTk; 

	if (funcParam()) {
		while (consume(COMMA)) {
			if (funcParam()) {
				return true;
			}
		}
	}

	iTk = start; 
	return false;
}

/**
 * @brief funcParam ::= ID COLON baseType
*/
bool funcParam() {
	int start = iTk; 

	if (consume(ID)) {
		if (consume(COLON)) {
			if (baseType()) {
				return true; 
			}
		}
	}

	iTk = start; 
	return false;
}

/**
 * instr ::= expr? SEMICOLON
 *		| IF LPAR expr RPAR block ( ELSE block )? END
 *		| RETURN expr SEMICOLON
 *		| WHILE LPAR expr RPAR block END
*/
bool instr() {
	int start = iTk; 

	if (expr()) {
	
	}
	if (consume(SEMICOLON)) {
		if (consume(IF)) {
			if (consume(LPAR)) {
				if (expr()) {
					if (consume(RPAR)) {
						if (block()) {
							
						}
						if (consume(ELSE)) {
							if (block()) {
								if (consume(END)) {
									return true; 
								}
							}
						}
						if (consume(END)) {
							return true;
						}
					}
				}
			}
		}
	}

	if (consume(RETURN)) {
		if (expr()) {
			if (consume(SEMICOLON)) {
				return true;
			}
		}
	}

	if (consume(WHILE)) {
		if (consume(LPAR)) {
			if (expr()) {
				if (consume(RPAR)) {
					if (block()) {
						if (consume(END)) {
							return true;
						}
					}
				}
			}
		}
	}

	iTk = start; 
	return false;
}

/**
 * @brief expr ::= exprLogic
*/
bool expr() {
	int start = iTk; 

	if (exprLogic()) {
		return true;
	}

	iTk = start; 
	return false;
}

/**
 * @brief exprLogic ::= exprAssign ( ( AND | OR ) exprAssign )*
*/
bool exprLogic() {
	int start = iTk; 

	if (exprAssign()) {
		while (consume(AND) || consume(OR)) {
			if (exprAssign()){
				// return true;
			}
		}
	}

	iTk = start; 
	return false;
}


/**
 * @brief exprAssign ::= ( ID ASSIGN )? exprComp
*/
bool exprAssign() {
	int start = iTk; 

	if (consume(ID)) {
		if (consume(ASSIGN)) {
			if (exprComp()) {
				return true; 
			}
		}
	}

	if (exprComp()) {
		return true;
	}
	
	iTk = start; 
	return false;
}

/**
 * @brief exprComp ::= exprAdd ( ( LESS | EQUAL ) exprAdd )?
*/
bool exprComp() {
	int start = iTk; 

	if (exprAdd()) {
		if (consume(LESS) || consume(EQUAL)) {
			if (exprAdd()) {
				return true;
			}
		}

		return true;
	}

	iTk = start; 
	return false;
}

/**
 * @brief exprAdd ::= exprMul ( ( ADD | SUB ) exprMul )*
*/
bool exprAdd() {
	int start = iTk; 

	if (exprMul()) {
		while (consume(ADD) || consume(SUB)) {
			if (exprMul()) {
				// return true;
			}
		}

		return true;
	}

	iTk = start; 
	return false;
}

/**
 * @brief exprMul ::= exprPrefix ( ( MUL | DIV ) exprPrefix )*
*/
bool exprMul() {
	int start = iTk; 

	if (exprPrefix()) {
		while (consume(MUL) || consume(DIV)){
			if (exprPrefix) {
				// return true; 
			}
		}

		return true;
	}

	iTk = start; 
	return false;
}


/**
 * @brief exprPrefix ::= (SUB | NOT)? factor
*/
bool exprPrefix() {
	int start = iTk; 

	if (consume(SUB) || consume(NOT)) {
		if (factor()) {
			return true;
		}
	}

	if (factor()) {
		return true;
	}

	iTk = start; 
	return false;
}


/**
 * factor ::= INT
| REAL
| STR
| LPAR expr RPAR
| ID ( LPAR ( expr ( COMMA expr )* )? RPAR )?
*/
bool factor() {
	int start = iTk; 

	if (consume(INT) || consume(REAL) || consume(STR)) {
		return true; 
	}

	if (consume(LPAR)) {
		if (expr()) {
			if (consume(RPAR)) {
				return true; 
			}
		}
	}

	if (consume(ID)) {
		if (consume(LPAR)) {
			if(expr()) {
				while (consume(COMMA)) {
					if (expr()) {
						// return true;
					}
				}
			}
			if (consume(RPAR)) {
				return true;
			}
		}

		return true;
	}
	
	iTk = start; 
	return false;
}


// This is a cheatsheet for doxygen

/**
 * @brief Example showing how to document a function with Doxygen.
 *
 * Description of what the function does. This part may refer to the parameters
 * of the function, like @p param1 or @p param2. A word of code can also be
 * inserted like @c this which is equivalent to <tt>this</tt> and can be useful
 * to say that the function returns a @c void or an @c int. If you want to have
 * more than one word in typewriter font, then just use @<tt@>.
 * We can also include text verbatim,
 * @verbatim like this@endverbatim
 * Sometimes it is also convenient to include an example of usage:
 * @code
 * BoxStruct *out = Box_The_Function_Name(param1, param2);
 * printf("something...\n");
 * @endcode
 * Or,
 * @code{.py}
 * pyval = python_func(arg1, arg2)
 * print pyval
 * @endcode
 * when the language is not the one used in the current source file (but
 * <b>be careful</b> as this may be supported only by recent versions
 * of Doxygen). By the way, <b>this is how you write bold text</b> or,
 * if it is just one word, then you can just do @b this.
 * @param param1 Description of the first parameter of the function.
 * @param param2 The second one, which follows @p param1.
 * @return Describe what the function returns.
 * @see Box_The_Second_Function
 * @see Box_The_Last_One
 * @see http://website/
 * @note Something to note.
 * @warning Warning.
 */
void func();