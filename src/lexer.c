#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <stdlib.h>

#include "lexer.h"
#include "utils.h"

Token tokens[MAX_TOKENS];
int nTokens;

int line = 1; // the current line in the input file

// adds a token to the end of the tokens list and returns it
// sets its code and line
Token *addTk(int code)
{
	if (nTokens == MAX_TOKENS)
		err("too many tokens");
	Token *tk = &tokens[nTokens];
	tk->code = code;
	tk->line = line;
	nTokens++;
	return tk;
}

// copy in the dst buffer the string between [begin,end)
char *copyn(char *dst, const char *begin, const char *end)
{
	char *p = dst;
	if (end - begin > MAX_STR)
		err("string too long");
	while (begin != end)
		*p++ = *begin++;
	*p = '\0';
	return dst;
}

void tokenize(const char *pch)
{
	const char *start;
	Token *tk;
	char buf[MAX_STR + 1];
	for (;;)
	{
		switch (*pch)
		{
		case ' ':
		case '\t':
			pch++;
			break;
		case '\r': // handles different kinds of newlines (Windows: \r\n, Linux: \n, MacOS, OS X: \r or \n)
			if (pch[1] == '\n')
				pch++;
			// fallthrough to \n
		case '\n':
			line++;
			pch++;
			break;
		case '\0':
			addTk(FINISH);
			return;
		case '#':
			while(*pch != '\n' && *pch != '\r' && *pch != '\0'){
				pch++;
			};
			break;
		case ',':
			addTk(COMMA);
			pch++;
			break;
		case ':':
			addTk(COLON);
			pch++;
			break;
		case ';':
			addTk(SEMICOLON);
			pch++;
			break;
		case '(':
			addTk(LPAR);
			pch++;
			break;
		case ')':
			addTk(RPAR);
			pch++;
			break;
		case '+':
			addTk(ADD);
			pch++;
			break;
		case '-':
			addTk(SUB);
			pch++;
			break;
		case '*':
			addTk(MUL);
			pch++;
			break;
		case '/':
			addTk(DIV);
			pch++;
			break;
		case '<':
			if(*(pch + 1) == '=') {
				addTk(LESSEQ);
				pch += 2;
			} else {
				addTk(LESS);
				pch++;
			}
			break;			
		case '>':
			if(*(pch + 1) == '=') {
				addTk(GREATERQ);
				pch += 2;
			} else {
				addTk(GREATER);
				pch++;
			}
			break;
		case '|':
			if(*(pch + 1) == '|') {
				addTk(OR);
				pch += 2;
			} else {
				err("unrecognized signed after '|' character");
			}
			break;
		case '&':
			if(*(pch + 1) == '&') {
				addTk(AND);
				pch += 2;
			} else {
				err("unrecognized signed after '&' character");
			}
			break;
		case '!':
			if (*(pch + 1) == '=')
			{
				addTk(NOTEQ);
				pch += 2;
			} else {
				addTk(NOT);
				pch++;
			}
			break;
		case '=':
			if (*(pch + 1) == '=')
			{
				addTk(EQUAL);
				pch += 2;
			}
			else
			{
				addTk(ASSIGN);
				pch++;
			}
			break;
		default:
			if (isalpha(*pch) || *pch == '_')
			{
				for (start = pch++; isalnum(*pch) || *pch == '_'; pch++);
				char *text = copyn(buf, start, pch);
				if (strcmp(text, "int") == 0)
					addTk(TYPE_INT);
				else if(strcmp(text, "real") == 0) {
					addTk(TYPE_REAL);
				}
				else if(strcmp(text, "str") == 0) {
					addTk(TYPE_STR);
				}
				else if(strcmp(text, "var") == 0) {
					addTk(VAR);
				}
				else if(strcmp(text, "function") == 0) {
					addTk(FUNCTION);
				}
				else if(strcmp(text, "if") == 0) {
					addTk(IF);
				}
				else if(strcmp(text, "else") == 0) {
					addTk(ELSE);
				}
				else if(strcmp(text, "while") == 0) {
					addTk(WHILE);
				}
				else if(strcmp(text, "end") == 0) {
					addTk(END);
				}
				else if(strcmp(text, "return") == 0) {
					addTk(RETURN);
				}
				else {
					tk = addTk(ID);
					strcpy(tk->text, text);
				}
			} else if(isdigit(*pch)) {
				start = pch;
				do
				{
					pch++;
				} while (isdigit(*pch) != 0);
				
				if(*pch == '.') {
					do
					{
						pch++;
					} while (isdigit(*pch));

					char *text = copyn(buf, start, pch);
					tk = addTk(REAL);
					tk->r = atof(text);
				} else {
					char *text = copyn(buf, start, pch);
					tk = addTk(INT);
					tk->i = atoi(text);
				}
			} else if(*pch == '"') {
				start = ++pch;
				do{
					pch++;
					if(*pch == '\0') {
						err("string not ended");
						return;
					}
				} while(*pch != '"');

				char *text = copyn(buf, start, pch);
				tk = addTk(STR);
				strcpy(tk->text, text);
				pch++;
			}
			else
				err("invalid char: %c (%d) at line %d\n", *pch, *pch, line);
		}
	}
}

