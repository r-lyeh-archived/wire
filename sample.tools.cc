#include <cassert>
#include <iostream>
#include "wire.hpp"

int main( int argc, const char **argv )
{
    /* wire::format() */
    std::string cformat;
    cformat = wire::format("%d %1.3f %s", 10, 3.14159f, "hello world");
    assert( cformat == "10 3.142 hello world" );

    /* $wire() introspective macro */
    int val1 = 1;
    int val123 = 123;
    std::string echo = $wire("\1='\2';", cformat,val1,val123 );
    assert( echo == "cformat='10 3.142 hello world';val1='1';val123='123';" );

    std::cout << "All ok." << std::endl;
    return 0;
}
