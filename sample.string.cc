#include "wire.hpp"

int main( int argc, const char **argv )
{
    /* many constructors */ {
    wire::string helloworld( "hello world" );                   // -> "hello world"
    wire::string h( 'h' );                                      // -> "h"
    wire::string hhh2( wire::string( 3, 'h' ) << "abc" );       // -> "hhhabc"
    wire::string hhh = wire::string( 'h', 3 ) << std::endl;     // -> "hhh\n"
    wire::string minusone = -1;                                 // -> "-1"
    wire::string zero = 0;                                      // -> "0"
    wire::string boolean = false;                               // -> "false"
    wire::string real = 3.1415926535897932384626433832795L;     // ~-> "3.14159"
    }

    /* safe c++ constructors */ {
    wire::string arg1( "hello \1", "world" );
    // -> "hello world"
    wire::string arg2( "hello \2 \1", "world", true );
    // -> "hello true world"
    wire::string arg3( "hello \1\2\3", '{', "world", (unsigned char)('}') );
    // -> "hello {world}"
    wire::string arg4( "hello \1 \2 \3 \4 \5 \6 \7", "world", 3.14159f, 3.14159L, false, '\x1', arg3, 0 );
    // -> "hello world 3.14159 3.14159 false \x1 hello {world} 0"
    }

    /* chaining */ {
    wire::string chain;
    chain << "hello world: " << 3 << 'a' << -1 << std::endl;    // -> "hello world: 3a-1\n"
    }

    /* implicit type conversion */ {
    std::string boolean = wire::string(false);          // -> "false"
    bool f = wire::string("false");                     // -> false
    int i = wire::string("123");                        // -> 123
    int j = wire::string(123);                          // -> 123
    }

    /* explicit type conversion */ {
    bool t = wire::string(100).as<bool>();              // -> true
    int k = wire::string(-456.123).as<int>();           // -> -456
    }

    /* extended methods */ {
    wire::string hi("Hi!");
    hi.at( 0);         // -> 'H'
    hi.at( 1);         // -> 'i'
    hi.at( 2);         // -> '!'
    hi.at( 3);         // -> 'H'
    hi.at( 4);         // -> 'i'
    hi.at( 5);         // -> '!'
    // [...]
    hi.at(-1);         // -> '!'
    hi.at(-2);         // -> 'i'
    hi.at(-3);         // -> 'H'
    hi.at(-4);         // -> '!'
    hi.at(-5);         // -> 'i'
    hi.at(-6);         // -> 'H'
    // [...]
    hi.at(5) = '?';    // hi = 'Hi?'
    hi.push_back(404); // hi = "Hi?404"
    hi.push_back('!'); // hi = "Hi?404!"
    hi.push_back(hi);  // hi = "Hi?404!Hi?404!"
    }

    /* quick introspection echo macro */ {
    int health = 100;
    float money = 123.25;
    const char *hello = "world!";
    std::string echo = $wire("\1=\2,", health,money,hello); // -> "health=100,money=123.25,hello=world!,"
    }

    return 0;
}
