#include "utils.hpp"
#include <algorithm>

namespace shimejifinder {

unsigned char asciitolower(unsigned char in) {
    if (in <= 'Z' && in >= 'A')
        return in - ('Z' - 'z');
    return in;
}

std::string to_lower(std::string data) {
    std::transform(data.begin(), data.end(), data.begin(),
        [](unsigned char c){ return asciitolower(c); });
    return data;
}

std::string file_extension(std::string const& path) {
    auto pos = path.rfind('.');
    if (pos == std::string::npos) {
        return "";
    }
    return path.substr(pos+1);
}

std::string last_component(std::string const& path) {
    return path.substr(path.rfind('/')+1);
}

}