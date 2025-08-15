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

#include "archive_entry.hpp"
#include "utils.hpp"
#include <cstring>

namespace shimejifinder {

archive_entry::archive_entry(): m_valid(false), m_index(-1) {}
archive_entry::archive_entry(int index): m_valid(false), m_index(index) {}
archive_entry::archive_entry(int index, std::string const& path): m_valid(true),
    m_index(index), m_path(path),
    m_lowername(to_lower(last_component(path))),
    m_extension(file_extension(m_lowername)) {}

bool archive_entry::valid() const {
    return m_valid;
}

void archive_entry::clear_targets() {
    m_extract_targets.clear();
}

std::string const& archive_entry::path() const {
    return m_path;
}

int archive_entry::index() const {
    return m_index;
}

std::vector<extract_target> const& archive_entry::extract_targets() const {
    return m_extract_targets;
}

std::string const& archive_entry::lower_name() const {
    return m_lowername;
}

std::string archive_entry::dirname() const {
    auto pos = m_path.rfind('/');
    if (pos == std::string::npos) {
        return "";
    }
    return m_path.substr(0, pos);
}

std::string const& archive_entry::lower_extension() const {
    return m_extension;
}

void archive_entry::add_target(extract_target const& target) {
    m_extract_targets.push_back(target);
}

}
