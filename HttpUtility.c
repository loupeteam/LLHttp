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
#include <ctype.h>

#define MIN(a,b) (((a)<(b))?(a):(b))
#define MAX(a,b) (((a)>(b))?(a):(b))

void getMethodString(signed long method, unsigned long dest, unsigned long destSize) {
	switch (method)
	{
		case LLHTTP_METHOD_GET: stringlcpy(dest, "GET", destSize); break;
		case LLHTTP_METHOD_HEAD: stringlcpy(dest, "HEAD", destSize); break;
		case LLHTTP_METHOD_POST: stringlcpy(dest, "POST", destSize); break;
		case LLHTTP_METHOD_PUT: stringlcpy(dest, "PUT", destSize); break;
		case LLHTTP_METHOD_DELETE: stringlcpy(dest, "DELETE", destSize); break;
		case LLHTTP_METHOD_CONNECT: stringlcpy(dest, "CONNECT", destSize); break;
		case LLHTTP_METHOD_OPTIONS: stringlcpy(dest, "OPTIONS", destSize); break;
		case LLHTTP_METHOD_TRACE: stringlcpy(dest, "TRACE", destSize); break;
		case LLHTTP_METHOD_PATCH: stringlcpy(dest, "PATCH", destSize); break;
		default:
			break;
	}
}

unsigned int parseMethodString(unsigned long method, unsigned long methodlen) {
	if(strncasecmp("GET", (char*)method, methodlen) == 0) return LLHTTP_METHOD_GET;
	if(strncasecmp("HEAD", (char*)method, methodlen) == 0) return LLHTTP_METHOD_HEAD;
	if(strncasecmp("POST", (char*)method, methodlen) == 0) return LLHTTP_METHOD_POST;
	if(strncasecmp("PUT", (char*)method, methodlen) == 0) return LLHTTP_METHOD_PUT;
	if(strncasecmp("DELETE", (char*)method, methodlen) == 0) return LLHTTP_METHOD_DELETE;
	if(strncasecmp("CONNECT", (char*)method, methodlen) == 0) return LLHTTP_METHOD_CONNECT;
	if(strncasecmp("OPTIONS", (char*)method, methodlen) == 0) return LLHTTP_METHOD_OPTIONS;
	if(strncasecmp("TRACE", (char*)method, methodlen) == 0) return LLHTTP_METHOD_TRACE;
	if(strncasecmp("PATCH", (char*)method, methodlen) == 0) return LLHTTP_METHOD_PATCH;
	return LLHTTP_METHOD_DEFAULT;
}

/*! Returns the standard HTTP reason phrase for a HTTP status code.
 * \param code An HTTP status code.
 * \return The standard HTTP reason phrase for the given \p code or \c NULL if no standard
 * phrase for the given \p code is known.
 */
const char* HttpStatusPhrase(signed short code)
{
	switch (code)
	{

		/*####### 1xx - Informational #######*/
		case 100: return "Continue";
		case 101: return "Switching Protocols";
		case 102: return "Processing";
		case 103: return "Early Hints";

		/*####### 2xx - Successful #######*/
		case 200: return "OK";
		case 201: return "Created";
		case 202: return "Accepted";
		case 203: return "Non-Authoritative Information";
		case 204: return "No Content";
		case 205: return "Reset Content";
		case 206: return "Partial Content";
		case 207: return "Multi-Status";
		case 208: return "Already Reported";
		case 226: return "IM Used";

		/*####### 3xx - Redirection #######*/
		case 300: return "Multiple Choices";
		case 301: return "Moved Permanently";
		case 302: return "Found";
		case 303: return "See Other";
		case 304: return "Not Modified";
		case 305: return "Use Proxy";
		case 307: return "Temporary Redirect";
		case 308: return "Permanent Redirect";

		/*####### 4xx - Client Error #######*/
		case 400: return "Bad Request";
		case 401: return "Unauthorized";
		case 402: return "Payment Required";
		case 403: return "Forbidden";
		case 404: return "Not Found";
		case 405: return "Method Not Allowed";
		case 406: return "Not Acceptable";
		case 407: return "Proxy Authentication Required";
		case 408: return "Request Timeout";
		case 409: return "Conflict";
		case 410: return "Gone";
		case 411: return "Length Required";
		case 412: return "Precondition Failed";
		case 413: return "Payload Too Large";
		case 414: return "URI Too Long";
		case 415: return "Unsupported Media Type";
		case 416: return "Range Not Satisfiable";
		case 417: return "Expectation Failed";
		case 418: return "I'm a teapot";
		case 422: return "Unprocessable Entity";
		case 423: return "Locked";
		case 424: return "Failed Dependency";
		case 426: return "Upgrade Required";
		case 428: return "Precondition Required";
		case 429: return "Too Many Requests";
		case 431: return "Request Header Fields Too Large";
		case 451: return "Unavailable For Legal Reasons";

		/*####### 5xx - Server Error #######*/
		case 500: return "Internal Server Error";
		case 501: return "Not Implemented";
		case 502: return "Bad Gateway";
		case 503: return "Service Unavailable";
		case 504: return "Gateway Time-out";
		case 505: return "HTTP Version Not Supported";
		case 506: return "Variant Also Negotiates";
		case 507: return "Insufficient Storage";
		case 508: return "Loop Detected";
		case 510: return "Not Extended";
		case 511: return "Network Authentication Required";

		default: return "";
	}

}

void copyHeaderLine(LLHttpHeaderField_typ* dest, struct phr_header* src) {
	unsigned long length, index;
	length = MIN(sizeof(dest->name)-1, src->name_len);
	for (index = 0; index < length; index++) {
		dest->name[index] = tolower(src->name[index]);
	}
	dest->name[index] = '\0';
	length = MIN(sizeof(dest->value)-1, src->value_len);
	for (index = 0; index < length; index++) {
		dest->value[index] = src->value[index];//tolower(src->value[index]);
	}
	dest->value[index] = '\0';
}

signed long LLHttpHandlerIndex(unsigned long _ident, unsigned long pHandler) {
	LLHttpServiceLink_typ* ident = (LLHttpServiceLink_typ*)_ident;
	LLHttpHandler_typ* handler = (LLHttpHandler_typ*)pHandler;
	
	if(!ident || !handler) return 1;
	
	LLHttpHandler_typ* handlerToRemove = (LLHttpHandler_typ*)pHandler;
	LLHttpHandler_typ* handle;
	unsigned long i;
	unsigned int status;
	
	for (i = 0; i < ident->handlers.NumberValues; i++) {
		// TODO: Status handling for errors
		handler = (LLHttpHandler_typ*)BufferGetItemAdr((UDINT)&ident->handlers, i, (UDINT)&status);
		
		// Compare to see if they are the same
		if(handler->self == handlerToRemove->self
		&& strcmp(handler->uri, handlerToRemove->uri) == 0) {
			return i;
		}
	}
	
	return -1;
}

