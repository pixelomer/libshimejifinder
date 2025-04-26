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

#include "fs_thumb_extractor.hpp"

namespace shimejifinder {

fs_thumb_extractor::fs_thumb_extractor(std::filesystem::path output):
    fs_extractor(output) {}

fs_thumb_extractor::~fs_thumb_extractor() {}

void fs_thumb_extractor::begin_write(extract_target const& target) {
    if (target.type() != extract_target::extract_type::IMAGE) {
        return;
    }
    auto path = output_path() / (target.shimeji_name() + ".png");
    fs_extractor::begin_write(path);
}

}
