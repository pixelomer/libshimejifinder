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

#include "archive.hpp"
#include <stdexcept>
#include <stdlib.h>
#include <cstdio>
#include <unarr.h>
#include <functional>
#include "unarr_FILE.h"
#include "../utf8_convert.hpp"

static ar_archive *ar_open_any_archive(ar_stream *stream) {
    ar_archive *ar = ar_open_rar_archive(stream);
    if (!ar) ar = ar_open_zip_archive(stream, false);
    if (!ar) ar = ar_open_7z_archive(stream);
    if (!ar) ar = ar_open_tar_archive(stream);
    return ar;
}

namespace shimejifinder {
namespace libunarr {

ar_stream *archive::open_stream() {
    ar_stream *stream;
    if (has_filename()) {
        auto name = filename();
        stream = ar_open_file(name.c_str());
    }
    else {
        stream = ar_open_FILE(open_file());
    }
    return stream;
}

void archive::iterate_archive(std::function<void (int, ar_archive *)> cb) {
    ar_stream *stream = open_stream();
    if (stream == nullptr) {
        throw std::runtime_error("open_stream() failed");
    }
    ar_archive *archive = ar_open_any_archive(stream);
    if (archive == nullptr) {
        ar_close(stream);
        throw std::runtime_error("ar_open_any_archive() failed");
    }
    for (int i=0; ar_parse_entry(archive); ++i) {
        cb(i, archive);
    }
    ar_close_archive(archive);
    ar_close(stream);
}

void archive::fill_entries() {
    iterate_archive([this](int idx, ar_archive *ar) {
        const char *c_pathname = ar_entry_get_name(ar);
        std::string pathname;
        if (c_pathname == nullptr) {
            c_pathname = ar_entry_get_raw_name(ar);
            if (c_pathname == nullptr) {
                return;
            }
            pathname = c_pathname;
        }
        #if SHIMEJIFINDER_HAS_UTF8_CONVERT
            if (!is_valid_utf8(pathname) && !shift_jis_to_utf8(pathname)) {
                // never allow invalid utf-8
                return;
            }
        #endif
        add_entry({ idx, pathname });
    });
}

void archive::extract() {
    std::vector<uint8_t> data(10240);
    size_t stored_idx = 0;
    iterate_archive([this, &data, &stored_idx](int idx, ar_archive *ar) {
        if (stored_idx >= size()) {
            return;
        }
        auto entry = at(stored_idx);
        if (idx != entry->index()) {
            return;
        }
        ++stored_idx;
        if (!entry->valid() || entry->extract_targets().empty()) {
            return;
        }
        for (auto &target : entry->extract_targets()) {
            begin_write(target);
        }
        size_t remaining = ar_entry_get_size(ar);
        size_t offset = 0;
        while (remaining > 0) {
            size_t read = std::min(data.size(), remaining);
            if (!ar_entry_uncompress(ar, &data[0], read)) {
                break;
            }
            write_next(offset, &data[0], read);
            offset += read;
            remaining -= read;
        }
        end_write();
    });
}

}
}

#endif
