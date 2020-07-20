#pragma once

#include <string>
#include <map>

#include "helpers.hpp"
#include "windbg_field.hpp"

class windbg_enum {
public:
    windbg_enum(const std::string& text);

    static bool is_header(const std::string& line);

    const std::string& get_name() const { return _name; }

    std::string as_string(int tabcount = 0) const;

private:
    static std::unique_ptr<windbg_enum_field> parse_enum_field(const std::string& line);


private:
    std::string _name;
    std::vector<std::unique_ptr<windbg_enum_field>> _fields;
};