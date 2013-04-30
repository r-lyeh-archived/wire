#include <cassert>
#include <iostream>
#include "wire.hpp"

int main( int argc, const char **argv )
{
    /* wire tools */
    std::string cformat;
    cformat = wire::format("%d %1.3f %s", 10, 3.14159f, "hello world");
    assert( cformat == "10 3.142 hello world" );

    double val1 = wire::eval("5*(4+4+1)");      // ->  45
    assert( val1 == 45 );

    double val2 = wire::eval("-5*(2*(1+3)+1)"); // -> -45
    assert( val2 == -45 );

    double val3 = wire::eval("5*((1+3)*2+1)");  // ->  45
    assert( val3 == 45 );

    std::cout << "All ok." << std::endl;
    return 0;
}
