
#ifdef __cplusplus
	extern "C"
	{
#endif

#include "LLHttpH.h"
#include "picohttpparser.h"
#include "HttpUtility.h"

#ifdef __cplusplus
	};
#endif


#include <string.h>
#include "stdlib.h"
#include "stdio.h"

#ifndef brsitoa
#define brsitoa(a,b) strlen(itoa(a,(char*)b,10))
#endif
#ifndef brsftoa
#include "math.h"
// Reverses a string 'str' of length 'len'
void reverse(char* str, int len)
{
    int i = 0, j = len - 1, temp;
    while (i < j) {
        temp = str[i];
        str[i] = str[j];
        str[j] = temp;
        i++;
        j--;
    }
}
  
// Converts a given integer x to string str[]. 
// d is the number of digits required in the output. 
// If d is more than the number of digits in x, 
// then 0s are added at the beginning.
int intToStr(int x, char str[], int d)
{
    int i = 0;
    while (x) {
        str[i++] = (x % 10) + '0';
        x = x / 10;
    }
  
    // If number of digits required is more, then
    // add 0s at the beginning
    while (i < d)
        str[i++] = '0';
  
    reverse(str, i);
    str[i] = '\0';
    return i;
}
  
// Converts a floating-point/double number to a string.
size_t ftoa(float n, char* res, int afterpoint)
{
    // Extract integer part
    int ipart = (int)n;
  
    // Extract floating part
    float fpart = n - (float)ipart;
  
    // convert integer part to string
    int i = intToStr(ipart, res, 0);
  
    // check for display option after point
    if (afterpoint != 0) {
        res[i] = '.'; // add dot
  
        // Get the value of fraction part upto given no.
        // of points after dot. The third parameter 
        // is needed to handle cases like 233.007
        fpart = fpart * pow(10, afterpoint);
  
        intToStr((int)fpart, res + i + 1, afterpoint);
    }

	return strlen(res);
}
#define brsftoa(a,b) ftoa(a,(char*)b,14)
#endif
#ifndef brsatoi
#define brsatoi(a) atoi((char*)a)
#endif

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
	if(memcmp((void*)t->data, "HTTP", 4) == 0) {
		int minor_version, returnValue, status;
		size_t statusStringLen, numHeaders;
		struct phr_header headerLines[HTTP_MAI_NUM_HEADER_LINES+3] = {};
		char* statusString;
		
		numHeaders = sizeof(headerLines)/sizeof(headerLines[0]);
		
		returnValue = phr_parse_response((char*)t->data, t->dataLength, &t->header.version, &t->header.status, &statusString, &statusStringLen, headerLines, &numHeaders, 0);
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
				stringlcpy(t->header.contentType, headerLines[index].value, min(headerLines[index].value_len+1, sizeof(t->header.contentType)));
			}
			
			copyHeaderLine(&t->header.lines[index], &headerLines[index]);
		}
		
		// We do not have content if status is 1XX, 204, 304, or content length is 0
		// Its possible that we still do not have content if the request was a HEAD. We do not have anyway to know this
		// See spec at https://greenbytes.de/tech/webdav/rfc7230.html#message.body
		if(t->header.status/100 != 1 && t->header.status%100 != 4 && t->header.contentLength != 0) {
			// We have content (maybe ;)
			t->contentPresent = 1;
			t->partialContent = (returnValue+t->header.contentLength > t->dataLength);
		}
		
	}
	else {
		int minor_version, returnValue, status;
		size_t methodlen, numHeaders, pathlen;
		char *method, *path;
		struct phr_header headerLines[HTTP_MAI_NUM_HEADER_LINES+3] = {};
		numHeaders = sizeof(headerLines)/sizeof(headerLines[0]);
		returnValue = phr_parse_request((char*)t->data, t->dataLength, (const char**)&method, &methodlen, &path, &pathlen, &t->header.version, headerLines, &numHeaders, 0);
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
		stringlcpy((UDINT)t->header.uri, (UDINT)path, min(pathlen+1, sizeof(t->header.uri)));
		
		unsigned int index;
		for (index = 0; index < numHeaders; index++) {
			if(strncasecmp("content-length", headerLines[index].name, headerLines[index].name_len) == 0) { // TODO: handle case insensitivity
				t->header.contentLength = brsatoi(headerLines[index].value);
			}
			else if(strncasecmp("content-type", headerLines[index].name, headerLines[index].name_len) == 0) {
				stringlcpy(t->header.contentType, headerLines[index].value, min(headerLines[index].value_len+1, sizeof(t->header.contentType)));
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
