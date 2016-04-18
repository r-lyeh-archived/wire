#include <cassert>
#include <cmath>

#include <iostream>

#include "wire.hpp"

std::stringstream right, wrong;

#define test1(A) [&]() { auto _A_ = (A); if( _A_ != decltype(A)(0) ) \
    return right << "[ OK ] " __FILE__ ":" << __LINE__ << " -> " #A " -> " << _A_ << std::endl, true; else \
    return wrong << "[FAIL] " __FILE__ ":" << __LINE__ << " -> " #A " -> " << _A_ << std::endl, false; \
}()

#define test3(A,op,B) [&]() { auto _A_ = (A); auto _B_ = (B); if( _A_ op _B_ ) \
    return right << "[ OK ] " __FILE__ ":" << __LINE__ << " -> " #A " " #op " " #B " -> " << _A_ << " " #op " " << _B_ << std::endl, true; else \
    return wrong << "[FAIL] " __FILE__ ":" << __LINE__ << " -> " #A " " #op " " #B " -> " << _A_ << " " #op " " << _B_ << std::endl, false; \
}()

#define testN(NaN)    test3(NaN,!=,NaN)

struct ctest_stream1
{
    operator std::string()
    {
        return "im b";
    }
};

struct ctest_stream2
{
    ctest_stream2()
    {}

    ctest_stream2( const std::string &t )
    {
        std::cout << t << std::endl;
    }

    void test()
    {}
};

void tests_from_string_sample()
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

    test3( helloworld ,==, "hello world" );
    test3( h          ,==, "h" );
    test3( hhh2       ,==, "hhhabc" );
    test3( hhh        ,==, "hhh\n" );
    test3( minusone   ,==, "-1" );
    test3( zero       ,==, "0" );
    test3( boolean    ,==, "false" );
    test3( real       ,==, "3.14159" );
    }

    /* safe c++ constructors */ {
    wire::string arg1( "hello \1", "world" );
    // -> "hello world"
    wire::string arg2( "hello \2 \1", "world", true );
    // -> "hello true world"
    wire::string arg3( "hello \1\2\3", '{', "world", (unsigned char)('}') );
    // -> "hello {world}"
    wire::string arg4( "hello \1 \2 \3 \4 \5 \6 \7", "world", 3.14159f, 3.14159L, false, '\x1', arg3, 0 );
    // -> "hello world 3.14159 3.14159 false \1 hello {world} 0"

    test3( arg1 ,==, "hello world" );
    test3( arg2 ,==, "hello true world" );
    test3( arg3 ,==, "hello {world}" );
    test3( arg4 ,==, "hello world 3.14159 3.14159 false \x1 hello {world} 0" );
    }

    /* chaining */ {
    wire::string chain;
    chain << "hello world: " << 3 << 'a' << -1 << std::endl;    // -> "hello world: 3a-1\n"

    test3( chain ,==, "hello world: 3a-1\n" );
    }

    /* implicit type conversion */ {
    std::string boolean = wire::string(false);          // -> "false"
    bool f = wire::string("false");                     // -> false
    int i = wire::string("123");                        // -> 123
    int j = wire::string(123);                          // -> 123

    test3( boolean ,==, "false" );
    test3( f ,==, false );
    test3( i ,==, 123 );
    test3( j ,==, 123 );
    }

    /* explicit type conversion */ {
    bool t = wire::string(100).as<bool>();              // -> true
    int k = wire::string(-456.123).as<int>();           // -> -456

    test3( t ,==, true );
    test3( k ,==, -456 );
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

    test3( hi ,==, "Hi?404!Hi?404!" );
    }

    /* quick introspection echo macro */ {
    int health = 100;
    float money = 123.25;
    const char *hello = "world!";

    std::string echo = $wire("\1=\2;", health,money,hello);
    test3( echo, ==, "health=100;money=123.25;hello=world!;" );

    test3( std::string(), ==, $wire("", 0) );
    }
}

