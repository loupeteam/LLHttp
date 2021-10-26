#include <bur/plctypes.h>
#ifdef __cplusplus
	extern "C"
	{
#endif

#include "LLHttp.h"
#include "picohttpparser.h"
#include "HttpUtility.h"

#ifdef __cplusplus
	};
#endif


#include <string.h>

#define min(a,b) (((a)<(b))?(a):(b))

// TODO: header can have duplicate names, handle this in the future but not important yet

void HttpParse(HttpParse_typ* t) {
	if(!t->data) return;
	
	// Reset outputs
	t->partialPacket = 0;
	t->contentPresent = 0;
	t->content = 0;
	t->partialContent = 0;
	t->error = 0;
	t->errorId = 0;
	memset(&t->header, 0, sizeof(t->header));
	
	if(t->dataLength < 4) {
		t->partialPacket = 1;
		return;
	}
	if(memcmp(t->data, "HTTP", 4) == 0) {
		int minor_version, returnValue, status, numHeaders;
		size_t statusStringLen;
		struct phr_header headerLines[HTTP_MAI_NUM_HEADER_LINES+3] = {};
		char* statusString;
		
		numHeaders = sizeof(headerLines)/sizeof(headerLines[0]);
		
		returnValue = phr_parse_response(t->data, t->dataLength, &t->header.version, &t->header.status, &statusString, &statusStringLen, headerLines, &numHeaders, 0);
		switch (returnValue) {
			case 0:
			default:
				// Okay	
				t->content = t->data+returnValue;
				break;

			case -1:
				// Error
				t->error = 1;
				t->errorId = Http_ERR_PARSE;
				return;
				break;
			
			case -2:
				// Partial message
				t->partialPacket = 1;
				return;
				break;
			
			case -3:
				// Max headers reached
				t->error = 1;
				t->errorId = Http_ERR_MAX_HEADERS;
				return;
				break;
		}
		
		unsigned int index;
		for (index = 0; index < numHeaders; index++) {
			if(strncasecmp("content-length", headerLines[index].name, headerLines[index].name_len) == 0) { // TODO: handle case insensitivity
				t->header.contentLength = brsatoi(headerLines[index].value);
			}
			else if(strncasecmp("content-type", headerLines[index].name, headerLines[index].name_len) == 0) {
				stringlcpy(t->header.contentType, headerLines[index].name, headerLines[index].name_len+1);
			}
			
			copyHeaderLine(&t->header.lines[index], &headerLines[index]);
		}
		
		// We do not have content if status is 1XX, 204, 304, or content lenght is 0
		// Its possible that we still do not have content if the request was a HEAD. We do not have anyway to know this
		// See spec at https://greenbytes.de/tech/webdav/rfc7230.html#message.body
		if(t->header.status/100 != 1 && t->header.status%100 != 4 && t->header.contentLength != 0) {
			// We have content (maybe ;)
			t->contentPresent = 1;
			t->partialContent = (returnValue+t->header.contentLength > t->dataLength);
		}
		
     
	}
	else {
		int minor_version, returnValue, status, methodlen, pathlen, numHeaders;
		char *method, *path;
		struct phr_header headerLines[HTTP_MAI_NUM_HEADER_LINES+3] = {};
		numHeaders = sizeof(headerLines)/sizeof(headerLines[0]);
		returnValue = phr_parse_request(t->data, t->dataLength, &method, &methodlen, &path, &pathlen, &t->header.version, headerLines, &numHeaders, 0);
		switch (returnValue) {
			case 0:
			default:
				// Okay	
				t->content = t->data+returnValue;
				break;

			case -1:
				// Error
				t->error = 1;
				t->errorId = Http_ERR_PARSE;
				return;
				break;
			
			case -2:
				// Partial message
				t->partialPacket = 1;
				return;
				break;
			
			case -3:
				// Max headers reached
				t->error = 1;
				t->errorId = Http_ERR_MAX_HEADERS;
				return;
				break;
		}
		
		t->header.method = parseMethodString(method, methodlen);
		stringlcpy(t->header.uri, path, min(pathlen+1, sizeof(t->header.uri)));
		
		unsigned int index;
		for (index = 0; index < numHeaders; index++) {
			if(strncasecmp("content-length", headerLines[index].name, headerLines[index].name_len) == 0) { // TODO: handle case insensitivity
				t->header.contentLength = brsatoi(headerLines[index].value);
			}
			else if(strncasecmp("content-type", headerLines[index].name, headerLines[index].name_len) == 0) {
				stringlcpy(t->header.contentType, headerLines[index].name, headerLines[index].name_len+1);
			}
			
			copyHeaderLine(&t->header.lines[index], &headerLines[index]);
		}
		
		
	}
	
	
	// Parse first line
	
}

signed short HttpgetHeaderIndex(unsigned long headerlines, unsigned long name, unsigned long value) {
	if(!headerlines || !name) return Http_ERR_NOT_FOUND;
	HttpHeaderLine_typ* lines = headerlines;
	signed int index;
	for (index = 0; index < sizeof(lines->name)/sizeof(lines->name[0]); index++) {
		if(lines[index].name[0] == '\0') break;
		if(strcmp(lines[index].name, name) == 0) {
			if((value && strcmp(lines[index].value, value)) || (!value)) return index;
			return Http_ERR_VALUE_MISMATCH;
		}
	}
	
	return Http_ERR_NOT_FOUND;
}

plcbit HttpHeaderContains(unsigned long headerlines, unsigned long name, unsigned long value) {
	return HttpgetHeaderIndex(headerlines, name, value) >= 0 ? 1 : 0;
}
