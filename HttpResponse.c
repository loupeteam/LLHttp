
#ifdef __cplusplus
	extern "C"
	{
#endif

#include "LLHttpH.h"

#ifdef __cplusplus
	};
#endif

#include <string.h>

#define MIN(a,b) (((a)<(b))?(a):(b))
#define MAX(a,b) (((a)>(b))?(a):(b))

void onNewMessageCallback(HttpResponse_typ* t, HttpServiceLink_typ* api, HttpHeader_typ* header, unsigned long content) {
	if(t) {
		t->requestLength = header->contentLength;
		memcpy(&t->requestHeader, header, sizeof(t->requestHeader));
		if(t->pRequest) {
			memcpy(t->pRequest, content, MIN(header->contentLength, t->requestSize-1));
			((char*)t->pRequest)[MIN(header->contentLength, t->requestSize-1)] = '\0';
		}
		
		t->internal.newRequest = 1;
		t->internal.clientApi = api;
	}
}

void responseSuccessCallback(HttpResponse_typ* t, HttpServiceLink_typ* api, HttpHeader_typ* header, unsigned long content) {
	if(t) {
		t->internal.error = 0;
		t->internal.done  = 1;
		t->internal.busy = 0;
		
//		if(header) {
//			memcpy(&t->header, header, sizeof(t->header));
//			t->contentLength = t->header.contentLength;
//			if(content) {
//				unsigned long length = MIN(t->header.contentLength, t->responseSize);
//				memcpy(t->response, content, length);
//			}
//		}
	}
}

void responseErrorCallback(HttpResponse_typ* t, HttpServiceLink_typ* api, HttpHeader_typ* header, unsigned long content) {
	if(t) {
		t->internal.error = 1;
		t->internal.done = 0;
		t->internal.busy = 0;
		
//		if(header && header->status != 0) {
//			memcpy(&t->header, header, sizeof(t->header));
//			t->contentLength = t->header.contentLength;
//			if(content) {
//				unsigned long length = MIN(t->header.contentLength, t->responseSize);
//				memcpy(t->response, content, length);
//			}
//		}
	}
}

void HttpResponse(HttpResponse_typ* t) {
	// TODO: Check for invalid ident
	if(!t || t->ident == 0) return;
	
	HttpServiceLink_typ* ident = t->ident;
	
	if(t->enable && !t->internal.initialized) {
		// Cmd to Listen but not listening yet
		
		t->internal.handler.self = t;
		t->internal.handler.method = t->method;
		strcpy(t->internal.handler.uri, t->uri);
		t->internal.handler.newMessageCallback = &onNewMessageCallback;
		
		HttpAddHandler(t->ident, &t->internal.handler);
		t->internal.initialized = 1;
	}
	else if(!t->enable && t->internal.initialized) {
		// Cmd to stop not listen but still listening
		
		HttpRemoveHandler(t->ident, &t->internal.handler);
		t->internal.initialized = 0;
	}
	else if(t->enable) {
		// listening
		t->enabled = 1;
		
		if(t->send && !t->internal.send) {
			
			unsigned long i, len;
			HttpServiceResponse_typ response = {};
			response.self = (UDINT)t;
			strcpy(response.uri, t->uri);
			response.status = t->status;
			response.pPayload = t->pContent;
			response.payloadLength = t->contentLength;
			
			response.successCallback = &responseSuccessCallback;
			response.errorCallback = &responseErrorCallback;
			
			len = MIN(sizeof(response.userHeader)/sizeof(response.userHeader[0]), t->numUserHeaders);
			if(t->pUserHeader) {
				HttpHeaderLine_typ* headerLine = t->pUserHeader;
				for (i = 0; i < len; i++) {
					strcpy(response.userHeader[i].name, headerLine[i].name);
					strcpy(response.userHeader[i].value, headerLine[i].value);
				}
			}
			
			if(t->internal.clientApi) {
				BufferAddToBottom(&t->internal.clientApi->responseBuffer, &response);
			
				t->internal.error = 0;
				t->internal.done = 0;
				t->internal.busy = 1;
			}
			else {
				t->internal.error = 1;
				t->internal.done = 0;
				t->internal.busy = 0;
			}
	}
		
		
		
	}
	else {
		// NOT listening
		t->internal.done = 0;
		t->enabled = 0;
	}
	
	t->internal.send = t->send;
	t->error = t->internal.error;
	t->done = t->internal.done;
	t->busy = t->internal.busy;
	t->newRequest = t->internal.newRequest;
	t->internal.newRequest = 0;
	
	if(!t->internal.send) {
		t->internal.error = 0;
		t->internal.done = 0;
		t->internal.busy = 0;
	}
	
	
	ident->responseBuffer;

}
