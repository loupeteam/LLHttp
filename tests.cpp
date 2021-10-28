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

int test_all()
{
    test_uriMatch();
    /* code */
    return 0;
}

void test_uriMatch(void) {
    #define testHttpUriMatch(a,b) printf("\033[0mChecking URI Match: Selctor: \033[0;33m%s\033[0m, uri: \033[0;33m%s\033[0m, is a match? %s\n", a, b, (HttpUriMatch((UDINT)a, (UDINT)b) ? "\033[0;32mtrue" : "\033[0;31mfalse"))
    
    testHttpUriMatch("/files/*", "/files/");
    testHttpUriMatch("/files/*", "/files/test");
    testHttpUriMatch("/files/*", "/files/test.cpp");
    testHttpUriMatch("/files/*", "/files/deeper/");
    testHttpUriMatch("/files/*", "/files/deeper/test.cpp");
    testHttpUriMatch("/files/*", "/other");
    testHttpUriMatch("/files/*", "/");

    printf("\033[0m\n");

    testHttpUriMatch("/files/*/", "/files/");
    testHttpUriMatch("/files/*/", "/files/test");
    testHttpUriMatch("/files/*/", "/files/test.cpp");
    testHttpUriMatch("/files/*/", "/files/deeper/");
    testHttpUriMatch("/files/*/", "/files/deeper/test.cpp");
    testHttpUriMatch("/files/*/", "/other");
    testHttpUriMatch("/files/*/", "/");

    printf("\033[0m\n");

    testHttpUriMatch("/files/**", "/files/");
    testHttpUriMatch("/files/**", "/files/test");
    testHttpUriMatch("/files/**", "/files/test.cpp");
    testHttpUriMatch("/files/**", "/files/deeper/");
    testHttpUriMatch("/files/**", "/files/deeper/test.cpp");
    testHttpUriMatch("/files/**", "/other");
    testHttpUriMatch("/files/**", "/");

    printf("\033[0m\n");

    testHttpUriMatch("/files/test.cpp", "/files/");
    testHttpUriMatch("/files/test.cpp", "/files/test");
    testHttpUriMatch("/files/test.cpp", "/files/test.cpp");
    testHttpUriMatch("/files/test.cpp", "/files/deeper/");
    testHttpUriMatch("/files/test.cpp", "/files/deeper/test.cpp");
    testHttpUriMatch("/files/test.cpp", "/other");
    testHttpUriMatch("/files/test.cpp", "/");

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
