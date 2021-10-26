#include <bur/plctypes.h>
#ifdef __cplusplus
	extern "C"
	{
#endif
#include "picohttpparser.h"
#ifdef __cplusplus
	};
#endif

typedef void (* HttpCallback)( UDINT context, HttpServiceLink_typ * api,  HttpHeader_typ * header, unsigned char * data);

void getMethodString(unsigned int method, unsigned int dest, unsigned int destSize);
const char* HttpStatusPhrase(signed short code);
void copyHeaderLine(HttpHeaderLine_typ* dest, struct phr_header* src);
