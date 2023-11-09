/*
 * File: HttpUtility.h
 * Copyright (c) 2023 Loupe
 * https://loupe.team
 * 
 * This file is part of LLHttp, licensed under the MIT License.
 */

// #include <bur/plctypes.h>
#ifdef __cplusplus
	extern "C"
	{
#endif
#include "LLHttpH.h"
#include "picohttpparser.h"
#ifdef __cplusplus
	};
#endif

typedef void (* LLHttpCallback)( UDINT context, LLHttpServiceLink_typ * api,  LLHttpHeader_typ * header, unsigned char * data);

void getMethodString(signed long method, unsigned long dest, unsigned long destSize);
unsigned int parseMethodString(unsigned long method, unsigned long methodlen);
const char* HttpStatusPhrase(signed short code);
void copyHeaderLine(LLHttpHeaderField_typ* dest, struct phr_header* src);
signed long LLHttpHandlerIndex(unsigned long _ident, unsigned long pHandler);
plcbit LLHttpRemoveHandler(unsigned long _ident, unsigned long pHandler);
