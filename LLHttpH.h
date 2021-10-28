#ifndef _LLHTTPH_
#define _LLHTTPH_
#ifdef __cplusplus
extern "C" 
{
#endif

#if !defined(_SG3) && !defined(_SG4) && !defined(_SGC)
#define __asm__(x) //x
// In this case we are building outside BR
#include "bur.h"
// #include "AsTCP.h"
// #include "RingBufLib.h"
// #include "StringExt.h"
// #include "standard.h"
#endif

#include <bur/plctypes.h>
#include "LLHttp.h"
#ifdef _NOT_BR
#undef __asm__
#endif
#ifdef __cplusplus
	};
#endif

#endif