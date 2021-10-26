
TYPE
	HttpInternalRequest_typ : 	STRUCT 
		header : HttpHeader_typ;
		contentStart : USINT;
	END_STRUCT;
	HttpHandler_typ : 	STRUCT 
		method : HttpMethod_enum;
		uri : STRING[HTTP_MAX_LEN_URI];
		self : UDINT;
		newMessageCallback : UDINT;
		cyclicCallback : UDINT;
		client : REFERENCE TO HttpServerInternalClient_typ;
		busy : BOOL;
	END_STRUCT;
	HttpHeader_typ : 	STRUCT 
		contentType : STRING[HTTP_MAX_LEN_CONTENT_TYPE];
		contentLength : UDINT;
		uri : STRING[HTTP_MAX_LEN_URI];
		accept : STRING[HTTP_MAX_LEN_CONTENT_ACCEPT];
		lines : ARRAY[0..HTTP_MAI_NUM_HEADER_LINES]OF HttpHeaderLine_typ;
		version : INT;
		status : INT;
		method : HttpMethod_enum;
	END_STRUCT;
	HttpHeaderLine_typ : 	STRUCT 
		name : STRING[HTTP_MAX_LEN_HEADER_NAME];
		value : STRING[HTTP_MAX_LEN_HEADER_VALUE];
	END_STRUCT;
	HttpServiceLink_typ : 	STRUCT 
		requestBuffer : Buffer_typ;
		responseBuffer : Buffer_typ;
		handlers : Buffer_typ;
	END_STRUCT;
	HttpServiceResponse_typ : 	STRUCT 
		self : UDINT;
		uri : STRING[HTTP_MAX_LEN_URI];
		status : UDINT;
		userHeader : ARRAY[0..HTTP_MAI_NUM_HEADER_LINES]OF HttpHeaderLine_typ;
		ip : STRING[80];
		pPayload : UDINT;
		payloadLength : UDINT;
		successCallback : UDINT;
		errorCallback : UDINT;
	END_STRUCT;
	HttpServiceRequest_typ : 	STRUCT 
		self : UDINT;
		uri : STRING[HTTP_MAX_LEN_URI];
		method : UDINT;
		userHeader : ARRAY[0..HTTP_MAI_NUM_HEADER_LINES]OF HttpHeaderLine_typ;
		pPayload : UDINT;
		payloadLength : UDINT;
		successCallback : UDINT;
		errorCallback : UDINT;
	END_STRUCT;
	HttpServerInternalClient_typ : 	STRUCT 
		connected : USINT;
		tcpStream : TCPStream_typ;
		pReceiveData : {REDUND_UNREPLICABLE} UDINT;
		receiveDataSize : UDINT;
		pSendData : {REDUND_UNREPLICABLE} UDINT;
		sendDataSize : UDINT;
		sendBuffer : {REDUND_UNREPLICABLE} Buffer_typ;
		receivedBuffer : {REDUND_UNREPLICABLE} Buffer_typ;
		requestTimer : TON_10ms;
		recvLength : UDINT;
		parser : HttpParse;
		requestActive : BOOL;
		pCurrentRequest : REFERENCE TO HttpInternalRequest_typ;
		pRecvRequest : REFERENCE TO HttpInternalRequest_typ;
		pCurrentResponse : REFERENCE TO HttpServiceResponse_typ;
		api : HttpServiceLink_typ;
		bufferStatus : UINT;
		error : BOOL;
		errorId : DINT;
	END_STRUCT;
	HttpServerInternal_typ : 	STRUCT 
		responseTimeout : TON;
		retries : UINT;
		rawrecvData : ARRAY[0..19]OF STRING[80];
		rawSendData : ARRAY[0..19]OF STRING[80];
		bufferSize : UDINT;
		tcpStream : TCPStream_typ;
		tcpMgr : TCPConnectionMgr_typ;
		bufferStatus : UDINT;
		connected : BOOL;
		state : HttpState_enum;
		parser : HttpParse;
		pClients : REFERENCE TO HttpServerInternalClient_typ;
		clients : ARRAY[0..HTTP_MAI_NUM_CLIENTS]OF HttpServerInternalClient_typ; (*For Debug*)
		numClientsConnected : UDINT;
		numClients : UINT;
		initialized : BOOL;
		api : HttpServiceLink_typ;
	END_STRUCT;
	HttpClientInternal_typ : 	STRUCT 
		responseTimeout : TON;
		retries : UINT;
		rawrecvData : ARRAY[0..19]OF STRING[80];
		sendHeader : HttpHeaderLine_typ;
		rawSendData : ARRAY[0..19]OF STRING[80];
		bufferSize : UDINT;
		tcpStream : TCPStream_typ;
		tcpMgr : TCPConnectionMgr_typ;
		currentRequest : HttpServiceRequest_typ;
		api : HttpServiceLink_typ;
		bufferStatus : UDINT;
		connected : BOOL;
		state : HttpState_enum;
		parser : HttpParse;
		recvLength : UDINT;
		tempBuffer : STRING[1000];
	END_STRUCT;
	HttpConfiguration_typ : 	STRUCT 
		hostname : STRING[80];
		port : UINT;
		username : STRING[80];
		password : STRING[80];
		localIPAddress : STRING[80];
		localPort : UINT;
		https : BOOL;
		httpVersion : DINT;
		sslIndex : UINT;
	END_STRUCT;
	HttpResponseInternal_typ : 	STRUCT 
		send : BOOL;
		done : BOOL;
		busy : BOOL;
		error : BOOL;
		newRequest : BOOL;
		handler : HttpHandler_typ;
		clientApi : REFERENCE TO HttpServiceLink_typ;
		initialized : BOOL;
	END_STRUCT;
	HttpRequestInternal_typ : 	STRUCT 
		send : BOOL;
		done : BOOL;
		busy : BOOL;
		error : BOOL;
	END_STRUCT;
	HttpMethod_enum : 
		(
		HTTP_METHOD_GET,
		HTTP_METHOD_HEAD,
		HTTP_METHOD_POST,
		HTTP_METHOD_PUT,
		HTTP_METHOD_DELETE,
		HTTP_METHOD_CONNECT,
		HTTP_METHOD_OPTIONS,
		HTTP_METHOD_PATCH,
		HTTP_METHOD_TRACE,
		HTTP_METHOD_ANY,
		HTTP_METHOD_DEFAULT
		);
	HttpState_enum : 
		(
		HTTP_ST_IDLE,
		HTTP_ST_HEADER,
		HTTP_ST_AUTH,
		HTTP_ST_SEND,
		HTTP_ST_LISTEN,
		HTTP_ST_PARSE,
		HTTP_ST_ERROR,
		HTTP_ST_CLEAN,
		HTTP_ST_
		);
	HttpErr_enum : 
		(
		Http_ERR_VALUE_MISMATCH := -2,
		Http_ERR_NOT_FOUND := -1,
		Http_ERR_PARSE := 50100,
		Http_ERR_MAX_HEADERS,
		Http_ERR_
		);
