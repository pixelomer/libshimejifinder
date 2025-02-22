// 
// libshimejifinder - library for finding and extracting shimeji from archives
// Copyright (C) 2025 pixelomer
// 
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <https://www.gnu.org/licenses/>.
// 

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
