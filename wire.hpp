/*
 * Extended C++ standard string classes, string interpolation and casting macros.
 * Copyright (c) 2010-2013, Mario 'rlyeh' Rodriguez

 * wire::eval() based on code by Peter Kankowski (see http://goo.gl/Kx6Oi)
 * wire::format() based on code by Adam Rosenfield (see http://goo.gl/XPnoe)
 * wire::format() based on code by Tom Distler (see http://goo.gl/KPT66)

 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.

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

#include <cstdarg>
#include <cstring>

#include <algorithm>
#include <cctype>
#include <deque>
#include <iomanip>
#include <limits>
#include <map>
#include <sstream>
#include <string>
#include <vector>

namespace wire
{
    /* Public API */
    // Function tools

    // Function to do safe C-style formatting
    std::string format( const char *fmt, ... );

    // Function to save numbers in most precise way
    namespace {
    template<typename T>
    inline std::string precise( const T &t ) {
        /**/ if( t ==  std::numeric_limits< T >::infinity() ) return  "INF";
        else if( t == -std::numeric_limits< T >::infinity() ) return "-INF";
        else if( t != t ) return "NaN";
        // C++11 way to do <<std::fixed<<std::scientific: <<std::hexfloat
        std::stringstream ss;
        ss  << std::fixed
          /*<< std::scientific*/
            << std::setprecision( std::numeric_limits< T >::digits10 + 1 )
            << t;
        return ss.str();
    } }

    // Function to evaluate simple numeric expressions
    double eval( const std::string &expression );

    /* Public API */
    // Main class

    namespace
    {
        template< typename T >
        inline T as( const std::string &self ) { const std::string *that = &self;
            T t;
            if( !( std::istringstream(*that) >> t ) ) {
                bool is_true = that->size() && (*that) != "0" && (*that) != "false";
                t = (T)(is_true);
            }
            return t;
        }

        template<>
        inline char as( const std::string &self ) { const std::string *that = &self;
            if( that->size() == 1 )
                return (char)(that->operator[](0));
            int t;
            if( !( std::istringstream(*that) >> t ) ) {
                bool is_true = that->size() && (*that) != "0" && (*that) != "false";
                t = (int)(is_true);
            }
            return (char)(t);
        }
        template<>
        inline signed char as( const std::string &self ) { const std::string *that = &self;
            if( that->size() == 1 )
                return (signed char)(that->operator[](0));
            int t;
            if( !( std::istringstream(*that) >> t ) ) {
                bool is_true = that->size() && (*that) != "0" && (*that) != "false";
                t = (int)(is_true);
            }
            return (signed char)(t);
        }
        template<>
        inline unsigned char as( const std::string &self ) { const std::string *that = &self;
            if( that->size() == 1 )
                return (unsigned char)(that->operator[](0));
            int t;
            if( !( std::istringstream(*that) >> t ) ) {
                bool is_true = that->size() && (*that) != "0" && (*that) != "false";
                t = (int)(is_true);
            }
            return (unsigned char)(t);
        }

        template<>
        inline const char *as( const std::string &self ) { const std::string *that = &self;
            return that->c_str();
        }
        template<>
        inline std::string as( const std::string &self ) { const std::string *that = &self;
            return *that;
        }
    }

    class string : public std::string
    {
        public:

        // basic constructors

        string() : std::string()
        {}

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
            std::stringstream ss;
            if( ss << (long double)(t) )
                this->assign( ss.str() );
        }

        string( const double &t ) : std::string()
        {
            std::stringstream ss;
            if( ss << (long double)(t) )
                this->assign( ss.str() );
        }

        string( const long double &t ) : std::string()
        {
            std::stringstream ss;
            if( ss << (long double)(t) )
                this->assign( ss.str() );
        }

        // extended constructors; safe formatting

        private:
        template<unsigned N>
        std::string &formatsafe( const std::string &fmt, std::string (&t)[N] )
        {
            for( std::string::const_iterator it = fmt.begin(), end = fmt.end(); it != end; ++it ) {
                const char &in = *it;
                if( unsigned(in) > N ) t[ 0 ] += in;
                else t[ 0 ] += t[ unsigned(in) ];
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
            return wire::as<T>(*this) == t;
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
                return this->data()[ pos >= 0 ? pos % size : size - 1 + ((pos+1) % size) ];
            static std::map< const string *, char > map;
            return ( ( map[ this ] = map[ this ] ) = '\0' );
        }

        char &at( const int &pos )
        {
            signed size = (signed)(this->size());
            if( size )
                return const_cast<char &>( this->data()[ pos >= 0 ? pos % size : size - 1 + ((pos+1) % size) ] );
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

        bool matches( const std::string &pattern ) const
        {
            struct local {
                static bool match( const char *pattern, const char *str ) {
                    if( *pattern=='\0' ) return !*str;
                    if( *pattern=='*' )  return match(pattern+1, str) || *str && match(pattern, str+1);
                    if( *pattern=='?' )  return *str && (*str != '.') && match(pattern+1, str+1);
                    return (*str == *pattern) && match(pattern+1, str+1);
                }
            };

            return local::match( pattern.c_str(), (*this).c_str() );
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
                    if( !std::memcmp( &this->at(i), &target.at(0), match_length ) )
                    {
                        i += target.size();

                        out += replacement;

                        found = true;
                    }
                }

                if( !found )
                    out += this->at(i++);
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
                    while( i < len && std::isspace( this->operator[]( i ) ))
                        i++;

                if( strip_right && j ) {
                    do j--; while( j >= i && std::isspace( this->operator[]( j ) ));
                    j++;
                }
            }
            else
            {
                const char *sep = chars.c_str();

                if( strip_left )
                    while( i < len && std::memchr( sep, this->operator[]( i ), charslen ))
                        i++;

                if( strip_right && j ) {
                    do j--; while( j >= i && std::memchr( sep, this->operator[]( j ), charslen ));
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

        bool starts_with( const std::string &prefix, bool is_case_sensitive ) const
        {
            return is_case_sensitive ? starts_with( prefix ) : this->uppercase().starts_with( string(prefix).uppercase() );
        }

        bool ends_with( const std::string &suffix ) const
        {
            return this->size() < suffix.size() ? false : this->substr( this->size() - suffix.size() ) == suffix;
        }

        bool ends_with( const std::string &suffix, bool is_case_sensitive ) const
        {
            return is_case_sensitive ? ends_with( suffix ) : this->uppercase().ends_with( string(suffix).uppercase() );
        }

        std::deque< string > tokenize( const std::string &chars ) const
        {
            std::string map( 256, '\0' );

            for( std::string::const_iterator it = chars.begin(), end = chars.end(); it != end; ++it )
                map[ *it ] = '\1';

            std::deque< string > tokens;

            tokens.push_back( string() );

            for( int i = 0, end = this->size(); i < end; ++i )
            {
                unsigned char c = at(i);

                std::string &str = tokens.back();

                if( !map.at(c) )
                    str.push_back( c );
                else
                if( str.size() )
                    tokens.push_back( string() );
            }

            while( tokens.size() && !tokens.back().size() )
                tokens.pop_back();

            return tokens;
        }

        // tokenize_incl_separators
        std::deque< string > split( const std::string& delimiters ) const
        {
            std::deque< string > tokens;
            std::string str;

            for( size_t i = 0; i < this->size(); ++i )
            {
                char c = this->operator[](i);

                if( delimiters.find_first_of( c ) == std::string::npos )
                {
                    str += c;
                }
                else
                {
                    if( str.size() )
                        tokens.push_back( str );

                    tokens.push_back( c );

                    str = "";
                }
            }

            tokens.push_back( str );
            return tokens;
        }
    };

    class strings : public std::deque< string >
    {
        public:

        strings()
        {}

        template <typename contained>
        strings( const std::deque<contained> &c )
        {
            operator=( c );
        }

        template <typename contained>
        strings &operator =( const std::deque<contained> &c )
        {
            this->resize( c.size() );

            std::copy( c.begin(), c.end(), this->begin() );

            return *this;
        }

        template <typename contained>
        strings( const std::vector<contained> &c )
        {
            operator=( c );
        }

        template <typename contained>
        strings &operator =( const std::vector<contained> &c )
        {
            this->resize( c.size() );

            std::copy( c.begin(), c.end(), this->begin() );

            return *this;
        }

        strings( const int &argc, const char **&argv )
        {
            for( int i = 0; i < argc; ++i )
                this->push_back( argv[i] );
        }

        strings( const int &argc, char **&argv )
        {
            for( int i = 0; i < argc; ++i )
                this->push_back( argv[i] );
        }

        template< typename T, const size_t N >
        strings( const T (&args)[N] )
        {
            this->resize( N );
            for( int n = 0; n < N; ++n )
                (*this)[ n ] = args[ n ];
        }

        template< typename T > strings( const T &t0, const T &t1 )
        { this->resize(2); (*this)[0] = t0; (*this)[1] = t1; }
        template< typename T > strings( const T &t0, const T &t1, const T &t2 )
        { this->resize(3); (*this)[0] = t0; (*this)[1] = t1; (*this)[2] = t2; }
        template< typename T > strings( const T &t0, const T &t1, const T &t2, const T &t3 )
        { this->resize(4); (*this)[0] = t0; (*this)[1] = t1; (*this)[2] = t2; (*this)[3] = t3; }
        template< typename T > strings( const T &t0, const T &t1, const T &t2, const T &t3, const T &t4 )
        { this->resize(5); (*this)[0] = t0; (*this)[1] = t1; (*this)[2] = t2; (*this)[3] = t3; (*this)[4] = t4; }
        template< typename T > strings( const T &t0, const T &t1, const T &t2, const T &t3, const T &t4, const T &t5 )
        { this->resize(6); (*this)[0] = t0; (*this)[1] = t1; (*this)[2] = t2; (*this)[3] = t3; (*this)[4] = t4; (*this)[5] = t5; }
        template< typename T > strings( const T &t0, const T &t1, const T &t2, const T &t3, const T &t4, const T &t5, const T &t6 )
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

        operator std::deque<std::string>() const //faster way to do this? reinterpret_cast?
        {
            std::deque<std::string> out( this->size() );
            std::copy( this->begin(), this->end(), out.begin() );
            return out;
        }

        operator std::vector<std::string>() const //faster way to do this? reinterpret_cast?
        {
            std::vector<std::string> out( this->size() );
            std::copy( this->begin(), this->end(), out.begin() );
            return out;
        }

        std::string str( const char *format1 = "\1\n", const std::string &pre = std::string(), const std::string &post = std::string() ) const
        {
            if( this->size() == 1 )
                return *this->begin();

            std::string out( pre );

            for( const_iterator it = this->begin(); it != this->end(); ++it )
                out += string( format1, (*it) );

            return out + post;
        }
    };
}

#include <iostream>

std::ostream &operator <<( std::ostream &os, const wire::strings &s );

// String interpolation and string casting macros. MIT licensed.

namespace wire
{
    // public api, define/update

#   define $(a)         wire::locate( "$" #a )

    // public api, translate

#   define $$(a)        wire::translate( a )

    // public api, cast and sugars

#   define $cast(a,b)   $( a ).as<b>()

#   define $string(a)   $( a )               // no need to cast
#   define $bool(a)     $cast( a, bool     )
#   define $char(a)     $cast( a, char     )
#   define $int(a)      $cast( a, int      )
#   define $float(a)    $cast( a, float    )
#   define $double(a)   $cast( a, double   )
#   define $size_t(a)   $cast( a, size_t   )
#   define $unsigned(a) $cast( a, unsigned )

    // private details

    std::vector< std::string > extract( const wire::string &dollartext, char sep0 = '$', char sep1 = '\0' );
    wire::string translate( const wire::string &dollartext, const wire::string &recursive_parent = std::string() );
    wire::string &locate( const wire::string &text );
}

//

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
