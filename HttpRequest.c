#include <bur/plctypes.h>
#ifdef __cplusplus
	extern "C"
	{
#endif

#include "LLHttp.h"

#ifdef __cplusplus
	};
#endif

#include <string.h>

#define MIN(a,b) (((a)<(b))?(a):(b))
#define MAX(a,b) (((a)>(b))?(a):(b))

void successCallback(HttpRequest_typ* t, HttpServiceLink_typ* api, HttpHeader_typ* header, unsigned long content) {
	if(t) {
		t->internal.error = 0;
		t->internal.done  = 1;
		t->internal.busy = 0;
		
		if(header) {
			memcpy(&t->header, header, sizeof(t->header));
			t->contentLength = t->header.contentLength;
			if(content) {
				unsigned long length = MIN(t->header.contentLength, t->responseSize);
				if(t->pResponse) memcpy(t->pResponse, content, length);
			}
		}
	}
}

void errorCallback(HttpRequest_typ* t, HttpServiceLink_typ* api, HttpHeader_typ* header, unsigned long content) {
	if(t) {
		t->internal.error = 1;
		t->internal.done = 0;
		t->internal.busy = 0;
		
		if(header && header->status != 0) {
			memcpy(&t->header, header, sizeof(t->header));
			t->contentLength = t->header.contentLength;
			if(content) {
				unsigned long length = MIN(t->header.contentLength, t->responseSize);
				if(t->pResponse) memcpy(t->pResponse, content, length);
			}
		}
	}
}

void HttpRequest(HttpRequest_typ* t) {
	// TODO: Check for invalid ident
	if(!t || t->ident == 0) return;
	
	HttpServiceLink_typ* ident = t->ident;
	if(t->send && !t->internal.send) {
		unsigned long i, len;
		HttpServiceRequest_typ request = {};
		request.self = (UDINT)t;
		strcpy(request.uri, t->uri);
		request.method = t->method;
		request.pPayload = t->pContent;
		request.payloadLength = t->contentLength;
		request.successCallback = &successCallback;
		request.errorCallback = &errorCallback;
		
		len = MIN(sizeof(request.userHeader)/sizeof(request.userHeader[0]), t->numUserHeaders);
		if(t->pUserHeader) {
			HttpHeaderLine_typ* headerLine = t->pUserHeader;
			for (i = 0; i < len; i++) {
				strcpy(request.userHeader[i].name, headerLine[i].name);
				strcpy(request.userHeader[i].value, headerLine[i].value);
			}
		}
		
		BufferAddToBottom(&ident->requestBuffer, (UDINT)&request);
		
		t->internal.error = 0;
		t->internal.done = 0;
		t->internal.busy = 1;
	}
	
	t->internal.send = t->send;
	t->error = t->internal.error;
	t->done = t->internal.done;
	t->busy = t->internal.busy;
	
	if(!t->internal.send) {
		t->internal.error = 0;
		t->internal.done = 0;
		t->internal.busy = 0;
	}
}
