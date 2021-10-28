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

typedef void (* HttpCallback)( UDINT context, HttpServiceLink_typ * api,  HttpHeader_typ * header, unsigned char * data);

void getMethodString(signed long method, unsigned long dest, unsigned long destSize);
const char* HttpStatusPhrase(signed short code);
void copyHeaderLine(HttpHeaderLine_typ* dest, struct phr_header* src);
signed long HttpHandlerIndex(unsigned long _ident, unsigned long pHandler);
