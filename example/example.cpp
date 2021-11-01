#ifdef __cplusplus
extern "C" {
#endif

#include "../LLHttpH.h"
#include "../HttpUtility.h"

#ifdef __cplusplus
}
#endif


#include <iostream>
#include <string.h>

using namespace std;
int main(int argc, char const *argv[]) {
	printf("This is an example");

	HttpParse_typ parser = {};

	parser.data = (UDINT)&"HTTP/1.0 200 OK\r\ncustom-header: 1\r\n\r\n";
	parser.dataLength = strlen((char*)parser.data);
	HttpParse(&parser);

	/* code */
	return 0;
}
