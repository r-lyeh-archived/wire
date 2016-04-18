/* Extended C++ standard string classes, string interpolation and casting macros.
 * Copyright (c) 2010-2014, Mario 'rlyeh' Rodriguez, zlib/libpng licensed.

 * wire::format() based on code by Adam Rosenfield (see http://goo.gl/XPnoe)
 * wire::format() based on code by Tom Distler (see http://goo.gl/KPT66)

 * @todo:
 * - string::replace_map(): specialize for target_t == char || replacement_t == char
 * - string::replace_map(): specialize for (typename<size_t N> const char (&from)[N], const char (&to)[N])
 * - strings::subset( 0, EOF )
 * - strings::subset( N, EOF )
 * - strings::subset( 0, -1 ) -> 0, EOF - 1
 * - strings::subset( -2, -1 ) -> EOF-2, EOF-1

 * - rlyeh
 */

#pragma once

#include <cctype>
#include <cstdarg>
#include <cstdio>
#include <cstring>

#include <algorithm>
#include <deque>
#include <iomanip>
#include <iostream>
#include <limits>
#include <map>
#include <sstream>
#include <string>
#include <vector>

#define WIRE_VERSION "2.2.0" /* (2016/04/18) - Moved getopt to a library apart.
#define WIRE_VERSION "2.1.0" // (2015/09/19) - Moved .ini reader/writer to a library apart.
#define WIRE_VERSION "2.0.0" // (2015/08/09) - Moved string interpolator to a library apart; Improved INI reader;
#define WIRE_VERSION "1.0.0" // (2015/06/12) - Removed a few warnings
#define WIRE_VERSION "0.0.0" // (2010/xx/xx) - Initial commit */

#ifdef _MSC_VER
#    define wire$vsnprintf _vsnprintf
#    pragma warning( push )
#    pragma warning( disable : 4996 )
#else
#    define wire$vsnprintf  vsnprintf
#endif

namespace wire
{
    /* Public API */
    // Function tools

    // Function to do safe C-style formatting
    static inline std::string format( const char *fmt, ... ) {
        int len;
        std::string self;
        using namespace std;

        // Calculate the final length of the formatted string
        {
            va_list args;
            va_start( args, fmt );
            len = wire$vsnprintf( 0, 0, fmt, args );
            va_end( args );
        }

        // Allocate a buffer (including room for null termination)
        char* target_string = new char[++len];

        // Generate the formatted string
        {
            va_list args;
            va_start( args, fmt );
            wire$vsnprintf( target_string, len, fmt, args );
            va_end( args );
        }

        // Assign the formatted string
        self.assign( target_string );

        // Clean up
        delete [] target_string;
        return self;
    }

    // Function to convert strings <-> numbers in most precise way (C99)
    static inline std::string precise( const long double &t ) {
        /**/ if( t ==  std::numeric_limits< long double >::infinity() ) return  "INF";
        else if( t == -std::numeric_limits< long double >::infinity() ) return "-INF";
        else if( t != t ) return "NaN";
        // C99 way {
        char buf[ 32 ];
        sprintf(buf, "%La", t);
        return buf;
        // }
    }
    static inline long double precise( const std::string &t ) {
        long double ld;
        sscanf(t.c_str(), "%La", &ld);
        return ld;
    }

    /* Public API */
    // Main class

    namespace
    {
        template< typename T >
        inline T as( const std::string &self ) {
            T t;
            if( std::istringstream(self) >> t )
                return t;
            bool is_true = self.size() && (self != "0") && (self != "false");
            return (T)(is_true);
        }

        template<>
        inline char as( const std::string &self ) {
            return self.size() == 1 ? (char)(self[0]) : (char)(as<int>(self));
        }
        template<>
        inline signed char as( const std::string &self ) {
            return self.size() == 1 ? (signed char)(self[0]) : (signed char)(as<int>(self));
        }
        template<>
        inline unsigned char as( const std::string &self ) {
            return self.size() == 1 ? (unsigned char)(self[0]) : (unsigned char)(as<int>(self));
        }

        template<>
        inline const char *as( const std::string &self ) {
            return self.c_str();
        }
        template<>
        inline std::string as( const std::string &self ) {
            return self;
        }
    }

    class string : public std::string
    {
        public:

        // basic constructors

