/*
 * File: test.cpp
 * Copyright (c) 2023 Loupe
 * https://loupe.team
 * 
 * This file is part of LLHttp, licensed under the MIT License.
 */
#ifdef __cplusplus
extern "C" {
#endif

#include "LLHttpH.h"
#include "HttpUtility.h"

#ifdef __cplusplus
}
#endif


#include <iostream>
#include <string.h>

#define CONFIG_CATCH_MAIN
#include <catch2/catch_test_macros.hpp>
#include <catch2/benchmark/catch_benchmark_all.hpp>
#include <catch2/matchers/catch_matchers_all.hpp>
#include <catch2/generators/catch_generators_all.hpp>


using namespace std;

TEST_CASE( "Test HTTP URI Match", "[LLHttp]" ) {
	#define testHttpUriMatch(a,b,c) CHECK(LLHttpUriMatch((UDINT)a, (UDINT)b) == c)
	
	SECTION( "Should match single wildcard at end of path" ) {

		testHttpUriMatch("/files/*", "/files/", true);
		testHttpUriMatch("/files/*", "/files/test", true);
		testHttpUriMatch("/files/*", "/files/test.cpp", true);
		testHttpUriMatch("/files/*", "/files/deeper/", false);
		testHttpUriMatch("/files/*", "/files/deeper/test.cpp", false);
		testHttpUriMatch("/files/*", "/other", false);
		testHttpUriMatch("/files/*", "/", false);

	}
	SECTION( "Should match single wildcard in path" ) {

	testHttpUriMatch("/files/*/", "/files/", false);
	testHttpUriMatch("/files/*/", "/files/test", false);
	testHttpUriMatch("/files/*/", "/files/test.cpp", false);
	testHttpUriMatch("/files/*/", "/files/deeper/", true);
	testHttpUriMatch("/files/*/", "/files/deeper/test.cpp", false);
	testHttpUriMatch("/files/*/", "/other", false);
	testHttpUriMatch("/files/*/", "/", false);

	}
	SECTION( "Should match double wildcard" ) {

	testHttpUriMatch("/files/**", "/files/", true);
	testHttpUriMatch("/files/**", "/files/test", true);
	testHttpUriMatch("/files/**", "/files/test.cpp", true);
	testHttpUriMatch("/files/**", "/files/deeper/", true);
	testHttpUriMatch("/files/**", "/files/deeper/test.cpp", true);
	testHttpUriMatch("/files/**", "/other", false);
	testHttpUriMatch("/files/**", "/", false);

	}
	SECTION( "Should match exact path" ) {

	testHttpUriMatch("/files/test.cpp", "/files/", false);
	testHttpUriMatch("/files/test.cpp", "/files/test", false);
	testHttpUriMatch("/files/test.cpp", "/files/test.cpp", true);
	testHttpUriMatch("/files/test.cpp", "/files/deeper/", false);
	testHttpUriMatch("/files/test.cpp", "/files/deeper/test.cpp", false);
	testHttpUriMatch("/files/test.cpp", "/other", false);
	testHttpUriMatch("/files/test.cpp", "/", false);

	}
	SECTION( "Should only ignore query parameters when the full path matched" ) {

	testHttpUriMatch("/api/brew", "/api/brew?durationS=180", true);
	testHttpUriMatch("/api/brew/abort", "/api/brew?durationS=180", false);
	testHttpUriMatch("/api/brew", "/api/brew/abort?x=1", false);
	testHttpUriMatch("/api/brew/abort", "/api/brew/abort?x=1", true);
	testHttpUriMatch("/api/brewery", "/api/brew?x=1", false);
	testHttpUriMatch("/api/brew", "/api/brewery?x=1", false);
	testHttpUriMatch("/files/*", "/files/test?x=1", true);
	testHttpUriMatch("/files/**", "/files/deeper/test?x=1", true);

	}

	#undef testHttpUriMatch
}