void showTokens()
{
	for (int i = 0; i < nTokens; i++)
	{
		Token *tk = &tokens[i];
		printf("%d ", tk->line);

		switch (tk->code){
			case ID: 
				printf("%s:%s\n", "ID", tk->text);
				break;
			case VAR: 
				printf("%s\n", "VAR");
				break;
			case FUNCTION: 
				printf("%s\n", "FUNCTION");
				break;
			case IF: 
				printf("%s\n", "IF");
				break;
			case ELSE: 
				printf("%s\n", "ELSE");
				break;
			case WHILE: 
				printf("%s\n", "WHILE");
				break;
			case END: 
				printf("%s\n", "END");
				break;
			case RETURN: 
				printf("%s\n", "RETURN");
				break;
			case TYPE_INT: 
				printf("%s\n", "TYPE_INT");
				break;
			case TYPE_REAL: 
				printf("%s\n", "TYPE_REAL");
				break;
			case TYPE_STR: 
				printf("%s\n", "TYPE_STR");
				break;
			case INT: 
				printf("%s:%d\n", "INT", tk->i);
				break;
			case REAL: 
				printf("%s:%.5f\n", "REAL", tk->r);
				break;
			case STR: 
				printf("%s:%s\n", "STR", tk->text);
				break;
			case COMMA: 
				printf("%s\n", "COMMA");
				break;
			case COLON: 
				printf("%s\n", "COLON");
				break;
			case SEMICOLON:
				printf("%s\n", "SEMICOLON");
				break;
			case LPAR: 
				printf("%s\n", "LPAR");
				break;
			case RPAR: 
				printf("%s\n", "RPAR");
				break;
			case FINISH: 
				printf("%s\n", "FINISH");
				break;
			case ADD: 
				printf("%s\n", "ADD");
				break;
			case SUB: 
				printf("%s\n", "SUB");
				break;
			case MUL: 
				printf("%s\n", "MUL");
				break;
			case DIV: 
				printf("%s\n", "DIV");
				break;
			case AND: 
				printf("%s\n", "AND");
				break;
			case OR: 
				printf("%s\n", "OR");
				break;
			case NOT: 
				printf("%s\n", "NOT");
				break;
			case ASSIGN: 
				printf("%s\n", "ASSIGN");
				break;
			case EQUAL: 
				printf("%s\n", "EQUAL");
				break;
			case NOTEQ: 
				printf("%s\n", "NOTEQ");
				break;
			case LESS: 
				printf("%s\n", "LESS");
				break;
			case LESSEQ: 
				printf("%s\n", "LESSEQ");
				break;
			case GREATER: 
				printf("%s\n", "GREATER");
				break;
			case GREATERQ: 
				printf("%s\n", "GREATERQ");
				break;
			default:
				break;
		}
	}
}
