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


using namespace std;

TEST_CASE( "Test HTTP URI Match", "[LLHttp]" ) {
    #define testHttpUriMatch(a,b,c) REQUIRE(HttpUriMatch((UDINT)a, (UDINT)b) == c)
    
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
    HttpParse_typ parser = {};

    #define ParseTest(x) parser.data = (UDINT)x;parser.dataLength = strlen((char*)parser.data);HttpParse(&parser);

    SECTION( "Parse simple request with no body and no header values" ) {
        ParseTest("GET / HTTP/1.0\r\n");
        REQUIRE(parser.error == false);
        REQUIRE(parser.header.method == HTTP_METHOD_GET);
        REQUIRE(strcmp(parser.header.uri, "/") == 0);
    }

    SECTION( "Parse simple response with basic body" ) {
        ParseTest("HTTP/1.0 200 OK\r\ncontent-length: 6\r\ncontent-type: text\r\n\r\nsimple");
        REQUIRE(parser.error == false);
        REQUIRE(parser.contentPresent == true);
        REQUIRE(parser.header.contentLength == 6);
        REQUIRE(strcmp((char*)parser.content, "simple") == 0);
        REQUIRE(strcmp((char*)parser.header.contentType, "text") == 0);
    }

    SECTION( "Parse simple response with custom header value" ) {
        ParseTest("HTTP/1.0 200 OK\r\ncustom-header: 1\r\n\r");
        REQUIRE(parser.error == false);
        REQUIRE(parser.contentPresent == false);
        REQUIRE(parser.header.status == 200);
        REQUIRE(strcmp(parser.header.lines[0].name, "custom-header") == 0);
        REQUIRE(strcmp(parser.header.lines[0].value, "1") == 0);
    }

    // TODO: Test response with missing body
    // TODO: Test request with missing body
    // TODO: Test partial packets

    #undef ParserTest
}

TEST_CASE( "Test HTTP Utility FNs", "[LLHttp]" ) {

    char dest[250] = {};

    SECTION( "Verify return values for getMethodString" ) {
        #define getMethodStringTest(m, str) getMethodString(m, (UDINT)&dest, sizeof(dest)); REQUIRE(strcmp(str, dest) == 0)
        getMethodStringTest(HTTP_METHOD_GET, "GET");
        getMethodStringTest(HTTP_METHOD_PUT, "PUT");
        getMethodStringTest(HTTP_METHOD_POST, "POST");
        getMethodStringTest(HTTP_METHOD_DELETE, "DELETE");
        getMethodStringTest(HTTP_METHOD_HEAD, "HEAD");
        getMethodStringTest(HTTP_METHOD_OPTIONS, "OPTIONS");
        getMethodStringTest(HTTP_METHOD_PATCH, "PATCH");
        getMethodStringTest(HTTP_METHOD_TRACE, "TRACE");
        #undef getMethodStringTest
    }

    SECTION( "Verify return values for parseMethodString" ) {
        #define getMethodFromStringTest(mexpt, mstr) REQUIRE(parseMethodString((UDINT)mstr, strlen(mstr)) == mexpt)
        getMethodFromStringTest(HTTP_METHOD_GET, "GET");
        getMethodFromStringTest(HTTP_METHOD_PUT, "PUT");
        getMethodFromStringTest(HTTP_METHOD_POST, "POST");
        getMethodFromStringTest(HTTP_METHOD_DELETE, "DELETE");
        getMethodFromStringTest(HTTP_METHOD_HEAD, "HEAD");
        getMethodFromStringTest(HTTP_METHOD_OPTIONS, "OPTIONS");
        getMethodFromStringTest(HTTP_METHOD_PATCH, "PATCH");
        getMethodFromStringTest(HTTP_METHOD_TRACE, "TRACE");
        #undef getMethodFromStringTest
    }
}
