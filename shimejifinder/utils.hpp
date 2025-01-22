#pragma once
#include <string>

namespace shimejifinder {

std::string to_lower(std::string data);
unsigned char asciitolower(unsigned char in);
std::string file_extension(std::string const& path);
std::string last_component(std::string const& path);

}