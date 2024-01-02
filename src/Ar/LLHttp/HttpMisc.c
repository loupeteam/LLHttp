/*
 * File: HttpMisc.c
 * Copyright (c) 2023 Loupe
 * https://loupe.team
 * 
 * This file is part of LLHttp, licensed under the MIT License.
 */

#ifdef __cplusplus
	extern "C"
	{
#endif

#include "LLHttpH.h"

#ifdef __cplusplus
	};
#endif

#include "HttpUtility.h"
#include <string.h>

plcbit LLHttpStatus_isInformational(signed short code) { return (code >= 100 && code < 200); } /*!< \returns \c true if the given \p code is an informational code. */
plcbit LLHttpStatus_isSuccessful(signed short code)    { return (code >= 200 && code < 300); } /*!< \returns \c true if the given \p code is a successful code. */
plcbit LLHttpStatus_isRedirection(signed short code)   { return (code >= 300 && code < 400); } /*!< \returns \c true if the given \p code is a redirectional code. */
plcbit LLHttpStatus_isClientError(signed short code)   { return (code >= 400 && code < 500); } /*!< \returns \c true if the given \p code is a client error code. */
plcbit LLHttpStatus_isServerError(signed short code)   { return (code >= 500 && code < 600); } /*!< \returns \c true if the given \p code is a server error code. */
plcbit LLHttpStatus_isError(signed short code)         { return (code >= 400); }               /*!< \returns \c true if the given \p code is any type of error code. */

plcbit LLHttpStatus_getDescription(signed short code, unsigned long dest) {
	strcpy((char*)dest, HttpStatusPhrase(code));
	return 0;
}

plcbit LLHttpMethodMatch(LLHttpMethod_enum a, LLHttpMethod_enum b) {
	return (a == b || a == LLHTTP_METHOD_ANY || b == LLHTTP_METHOD_ANY);
}

plcbit LLHttpUriMatch(unsigned long _a, unsigned long _b) {
	if(!_a || !_b) return 0;
	//if(strcmp(_a, "*") == 0) return 1;
	//if(strcmp(_a, _b) == 0) return 1;
	
	char* a = (char*)_a;
	char* b = (char*)_b;
	
	// /en-US/docs/Learn/
	while(1) {
		
		if(*a == '/' && *b == '/') {
			a++;
			b++;
		}
		
		if(*b == '?') { // We do not support matching parameters
			return 1; // We have matched everything but the parameters
		}
		
		if(*a == '*') { // Match next element
			if(*(++a) == '*') {
				// We have matched so far
				// And we are told to match anything else
				// So we have a match
				return 1;
			}
			// else
			// Else we match anything until next '/'
			while(*b != '/' && *b != '\0') b++;
		}
		if(*a != *b) return 0;
		if(!*a || !*b) return 1; 
		
		a++;
		b++;
	}
	
	return (*a == *b);
}

plcbit LLHttpAddHandler(unsigned long _ident, unsigned long pHandler) {
	LLHttpServiceLink_typ* ident = (LLHttpServiceLink_typ*)_ident;
	LLHttpHandler_typ* handler = (LLHttpHandler_typ*)pHandler;
	
	if(!ident || !handler) return 1;
	
	if(LLHttpHandlerIndex(_ident, pHandler) == -1) {
		BufferAddToTop((UDINT)&ident->handlers, (UDINT)handler);
	}
	
	return 0;
}
	
plcbit LLHttpRemoveHandler(unsigned long _ident, unsigned long pHandler) {
	LLHttpServiceLink_typ* ident = (LLHttpServiceLink_typ*)_ident;
	LLHttpHandler_typ* handler = (LLHttpHandler_typ*)pHandler;
	
	if(!ident || !handler) return 1;
	
	LLHttpHandler_typ* handlerToRemove = (LLHttpHandler_typ*)pHandler;
	LLHttpHandler_typ* handle;
	signed long i = LLHttpHandlerIndex(_ident, pHandler);
	unsigned int status;
	
	if(i >= 0) {
		// TODO: Status handling for errors
		BufferRemoveOffset((UDINT)&ident->handlers, i, status);
	}
	
	return 0;
}

signed long LLHttpAddHeaderField(unsigned long headerlines, unsigned long numLines, unsigned long name, unsigned long value) {
	if(!headerlines || !name) return LLHTTP_ERR_INVALID_INPUT;
	if(!numLines) return LLHTTP_ERR_MAX_HEADERS;
	LLHttpHeaderField_typ* lines = headerlines;
	signed int index;
	for (index = 0; index < numLines; index++) {
		if(lines[index].name[0] != '\0') continue; // TODO: Maybe change header value if exists?
		stringlcpy(lines[index].name, name, sizeof(lines[index].name)); // TODO: Maybe to lower here?
		stringlcpy(lines[index].value, value, sizeof(lines[index].value));
		return LLHTTP_ERR_OK;
	}
	
	// Fall through means we didnt find a place to add field
	return LLHTTP_ERR_MAX_HEADERS;
}
