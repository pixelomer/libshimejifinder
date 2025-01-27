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

static void iterate_archive(FILE *file, std::function<void (int, ar_archive *)> cb) {
    ar_stream *stream = ar_open_FILE(file);
    if (stream == nullptr) {
        throw std::runtime_error("ar_open_FILE() failed");
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

namespace shimejifinder {
namespace libunarr {

void archive::fill_entries(FILE *file) {
    iterate_archive(file, [this](int idx, ar_archive *ar) {
        const char *pathname = ar_entry_get_name(ar);
        if (pathname == nullptr) {
            add_entry({ idx });
        }
        else {
            add_entry({ idx, pathname });
        }
    });
}

void archive::extract(FILE *file) {
    std::vector<uint8_t> data(10240);
    iterate_archive(file, [this, &data](int idx, ar_archive *ar) {
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