TEST_CASE( "Test HTTP Parser", "[LLHttp]" ) {
	LLHttpParse_typ parser = {};

	#define ParseTest(x) parser.data = (UDINT)x;parser.dataLength = strlen((char*)parser.data);LLHttpParse(&parser);

	SECTION( "Parse simple request with no body and no header values" ) {
		ParseTest("GET / HTTP/1.0\r\n\r\n");
		CHECK(parser.error == false);
		CHECK(parser.header.method == LLHTTP_METHOD_GET);
		CHECK_THAT(parser.header.uri, Catch::Matchers::Equals("/"));
	}

	SECTION( "Parse simple request with basic body" ) {
		ParseTest("POST /api/data HTTP/1.1\r\ncontent-length: 6\r\ncontent-type: text\r\n\r\nsimple");
		CHECK(parser.error == false);
		CHECK(parser.header.method == LLHTTP_METHOD_POST);
		CHECK(parser.contentPresent == true);
		CHECK(parser.partialContent == false);
		CHECK(parser.header.contentLength == 6);
		CHECK_THAT((char*)parser.content, Catch::Matchers::Equals("simple"));
		CHECK_THAT((char*)parser.header.contentType, Catch::Matchers::Equals("text"));
	}

	SECTION( "Parse request with partial body" ) {
		ParseTest("POST /api/data HTTP/1.1\r\ncontent-length: 12\r\n\r\nsimple");
		CHECK(parser.error == false);
		CHECK(parser.contentPresent == true);
		CHECK(parser.partialContent == true);
	}

	SECTION( "Parse simple response with basic body" ) {
		ParseTest("HTTP/1.0 200 OK\r\ncontent-length: 6\r\ncontent-type: text\r\n\r\nsimple");
		CHECK(parser.error == false);
		CHECK(parser.contentPresent == true);
		CHECK(parser.header.contentLength == 6);
		CHECK_THAT((char*)parser.content, Catch::Matchers::Equals("simple"));
		CHECK_THAT((char*)parser.header.contentType, Catch::Matchers::Equals("text"));
	}

	SECTION( "Parse simple response with custom header value" ) {
		ParseTest("HTTP/1.0 200 OK\r\ncustom-header: 1\r\n\r\n");
		CHECK(parser.error == false);
		CHECK(parser.contentPresent == false);
		CHECK(parser.header.status == 200);
		CHECK_THAT(parser.header.lines[0].name, Catch::Matchers::Equals("custom-header"));
		CHECK_THAT(parser.header.lines[0].value, Catch::Matchers::Equals("1"));
	}

	SECTION("Benchmark HTTP Parser") {
		BENCHMARK("Http Parser Request Sm") {
			return ParseTest(
				"GET /hello.htm HTTP/1.1"\
				"User-Agent: Mozilla/4.0 (compatible; MSIE5.01; Windows NT)"\
				"Host: www.tutorialspoint.com"\
				"Accept-Language: en-us"\
				"Accept-Encoding: gzip, deflate"\
				"Connection: Keep-Alive"
				);
		};
		BENCHMARK("Http Parser Response Sm") {
			return ParseTest(
				"HTTP/1.1 200 OK"\
				"Date: Mon, 27 Jul 2009 12:28:53 GMT"\
				"Server: Apache/2.2.14 (Win32)"\
				"Last-Modified: Wed, 22 Jul 2009 19:15:56 GMT"\
				"Content-Length: 88"\
				"Content-Type: text/html"\
				"Connection: Closed"
				""\
				"<html>"\
				"<body>"\
				"<h1>Hello, World!</h1>"\
				"</body>"\
				"</html>"
				);
		};
	}

	// TODO: Test response with missing body
	// TODO: Test request with missing body
	// TODO: Test partial packets

	#undef ParserTest
}

TEST_CASE( "Test HTTP Utility FNs", "[LLHttp]" ) {

	char dest[250] = {};

	SECTION( "Verify return values for getMethodString" ) {
		#define getMethodStringTest(m, str) getMethodString(m, (UDINT)&dest, sizeof(dest)); CHECK_THAT(dest, Catch::Matchers::Equals(str))
		getMethodStringTest(LLHTTP_METHOD_GET, "GET");
		getMethodStringTest(LLHTTP_METHOD_PUT, "PUT");
		getMethodStringTest(LLHTTP_METHOD_POST, "POST");
		getMethodStringTest(LLHTTP_METHOD_DELETE, "DELETE");
		getMethodStringTest(LLHTTP_METHOD_HEAD, "HEAD");
		getMethodStringTest(LLHTTP_METHOD_OPTIONS, "OPTIONS");
		getMethodStringTest(LLHTTP_METHOD_PATCH, "PATCH");
		getMethodStringTest(LLHTTP_METHOD_TRACE, "TRACE");
		#undef getMethodStringTest
	}

	SECTION( "Verify return values for parseMethodString" ) {
		#define getMethodFromStringTest(mexpt, mstr) CHECK(parseMethodString((UDINT)mstr, strlen(mstr)) == mexpt)
		getMethodFromStringTest(LLHTTP_METHOD_GET, "GET");
		getMethodFromStringTest(LLHTTP_METHOD_PUT, "PUT");
		getMethodFromStringTest(LLHTTP_METHOD_POST, "POST");
		getMethodFromStringTest(LLHTTP_METHOD_DELETE, "DELETE");
		getMethodFromStringTest(LLHTTP_METHOD_HEAD, "HEAD");
		getMethodFromStringTest(LLHTTP_METHOD_OPTIONS, "OPTIONS");
		getMethodFromStringTest(LLHTTP_METHOD_PATCH, "PATCH");
		getMethodFromStringTest(LLHTTP_METHOD_TRACE, "TRACE");
		#undef getMethodFromStringTest
		}
}

