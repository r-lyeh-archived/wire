// simple getopt replacement class. mit licensed
// - rlyeh

// this geptop class is a std::map replacement where key/value are wire::string.
// given invokation './app.out --user=me --pass=123 -h' this class delivers:
// map[0] = "./app.out", map[1] = "--user=me", map[2]="--pass=boo", map[3]='-h'
// but also, map["user"]="me", map["pass"]="123" and also, map["h"]=true

// .cmdline() for a print app invokation string
// .str() for pretty map printing

#include <iostream>
#include "wire.hpp"

struct getopt : public std::map< wire::string, wire::string >
{
    getopt()
    {}

    explicit
    getopt( int argc, const char **argv ) {
        wire::strings args( argc, argv );

        // create key=value and key= args as well
        for( auto &it : args ) {
            wire::strings tokens = it.split( "=" );

            if( !tokens.empty() )
                while( tokens[0].at(0) == '-' )
                    tokens[0].pop_front();

            if( tokens.size() == 3 && tokens[1] == "=" )
                (*this)[ tokens[0] ] = tokens[2];
            else
            if( tokens.size() == 2 && tokens[1] == "=" )
                (*this)[ tokens[0] ] = true;
            else
            if( tokens.size() == 1 && tokens[0] != argv[0] )
                (*this)[ tokens[0] ] = true;
        }

        // create args
        while( argc-- ) {
            (*this)[ argc ] = argv[argc];
        }

    }

    wire::string &operator []( const wire::string& t ) {
        std::map< wire::string, wire::string > &self = *this;
        return ( self[t] = self[t] );
    }

    bool has( const wire::string &op ) const {
        return this->find(op) != this->end();
    }

    std::string str() const {
        wire::string ss;
        for( auto &it : *this )
            ss << it.first << "=" << it.second << ',';
        return ss.str();
    }

    std::string cmdline() const {
        wire::string cmd;

        // concatenate args
        for( unsigned i = 0; has(i); ++i ) {
            const auto it = this->find(i);
            cmd << it->second << ' ';
        }

        // remove trailing space, if needed
        if( cmd.size() )
            cmd.pop_back();

        return cmd;
    }
};

int main( int argc, const char **argv ) {
    getopt args( argc, argv );

    if( args.has("h") || args.has("help") || args.has("?") || args.size() == 1 ) {
        std::cout << args[0] << " [-?|-h|--help] [-v|--version] [--depth=number]" << std::endl;
        return -1;
    }

    if( args.has("v") || args.has("version") ) {
        std::cout << args[0] << " sample v1.0.0. Compiled on " << __DATE__ << std::endl;
    }

    if( args.has("depth") ) {
        int depth = args["depth"];
        std::cout << "depth set to " << depth << std::endl;
    }

    std::cout << "Provided args: " << args.str() << std::endl;
    return 0;
}
