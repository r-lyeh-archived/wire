#include <iostream>
#include "wire.hpp"

int main( int argc, const char **argv ) {
    wire::getopt args( argc, argv );

    if( args.has("-h") || args.has("--help") || args.has("-?") || args.size() == 1 ) {
        std::cout << args["0"] << " [-?|-h|--help] [-v|--version] [--depth=number]" << std::endl;
        return 0;
    }

    if( args.has("-v") || args.has("--version") ) {
        std::cout << args["0"] << " sample v1.0.0. Compiled on " << __DATE__ << std::endl;
    }

    if( args.has("--depth") ) {
        int depth = args["--depth"];
        std::cout << "depth set to " << depth << std::endl;
    }

    std::cout << args.size() << " provided args: " << args.str() << std::endl;
    return 0;
}
