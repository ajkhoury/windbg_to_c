#include "windbg_structure.hpp"
#include "helpers.hpp"
#include <algorithm>
#include <sstream>

windbg_structure::windbg_structure(const std::string& text)
{
    auto lines = split_string(text, "\n");

    std::transform(lines.begin(), lines.end(), lines.begin(),
        [](std::string& str) { return trim_spaces(str); }
    );

    // For each line...
    for (auto it = lines.begin(); it+1 < lines.end(); /*++it*/)
    {
        if (it->empty()) 
        {
            continue;
            it++;
        }
        if (is_header(*it))
        {
            _name = it->substr(it->find('!') + 1);
            if (_name[0] == '_')
                _name = _name.substr(1);
            it++;
            continue;
        }
        _fields.emplace_back(handle_field(it));
        
    }
}

std::unique_ptr<windbg_field> windbg_structure::handle_field(std::vector<std::string>::iterator& it)
{
    if (is_union_or_bitfield(it)) 
    {
        std::unique_ptr<windbg_union> union_field = std::make_unique<windbg_union>(parse_field_offset(*it));

        std::vector<std::string>::iterator end_member = it;
        
        size_t count = find_the_end_union_member(it, end_member);

        while (it != end_member) 
        {
            if (is_pack(it, end_member)) 
            {
                std::unique_ptr<windbg_pack> pack = std::make_unique<windbg_pack>(parse_field_offset(*it));

                auto& end_structure_member = find_the_end_structure_member_in_union(it, end_member);

                pack->add_pack_member(parse_field(*it));
                it++;
                while (it < end_structure_member) 
                {
                    pack->add_pack_member(handle_field(it));
                }
                
                union_field->add_union_member(std::move(pack));
            }
            else if (is_bitfield(*it)) 
            {
                std::unique_ptr<windbg_pack> pack = std::make_unique<windbg_pack>(parse_field_offset(*it));
                size_t bitfield_count = 0;
                do {
                    pack->add_pack_member(parse_field(*it));
                    bitfield_count++;
                    it++;
                    
                } while(is_bitfield(*it) && it != end_member);

                if (bitfield_count == count)
                    return pack;    
                else
                    union_field->add_union_member(std::move(pack));
            }
            else 
            {
                union_field->add_union_member(parse_field(*it));
                /*if ((it + 1) >= end_member)
                    break;*/
                it++;
            }
        }

        return union_field;
    }
    else 
    {
        std::unique_ptr<windbg_field> field = parse_field(*it);
        it++;
        return field;
    }
        
    
}


bool windbg_structure::is_header( const std::string& line )
{
    return line.find( '!' ) != std::string::npos;
}

std::unique_ptr<windbg_field> windbg_structure::parse_field( const std::string& line )
{
    using namespace std::string_literals;

    auto pointer_count = 0;
    auto is_array = false;
    auto array_len = 0;

    auto offset_string = line.substr( 3, line.find_first_of( ' ' ) - 3 );
    auto offset = strtoul( std::data( offset_string ), nullptr, 16 );

    auto name_start = line.find_first_of( ' ' ) + 1;
    auto name_end = line.find_first_of( ' ', name_start );
    auto name_string = line.substr( name_start, name_end - name_start );

    auto type_start = line.find_first_of( ':' ) + 2;
    auto type_string = line.substr( type_start );

    type_string = trim_spaces( type_string );
    name_string = trim_spaces( name_string );

    // Check if this is a bitfield. We return early if it is.
    if (type_string.find( "Pos" ) == 0)
    {
        auto separator = type_string.find( ',' );
        auto pos = type_string.substr( 4, separator - 4 );
        auto len = type_string.substr( separator + 2 );
        auto bitfield_pos = std::strtoul( std::data( pos ), nullptr, 10 );
        auto bitfield_len = std::strtoul( std::data( len ), nullptr, 10 );

        // Get type based on sum of bits
        auto type = "UCHAR"s;
        if (bitfield_len > 32)
            type = "ULONGLONG";
        else if (bitfield_len > 16)
            type = "ULONG";
        else if (bitfield_len > 8)
            type = "USHORT";

        return std::make_unique<windbg_bitfield>( name_string, type, offset, bitfield_pos, bitfield_len );
    }

    if (type_string[0] == '[')
    {
        is_array = true;
        auto array_end = type_string.find( ']' );
        auto subscript = type_string.substr( 1, array_end - 1 );
        array_len = std::strtoul( std::data( subscript ), nullptr, 10 );

        type_string = type_string.substr( array_end + 2 );
    }

    while (type_string.find( "Ptr64" ) != std::string::npos) 
    {
        pointer_count++;
        type_string = type_string.substr( 6 );
    }

    type_string = trim_spaces( type_string );

    auto it = known_types.find( type_string );
    if (it != known_types.end( )) 
    {
        type_string = it->second;
    }
    else 
    {
        if (type_string[0] == '_')
            type_string = type_string.substr( 1 );
    }

    if (pointer_count > 1) 
    {
        type_string = "P"s + type_string;
        while (--pointer_count)
            type_string += "*";
    }
    else if (pointer_count == 1) 
    {
        type_string = "P"s + type_string;
    }

    if (is_array)
        return std::make_unique<windbg_array>( name_string, type_string, offset, array_len );
    else
        return std::make_unique<windbg_simple>( name_string, type_string, offset );
}


std::string windbg_structure::as_string( int tabcount/* = 0*/ ) const
{
    std::stringstream out;
    out << std::string( tabcount * 4, ' ' ) << "typedef struct _" << _name << "\n{\n";
    for (auto& field : _fields) {
        out << field->as_string( tabcount + 1 ) << "\t\t\t\t// 0x" << std::hex << field->get_offset( ) << "\n";
    }

    out << std::string( tabcount * 4, ' ' ) << "} " << _name << ", *P" << _name << "; " << std::endl;
    //out << "// size=0x" << std::hex << _fields.at( _fields.size( ) - 1 )->get_offset( ) + 8 << std::endl;
    return out.str( );
}