        string() : std::string()
        {}

/*
        string( const string &x ) : std::string()
        {
            this->assign( x );
        }
        string &operator=( const string &other ){
            if( this != &other ) {
            }
            return *this;
        }

        string( string &&x ) : std::string{std::forward<std::string>(x)}
        {}
        string &operator=( string &&other ){
            if( this != &other ) {
            }
            return *this;
        }
*/

        /*
        string( string &&other ) : std::string()
        {
            *this = std::move(other);
        }
        */

        string( const std::string &s ) : std::string( s )
        {}

        string( const char &c ) : std::string( 1, c )
        {}

        string( const char &c, size_t n ) : std::string( n, c )
        {}

        string( size_t n, const char &c ) : std::string( n, c )
        {}

        string( const char *cstr ) : std::string( cstr ? cstr : "" )
        {}

        string( char * const &cstr ) : std::string( cstr ? cstr : "" )
        {}

        template<size_t N>
        string( const char (&cstr)[N] ) : std::string( cstr )
        {}

        string( const bool &val ) : std::string( val ? "true" : "false" )
        {}

        // constructor sugars

        template< typename T >
        string( const T &t ) : std::string()
        {
            std::stringstream ss;
            if( ss << /* std::boolalpha << */ t )
                this->assign( ss.str() );
        }

        string( const float &t ) : std::string()
        {
            *this = string( (long double)t );
        }

        string( const double &t ) : std::string()
        {
            *this = string( (long double)t );
        }

        string( const long double &t ) : std::string()
        {
            // enum { max_digits = std::numeric_limits<long double>::digits10 + 2 };
            std::stringstream ss;
            if( ss << (long double)(t) )
                this->assign( ss.str() );
        }

        // extended constructors; safe formatting

        private:
        template<unsigned N>
        std::string &formatsafe( const std::string &fmt, std::string (&t)[N] )
        {
            for( const unsigned char &ch : fmt ) {
                if( ch > N ) t[0] += char(ch);
                else t[0] += t[ ch ];
            }
            return t[0];
        }
        public:

        template< typename T1 >
        string( const std::string &fmt, const T1 &t1 ) : std::string()
        {
            std::string t[] = { std::string(), string(t1) };
            assign( formatsafe( fmt, t ) );
        }

        template< typename T1, typename T2 >
        string( const std::string &fmt, const T1 &t1, const T2 &t2 ) : std::string()
        {
            std::string t[] = { std::string(), string(t1), string(t2) };
            assign( formatsafe( fmt, t ) );
        }

        template< typename T1, typename T2, typename T3 >
        string( const std::string &fmt, const T1 &t1, const T2 &t2, const T3 &t3 ) : std::string()
        {
            std::string t[] = { std::string(), string(t1), string(t2), string(t3) };
            assign( formatsafe( fmt, t ) );
        }

        template< typename T1, typename T2, typename T3, typename T4 >
        string( const std::string &fmt, const T1 &t1, const T2 &t2, const T3 &t3, const T4 &t4 ) : std::string()
        {
            std::string t[] = { std::string(), string(t1), string(t2), string(t3), string(t4) };
            assign( formatsafe( fmt, t ) );
        }

        template< typename T1, typename T2, typename T3, typename T4, typename T5 >
        string( const std::string &fmt, const T1 &t1, const T2 &t2, const T3 &t3, const T4 &t4, const T5 &t5 ) : std::string()
        {
            std::string t[] = { std::string(), string(t1), string(t2), string(t3), string(t4), string(t5) };
            assign( formatsafe( fmt, t ) );
        }

        template< typename T1, typename T2, typename T3, typename T4, typename T5, typename T6 >
        string( const std::string &fmt, const T1 &t1, const T2 &t2, const T3 &t3, const T4 &t4, const T5 &t5, const T6 &t6 ) : std::string()
        {
            std::string t[] = { std::string(), string(t1), string(t2), string(t3), string(t4), string(t5), string(t6) };
            assign( formatsafe( fmt, t ) );
        }

        template< typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7 >
        string( const std::string &fmt, const T1 &t1, const T2 &t2, const T3 &t3, const T4 &t4, const T5 &t5, const T6 &t6, const T7 &t7 ) : std::string()
        {
            std::string t[] = { std::string(), string(t1), string(t2), string(t3), string(t4), string(t5), string(t6), string(t7) };
            assign( formatsafe( fmt, t ) );
        }

        wire::string &operator()() {
            return *this;
        }