TEST_CASE( "Test HTTP Build Response", "[LLHttp]" ) {
	char buffer[500];
	LLHttpServiceResponse_typ response = {};
	UDINT bufferLen = 0;

	SECTION("Build Http Response Sm") {
		strcpy(response.uri, "/");
		response.pPayload = (UDINT)&"simple";
		response.payloadLength = strlen((char*)response.pPayload);
		response.status = 200;
		strcpy(response.userHeader[0].name, "content-type");
		strcpy(response.userHeader[0].value, "text");
		LLHttpBuildResponse((UDINT)&buffer, (UDINT)&response, sizeof(buffer), &bufferLen);
		const std::string contentType{ "\r\ncontent-type: text\r\n" }; // Some reason Catch::Mathers::Contains is not working with char* but works with string
		const std::string contentLength{ "\r\ncontent-length: 6\r\n" }; 
		const std::string date{ "\r\nDate:" };
		// HTTP/1.1 200 OK\r\ncontent-length: 6\r\ncontent-type: text\r\n\r\nsimple
		CHECK_THAT(buffer, Catch::Matchers::StartsWith("HTTP/1.1 200 OK"));
		CHECK_THAT(buffer, Catch::Matchers::EndsWith("\r\n\r\nsimple"));
		CHECK_THAT(buffer, Catch::Matchers::ContainsSubstring(contentType));
		CHECK_THAT(buffer, Catch::Matchers::ContainsSubstring(contentLength));
		CHECK_THAT(buffer, Catch::Matchers::ContainsSubstring(date));
	}

	SECTION("Build Http Response with non-200 status") {
		strcpy(response.uri, "/");
		response.pPayload = (UDINT)&"conflict";
		response.payloadLength = strlen((char*)response.pPayload);
		response.status = 409;
		LLHttpBuildResponse((UDINT)&buffer, (UDINT)&response, sizeof(buffer), &bufferLen);
		CHECK_THAT(buffer, Catch::Matchers::StartsWith("HTTP/1.1 409 Conflict\r\n"));
	}

	SECTION("Build Http Response defaults status 0 to 200 OK") {
		strcpy(response.uri, "/");
		response.pPayload = (UDINT)&"simple";
		response.payloadLength = strlen((char*)response.pPayload);
		response.status = 0; // Application never set the status input
		LLHttpBuildResponse((UDINT)&buffer, (UDINT)&response, sizeof(buffer), &bufferLen);
		CHECK_THAT(buffer, Catch::Matchers::StartsWith("HTTP/1.1 200 OK\r\n"));
	}

	SECTION("Build Http Response with empty payload includes content-length: 0") {
		strcpy(response.uri, "/");
		response.status = 202;
		LLHttpBuildResponse((UDINT)&buffer, (UDINT)&response, sizeof(buffer), &bufferLen);
		CHECK_THAT(buffer, Catch::Matchers::StartsWith("HTTP/1.1 202 Accepted\r\n"));
		const std::string contentLengthZero{ "\r\ncontent-length: 0\r\n" };
		CHECK_THAT(buffer, Catch::Matchers::ContainsSubstring(contentLengthZero));
		CHECK_THAT(buffer, Catch::Matchers::EndsWith("\r\n\r\n"));
	}

	SECTION("Build Http Response with 204 status omits content-length") {
		strcpy(response.uri, "/");
		response.status = 204; // RFC 9110: no content-length on 204
		LLHttpBuildResponse((UDINT)&buffer, (UDINT)&response, sizeof(buffer), &bufferLen);
		CHECK_THAT(buffer, Catch::Matchers::StartsWith("HTTP/1.1 204 No Content\r\n"));
		CHECK_THAT(buffer, !Catch::Matchers::ContainsSubstring(std::string{"content-length"}));
	}
}

