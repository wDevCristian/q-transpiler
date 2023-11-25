#pragma once

#include <stddef.h>

// A simple implementation of a dynamic buffer in which chars are written.
// As chars are written, the buffer will grow.
typedef struct
{
	char *buf; // buffer
	size_t n;  // nr de caractere din buf
} Text;

// Same as printf, but the chars are written in the "text" buffer, not on screen.
void Text_write(Text *text, const char *fmt, ...);

// Deletes the chars from a buffer
void Text_clear(Text *text);

extern Text tBegin // for header file and global variabiles
	 ,
	 tMain // the Quick global code, which will be considered as the body of the C main function
	 ,
	 tFunctions // the functions from Quick
	 ,
	 tFnHeader // used temporarily at the function header generation
	 ;

// these pointers will point to different buffers
// depending on the current domain (in a function or global)
extern Text *crtCode; // if in a function, it points to tFunctions, else to tMain
extern Text *crtVar;	 // if in a function, it points to tFunctions, else to tBegin

// returns the C name for a Quick type (ex: TYPE_REAL -> double)
// type = TYPE_*
const char *cType(int type);
