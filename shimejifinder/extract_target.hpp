#pragma once

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

#include <string>

namespace shimejifinder {

class extract_target {
public:
    enum class extract_type {
        INVALID = 0,
        IMAGE,
        SOUND,
        XML
    };
private:
    std::string m_shimeji_name;
    std::string m_extract_name;
    extract_type m_type;
public:
    std::string const& shimeji_name() const;
    std::string const& extract_name() const;
    extract_type type() const;
    void set_extract_name(std::string const& name);
    extract_target(std::string const& shimeji_name, std::string const& filename,
        extract_type type);
};

}