TEST_CASE( "Test HTTP Header line utility", "[LLHttp]") {
	#define setHeaderLine(i,n,v) strcpy(lines[i].name, n);strcpy(lines[i].value, v);
	LLHttpHeaderField_typ lines[10] = {};
	setHeaderLine(0,"content-length","10");
	setHeaderLine(1,"content-type","text");
	setHeaderLine(2,"Connection","close");
	setHeaderLine(3,"Accept","text/html");
	setHeaderLine(4,"Accept-Encoding","gzip, deflate");

	SECTION("Check header lines get index invalid inputs") {
		CHECK(LLHttpgetHeaderIndex((UDINT)&lines, 0, 0) == LLHTTP_ERR_NOT_FOUND);
		CHECK(LLHttpgetHeaderIndex(0, (UDINT)&"content-length", 0) == LLHTTP_ERR_NOT_FOUND);
		CHECK(LLHttpgetHeaderIndex((UDINT)&lines, 0, (UDINT)&"10") == LLHTTP_ERR_NOT_FOUND);
		CHECK(LLHttpgetHeaderIndex(0, 0, 0) == LLHTTP_ERR_NOT_FOUND);
	}
	SECTION("Check header lines get index wo/value") {
		CHECK(LLHttpgetHeaderIndex((UDINT)&lines, (UDINT)&"content", 0) == LLHTTP_ERR_NOT_FOUND);
		CHECK(LLHttpgetHeaderIndex((UDINT)&lines, (UDINT)&"content-length", 0) == 0);
		CHECK(LLHttpgetHeaderIndex((UDINT)&lines, (UDINT)&"Content-Length", 0) == 0);
		CHECK(LLHttpgetHeaderIndex((UDINT)&lines, (UDINT)&"content-type", 0) == 1);
		CHECK(LLHttpgetHeaderIndex((UDINT)&lines, (UDINT)&"Content-Type", 0) == 1);
		CHECK(LLHttpgetHeaderIndex((UDINT)&lines, (UDINT)&"connection", 0) == 2);
		CHECK(LLHttpgetHeaderIndex((UDINT)&lines, (UDINT)&"Connection", 0) == 2);
		CHECK(LLHttpgetHeaderIndex((UDINT)&lines, (UDINT)&"accept", 0) == 3);
		CHECK(LLHttpgetHeaderIndex((UDINT)&lines, (UDINT)&"Accept", 0) == 3);
		CHECK(LLHttpgetHeaderIndex((UDINT)&lines, (UDINT)&"accept-encoding", 0) == 4);
		CHECK(LLHttpgetHeaderIndex((UDINT)&lines, (UDINT)&"Accept-Encoding", 0) == 4);
	}
	SECTION("Check header lines get index w/value") {
		CHECK(LLHttpgetHeaderIndex((UDINT)&lines, (UDINT)&"content", (UDINT)&"any") == LLHTTP_ERR_NOT_FOUND);
		CHECK(LLHttpgetHeaderIndex((UDINT)&lines, (UDINT)&"content-length", (UDINT)&"10") == 0);
		CHECK(LLHttpgetHeaderIndex((UDINT)&lines, (UDINT)&"content-length", (UDINT)&"110") == LLHTTP_ERR_VALUE_MISMATCH);
		CHECK(LLHttpgetHeaderIndex((UDINT)&lines, (UDINT)&"content-type", (UDINT)&"text") == 1);
		CHECK(LLHttpgetHeaderIndex((UDINT)&lines, (UDINT)&"Content-Type", (UDINT)&"text/html") == LLHTTP_ERR_VALUE_MISMATCH);
		CHECK(LLHttpgetHeaderIndex((UDINT)&lines, (UDINT)&"connection", (UDINT)&"close") == 2);
		CHECK(LLHttpgetHeaderIndex((UDINT)&lines, (UDINT)&"Connection", (UDINT)&"keep-open") == LLHTTP_ERR_VALUE_MISMATCH);
		CHECK(LLHttpgetHeaderIndex((UDINT)&lines, (UDINT)&"accept", (UDINT)&"text/html") == 3);
		CHECK(LLHttpgetHeaderIndex((UDINT)&lines, (UDINT)&"Accept", (UDINT)&"html") == LLHTTP_ERR_VALUE_MISMATCH);
		CHECK(LLHttpgetHeaderIndex((UDINT)&lines, (UDINT)&"accept-encoding", (UDINT)&"gzip, deflate") == 4);
		CHECK(LLHttpgetHeaderIndex((UDINT)&lines, (UDINT)&"Accept-Encoding", (UDINT)&"any") == LLHTTP_ERR_VALUE_MISMATCH);
	}
	SECTION("Check header lines contains wo/value") {
		CHECK(LLHttpHeaderContains((UDINT)&lines, (UDINT)&"content", 0) == false);
		CHECK(LLHttpHeaderContains((UDINT)&lines, (UDINT)&"content-length", 0) == true);
		CHECK(LLHttpHeaderContains((UDINT)&lines, (UDINT)&"Content-Length", 0) == true);
		CHECK(LLHttpHeaderContains((UDINT)&lines, (UDINT)&"content-type", 0) == true);
		CHECK(LLHttpHeaderContains((UDINT)&lines, (UDINT)&"Content-Type", 0) == true);
		CHECK(LLHttpHeaderContains((UDINT)&lines, (UDINT)&"connection", 0) == true);
		CHECK(LLHttpHeaderContains((UDINT)&lines, (UDINT)&"Connection", 0) == true);
		CHECK(LLHttpHeaderContains((UDINT)&lines, (UDINT)&"accept", 0) == true);
		CHECK(LLHttpHeaderContains((UDINT)&lines, (UDINT)&"Accept", 0) == true);
		CHECK(LLHttpHeaderContains((UDINT)&lines, (UDINT)&"accept-encoding", 0) == true);
		CHECK(LLHttpHeaderContains((UDINT)&lines, (UDINT)&"Accept-Encoding", 0) == true);
	}
	SECTION("Check header lines contains w/value") {
		CHECK(LLHttpHeaderContains((UDINT)&lines, (UDINT)&"content", (UDINT)&"any") == false);
		CHECK(LLHttpHeaderContains((UDINT)&lines, (UDINT)&"content-length", (UDINT)&"10") == true);
		CHECK(LLHttpHeaderContains((UDINT)&lines, (UDINT)&"content-length", (UDINT)&"110") == false);
		CHECK(LLHttpHeaderContains((UDINT)&lines, (UDINT)&"content-type", (UDINT)&"text") == true);
		CHECK(LLHttpHeaderContains((UDINT)&lines, (UDINT)&"Content-Type", (UDINT)&"text/html") == false);
		CHECK(LLHttpHeaderContains((UDINT)&lines, (UDINT)&"connection", (UDINT)&"close") == true);
		CHECK(LLHttpHeaderContains((UDINT)&lines, (UDINT)&"Connection", (UDINT)&"keep-open") == false);
		CHECK(LLHttpHeaderContains((UDINT)&lines, (UDINT)&"accept", (UDINT)&"text/html") == true);
		CHECK(LLHttpHeaderContains((UDINT)&lines, (UDINT)&"Accept", (UDINT)&"html") == false);
		CHECK(LLHttpHeaderContains((UDINT)&lines, (UDINT)&"accept-encoding", (UDINT)&"gzip, deflate") == true);
		CHECK(LLHttpHeaderContains((UDINT)&lines, (UDINT)&"Accept-Encoding", (UDINT)&"any") == false);
	}
	SECTION("Check header lines copy header lines") {
		phr_header src = {};
		#define copyHeaderLineTest(n,v)\
		src.name = n;\
		src.name_len = strlen(src.name);\
		src.value = v;\
		src.value_len = strlen(src.value);\
		copyHeaderLine((LLHttpHeaderField_typ*)&lines[5], &src);\
		CHECK(LLHttpHeaderContains((UDINT)&lines, (UDINT)&n, (UDINT)&v) == true);

		copyHeaderLineTest("name", "value");
		copyHeaderLineTest("Accept-Language", "en-US");

		#undef copyHeaderLineTest
	}
	SECTION("Add header lines") {
		#define addHeaderLineTest(n,v)\
		LLHttpAddHeaderField((UDINT)&lines, sizeof(lines)/sizeof(lines[0]), (UDINT)n, (UDINT)v);\
		CHECK(LLHttpHeaderContains((UDINT)&lines, (UDINT)&n, (UDINT)&v) == true);

		addHeaderLineTest("name", "value");
		addHeaderLineTest("Accept-Language", "en-US");

		#undef addHeaderLineTest
	}
	
	#undef setHeaderLine
}