        template< typename T1 >
        wire::string &operator()( const T1 &t1 ) {
            return assign( string( *this, t1 ) ), *this;
        }

        template< typename T1, typename T2 >
        wire::string &operator()( const T1 &t1, const T2 &t2 ) {
            return assign( string( *this, t1, t2 ) ), *this;
        }

        template< typename T1, typename T2, typename T3 >
        wire::string &operator()( const T1 &t1, const T2 &t2, const T3 &t3 ) {
            return assign( string( *this, t1, t2, t3 ) ), *this;
        }

        template< typename T1, typename T2, typename T3, typename T4 >
        wire::string &operator()( const T1 &t1, const T2 &t2, const T3 &t3, const T4 &t4 ) {
            return assign( string( *this, t1, t2, t3, t4 ) ), *this;
        }

        template< typename T1, typename T2, typename T3, typename T4, typename T5 >
        wire::string &operator()( const T1 &t1, const T2 &t2, const T3 &t3, const T4 &t4, const T5 &t5 ) {
            return assign( string( *this, t1, t2, t3, t4, t5 ) ), *this;
        }

        template< typename T1, typename T2, typename T3, typename T4, typename T5, typename T6 >
        wire::string &operator()( const T1 &t1, const T2 &t2, const T3 &t3, const T4 &t4, const T5 &t5, const T6 &t6 ) {
            return assign( string( *this, t1, t2, t3, t4, t5, t6 ) ), *this;
        }

        template< typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7 >
        wire::string &operator()( const T1 &t1, const T2 &t2, const T3 &t3, const T4 &t4, const T5 &t5, const T6 &t6, const T7 &t7 ) {
            return assign( string( *this, t1, t2, t3, t4, t5, t6, t7 ) ), *this;
        }

        // conversion

        template< typename T >
        T as() const
        {
            return wire::as<T>(*this);
        }

        template< typename T >
        operator T() const
        {
            return wire::as<T>(*this);
        }

        // chaining operators

        template <typename T>
        string &operator <<( const T &t )
        {
            //*this = *this + string(t);
            this->append( string(t) );
            return *this;
        }

        string &operator <<( std::ostream &( *pf )(std::ostream &) )
        {
            return *pf == static_cast<std::ostream& ( * )(std::ostream&)>( std::endl ) ? (*this) += "\n", *this : *this;
        }

        template< typename T >
        string &operator +=( const T &t )
        {
            return operator<<(t);
        }

        string &operator +=( std::ostream &( *pf )(std::ostream &) )
        {
            return operator<<(pf);
        }

        // assignment sugars

        template< typename T >
        string &operator=( const T &t )
        {
            this->assign( string(t) );
            return *this;
        }

        // comparison sugars
/*
        operator const bool() const
        {
            return wire::as<bool>(*this);
        }
*/
        template<typename T>
        bool operator ==( const T &t ) const
        {
            return wire::as<T>(*this) == wire::string(t).as<T>();
        }
        bool operator ==( const wire::string &t ) const
        {
            return this->compare( t ) == 0;
        }
        bool operator ==( const char *t ) const
        {
            return this->compare( t ) == 0;
        }

        // extra methods

        // at() classic behaviour: "hello"[5] = undefined, "hello"[-1] = undefined
        // at() extended behaviour: "hello"[5] = h, "hello"[-1] = o,

        const char &at( const int &pos ) const
        {
            signed size = (signed)(this->size());
            if( size )
                return this->std::string::at( pos >= 0 ? pos % size : size - 1 + ((pos+1) % size) );
            static std::map< const string *, char > map;
            return ( ( map[ this ] = map[ this ] ) = '\0' );
        }

        char &at( const int &pos )
        {
            signed size = (signed)(this->size());
            if( size )
                return this->std::string::at( pos >= 0 ? pos % size : size - 1 + ((pos+1) % size) );
            static std::map< const string *, char > map;
            return ( ( map[ this ] = map[ this ] ) = '\0' );
        }

        const char &operator[]( const int &pos ) const {
            return this->at(pos);
        }
        char &operator[]( const int &pos ) {
            return this->at(pos);
        }

        void pop_back()
        {
            if( this->size() )
                this->erase( this->end() - 1 );
        }

        void pop_front()
        {
            if( this->size() )
                this->erase( 0, 1 ); //this->substr( 1 ); //this->assign( this->begin() + 1, this->end() );
        }

