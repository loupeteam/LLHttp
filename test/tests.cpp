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

#define CONFIG_CATCH_MAIN
#include <catch2/catch_test_macros.hpp>
#include <catch2/benchmark/catch_benchmark_all.hpp>
#include <catch2/matchers/catch_matchers_all.hpp>


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

	#undef testHttpUriMatch
}

TEST_CASE( "Test HTTP Parser", "[LLHttp]" ) {
	LLHttpParse_typ parser = {};

	#define ParseTest(x) parser.data = (UDINT)x;parser.dataLength = strlen((char*)parser.data);LLHttpParse(&parser);

	SECTION( "Parse simple request with no body and no header values" ) {
		ParseTest("GET / HTTP/1.0\r\n\r\n");
		CHECK(parser.error == false);
		CHECK(parser.header.method == LLHTTP_METHOD_GET);
		REQUIRE_THAT(parser.header.uri, Catch::Matchers::Equals("/"));
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
		LLHttpBuildResponse((UDINT)&buffer, (UDINT)&response, sizeof(buffer), (UDINT)&bufferLen);
		const std::string contentType{ "\r\ncontent-type: text\r\n" }; // Some reason Catch::Mathers::Contains is not working with char* but works with string
		const std::string contentLength{ "\r\ncontent-length: 6\r\n" }; 
		const std::string date{ "\r\nDate:" };
		// HTTP/1.1 200 OK\r\ncontent-length: 6\r\ncontent-type: text\r\n\r\nsimple
		CHECK_THAT(buffer, Catch::Matchers::StartsWith("HTTP/1.1 200 OK"));
		CHECK_THAT(buffer, Catch::Matchers::EndsWith("\r\n\r\nsimple"));
		CHECK_THAT(buffer, Catch::Matchers::Contains(contentType));
		CHECK_THAT(buffer, Catch::Matchers::Contains(contentLength));
		CHECK_THAT(buffer, Catch::Matchers::Contains(date));
	}
}