TEST_CASE( "Test HTTP Partial Packets", "[LLHttp]") {



	LLHttpClient_typ client = {};

	client.enable = true;
	client.internal.tcpMgr.OUT.NewConnectionAvailable = true;
	client.internal.state = LLHTTP_ST_SEND;


	strcpy( (char*) &client.internal.rawrecvData, (const char*) &"HTTP/1.1 200 OK\r\n"\
				"Date: Mon, 27 Jul 2009 12:28:53 GMT\r\n"\
				"Server: Apache/2.2.14 (Win32)\r\n"\
				"Last-Modified: Wed, 22 Jul 2009 19:15:56 GMT\r\n"\
				"Content-Length: 46\r\n"\
				"Content-Type: text/html\r\n"\
				"Connection: Closed\r\n"
				"\r\n"\
				"<html>"\
				"<body>");
	client.internal.tcpStream.Internal.FUB.Receive.recvlen = strlen((char*)&client.internal.rawrecvData);	

	//Get through init
	LLHttpClient( &client );

	strcpy( (char*) client.internal.tcpStream.IN.PAR.pReceiveData, (const char*)&"<h1>Hello, World!</h1>"\
				"</body>"\
				"</html>");

	client.internal.tcpStream.Internal.FUB.Receive.recvlen = strlen((char*)client.internal.tcpStream.IN.PAR.pReceiveData) + 1;

	//Get through send
	LLHttpClient( &client );
	CHECK( client.error == false );
	CHECK_THAT( (char*)client.internal.parser.content, Catch::Matchers::Equals(
				"<html>"\
				"<body>"\
				"<h1>Hello, World!</h1>"\
				"</body>"\
				"</html>"
				));

}