END_TYPE

(*Enum for the HTTP status codes.*)

TYPE
	HttpStatusCode_enum : 
		(
		HTTP_STAT_Continue := 100, (* Indicates that the initial part OF a request has been received and has not yet been rejected by the server. *)
		HTTP_STAT_SwitchingProtocols := 101, (* Indicates that the server understands and is willing TO comply with the client's request, via the Upgrade header field, for a change in the application protocol being used on this connection. *)
		HTTP_STAT_Processing := 102, (* Is an interim response used TO inform the client that the server has accepted the complete request, but has not yet completed it. *)
		HTTP_STAT_EarlyHints := 103, (* Indicates TO the client that the server is likely TO send a final response with the header fields included in the informational response. *)
		HTTP_STAT_OK := 200, (* Indicates that the request has succeeded. *)
		HTTP_STAT_Created := 201, (* Indicates that the request has been fulfilled and has resulted in one or more new resources being created. *)
		HTTP_STAT_Accepted := 202, (* Indicates that the request has been accepted for processing, but the processing has not been completed. *)
		HTTP_STAT_NonAuthoritativeInfo := 203, (* Indicates that the request was successful but the enclosed payload has been modified from that OF the origin server's 200 (OK) response by a transforming proxy. *)
		HTTP_STAT_NoContent := 204, (* Indicates that the server has successfully fulfilled the request and that there is no additional content TO send in the response payload body. *)
		HTTP_STAT_ResetContent := 205, (* Indicates that the server has fulfilled the request and desires that the user agent reset the \"document view\", which caused the request TO be sent, to its original state as received from the origin server. *)
		HTTP_STAT_PartialContent := 206, (* Indicates that the server is successfully fulfilling a range request for the target resource by transferring one or more parts OF the selected representation that correspond TO the satisfiable ranges found in the requests's Range header field. *)
		HTTP_STAT_MultiStatus := 207, (* Provides status for multiple independent operations. *)
		HTTP_STAT_AlreadyReported := 208, (* Used inside a DAV:propstat response element TO avoid enumerating the internal members OF multiple bindings TO the same collection repeatedly. [RFC 5842] *)
		HTTP_STAT_IMUsed := 226, (* The server has fulfilled a GET request for the resource, and the response is a representation OF the result OF one or more instance-manipulations applied TO the current instance. *)
		HTTP_STAT_MultipleChoices := 300, (* Indicates that the target resource has more than one representation, each with its own more specific identifier, and information about the alternatives is being provided so that the user (or user agent) can select a preferred representation by redirecting its request TO one or more OF those identifiers. *)
		HTTP_STAT_MovedPermanently := 301, (* Indicates that the target resource has been assigned a new permanent URI and any future references TO this resource ought TO use one OF the enclosed URIs. *)
		HTTP_STAT_Found := 302, (* Indicates that the target resource resides temporarily under a different URI. *)
		HTTP_STAT_SeeOther := 303, (* Indicates that the server is redirecting the user agent TO a different resource, as indicated by a URI in the Location header field, that is intended TO provide an indirect response TO the original request. *)
		HTTP_STAT_NotModified := 304, (* Indicates that a conditional GET request has been received and would have resulted in a 200 (OK) response if it were not for the fact that the condition has evaluated TO false. *)
		HTTP_STAT_UseProxy := 305, (* \deprecated \parblock Due TO security concerns regarding in-band configuration OF a proxy. \endparblock
 The requested resource MUST be accessed through the proxy given by the Location field. *)
		HTTP_STAT_TemporaryRedirect := 307, (* Indicates that the target resource resides temporarily under a different URI and the user agent MUST NOT change the request method if it performs an automatic redirection TO that URI. *)
		HTTP_STAT_PermanentRedirect := 308, (* The target resource has been assigned a new permanent URI and any future references TO this resource ought TO use one OF the enclosed URIs. [...] This status code is similar TO 301 Moved Permanently (Section 7.3.2 OF rfc7231), except that it does not allow rewriting the request method from POST TO GET. *)
		HTTP_STAT_BadRequest := 400, (* Indicates that the server cannot or will not process the request because the received syntax is invalid, nonsensical, or exceeds some limitation on what the server is willing TO process. *)
		HTTP_STAT_Unauthorized := 401, (* Indicates that the request has not been applied because it lacks valid authentication credentials for the target resource. *)
		HTTP_STAT_PaymentRequired := 402, (* *Reserved* *)
		HTTP_STAT_Forbidden := 403, (* Indicates that the server understood the request but refuses TO authorize it. *)
		HTTP_STAT_NotFound := 404, (* Indicates that the origin server did not find a current representation for the target resource or is not willing TO disclose that one exists. *)
		HTTP_STAT_MethodNotAllowed := 405, (* Indicates that the method specified in the request-line is known by the origin server but not supported by the target resource. *)
		HTTP_STAT_NotAcceptable := 406, (* Indicates that the target resource does not have a current representation that would be acceptable TO the user agent, according TO the proactive negotiation header fields received in the request, and the server is unwilling TO supply a default representation. *)
		HTTP_STAT_ProxyAuthRequired := 407, (* Is similar TO 401 (Unauthorized), but indicates that the client needs TO authenticate itself in order TO use a proxy. *)
		HTTP_STAT_RequestTimeout := 408, (* Indicates that the server did not receive a complete request message within the TIME that it was prepared TO wait. *)
		HTTP_STAT_Conflict := 409, (* Indicates that the request could not be completed due TO a conflict with the current state OF the resource. *)
		HTTP_STAT_Gone := 410, (* Indicates that access TO the target resource is no longer available at the origin server and that this condition is likely TO be permanent. *)
		HTTP_STAT_LengthRequired := 411, (* Indicates that the server refuses TO accept the request without a defined Content-Length. *)
		HTTP_STAT_PreconditionFailed := 412, (* Indicates that one or more preconditions given in the request header fields evaluated TO FALSE when tested on the server. *)
		HTTP_STAT_PayloadTooLarge := 413, (* Indicates that the server is refusing TO process a request because the request payload is larger than the server is willing or able TO process. *)
		HTTP_STAT_URITooLong := 414, (* Indicates that the server is refusing TO service the request because the request-target is longer than the server is willing TO interpret. *)
		HTTP_STAT_UnsupportedMediaType := 415, (* Indicates that the origin server is refusing TO service the request because the payload is in a format not supported by the target resource for this method. *)
		HTTP_STAT_RangeNotSatisfiable := 416, (* Indicates that none OF the ranges in the request's Range header field overlap the current extent of the selected resource or that the set of ranges requested has been rejected due to invalid ranges or an excessive request of small or overlapping ranges. *)
		HTTP_STAT_ExpectationFailed := 417, (* Indicates that the expectation given in the request's Expect header field could not be met by at least one of the inbound servers. *)
		HTTP_STAT_ImATeapot := 418, (* Any attempt TO brew coffee with a teapot should result in the error code 418 I'm a teapot. *)
		HTTP_STAT_UnprocessableEntity := 422, (* Means the server understands the content TYPE OF the request entity (hence a 415(Unsupported Media TYPE) status code is inappropriate), and the syntax OF the request entity is correct (thus a 400 (Bad Request) status code is inappropriate) but was unable TO process the contained instructions. *)
		HTTP_STAT_Locked := 423, (* Means the source or destination resource OF a method is locked. *)
		HTTP_STAT_FailedDependency := 424, (* Means that the method could not be performed on the resource because the requested action depended on another action and that action failed. *)
		HTTP_STAT_UpgradeRequired := 426, (* Indicates that the server refuses TO perform the request using the current protocol but might be willing TO do so after the client upgrades TO a different protocol. *)
		HTTP_STAT_PreconditionRequired := 428, (* Indicates that the origin server requires the request TO be conditional. *)
		HTTP_STAT_TooManyRequests := 429, (* Indicates that the user has sent too many requests in a given amount OF TIME (\"rate limiting\"). *)
		HTTP_STAT_HeaderFieldsTooLarge := 431, (* Indicates that the server is unwilling TO process the request because its header fields are too large. *)
		HTTP_STAT_UnavailableLegalReason := 451, (* This status code indicates that the server is denying access TO the resource in response TO a legal demand. *)
		HTTP_STAT_InternalServerError := 500, (* Indicates that the server encountered an unexpected condition that prevented it from fulfilling the request. *)
		HTTP_STAT_NotImplemented := 501, (* Indicates that the server does not support the functionality required TO fulfill the request. *)
		HTTP_STAT_BadGateway := 502, (* Indicates that the server, while acting as a gateway or proxy, received an invalid response from an inbound server it accessed while attempting TO fulfill the request. *)
		HTTP_STAT_ServiceUnavailable := 503, (* Indicates that the server is currently unable TO handle the request due TO a temporary overload or scheduled maintenance, which will likely be alleviated after some delay. *)
		HTTP_STAT_GatewayTimeout := 504, (* Indicates that the server, while acting as a gateway or proxy, did not receive a timely response from an upstream server it needed TO access in order TO complete the request. *)
		HTTP_STAT_VersionNotSupported := 505, (* Indicates that the server does not support, or refuses TO support, the protocol version that was used in the request message. *)
		HTTP_STAT_VariantAlsoNegotiates := 506, (* Indicates that the server has an internal configuration error: the chosen variant resource is configured TO engage in transparent content negotiation itself, and is therefore not a proper end point in the negotiation process. *)
		HTTP_STAT_InsufficientStorage := 507, (* Means the method could not be performed on the resource because the server is unable TO store the representation needed TO successfully complete the request. *)
		HTTP_STAT_LoopDetected := 508, (* Indicates that the server terminated an operation because it encountered an infinite loop while processing a request with "Depth: infinity". [RFC 5842] *)
		HTTP_STAT_NotExtended := 510, (* The policy for accessing the resource has not been met in the request. [RFC 2774] *)
		HTTP_STAT_NetworkAuthRequired := 511, (* Indicates that the client needs TO authenticate TO gain network access. *)
		HTTP_STAT_xxx_max := 1023
		);
END_TYPE