        template<typename T>
        void push_back( const T& t ) {
            //std::swap( *this, *this + string(t) );
            *this = *this + string(t);
        }

        template<typename T>
        void push_front( const T& t ) {
            //std::swap( *this, string(t) + *this );
            *this = string(t) + *this;
        }

        const char &back() const
        {
            return at(-1);
        }

        char &back()
        {
            return at(-1);
        }

        const char &front() const
        {
            return at(0);
        }

        char &front()
        {
            return at(0);
        }

        // tools

        std::string str( const std::string &pre = std::string(), const std::string &post = std::string() ) const
        {
            return pre + *this + post;
        }

        string uppercase() const
        {
            std::string s = *this;

            std::transform( s.begin(), s.end(), s.begin(), (int(*)(int)) std::toupper );

            return s;
        }

        string lowercase() const
        {
            std::string s = *this;

            std::transform( s.begin(), s.end(), s.begin(), (int(*)(int)) std::tolower );

            return s;
        }

        bool matches( const std::string &pattern ) const
        {
            struct local {
                static bool match( const char *pattern, const char *str ) {
                    if( *pattern=='\0' ) return !*str;
                    if( *pattern=='*' )  return match(pattern+1, str) || (*str && match(pattern, str+1));
                    if( *pattern=='?' )  return *str && (*str != '.') && match(pattern+1, str+1);
                    return (*str == *pattern) && match(pattern+1, str+1);
                }
            };

            return local::match( pattern.c_str(), (*this).c_str() );
        }

        bool matchesi( const std::string &pattern ) const
        {
            return this->uppercase().matches( string(pattern).uppercase() );
        }

        size_t count( const std::string &substr ) const
        {
            size_t n = 0;
            std::string::size_type pos = 0;
            while( (pos = this->find( substr, pos )) != std::string::npos ) {
                n++;
                pos += substr.size();
            }
            return n;
        }

        string left_of( const std::string &substring ) const
        {
            string::size_type pos = this->find( substring );
            return pos == std::string::npos ? *this : (string)this->substr(0, pos);
        }

        string right_of( const std::string &substring ) const
        {
            std::string::size_type pos = this->find( substring );
            return pos == std::string::npos ? *this : (string)this->substr(pos + 1);
        }

        string replace1( const std::string &target, const std::string &replacement ) const {
            std::string str = *this;
            auto found = str.find(target);
            return found == string::npos ? str : (str.replace(found, target.length(), replacement), str);
        }

        string replace( const std::string &target, const std::string &replacement ) const
        {
            size_t found = 0;
            std::string s = *this;

            while( ( found = s.find( target, found ) ) != string::npos )
            {
                s.replace( found, target.length(), replacement );
                found += replacement.length();
            }

            return s;
        }

        string replace_map( const std::map< std::string, std::string > &replacements ) const
        {
            string out;

            for( size_t i = 0; i < this->size(); )
            {
                bool found = false;
                size_t match_length = 0;

                std::map< std::string, std::string >::const_reverse_iterator it;
                for( it = replacements.rbegin(); !found && it != replacements.rend(); ++it )
                {
                    const std::string &target = it->first;
                    const std::string &replacement = it->second;

                    if( match_length != target.size() )
                        match_length = target.size();

                    if( this->size() - i >= target.size() )
                    if( !std::memcmp( &this->at(int(i)), &target.at(0), (int)match_length ) )
                    {
                        i += target.size();

                        out += replacement;

                        found = true;
                    }
                }

                if( !found )
                    out += this->at(int(i++));
            }

           return out;
        }

        private:

        string strip( const std::string &chars, bool strip_left, bool strip_right ) const
        {
            std::string::size_type len = this->size(), i = 0, j = len, charslen = chars.size();

            if( charslen == 0 )
            {
                if( strip_left )
                    while( i < len && std::isspace( this->operator[]( int(i) ) ))
                        i++;

                if( strip_right && j ) {
                    do j--; while( j >= i && std::isspace( this->operator[]( int(j) ) ));
                    j++;
                }
            }
            else
            {
                const char *sep = chars.c_str();

                if( strip_left )
                    while( i < len && std::memchr( sep, this->operator[]( int(i) ), charslen ))
                        i++;

                if( strip_right && j ) {
                    do j--; while( j >= i && std::memchr( sep, this->operator[]( int(j) ), charslen ));
                    j++;
                }
            }

            if( j - i == len ) return *this;
            return this->substr( i, j - i );
        }