static int serverCallbackCount = 0;
static char serverCallbackBody[500];
static LLHttpHeader_typ serverCallbackHeader;

static void serverNewMessageCallback(UDINT context, LLHttpServiceLink_typ* api, LLHttpHeader_typ* header, unsigned char* data) {
	serverCallbackCount++;
	memcpy(&serverCallbackHeader, header, sizeof(serverCallbackHeader));
	strncpy(serverCallbackBody, (char*)data, sizeof(serverCallbackBody)-1);
}

TEST_CASE( "Test HTTP Server response honors the LLHttpResponse status input", "[LLHttp]") {

	struct StatusCase { UDINT status; const char* statusLine; };
	StatusCase c = GENERATE(
				StatusCase{200, "HTTP/1.1 200 OK\r\n"},
				StatusCase{202, "HTTP/1.1 202 Accepted\r\n"},
				StatusCase{204, "HTTP/1.1 204 No Content\r\n"},
				StatusCase{400, "HTTP/1.1 400 Bad Request\r\n"},
				StatusCase{404, "HTTP/1.1 404 Not Found\r\n"},
				StatusCase{409, "HTTP/1.1 409 Conflict\r\n"},
				StatusCase{500, "HTTP/1.1 500 Internal Server Error\r\n"},
				StatusCase{0,   "HTTP/1.1 200 OK\r\n"}); // Status left at 0 defaults to 200

	CAPTURE(c.status);

	LLHttpServer_typ server = {};
	server.enable = true;
	server.numClients = LLHTTP_MAX_NUM_CLIENTS;
	server.bufferSize = 2000;

	// First cycle initializes internals and publishes server.ident
	LLHttpServer(&server);

	LLHttpServerInternalClient_typ* client = &server.internal.pClients[0];

	// Service FB listening for the request, like a PLC task would set it up
	LLHttpResponse_typ fbResp = {};
	fbResp.ident = server.ident;
	fbResp.method = LLHTTP_METHOD_POST;
	strcpy(fbResp.uri, "/api/brew");
	fbResp.enable = true;
	LLHttpResponse(&fbResp); // Registers the handler

	const char* request =
				"POST /api/brew HTTP/1.1\r\n"\
				"Host: plc.local\r\n"\
				"content-type: application/json\r\n"\
				"content-length: 13\r\n"\
				"\r\n"\
				"{\"brew\":true}";

	// Preload the request so it is in place when the stubbed connection manager accepts
	strcpy((char*)client->pReceiveData, request);
	client->tcpStream.Internal.FUB.Receive.recvlen = strlen(request);

	// Cycle until the connection is accepted and the request dispatched to the service FB
	int cycles = 0;
	while(!(client->connected && fbResp.newRequest) && ++cycles < 10) {
		LLHttpServer(&server);
		LLHttpResponse(&fbResp);
	}
	REQUIRE(client->connected == 1);
	REQUIRE(fbResp.newRequest == 1);

	// Respond like an application would: status and send set in the same cycle,
	// with the FB called at the bottom of the cyclic
	const char* body = "{\"queued\":true}";
	fbResp.status = c.status;
	fbResp.pContent = (UDINT)body;
	fbResp.contentLength = strlen(body);
	fbResp.send = true;
	LLHttpResponse(&fbResp); // Queues the response

	// Next server cycle dequeues the response, builds the packet, and sends it
	LLHttpServer(&server);

	CHECK_THAT((char*)client->pSendData, Catch::Matchers::StartsWith(c.statusLine));
	CHECK_THAT((char*)client->pSendData, Catch::Matchers::EndsWith(body));

	// FB reports completion on its next call
	LLHttpResponse(&fbResp);
	CHECK(fbResp.done == 1);
	CHECK(fbResp.error == 0);

}

