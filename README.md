Wire <a href="https://travis-ci.org/r-lyeh/wire"><img src="https://api.travis-ci.org/r-lyeh/wire.svg?branch=master" align="right" /></a>
====

- Wire is an extended C++ std::string drop-in replacement (C++11).
- Wire features all std::string basics plus new extended functionality.
- Wire features both C and C++ style safe formatters.
- Wire is cross-platform, header-only, no dependencies.
- Wire is zlib/libpng licensed. Binaries do not require license attribution.

### wire::string()
Extended ```std::string``` replacement.

```c++

/* extended constructors */
wire::string ch( 'h' );                                     // -> "h"
wire::string h1( wire::string( 3, 'h' ) << "abc" );         // -> "hhhabc"
wire::string h2 = wire::string( 'h', 3 ) << std::endl;      // -> "hhh\n"
wire::string minusone = -1;                                 // -> "-1"
wire::string zero = 0;                                      // -> "0"
wire::string boolean = false;                               // -> "false"
wire::string real = 3.1415926535897932384626433832795L;     // ~-> "3.14159"

/* extended c++ format safe constructors */
wire::string arg1( "abc \1", "hi" );
// -> "abc hi"
wire::string arg2( "abc \2 \1", "hi", true );
// -> "abc true hi"
wire::string arg3( "abc \1\2\3", '{', "hi", (unsigned char)('}') );
// -> "abc {hi}"
wire::string arg4( "abc \1 \2 \3 \4 \5 \6 \7", "hi", 3.14159f, 3.14159L, false, '\x1', arg3, 0 );
// -> "abc hi 3.14159 3.14159 false \x1 abc {hi} 0"

/* extended methods */
// .at()/[] classic behaviour: "hello"[5] = undefined, "hello"[-1] = undefined
// .at()/[] extended behaviour: "hello"[5] = h, "hello"[-1] = o,
// .at()/[] never throw, return internal reference to char '\0' instead
// .front() is alias for .at(0), .back() is alias for .at(-1)

hi.at( 0) == 'h';
hi.at( 1) == 'i';
hi.at( 2) == 'h'; // ...
hi.at(-1) == 'i';
hi.at(-2) == 'h';
hi.at(-3) == 'i'; // ...
hi.at( 5) = '!';  // hi == 'h!'

// template<typename T> void push_back( const T& t )
// template<typename T> void push_front( const T& t )

hello.push_back('!');   // hello == "hello!"
hello.push_back(404);   // hello == "hello!404"
hello.push_back(hello); // hello == "hello!404hello!404"

/* extended false comparison operator */
wire::string() == false;
wire::string(0) == false;
wire::string("false") == false;
// note: any other combination is true

/* new, chaining << += */
wire::string chain;
chain << "hello world: " << 3 << 'a' << -1 << std::endl;    // -> "hello world: 3a-1\n"
chain += 123;                                               // -> "hello world: 3a-1\n123"

/* new, implicit type conversion */
wire::string number = 123.f; // number == "123"
wire::string boolean = true; // boolean == "true"

/* new, implicit return type conversion */
std::string boolean = wire::string(false);          // boolean == "false"
bool f = wire::string("false");                     // f == false
int i = wire::string("123");                        // i == 123
int j = wire::string(123);                          // j == 123

/* new, explicit return type conversion */
bool t = wire::string(100).as<bool>();              // t == true
int k = wire::string(-456.123).as<int>();           // k == -456

/* new, quick api review */
hello.str() == "hello";
hello.str( "1", "2" ) == "1hello2";
hello.matches("hel*") == true;
Hello.matchesi("hel*") == true;
hello.uppercase() == "HELLO";
Hello.lowercase() == "hello";
hellohello.count("he") == 2;
hello123.left_of("123") == "hello";
hello123.right_of("hello") == "123";
hello.starts_with("he") == true;
Hello.starts_withi("he") == true;
hello.ends_with("lo") == true;
HELLO.ends_withi("lo") == true;
mammy.replace("m", "d") == "daddy";   // see also .replace_map()
aabc.lstrip('a') == "bc";             // ltrim() alias too
abcc.rstrip('c') == "ab";             // rtrim() alias too
aabacaa.strip('a') == "bac";          // trim() alias too
a_b_c_d_e.tokenize("_") == vector<string>({"a","b","c","d","e"});
a_b_c_d_e.split("_") == vector<string>({"a","_","b","_","c","_","d","_","e"});

/* new, operator() */
string("\1\2\3")("hello", "world", 12) == "helloworld12";
```

### wire::strings()
Extended ```deque<wire::string>``` replacement

@todocument
```
strings( const std::deque<T> &c )
strings( const std::vector<T> &c )

strings( const int &argc, const char **&argv )
strings( const int &argc, char **&argv )

strings( const T (&args)[N] )

template<> strings( const T &t0, const T &t1 )
template<> strings( const T &t0, const T &t1, const T &t2 )
template<> strings( const T &t0, const T &t1, const T &t2, const T &t3 )
template<> strings( const T &t0, const T &t1, const T &t2, const T &t3, const T &t4 )
template<> strings( const T &t0, const T &t1, const T &t2, const T &t3, const T &t4, const T &t5 )
template<> strings( const T &t0, const T &t1, const T &t2, const T &t3, const T &t4, const T &t5, const T &t6 )

const string &at( pos )
string &at( pos )

operator std::deque<std::string>() const
operator std::vector<std::string>() const

std::string str( fmt1 = "\1\n", pre = string(), post = std::string() ) const
```

### wire::format()
Safe C format

```c++
std::string fmt = wire::format("hello %s %3d\n", "world", 123);
// fmt == "hello world 123\n"
```

### $wire()
Quick introspection echo macro

```c++
int health = 100;
float money = 123.25;
const char *hello = "world!";
std::string echo = $wire("\1=\2,", health,money,hello);
// echo == "health=100,money=123.25,hello=world!,"
```

### wire::str()
Generic formatters

@todocument
```
std::string wire::str( const T&, fmt1, pre = string(), post = string() )
std::string wire::str1( const T&, fmt1, pre = string(), post = string() )
std::string wire::str2( const T&, fmt1, pre = string(), post = string() )
std::string wire::str12( const T&, fmt12, pre = string(), post = string() )
```

### wire::getopt()
Arguments parser

@todocument

### Changelog
- v2.1.0 (2015/09/19)
  - Moved .ini reader/writer to a library apart.
- v2.0.0 (2015/08/09)
  - Moved string interpolator to a library apart.
- v1.0.0 (2015/06/12)
  - Removed a few warnings
- v0.0.0 (2010/xx/xx)
  - Initial commit
