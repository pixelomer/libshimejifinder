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

#include "memory_extractor.hpp"
#include <filesystem>
#include <fstream>

namespace shimejifinder {

memory_extractor::memory_extractor() {}
memory_extractor::~memory_extractor() {}

void memory_extractor::begin_write(extract_target const& target) {
    m_active_writes.emplace_back(target.extract_name());
}

void memory_extractor::write_next(size_t offset, const void *buf, size_t size) {
    m_buffer.seekp(offset);
    m_buffer.write((const char *)buf, size);
}

void memory_extractor::end_write() {
    auto result = m_buffer.str();
    for (auto target : m_active_writes) {
        m_output[target] = result;
    }
    m_buffer.str("");
    m_buffer.clear();
    m_active_writes.clear();
}

bool memory_extractor::contains(std::string const& name) const {
    return m_output.count(name) == 1;
}

std::string const& memory_extractor::data(std::string const& target) const {
    static const std::string no_data = "";
    if (m_output.count(target) == 1) {
        return m_output.at(target);
    }
    else {
        return no_data;
    }
}

}