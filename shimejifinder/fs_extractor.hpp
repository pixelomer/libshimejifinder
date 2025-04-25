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

#pragma once
#include "extractor.hpp"
#include <filesystem>
#include <vector>

namespace shimejifinder {

class fs_extractor : public extractor {
public:
    fs_extractor(std::filesystem::path output);
    virtual void begin_write(extract_target const& target);
    virtual void write_next(size_t offset, const void *buf, size_t size);
    virtual void end_write();
private:
    std::filesystem::path m_output_path;
    std::vector<std::ofstream> m_active_writes;
};

}
