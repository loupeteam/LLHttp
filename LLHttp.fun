(*Http Usage FUBs*)

FUNCTION_BLOCK LLHttpServer (*Http Server*)
	VAR_INPUT
		enable : BOOL; (*Enable server*)
		bufferSize : UDINT; (*Size of send / recieve buffer*)
		numClients : UINT; (*Max number of clients at one time*)
		https : BOOL; (*Enable Https *)
		ipAddress : STRING[TCPCOMM_STRLEN_IPADDRESS]; (*Local IP*)
		port : UDINT; (*Local port*)
		sslIndex : UINT; (*SSl index for Https*)
		handleTrace : BOOL;
	END_VAR
	VAR_OUTPUT
		ident : UDINT; (*Connection Ident*)
		numConnectedClients : UINT; (*Number of clients connected*)
		error : BOOL;
		errorId : DINT;
	END_VAR
	VAR
		internal : LLHttpServerInternal_typ;
	END_VAR
END_FUNCTION_BLOCK

FUNCTION_BLOCK LLHttpClient (*Http Client*)
	VAR_INPUT
		enable : BOOL; (*Enable client, Client will continously try and connect to server*)
		bufferSize : UDINT; (*Send / Receive buffer size*)
		hostname : STRING[TCPCOMM_STRLEN_IPADDRESS];
		port : UINT;
		localIPAddress : STRING[TCPCOMM_STRLEN_IPADDRESS];
		localPort : UINT;
		https : BOOL;
		sslIndex : UINT;
	END_VAR
	VAR_OUTPUT
		connected : BOOL; (*Connected to server *)
		ident : UDINT; (*Cient ident*)
		error : BOOL;
		errorId : DINT;
	END_VAR
	VAR
		internal : LLHttpClientInternal_typ;
	END_VAR
END_FUNCTION_BLOCK

FUNCTION_BLOCK LLHttpRequest (*Request from header*)
	VAR_INPUT
		send : BOOL; (*Send message*)
		method : LLHttpMethod_enum; (*Method*)
		ident : UDINT; (*Client ident*)
		uri : STRING[LLHTTP_MAX_LEN_URI]; (*Host uri*)
		pUserHeader : UDINT;
		numUserHeaders : UDINT;
		pContent : UDINT; (*Body content*)
		contentType : STRING[LLHTTP_MAX_LEN_CONTENT_TYPE];
		contentLength : UDINT; (*Length of content*)
		pResponse : UDINT; (*Buffer for response body*)
		responseSize : UDINT; (*Size of buffer for response body *)
	END_VAR
	VAR_OUTPUT
		header : LLHttpHeader_typ; (*Response header*)
		responseLength : UDINT; (*Response body length*)
		busy : BOOL;
		done : BOOL;
		error : BOOL;
	END_VAR
	VAR
		internal : LLHttpRequestInternal_typ;
	END_VAR
END_FUNCTION_BLOCK

FUNCTION_BLOCK LLHttpResponse (*Respond to requests*)
	VAR_INPUT
		ident : UDINT; (*Client ident*)
		method : LLHttpMethod_enum; (*Request method to listen for*)
		uri : STRING[80]; (*Request URI to listen for*)
		enable : BOOL; (*Listen for requests*)
		send : BOOL; (*Send message*)
		pUserHeader : UDINT;
		numUserHeaders : UDINT;
		status : UDINT; (*Response status*)
		contentType : STRING[80];
		pContent : UDINT; (*Response body content*)
		contentLength : UDINT; (*Length of response content*)
		pRequest : UDINT; (*Buffer for request body*)
		requestSize : UDINT; (*Size of buffer for request body *)
	END_VAR
	VAR_OUTPUT
		enabled : BOOL; (*Listening for requests*)
		newRequest : BOOL; (*New request recieved *)
		requestHeader : LLHttpHeader_typ; (*Response header*)
		requestLength : UDINT; (*Response body length*)
		busy : BOOL; (*Response sending*)
		done : BOOL; (*Response sent*)
		error : BOOL; (*Response error*)
	END_VAR
	VAR
		internal : {REDUND_UNREPLICABLE} LLHttpResponseInternal_typ;
	END_VAR