TEST_CASE( "Test HTTP Server query string request dispatches only to the matching route", "[LLHttp]") {

	// Regression for custom status codes arriving as 200 at the client: a request
	// with a query string ('POST /api/brew?durationS=180') used to match every
	// handler whose URI is a prefix of the request path ('/api/brew/abort' too).
	// Both routes then answered the same request and whichever response was queued
	// first won the wire - typically a 200 that masked the real route's 202/409

	LLHttpServer_typ server = {};
	server.enable = true;
	server.numClients = LLHTTP_MAX_NUM_CLIENTS;
	server.bufferSize = 2000;

	// First cycle initializes internals and publishes server.ident
	LLHttpServer(&server);

	LLHttpServerInternalClient_typ* client = &server.internal.pClients[0];

	LLHttpResponse_typ fbBrew = {};
	fbBrew.ident = server.ident;
	fbBrew.method = LLHTTP_METHOD_POST;
	strcpy(fbBrew.uri, "/api/brew");
	fbBrew.enable = true;
	LLHttpResponse(&fbBrew);

	LLHttpResponse_typ fbAbort = {};
	fbAbort.ident = server.ident;
	fbAbort.method = LLHTTP_METHOD_POST;
	strcpy(fbAbort.uri, "/api/brew/abort");
	fbAbort.enable = true;
	LLHttpResponse(&fbAbort);

	const char* request =
				"POST /api/brew?durationS=180&batchSize=50 HTTP/1.1\r\n"\
				"Host: plc.local\r\n"\
				"\r\n";

	strcpy((char*)client->pReceiveData, request);
	client->tcpStream.Internal.FUB.Receive.recvlen = strlen(request);

	int cycles = 0;
	while(!(client->connected && fbBrew.newRequest) && ++cycles < 10) {
		LLHttpServer(&server);
		LLHttpResponse(&fbBrew);
		LLHttpResponse(&fbAbort);
	}
	REQUIRE(client->connected == 1);
	REQUIRE(fbBrew.newRequest == 1);

	// The sibling route must not see the request
	CHECK(fbAbort.newRequest == 0);

	// The abort route answers 200 immediately, the way a one-cycle route state
	// machine would. If it was wrongly dispatched, this 200 is queued ahead of
	// the brew route's 202 and is what the client reads
	fbAbort.status = 200;
	fbAbort.send = true;
	LLHttpResponse(&fbAbort);

	// The brew route accepts the command with a 202
	fbBrew.status = 202;
	fbBrew.send = true;
	LLHttpResponse(&fbBrew);

	LLHttpServer(&server);

	CHECK_THAT((char*)client->pSendData, Catch::Matchers::StartsWith("HTTP/1.1 202 Accepted\r\n"));

}

TEST_CASE( "Test HTTP Server POST body split across TCP segments", "[LLHttp]") {

	LLHttpServer_typ server = {};

	// .NET HttpClient (among others) sends the headers and the body of a POST in
	// separate writes, so the server receives them as two TCP segments
	const char* headerSegment =
				"POST /api/data HTTP/1.1\r\n"\
				"Host: plc.local\r\n"\
				"content-type: application/json\r\n"\
				"content-length: 27\r\n"\
				"\r\n";
	const char* bodySegment = "{\"name\":\"test\",\"value\":420}";
	REQUIRE(strlen(bodySegment) == 27);

	server.enable = true;
	server.numClients = LLHTTP_MAX_NUM_CLIENTS;
	server.bufferSize = 2000;

	// First cycle initializes internals and allocates the client receive buffers
	LLHttpServer(&server);

	LLHttpServerInternalClient_typ* client = &server.internal.pClients[0];

	// Register a handler for the POST request
	serverCallbackCount = 0;
	memset(serverCallbackBody, 0, sizeof(serverCallbackBody));
	LLHttpHandler_typ handler = {};
	handler.method = LLHTTP_METHOD_POST;
	strcpy(handler.uri, "/api/data");
	handler.newMessageCallback = (UDINT)&serverNewMessageCallback;
	REQUIRE(LLHttpAddHandler(server.ident, (UDINT)&handler) == 0);

	// Preload the first TCP segment (headers only). The stubbed TCP layer reports received
	// data on every cycle, so the data must already be in place when the stubbed connection
	// manager accepts the incoming connection
	strcpy((char*)client->pReceiveData, headerSegment);
	client->tcpStream.Internal.FUB.Receive.recvlen = strlen(headerSegment);

	// Cycle until the connection is accepted and the first segment has been received
	int cycles = 0;
	while(!client->connected && ++cycles < 10) {
		LLHttpServer(&server);
	}
	REQUIRE(client->connected == 1);

	CHECK(server.error == false);
	CHECK(client->parser.partialContent == true);
	CHECK(serverCallbackCount == 0); // Request should not be dispatched until the body arrives

	// Next cycle: the second segment (body) is received at the shifted receive pointer
	strcpy((char*)client->tcpStream.IN.PAR.pReceiveData, bodySegment);
	client->tcpStream.Internal.FUB.Receive.recvlen = strlen(bodySegment);
	LLHttpServer(&server);

	CHECK(server.error == false);
	CHECK(serverCallbackCount == 1);
	CHECK(serverCallbackHeader.method == LLHTTP_METHOD_POST);
	CHECK_THAT(serverCallbackHeader.uri, Catch::Matchers::Equals("/api/data"));
	CHECK(serverCallbackHeader.contentLength == strlen(bodySegment));
	CHECK_THAT(serverCallbackBody, Catch::Matchers::Equals(bodySegment));
	CHECK_THAT((char*)&client->pCurrentRequest->contentStart, Catch::Matchers::Equals(bodySegment));

}

