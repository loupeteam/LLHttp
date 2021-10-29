#ifdef __cplusplus
extern "C" {
#endif

#include "LLHttpH.h"

#ifdef __cplusplus
}
#endif


#include <iostream>
#include <string.h>

using namespace std;

void test_uriMatch(void);
int test_all();

inline constexpr unsigned char operator "" _uchar( unsigned long long arg ) noexcept
{
    return static_cast< unsigned char >( arg );
}


int test_all()
{
    test_uriMatch();
    /* code */
    return 0;
}

void test_uriMatch(void) {
    #define testHttpUriMatch(a,b,c) \
    ({\
    unsigned char r = HttpUriMatch((UDINT)a, (UDINT)b); \
    printf("\033[0mChecking URI Match: Selctor: \033[0;33m%s\033[0m, uri: \033[0;33m%s\033[0m, is a match? %s%s\n", a, b, (c==r?"\033[0;32m":"\033[0;31m"), (r ? "true" : "false")); \
    })
    
    testHttpUriMatch("/files/*", "/files/", 1_uchar);
    testHttpUriMatch("/files/*", "/files/test", 1_uchar);
    testHttpUriMatch("/files/*", "/files/test.cpp", 1_uchar);
    testHttpUriMatch("/files/*", "/files/deeper/", 0_uchar);
    testHttpUriMatch("/files/*", "/files/deeper/test.cpp", 0_uchar);
    testHttpUriMatch("/files/*", "/other", 0_uchar);
    testHttpUriMatch("/files/*", "/", 0_uchar);

    printf("\033[0m\n");

    testHttpUriMatch("/files/*/", "/files/", 0_uchar);
    testHttpUriMatch("/files/*/", "/files/test", 0_uchar);
    testHttpUriMatch("/files/*/", "/files/test.cpp", 0_uchar);
    testHttpUriMatch("/files/*/", "/files/deeper/", 1_uchar);
    testHttpUriMatch("/files/*/", "/files/deeper/test.cpp", 0_uchar);
    testHttpUriMatch("/files/*/", "/other", 0_uchar);
    testHttpUriMatch("/files/*/", "/", 0_uchar);

    printf("\033[0m\n");

    testHttpUriMatch("/files/**", "/files/", 1_uchar);
    testHttpUriMatch("/files/**", "/files/test", 1_uchar);
    testHttpUriMatch("/files/**", "/files/test.cpp", 1_uchar);
    testHttpUriMatch("/files/**", "/files/deeper/", 1_uchar);
    testHttpUriMatch("/files/**", "/files/deeper/test.cpp", 1_uchar);
    testHttpUriMatch("/files/**", "/other", 0_uchar);
    testHttpUriMatch("/files/**", "/", 0_uchar);

    printf("\033[0m\n");

    testHttpUriMatch("/files/test.cpp", "/files/", 0);
    testHttpUriMatch("/files/test.cpp", "/files/test", 0);
    testHttpUriMatch("/files/test.cpp", "/files/test.cpp", 1);
    testHttpUriMatch("/files/test.cpp", "/files/deeper/", 0);
    testHttpUriMatch("/files/test.cpp", "/files/deeper/test.cpp", 0);
    testHttpUriMatch("/files/test.cpp", "/other", 0);
    testHttpUriMatch("/files/test.cpp", "/", 0);

    printf("\033[0m\n");

    #undef testHttpUriMatch
}

int main(int argc, char const *argv[])
{
    cout << "\n-------Starting---------\n" << endl;

    test_all();

    cout << "\n-------Complete---------\n" << endl;

    /* code */
    return 0;
}