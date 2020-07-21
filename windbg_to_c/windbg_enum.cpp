#include "windbg_enum.hpp"
#include "helpers.hpp"
#include <algorithm>
#include <sstream>

windbg_enum::windbg_enum(const std::string& text)
{
    auto lines = split_string(text, "\n");

    std::transform(lines.begin(), lines.end(), lines.begin(),
        [](std::string& str) {
            if ((str.find('!') == std::string::npos) && (str.find("= 0n") == std::string::npos)) //Invalid field, empty it.
            {
                std::string().swap(str);
                return str;
            }
            return trim_spaces(str);
        }
    );

    // For each line...
    for (auto it = lines.begin(); it + 1 < lines.end(); it++)
    {
        if (it->empty())            
            continue;
        if (is_header(*it))
        {
            _name = it->substr(it->find('!') + 1);
            if (_name[0] == '_')
                _name = _name.substr(1);
            continue;
        }

        _fields.emplace_back(parse_enum_field(*it));
    }
}

std::unique_ptr<windbg_enum_field> windbg_enum::parse_enum_field(const std::string& line)
{
    auto name_end = line.find_first_of(' ');
    auto name_string = line.substr(0, name_end);

    auto value_start = line.find_last_of("n") + 1;
    auto value_string = line.substr(value_start, line.size() - value_start);
    auto value = strtoul(std::data(value_string), nullptr, 10);

    return std::make_unique<windbg_enum_field>(name_string, value);
}

bool windbg_enum::is_header(const std::string& line)
{
    return line.find('!') != std::string::npos;
}

std::string windbg_enum::as_string(int tabcount/* = 0*/) const
{
    std::stringstream out;

    out << std::string(tabcount * 4, ' ') << "enum _" << _name << "\n{\n";

    if (_fields.size() != 0)
    {
        for (auto& field : _fields) {
            out << field->as_string(tabcount + 1) << "\n";
        }
    }

    out << std::string(tabcount * 4, ' ') << "}; \n";
    out << std::string(tabcount * 4, ' ') << "typedef enum _" << _name << " " << _name << ", *P" << _name << ";\n" << std::endl;
    return out.str();
}