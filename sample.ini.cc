#include <iostream>
#include <cassert>
#include "wire.hpp"

int main() {
    wire::ini ini;
    ini.load( "[test]\nnumber=123\nstring=hello world;this is a comment" );

    assert( ini["test.number"] == 123 );
    assert( ini["test.string"] == "hello world" );

    ini["added.number"] = 456;

    std::cout << ini.save() << std::endl;
    std::cout << "All ok." << std::endl;
}