TEST_CASE( "Test HTTP Server oversized request does not roll over receive window", "[LLHttp]") {

	LLHttpServer_typ server = {};

	// A request whose body can never fit in the receive buffer. The server should error
	// out with LLHTTP_ERR_PACKET_SIZE_TOO_BIG instead of letting MaxReceiveLength roll
	// over (UDINT) and the TCP layer overrun the buffer
	const char* headerSegment =
				"POST /api/data HTTP/1.1\r\n"\
				"Host: plc.local\r\n"\
				"content-type: application/octet-stream\r\n"\
				"content-length: 10000000\r\n"\
				"\r\n";

	server.enable = true;
	server.numClients = LLHTTP_MAX_NUM_CLIENTS;
	server.bufferSize = 2000;

	// First cycle initializes internals and allocates the client receive buffers
	LLHttpServer(&server);

	LLHttpServerInternalClient_typ* client = &server.internal.pClients[0];

	// Preload the header segment so it is in place when the stubbed connection manager
	// accepts the incoming connection
	strcpy((char*)client->pReceiveData, headerSegment);
	client->tcpStream.Internal.FUB.Receive.recvlen = strlen(headerSegment);

	int cycles = 0;
	while(!client->connected && ++cycles < 10) {
		LLHttpServer(&server);
	}
	REQUIRE(client->connected == 1);
	CHECK(server.error == false);
	CHECK(client->parser.partialContent == true);

	// Keep feeding body segments until the buffer is exhausted. Like the real TCP layer,
	// never deliver more than the advertised MaxReceiveLength in one segment
	cycles = 0;
	while(!server.error && ++cycles < 1000) {
		UDINT chunk = client->tcpStream.IN.PAR.MaxReceiveLength < 1000 ? client->tcpStream.IN.PAR.MaxReceiveLength : 1000;
		memset((char*)client->tcpStream.IN.PAR.pReceiveData, 'a', chunk);
		client->tcpStream.Internal.FUB.Receive.recvlen = chunk;
		LLHttpServer(&server);
	}

	CHECK(server.error == true);
	CHECK(server.errorId == LLHTTP_ERR_PACKET_SIZE_TOO_BIG);
	CHECK(client->errorId == LLHTTP_ERR_PACKET_SIZE_TOO_BIG);
	CHECK(client->connected == 0); // Connection is dropped, like the client side does

	// The receive window must never extend past the allocated buffer
	CHECK(client->tcpStream.IN.PAR.MaxReceiveLength < client->receiveDataSize);
	CHECK(client->tcpStream.IN.PAR.pReceiveData + client->tcpStream.IN.PAR.MaxReceiveLength
			<= client->pReceiveData + client->receiveDataSize);

}

TEST_CASE( "Test HTTP Server debug client copy respects allocation size", "[LLHttp]") {

	LLHttpServer_typ server = {};

	server.enable = true;
	server.numClients = 2; // Fewer than the debug array holds (LLHTTP_MAX_NUM_CLIENTS)
	server.bufferSize = 2000;

	// First cycle initializes internals; pClients is allocated with only numClients elements
	LLHttpServer(&server);

	// Plant a sentinel in the debug array beyond numClients. If the debug copy reads a full
	// sizeof(clients) from the 2-element allocation, it reads past the heap block and
	// overwrites the sentinel with whatever lives there
	memset(&server.internal.clients[2], 0xAA, sizeof(LLHttpServerInternalClient_typ) * (LLHTTP_MAX_NUM_CLIENTS - 2));

	LLHttpServer(&server);

	// The first numClients elements mirror the allocation...
	CHECK(memcmp(&server.internal.clients[0], server.internal.pClients, sizeof(LLHttpServerInternalClient_typ) * 2) == 0);

	// ...and elements beyond numClients are untouched
	const unsigned char* sentinel = (const unsigned char*)&server.internal.clients[2];
	bool sentinelIntact = true;
	for(size_t k = 0; k < sizeof(LLHttpServerInternalClient_typ) * (LLHTTP_MAX_NUM_CLIENTS - 2); k++) {
		if(sentinel[k] != 0xAA) {
			sentinelIntact = false;
			break;
		}
	}
	CHECK(sentinelIntact);

}