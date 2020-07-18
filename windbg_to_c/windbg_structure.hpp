#pragma once

#include <string>
#include <map>

#include "helpers.hpp"
#include "windbg_field.hpp"

//
// Known WinDbg Data Types
//
const std::map<std::string, std::string> known_types = {
    {"void", "VOID"},
    {"Void", "VOID"},
    {"Char", "CHAR"},
    {"Int2B", "SHORT"},
    {"Int4B", "LONG"},
    {"Int8B", "LONGLONG"},
    {"UChar", "UCHAR"},
    {"Uint2B", "USHORT"},
    {"Uint4B", "ULONG"},
    {"Uint8B", "ULONGLONG"}
};

enum field_types {
    regular_field,
    array_field,
    bitfield,
};

class windbg_structure {
public:
    windbg_structure( const std::string& text );

    static bool is_header( const std::string& line );

    template<typename Iter>
    static bool is_union_or_bitfield(Iter it, Iter end)
    {
        try
        {
            size_t i = 1;
            while ((it + i) <= end)
            {
                if (parse_field_offset(*it) == parse_field_offset(*(it + i))) 
                {
                    //// is_bitfield
                    //while(1)
                    //{
                    //    if(parse_field_offset(*it) > parse_field_offset(*(it + i)))
                    //        return false;
                    //}
                    return true;
                }
                    
                i++;
            } 
            return false;
        }
        catch (const std::out_of_range&)
        {
            // end iterator throws when you try to increment past it
            return false;
        }
    }

    template<typename Iter>
    static bool is_pack(Iter it, Iter end)
    {
        try
        {
            if ((it + 1) == end)
                return false;
            if (parse_field_offset(*it) < parse_field_offset(*(it + 1)))
                return true;
            return false;
        }
        catch (const std::out_of_range&)
        {
            // end iterator throws when you try to increment past it
            return false;
        }
    }

    
    static bool is_bitfield(const std::string& line)
    {
        
        auto type_start = line.find_first_of(':') + 2;
        auto type_string = line.substr(type_start);
        return type_string.find("Pos") == 0;

    }


    template<typename Iter>
    static Iter find_the_end_structure_member_in_union(Iter it, Iter union_end)
    {
        Iter the_end_structure_member = it;
        size_t i = 1;
        try {
            while ((it + i) != union_end) {
                the_end_structure_member = it + i;
                if(parse_field_offset(*it) == parse_field_offset(*(it + i)))
                    return the_end_structure_member;
                i++;
            }
            return the_end_structure_member;
        }
        catch (const std::out_of_range&)
        {
            return the_end_structure_member;
        }
    }

    template<typename Iter>
    static size_t find_the_end_union_member(Iter& it, Iter& end, Iter& limit)
    {
        
        size_t i = 1, count = 1;
        try {
            while ((it+i) < limit) {
                if (parse_field_offset(*it) == parse_field_offset(*(it + i))) {
                    end = it + i + 1;
                    count = i + 1;
                }
                i++;
            }
            return count;
        }
        catch (const std::out_of_range&)
        {
            return count;
        }
    }

    const std::string& get_name( ) const { return _name; }

    std::string as_string( int tabcount = 0 ) const;

private:
    static std::string parse_field_name( const std::string& line )
    {
        auto name_start = line.find_first_of( ' ' ) + 1;
        auto name_end = line.find_first_of( ':' );
        auto temp = line.substr( name_start, name_end - name_start );
        return trim_spaces( temp );
    }

    static uint32_t parse_field_offset( const std::string& line )
    {
        auto temp = line.substr( 3, line.find_first_of( ' ' ) - 3 );
        return strtoul( std::data( temp ), nullptr, 16 );
    }

    static std::unique_ptr<windbg_field> parse_field( const std::string& line );
    std::unique_ptr<windbg_field> windbg_structure::handle_field(std::vector<std::string>::iterator& it, std::vector<std::string>::iterator& end);

private:
    std::string _name;
    std::vector<std::unique_ptr<windbg_field>> _fields;
};
