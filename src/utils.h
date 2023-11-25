// the C standard must be at least C11
// #pragma once
#ifndef UTILS_H
#define UTILS_H

// ********************* logging message *******************
#define __LOG_FILE

#ifdef __LOG_FILE
#define INFO "INFO"
#define ERROR "ERROR"
#else
#define INFO "\033[1;49;97mINFO\033[0m"
#define ERROR "\033[1;49;91mERROR\033[0m"
#endif

#define ILOG(fmt, ...) fprintf(stdout, "[" INFO "][%s] %s:%d - " fmt, getCurrentDateTime(), __FILE__, __LINE__, ##__VA_ARGS__)
#define ELOG(fmt, ...) fprintf(stdout, "[" ERROR "][%s] %s:%d - " fmt, getCurrentDateTime(), __FILE__, __LINE__, ##__VA_ARGS__)

// ********************* logging message *******************

#include <stddef.h>

// prints to stderr a message prefixed with "error: " and exit the program
// the arguments are the same as for printf
_Noreturn void err(const char *fmt, ...);

// allocs memory using malloc
// if succeeds, it returns the allocated memory, else it prints an error message and exit the program
void *safeAlloc(size_t nBytes);

// loads a text file in a dynamically allocated memory and returns it
// on error, prints a message and exit the program
char *loadFile(const char *fileName);

// get the current date and time as a string
char *getCurrentDateTime();

#endif