        public: // based on python string and pystring

        // Return a copy of the string with leading characters removed (default chars: space)
        string lstrip( const std::string &chars = std::string() ) const
        {
            return strip( chars, true, false );
        }
        string ltrim( const std::string &chars = std::string() ) const
        {
            return strip( chars, true, false );
        }

        // Return a copy of the string with trailing characters removed (default chars: space)
        string rstrip( const std::string &chars = std::string() ) const
        {
            return strip( chars, false, true );
        }
        string rtrim( const std::string &chars = std::string() ) const
        {
            return strip( chars, false, true );
        }

        // Return a copy of the string with both leading and trailing characters removed (default chars: space)
        string strip( const std::string &chars = std::string() ) const
        {
            return strip( chars, true, true );
        }
        string trim( const std::string &chars = std::string() ) const
        {
            return strip( chars, true, true );
        }

        bool starts_with( const std::string &prefix ) const
        {
            return this->size() >= prefix.size() ? this->substr( 0, prefix.size() ) == prefix : false;
        }

        bool starts_withi( const std::string &prefix ) const
        {
            return this->uppercase().starts_with( string(prefix).uppercase() );
        }

        bool ends_with( const std::string &suffix ) const
        {
            return this->size() < suffix.size() ? false : this->substr( this->size() - suffix.size() ) == suffix;
        }

        bool ends_withi( const std::string &suffix ) const
        {
            return this->uppercase().ends_with( string(suffix).uppercase() );
        }

        std::deque< string > tokenize( const std::string &delimiters ) const {
            std::string map( 256, '\0' );
            for( const unsigned char &ch : delimiters )
                map[ ch ] = '\1';
            std::deque< string > tokens(1);
            for( const unsigned char &ch : *this ) {
                /**/ if( !map.at(ch)          ) tokens.back().push_back( char(ch) );
                else if( tokens.back().size() ) tokens.push_back( string() );
            }
            while( tokens.size() && !tokens.back().size() ) tokens.pop_back();
            return tokens;
        }

        // tokenize_incl_separators
        std::deque< string > split( const std::string &delimiters ) const {
            std::string str;
            std::deque< string > tokens;
            for( auto &ch : *this ) {
                if( delimiters.find_first_of( ch ) != std::string::npos ) {
                    if( str.size() ) tokens.push_back( str ), str = "";
                    tokens.push_back( std::string() + ch );
                } else str += ch;
            }
            return str.empty() ? tokens : ( tokens.push_back( str ), tokens );
        }
    };

    class strings : public std::deque< string >
    {
        public:

        strings() : std::deque< string >()
        {}

        strings( const int &argc, const char **&argv ) : std::deque< string >()
        {
            for( int i = 0; i < argc; ++i )
                this->push_back( argv[i] );
        }

        strings( const int &argc, char **&argv ) : std::deque< string >()
        {
            for( int i = 0; i < argc; ++i )
                this->push_back( argv[i] );
        }

        template< typename T, const size_t N >
        strings( const T (&args)[N] ) : std::deque< string >()
        {
            this->resize( N );
            for( int n = 0; n < N; ++n )
                (*this)[ n ] = args[ n ];
        }

        template <typename CONTAINER>
        strings( const CONTAINER &other ) : std::deque< string >( other.begin(), other.end() )
        {}

        template <typename CONTAINER>
        strings &operator =( const CONTAINER &other ) {
            if( &other != this ) {
                *this = strings( other );
            }
            return *this;
        }

        template< typename T > strings( const T &t0, const T &t1 ) : std::deque< string >()
        { this->resize(2); (*this)[0] = t0; (*this)[1] = t1; }
        template< typename T > strings( const T &t0, const T &t1, const T &t2 ) : std::deque< string >()
        { this->resize(3); (*this)[0] = t0; (*this)[1] = t1; (*this)[2] = t2; }
        template< typename T > strings( const T &t0, const T &t1, const T &t2, const T &t3 ) : std::deque< string >()
        { this->resize(4); (*this)[0] = t0; (*this)[1] = t1; (*this)[2] = t2; (*this)[3] = t3; }
        template< typename T > strings( const T &t0, const T &t1, const T &t2, const T &t3, const T &t4 ) : std::deque< string >()
        { this->resize(5); (*this)[0] = t0; (*this)[1] = t1; (*this)[2] = t2; (*this)[3] = t3; (*this)[4] = t4; }
        template< typename T > strings( const T &t0, const T &t1, const T &t2, const T &t3, const T &t4, const T &t5 ) : std::deque< string >()
        { this->resize(6); (*this)[0] = t0; (*this)[1] = t1; (*this)[2] = t2; (*this)[3] = t3; (*this)[4] = t4; (*this)[5] = t5; }
        template< typename T > strings( const T &t0, const T &t1, const T &t2, const T &t3, const T &t4, const T &t5, const T &t6 ) : std::deque< string >()
        { this->resize(7); (*this)[0] = t0; (*this)[1] = t1; (*this)[2] = t2; (*this)[3] = t3; (*this)[4] = t4; (*this)[5] = t5; (*this)[6] = t6; }

