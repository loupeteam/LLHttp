/********************************************************************
 * COPYRIGHT --  
 ********************************************************************
 * Library: HTTPComm
 * File: HttpSocket.c
 * Author: sClaiborne
 * Created: April 12, 2021
 ********************************************************************
 * Implementation of library HTTPComm
 ********************************************************************/

#include <bur/plctypes.h>
#ifdef __cplusplus
	extern "C"
	{
#endif

#include "HTTPComm.h"

#ifdef __cplusplus
	};
#endif

#include "HttpUtility.h"
#include <string.h>

plcbit HttpStatus_isInformational(signed short code) { return (code >= 100 && code < 200); } /*!< \returns \c true if the given \p code is an informational code. */
plcbit HttpStatus_isSuccessful(signed short code)    { return (code >= 200 && code < 300); } /*!< \returns \c true if the given \p code is a successful code. */
plcbit HttpStatus_isRedirection(signed short code)   { return (code >= 300 && code < 400); } /*!< \returns \c true if the given \p code is a redirectional code. */
plcbit HttpStatus_isClientError(signed short code)   { return (code >= 400 && code < 500); } /*!< \returns \c true if the given \p code is a client error code. */
plcbit HttpStatus_isServerError(signed short code)   { return (code >= 500 && code < 600); } /*!< \returns \c true if the given \p code is a server error code. */
plcbit HttpStatus_isError(signed short code)         { return (code >= 400); }               /*!< \returns \c true if the given \p code is any type of error code. */

plcbit HttpStatus_getDescription(signed short code, unsigned long dest) {
	strcpy((char*)dest, HttpStatusPhrase(code));
	return 0;
}

plcbit HttpMethodMatch(HttpMethod_enum a, HttpMethod_enum b) {
	return (a == b || a == HTTP_METHOD_ANY || b == HTTP_METHOD_ANY);
}

plcbit HttpUriMatch(unsigned long _a, unsigned long _b) {
	if(!_a || !_b) return 0;
	//if(strcmp(_a, "*") == 0) return 1;
	//if(strcmp(_a, _b) == 0) return 1;
	
	char* a = (char*)_a;
	char* b = (char*)_b;
	
	// /en-US/docs/Learn/
	while(*a != '\0' && *b != '\0') {
		
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
		
		a++;
		b++;
	}
	
	return 1;
}

plcbit HttpAddHandler(unsigned long _ident, unsigned long pHandler) {
	HttpServiceLink_typ* ident = _ident;
	HttpHandler_typ* handler = pHandler;
	
	if(!ident || !handler) return 1;
	
	if(HttpHandlerIndex(_ident, pHandler) == -1) {
		BufferAddToTop(&ident->handlers, handler);
	}
	
	return 0;
}
	
plcbit HttpRemoveHandler(unsigned long _ident, unsigned long pHandler) {
	HttpServiceLink_typ* ident = _ident;
	HttpHandler_typ* handler = pHandler;
	
	if(!ident || !handler) return 1;
	
	HttpHandler_typ* handlerToRemove = pHandler;
	HttpHandler_typ* handle;
	signed long i = HttpHandlerIndex(_ident, pHandler);
	unsigned int status;
	
	if(i >= 0) {
		// TODO: Status handling for errors
		BufferRemoveOffset(&ident->handlers, i, status);
	}
	
	return 0;
}
