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

 * - rlyeh
 */

#include <stdio.h>

#include <iostream>
#include <vector>
#include <map>
#include <string>

#include "wire.hpp"

namespace wire
{
#   ifdef _MSC_VER
#       define wire$vsnprintf _vsnprintf
#       pragma warning( push )
#       pragma warning( disable : 4996 )
#   else
#       define wire$vsnprintf  vsnprintf
#   endif

    // following functions are used by C safe formatter

    std::string format( const char *fmt, ... )
    {
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

#   ifdef _MSC_VER
#       pragma warning( pop )
#   endif
#   undef wire$vsnprintf
}

namespace wire
{
    // following functions are used by $() and $$() macros

    namespace
    {
        std::map< std::string, wire::string > map;
    }

    wire::string &locate( const wire::string &text )
    {
        return ( map[ text ] = map[ text ] );
    }

    wire::string translate( const wire::string &dollartext, const wire::string &parent )
    {
        wire::string out, id;

        for( wire::string::const_iterator in = dollartext.begin(), end = dollartext.end(); in != end; ++in )
        {
            const char &it = *in;

            if( id.size() )
            {
                /**/ if( unsigned( it - 'a' ) <= 'z' - 'a' ) id += it;
                else if( unsigned( it - 'A' ) <= 'Z' - 'A' ) id += it;
                else if( unsigned( it - '0' ) <= '9' - '0' ) id += it;
                else if( it == '-' || it == '_' )            id += it;
                else {
                    if( map.find(id) != map.end() && id != parent )
                        out << translate(map.find(id)->second, id) << it;
                    else
                        out << id << it;

                    id = std::string();
                }
            }
            else
            {
                if( it == '$' )
                    id += it;
                else
                    out << it;
            }
        }

        if( id.size() )
        {
            if( map.find(id) != map.end() && id != parent )
                out << translate(map.find(id)->second, id);
            else
                out << id;
        }

        return out;
    }

    std::vector< std::string > extract( const wire::string &dollartext, char sep0, char sep1 )
    {
        std::string id;
        std::vector< std::string > out;

        for( wire::string::const_iterator in = dollartext.begin(), end = dollartext.end(); in != end; ++in )
        {
            const char &it = *in;

            if( id.size() )
            {
                /**/ if( unsigned( it - 'a' ) <= 'z' - 'a' ) id += it;
                else if( unsigned( it - 'A' ) <= 'Z' - 'A' ) id += it;
                else if( unsigned( it - '0' ) <= '9' - '0' ) id += it;
                else if( it == '-' || it == '_' )            id += it;
                else {
                    out.push_back( id );
                    id = std::string();
                }
            }
            else if( it == sep0 || it == sep1 ) id += it;
        }

        if( id.size() )
            out.push_back( id );

        return out;
    }
}

#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

namespace
{
    // ================================
    //   Simple expression evaluator
    // ================================
    // (c) Peter Kankowski, 2007. http://smallcode.weblogs.us mailto:kankowski@narod.ru

    // Error codes
    enum {
        EEE_NO_ERROR = 0,
        EEE_PARENTHESIS = 1,
        EEE_WRONG_CHAR = 2,
        EEE_DIVIDE_BY_ZERO = 3
    };

    class ExprEval {
    private:
        int _err;
        const char* _err_pos;
        int _paren_count;

        // Parse a number or an expression in parenthesis
        double ParseAtom(const char*& expr) {
            // Skip spaces
            while(*expr == ' ')
                expr++;

            // Handle the sign before parenthesis (or before number)
            bool negative = false;
            if(*expr == '-') {
                negative = true;
                expr++;
            }
            if(*expr == '+') {
                expr++;
            }

            // Check if there is parenthesis
            if(*expr == '(') {
                expr++;
                _paren_count++;
                double res = ParseSummands(expr);
                if(*expr != ')') {
                    // Unmatched opening parenthesis
                    _err = EEE_PARENTHESIS;
                    _err_pos = expr;
                    return 0;
                }
                expr++;
                _paren_count--;
                return negative ? -res : res;
            }

            // It should be a number; convert it to double
            char* end_ptr;
            double res = strtod(expr, &end_ptr);
            if(end_ptr == expr) {
                // Report error
                _err = EEE_WRONG_CHAR;
                _err_pos = expr;
                return 0;
            }
            // Advance the pointer and return the result
            expr = end_ptr;
            return negative ? -res : res;
        }

        // Parse multiplication and division
        double ParseFactors(const char*& expr) {
            double num1 = ParseAtom(expr);
            for(;;) {
                // Skip spaces
                while(*expr == ' ')
                    expr++;
                // Save the operation and position
                char op = *expr;
                const char* pos = expr;
                if(op != '/' && op != '*')
                    return num1;
                expr++;
                double num2 = ParseAtom(expr);
                // Perform the saved operation
                if(op == '/') {
                    // Handle division by zero
                    if(num2 == 0) {
                        _err = EEE_DIVIDE_BY_ZERO;
                        _err_pos = pos;
                        return 0;
                    }
                    num1 /= num2;
                }
                else
                    num1 *= num2;
            }
        }

        // Parse addition and subtraction
        double ParseSummands(const char*& expr) {
            double num1 = ParseFactors(expr);
            for(;;) {
                // Skip spaces
                while(*expr == ' ')
                    expr++;
                char op = *expr;
                if(op != '-' && op != '+')
                    return num1;
                expr++;
                double num2 = ParseFactors(expr);
                if(op == '-')
                    num1 -= num2;
                else
                    num1 += num2;
            }
        }

    public:
        double evaluate(const char* expr) {
            _paren_count = 0;
            _err = EEE_NO_ERROR;
            double res = ParseSummands(expr);
            // Now, expr should point to '\0', and _paren_count should be zero
            if(_paren_count != 0 || *expr == ')') {
                _err = EEE_PARENTHESIS;
                _err_pos = expr;
                return 0;
            }
            if(*expr != '\0') {
                _err = EEE_WRONG_CHAR;
                _err_pos = expr;
                return 0;
            }
            return res;
        };
        int get_error() const {
            return _err;
        }
        const char* get_error_pos() const {
            return _err_pos;
        }
    };
}

#include <limits>
#include <string>

namespace wire
{
    double eval( const std::string &str ) {
        ExprEval evaluator;
        double result = evaluator.evaluate( str.c_str() );

        int error = evaluator.get_error();
        /**/ if( error == EEE_NO_ERROR )       return result;
        else if( error == EEE_WRONG_CHAR )     "invalid character";
        else if( error == EEE_PARENTHESIS )    "parentheses don't match";
        else if( error == EEE_DIVIDE_BY_ZERO ) "division by zero";

        return std::numeric_limits<double>::quiet_NaN(); // sqrt(-1);
    }
}
