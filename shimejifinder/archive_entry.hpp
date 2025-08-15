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
#include <vector>
#include "extract_target.hpp"

namespace shimejifinder {

class archive_entry {
private:
    bool m_valid;
    int m_index;
    std::string m_path;
    std::string m_lowername;
    std::string m_dirname;
    std::string m_extension;
    std::vector<extract_target> m_extract_targets;
public:
    archive_entry();
    archive_entry(int index);
    archive_entry(int index, std::string const& path);
    bool valid() const;
    int index() const;
    std::vector<extract_target> const& extract_targets() const;
    std::string const& path() const;
    std::string const& lower_name() const;
    std::string dirname() const;
    std::string const& lower_extension() const;
    void add_target(extract_target const& target);
    void clear_targets();
};

}