        const string &at( const int &pos ) const
        {
            signed size = signed(this->size());
            if( size )
                return *( this->begin() + ( pos >= 0 ? pos % size : size - 1 + ((pos+1) % size) ) );
            static std::map< const strings *, string > map;
            return ( ( map[ this ] = map[ this ] ) = string() );
        }

        string &at( const int &pos )
        {
            signed size = signed(this->size());
            if( size )
                return *( this->begin() + ( pos >= 0 ? pos % size : size - 1 + ((pos+1) % size) ) );
            static std::map< const strings *, string > map;
            return ( ( map[ this ] = map[ this ] ) = string() );
        }

        const string &operator[]( const int &pos ) const {
            return at(pos);
        }
        string &operator[]( const int &pos ) {
            return at(pos);
        }

        std::string str( const char *format1 = "\1\n", const std::string &pre = std::string(), const std::string &post = std::string() ) const
        {
            if( this->size() == 1 )
                return pre + *this->begin() + post;

            std::string out( pre );

            for( const_iterator it = this->begin(); it != this->end(); ++it )
                out += string( format1, (*it) );

            return out + post;
        }

        inline friend std::ostream &operator <<( std::ostream &os, const wire::strings &self ) {
            return os << self.str(), os;
        }
    };
}

// Generic print containers

namespace wire
{
    template<typename T>
    inline std::string str( const T& t, const std::string &format1, const std::string &pre = std::string(), const std::string &post = std::string() )
    {
        wire::string out;

        for( typename T::const_iterator it = t.begin(), end = t.end(); it != end; ++it )
            out << wire::string( format1, *it );

        return wire::string() << pre << out << post;
    }

    template<typename T>
    inline std::string str1( const T& t, const std::string &format1, const std::string &pre = std::string(), const std::string &post = std::string() )
    {
        wire::string out;

        for( typename T::const_iterator it = t.begin(), end = t.end(); it != end; ++it )
            out << wire::string( format1, it->first );

        return wire::string() << pre << out << post;
    }

    template<typename T>
    inline std::string str2( const T& t, const std::string &format1, const std::string &pre = std::string(), const std::string &post = std::string() )
    {
        wire::string out;

        for( typename T::const_iterator it = t.begin(), end = t.end(); it != end; ++it )
            out << wire::string( format1, it->second );

        return wire::string() << pre << out << post;
    }

    template<typename T>
    inline std::string str12( const T& t, const std::string &format12, const std::string &pre = std::string(), const std::string &post = std::string() )
    {
        wire::string out;

        for( typename T::const_iterator it = t.begin(), end = t.end(); it != end; ++it )
            out << wire::string( format12, it->first, it->second );

        return wire::string() << pre << out << post;
    }
}

// $wire(), introspective macro

namespace wire
{
    // @todo: eq+sep+line is a c++11 constexpr
    struct parser : public wire::string {
        parser( const wire::string &fmt, const wire::string &line = std::string() ) {
            wire::strings all = line.tokenize(", \r\n\t");
            wire::strings::iterator it, begin, end;

            typedef std::pair<std::string,std::string> pair;
            std::vector< pair > results;

            for( it = begin = all.begin(), end = all.end(); it != end; ++it ) {
                results.push_back( pair( (*it).right_of(".").right_of("->"), std::string() + char(it - begin + '\1') ) );
            }

            assign( str12(results, fmt) );
        }
    };
}

#define $wire(FMT,...) wire::parser(FMT,#__VA_ARGS__)(__VA_ARGS__)

#ifdef _MSC_VER
#    pragma warning( pop )
#endif
#undef wire$vsnprintf
