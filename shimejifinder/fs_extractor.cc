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

#include "fs_extractor.hpp"
#include <filesystem>
#include <fstream>

namespace shimejifinder {

fs_extractor::fs_extractor(std::filesystem::path output):
    m_output_path(output) {}

fs_extractor::~fs_extractor() {}

void fs_extractor::begin_write(std::filesystem::path path) {
    std::filesystem::create_directories(path.parent_path());
    std::ofstream out;
    out.open(path, std::ios::out | std::ios::binary);
    m_active_writes.emplace_back(std::move(out));
}

void fs_extractor::begin_write(extract_target const& target) {
    auto output_path = m_output_path;
    output_path /= target.shimeji_name() + ".mascot";
    switch (target.type()) {
        case extract_target::extract_type::IMAGE:
            output_path /= "img";
            break;
        case extract_target::extract_type::SOUND:
            output_path /= "sound";
            break;
        case extract_target::extract_type::XML:
            break;
        default:
            throw std::runtime_error("invalid extract target");
    }
    output_path /= target.extract_name();
    begin_write(output_path);
}

void fs_extractor::write_next(size_t offset, const void *buf, size_t size) {
    for (auto &stream : m_active_writes) {
        stream.seekp(offset);
        stream.write((const char *)buf, size);
    }
}

void fs_extractor::end_write() {
    for (auto &stream : m_active_writes) {
        stream.close();
    }
    m_active_writes.clear();
}

std::filesystem::path const& fs_extractor::output_path() {
    return m_output_path;
}

}