int main( int argc, const char **argv )
{
    // tools
    test3( wire::format("%d %1.3f %s", 10, 3.14159f, "hello world"), ==, "10 3.142 hello world" );

    test3( wire::string(99.95f), ==, "99.95" );
    test3( wire::string(999.9999), ==, 999.9999 );
    test3( wire::precise(999.9999f), ==, "0x1.f3fffcp+9" );
    test3( wire::precise("0x1.f3fffcp+9"), ==, 999.9999 );

    test3( wire::string().strip(), ==, wire::string() );
    test3( wire::string("").strip(), ==, wire::string() );
    test3( wire::string("abc").strip(), ==, "abc" );
    test3( wire::string("abc ").strip(), ==, "abc" );
    test3( wire::string(" abc").strip(), ==, "abc" );
    test3( wire::string(" abc ").strip(), ==, "abc" );
    test3( wire::string("a b c").strip(), ==, "a b c" );
    test3( wire::string(" a b c").strip(), ==, "a b c" );
    test3( wire::string("a b c ").strip(), ==, "a b c" );
    test3( wire::string(" a b c ").strip(), ==, "a b c" );

    test3( wire::string("abc").lstrip(), ==, "abc" );
    test3( wire::string("abc ").lstrip(), ==, "abc " );
    test3( wire::string(" abc").lstrip(), ==, "abc" );
    test3( wire::string(" abc ").lstrip(), ==, "abc " );
    test3( wire::string("a b c").lstrip(), ==, "a b c" );
    test3( wire::string(" a b c").lstrip(), ==, "a b c" );
    test3( wire::string("a b c ").lstrip(), ==, "a b c " );
    test3( wire::string(" a b c ").lstrip(), ==, "a b c " );

    test3( wire::string("abc").rstrip(), ==, "abc" );
    test3( wire::string("abc ").rstrip(), ==, "abc" );
    test3( wire::string(" abc").rstrip(), ==, " abc" );
    test3( wire::string(" abc ").rstrip(), ==, " abc" );
    test3( wire::string("a b c").rstrip(), ==, "a b c" );
    test3( wire::string(" a b c").rstrip(), ==, " a b c" );
    test3( wire::string("a b c ").rstrip(), ==, "a b c" );
    test3( wire::string(" a b c ").rstrip(), ==, " a b c" );

    //ptr to method
    if( 1 )
    {
        ctest_stream2 cts2;
        std::cout << wire::string( &ctest_stream2::test ) << std::endl;
    }

    test1( !wire::string("false").as<int>() );
    test1(  wire::string( "true").as<int>() );

    int a = wire::string();
    int b = int( wire::string() );

    test1( !a );
    test1( !b );

    //TEST
    std::string test_stream1a = ctest_stream1();
    //wire::string test_stream1b = ctest_stream1();

    test3( test_stream1a, ==, "im b" );
    //te3t1( test_stream1b, ==, "im b" );

    const char *str_constchar = "hello world";

    wire::string str_explicit("hello world");

    wire::string str_formatted  = wire::string("hello \1", "world");
    wire::string str_stdstring  = std::string("hello world");
    wire::string str_copyctor1  = wire::string( "hello world" );
    wire::string str_copyctor2  = wire::string( str_stdstring );

    wire::string str_assignop;  str_assignop = str_explicit;

    test3( str_explicit , ==, "hello world" );

    test3( str_formatted, ==, "hello world" );
    test3( str_stdstring, ==, "hello world" );
    test3( str_copyctor1, ==, "hello world" );
    test3( str_copyctor2, ==, "hello world" );

    test3( str_assignop , ==, "hello world" );

    test3( wire::string( 'a' ), ==, 'a' );
    test3( wire::string( "hi" ), ==, "hi" );
    test3( wire::string( true ), ==, true );
    test3( wire::string( 16384 ), ==, 16384 );
    test3( wire::string( 3.14159 ), ==, 3.14159 );
    test3( wire::string( 3.14159f ), ==, 3.14159f );

    test3( wire::string(), ==, 0 );
    test3( wire::string(), ==, 0.f );
    test3( wire::string(), ==, 0.0 );
    test3( wire::string(), ==, '\0' );
    test3( wire::string(), ==, "" );
    test3( wire::string(), ==, false );

    test3( wire::string( 'a' ), ==, 'a' );
    test3( wire::string( 'a' ), ==, "a" );
//  3 test1( wire::string( 'a' ), ==, 97 );
    test3( wire::string( "a" ), ==, 'a' );
    test3( wire::string( "a" ), ==, "a" );
//  3 test1( wire::string( "a" ), ==, 97 );
//  3 test1( wire::string(  97 ), ==, 'a' );
//  3 test1( wire::string(  97 ), ==, "a" );
    test3( wire::string(  97 ), ==, 97 );
    test3( wire::string(  97 ).as<int>(), ==, 97 );
    test3( wire::string(  97 ).as<char>(), ==, 'a' );

    test3( wire::string(         ).as<bool>(), ==, false );
    test3( wire::string(       0 ).as<bool>(), ==, false );
    test3( wire::string(       1 ).as<bool>(), ==,  true );
    test3( wire::string(       2 ).as<bool>(), ==,  true );
    test3( wire::string(     "0" ).as<bool>(), ==, false );
    test3( wire::string(     "1" ).as<bool>(), ==,  true );
    test3( wire::string(     "2" ).as<bool>(), ==,  true );
    test3( wire::string(   false ).as<bool>(), ==, false );
    test3( wire::string(    true ).as<bool>(), ==,  true );
    test3( wire::string( "false" ).as<bool>(), ==, false );
    test3( wire::string(  "true" ).as<bool>(), ==,  true );

    test3( wire::string(   'a' ).as<char>(), ==,    'a' );
    test3( wire::string(       ).as<char>(), ==,   '\0' );
    test3( wire::string(     0 ).as<char>(), ==,    '0' );
    test3( wire::string(     1 ).as<char>(), ==,    '1' );
    test3( wire::string(    33 ).as<char>(), ==,    '!' );
    test3( wire::string( false ).as<char>(), ==,   '\0' );
    test3( wire::string(  true ).as<char>(), ==, '\x01' );

    //short
    //long

    test3( wire::string(         ).as<int>(), ==,  0 );
    test3( wire::string(   false ).as<int>(), ==,  0 );
    test3( wire::string(    true ).as<int>(), ==,  1 );
    test3( wire::string( "false" ).as<int>(), ==,  0 );
    test3( wire::string(  "true" ).as<int>(), ==,  1 );
    test3( wire::string(       0 ).as<int>(), ==,  0 );
    test3( wire::string(       1 ).as<int>(), ==,  1 );
    test3( wire::string(      -1 ).as<int>(), ==, -1 );

    test3( wire::string(         ).as<unsigned>(), ==,             0 );
    test3( wire::string(   false ).as<unsigned>(), ==,             0 );
    test3( wire::string(    true ).as<unsigned>(), ==,             1 );
    test3( wire::string( "false" ).as<unsigned>(), ==,             0 );
    test3( wire::string(  "true" ).as<unsigned>(), ==,             1 );
    test3( wire::string(      -1 ).as<unsigned>(), ==, (unsigned)(-1) );

    test3( wire::string(         ).as<size_t>(), ==,           0  );
    test3( wire::string(   false ).as<size_t>(), ==,           0  );
    test3( wire::string(    true ).as<size_t>(), ==,           1  );
    test3( wire::string( "false" ).as<size_t>(), ==,           0  );
    test3( wire::string(  "true" ).as<size_t>(), ==,           1  );
    test3( wire::string(      -1 ).as<size_t>(), ==, (size_t)(-1) );

    // add limits float, double
    test3( wire::string(         ).as<float>(), ==, 0.f );
    test3( wire::string(   false ).as<float>(), ==, 0.f );
    test3( wire::string(    true ).as<float>(), ==, 1.f );
    test3( wire::string( "false" ).as<float>(), ==, 0.f );
    test3( wire::string(  "true" ).as<float>(), ==, 1.f );
    test3( wire::string(     3.f ).as<float>(), ==, 3.f );

    test3( wire::string(         ).as<double>(), ==, 0.0 );
    test3( wire::string(   false ).as<double>(), ==, 0.0 );
    test3( wire::string(    true ).as<double>(), ==, 1.0 );
    test3( wire::string( "false" ).as<double>(), ==, 0.f );
    test3( wire::string(  "true" ).as<double>(), ==, 1.f );
    test3( wire::string(     3.0 ).as<double>(), ==, 3.0 );

    //del replacement
    test3( wire::string("%25hello%25%25world%25").replace("%25",""), ==, "helloworld" );
    //same replacement
    test3( wire::string("%25hello%25%25world%25").replace("%25","%25"), ==, "%25hello%25%25world%25" );
    //longer replacement
    test3( wire::string("%25hello%25%25world%25").replace("%25","%255"), ==, "%255hello%255%255world%255" );
    //shorter replacement
    test3( wire::string("%25hello%25%25world%25").replace("%25","%2"), ==, "%2hello%2%2world%2" );

    test3( wire::string().size(), ==, 0 );
    test3( wire::string("").size(), ==, 0 );
    test3( wire::string(), ==, "" );
    test3( wire::string(""), ==, "" );

    // wire::string<<T and std::cout<<wire::string support

/*
    test3( ( wire::string() << false << std::endl ), ==, "false\n" );
    test3( ( wire::string() << '1' << std::endl ), ==, "1\n" );
    test3( ( wire::string() << "2" << std::endl ), ==, "2\n" );
    test3( ( wire::string() << 3 << std::endl ), ==, "3\n" );
    test3( ( wire::string() << 4.f << std::endl ), ==, "4.f\n" );
    test3( ( wire::string() << 5.0 << std::endl ), ==, "5.0\n" );
    test3( ( wire::string() << std::string("6") << std::endl ), ==, "6\n" );
    test3( ( wire::string() << wire::string("7") << std::endl ), ==, "7\n" );
*/

    test3( wire::string("Hi!").at(-6), ==, 'H' );
    test3( wire::string("Hi!").at(-5), ==, 'i' );
    test3( wire::string("Hi!").at(-4), ==, '!' );
    test3( wire::string("Hi!").at(-3), ==, 'H' );
    test3( wire::string("Hi!").at(-2), ==, 'i' );
    test3( wire::string("Hi!").at(-1), ==, '!' );
    test3( wire::string("Hi!").at( 0), ==, 'H' );
    test3( wire::string("Hi!").at( 1), ==, 'i' );
    test3( wire::string("Hi!").at( 2), ==, '!' );
    test3( wire::string("Hi!").at( 3), ==, 'H' );
    test3( wire::string("Hi!").at( 4), ==, 'i' );
    test3( wire::string("Hi!").at( 5), ==, '!' );

    test3( wire::string().at(-1), ==, '\0' );
    test3( wire::string().at( 0), ==, '\0' );
    test3( wire::string().at( 1), ==, '\0' );

    // Other tests
    tests_from_string_sample();

    // End of tests. Show results.
    std::cout << right.str();
    std::cout << std::endl;
    std::cout << wrong.str();
    std::cout << std::endl;

    if( wrong.str().empty() )
        std::cout << "All ok :)" << std::endl;
    else
        std::cout << "Test(s) failed! :(" << std::endl;

    return 0;
}