END_FUNCTION_BLOCK
(*Http Header evaluation*)

FUNCTION_BLOCK LLHttpParse (*Parse Http request or response*)
	VAR_INPUT
		data : UDINT; (*Pointer to data (string) to parse*)
		dataLength : UDINT; (*Length of data*)
	END_VAR
	VAR_OUTPUT
		header : LLHttpHeader_typ; (*Parsed header*)
		partialPacket : BOOL; (*Indicates partial packet*)
		partialContent : BOOL; (*Indicate partial content *)
		contentPresent : BOOL; (*Indicates content is present in message*)
		content : UDINT; (*Pointer to content*)
		error : BOOL; (*Error occurred*)
		errorId : DINT;
	END_VAR
END_FUNCTION_BLOCK

FUNCTION LLHttpHeaderContains : BOOL (*Header contains key (value?)*)
	VAR_INPUT
		headerlines : UDINT; (*Pointer to header lines*)
		name : UDINT; (*Key to find*)
		value : UDINT; (*(optional) value to find*)
	END_VAR
END_FUNCTION

FUNCTION LLHttpgetHeaderIndex : INT (*Get index of header value*)
	VAR_INPUT
		headerlines : UDINT; (*Pointer to header lines*)
		name : UDINT; (*Key to find*)
		value : UDINT; (*(optional) value to find*)
	END_VAR
END_FUNCTION
(*Http Advance functions*)

FUNCTION LLHttpUriMatch : BOOL (*Compares two URIs*)
	VAR_INPUT
		a : UDINT;
		b : UDINT;
	END_VAR
END_FUNCTION

FUNCTION LLHttpMethodMatch : BOOL (*Compares two Methods*)
	VAR_INPUT
		a : LLHttpMethod_enum;
		b : LLHttpMethod_enum;
	END_VAR
END_FUNCTION

FUNCTION LLHttpBuildResponse : DINT (*Builds response from header data*)
	VAR_INPUT
		data : UDINT;
		response : UDINT;
		dataSize : UDINT;
		pLen : UDINT;
	END_VAR
END_FUNCTION

FUNCTION LLHttpAddHandler : BOOL (*Add request handler to HTTP Server*)
	VAR_INPUT
		ident : UDINT; (*Server Ident*)
		pHandler : UDINT; (*Pointer to Handler*)
	END_VAR
END_FUNCTION
(*Http response status *)

FUNCTION LLHttpStatus_isError : BOOL (*Status is an error*)
	VAR_INPUT
		code : INT; (*Status code (HttpStatusCode_enum)*)
	END_VAR
END_FUNCTION

FUNCTION LLHttpStatus_isServerError : BOOL (*Status is a server error*)
	VAR_INPUT
		code : INT; (*Status code (HttpStatusCode_enum)*)
	END_VAR
END_FUNCTION

FUNCTION LLHttpStatus_isClientError : BOOL (*Status is a client error*)
	VAR_INPUT
		code : INT; (*Status code (HttpStatusCode_enum)*)
	END_VAR
END_FUNCTION

FUNCTION LLHttpStatus_isRedirection : BOOL (*Status is a redirect *)
	VAR_INPUT
		code : INT; (*Status code (HttpStatusCode_enum)*)
	END_VAR
END_FUNCTION

FUNCTION LLHttpStatus_isSuccessful : BOOL (*Status is success*)
	VAR_INPUT
		code : INT; (*Status code (HttpStatusCode_enum)*)
	END_VAR
END_FUNCTION

FUNCTION LLHttpStatus_isInformational : BOOL (*Status is information*)
	VAR_INPUT
		code : INT; (*Status code (HttpStatusCode_enum)*)
	END_VAR
END_FUNCTION

FUNCTION LLHttpStatus_getDescription : BOOL (*Get description from status*)
	VAR_INPUT
		code : INT; (*Status code (HttpStatusCode_enum)*)
		dest : UDINT; (*Pointer to destination to store description. Dest should be a string[24] or larger*)
	END_VAR
END_FUNCTION
