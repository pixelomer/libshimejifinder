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

#include "extract_target.hpp"

namespace shimejifinder {

class extractor {
public:
    virtual void begin_write(extract_target const& entry) = 0;
    virtual void write_next(size_t offset, const void *buf, size_t size) = 0;
    virtual void end_write() = 0;
    virtual void finalize();
};

}
