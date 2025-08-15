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

#include "extract_target.hpp"

namespace shimejifinder {

std::string const& extract_target::shimeji_name() const {
    return m_shimeji_name;
}

std::string const& extract_target::extract_name() const {
    return m_extract_name;
}

extract_target::extract_type extract_target::type() const {
    return m_type;
}

void extract_target::set_extract_name(std::string const& name) {
    m_extract_name = name;
}

extract_target::extract_target(std::string const& filename):
    m_extract_name(filename), m_type(extract_type::UNSPECIFIED) {}

extract_target::extract_target(std::string const& shimeji_name, std::string const& filename,
    extract_type type): m_shimeji_name(shimeji_name), m_extract_name(filename),
    m_type(type) {}

}
