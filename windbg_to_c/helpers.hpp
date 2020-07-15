#pragma once

#include <string>
#include <vector>

inline std::vector<std::string> split_string( const std::string& str, const std::string& delimiter )
{
    std::vector<std::string> strings;

    std::string::size_type pos = 0;
    std::string::size_type prev = 0;
    while ((pos = str.find( delimiter, prev )) != std::string::npos) {
        strings.push_back( str.substr( prev, pos - prev ) );
        prev = pos + 1;
    }

    // To get the last substring (or only, if delimiter is not found)
    strings.push_back( str.substr( prev ) );

    return strings;
}

inline std::vector<std::string> split_string_not_drop_delimiter(const std::string& str, const std::string& delimiter)
{
    std::vector<std::string> strings;

    std::string::size_type pos = 0;
    std::string::size_type prev = 0;
    while ((pos = str.find(delimiter, prev+1)) != std::string::npos) {
        strings.push_back(str.substr(prev, pos - prev));
        prev = pos;
    }

    // To get the last substring (or only, if delimiter is not found)
    strings.push_back(str.substr(prev));

    return strings;
}

inline std::string trim_trailing_and_leading_whitespaces( std::string str )
{
    std::string::iterator it;

    for (it = str.begin( ); it != str.end( ); ++it)
    {
        if (isalnum( *it ))
            break;
        str.erase( it );
    }

    if (str.empty( ))
        return str;

    for (it = str.end( ) - 1; it != str.begin( ); --it)
    {
        if (isalnum( *it ))
            break;
        str.erase( it );
    }

    it = str.begin( );
    if (!isalnum( *it ))
        str.erase( it );

    return str;
}

inline std::string trim_trailing_spaces( std::string str )
{
    str.erase( str.find_first_not_of( '\n' ) );
    str.erase( 0, str.find_first_not_of( ' ' ) );
    str.erase( str.find_last_not_of( ' ' ) + 1 );
    str.erase( str.find_last_not_of( '\n' ) + 1 );
    return str;
}

inline std::string trim_spaces( std::string str )
{
    auto first_non_space = str.find_first_not_of( " \n" );
    if (first_non_space != std::string::npos)
        str = str.substr( first_non_space );
    auto last_non_space = str.find_last_not_of( " \n" );
    if (last_non_space != std::string::npos)
        str = str.substr( 0, last_non_space + 1 );
    return str;
}