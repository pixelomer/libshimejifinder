#if !SHIMEJIFINDER_NO_LIBUNARR

#include "archive.hpp"
#include <stdexcept>
#include <stdlib.h>
#include <cstdio>
#include <unarr.h>
#include <functional>
#include "unarr_FILE.h"

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
        const char *pathname = ar_entry_get_name(ar);
        if (pathname == nullptr) {
            add_entry({ idx });
        }
        else {
            add_entry({ idx, pathname });
        }
    });
}

void archive::extract() {
    std::vector<uint8_t> data(10240);
    iterate_archive([this, &data](int idx, ar_archive *ar) {
        auto entry = at(idx);
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
