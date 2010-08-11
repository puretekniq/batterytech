/*
 * win32types.h
 *
 *  Created on: Jul 29, 2010
 *      Author: rgreen
 */

#ifndef WIN32TYPES_H_
#define WIN32TYPES_H_
#ifdef _WIN32

// integers
typedef signed char S8;
typedef unsigned char U8;
typedef signed short S16;
typedef unsigned short U16;
typedef signed int S32;
typedef unsigned int U32;
typedef signed long long S64;
typedef unsigned long long U64;

// floats (real numbers)
typedef float F32;
typedef double F64;

// byte and boolean
typedef unsigned char BYTE8;
typedef U32 BOOL32;

#ifndef TRUE
#define TRUE 1
#endif

#ifndef FALSE
#define FALSE 0
#endif

#endif /* _WIN32 */
#endif /* TYPES_H_ */