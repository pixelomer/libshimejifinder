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

#if !SHIMEJIFINDER_NO_LIBUNARR

#include "../archive.hpp"

struct ar_archive_s;
typedef struct ar_archive_s ar_archive;
struct ar_stream_s;
typedef struct ar_stream_s ar_stream;

namespace shimejifinder {
namespace libunarr {

class archive : public shimejifinder::archive {
protected:
    void fill_entries() override;
    void extract() override;
private:
    void iterate_archive(std::function<void (int, ar_archive *)> cb);
    ar_stream *open_stream();
};

}
}

#endif
