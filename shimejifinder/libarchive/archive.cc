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

#if !SHIMEJIFINDER_NO_LIBARCHIVE

#include <string.h>
#include "archive.hpp"
#include <cstring>
#include <stdexcept>
#include <clocale>
#include <exception>
#include <string>
#include <stdlib.h>
#include "../utils.hpp"
#include <cstdio>
#include <sstream>
#include <archive.h>
#include <archive_entry.h>
#include <sys/stat.h>
#include <iostream>
#include <functional>
#include "../utf8_convert.hpp"

#if SHIMEJIFINDER_DYNAMIC_LIBARCHIVE
#include <dlfcn.h>
#endif

namespace shimejifinder {
namespace libarchive {

#if SHIMEJIFINDER_DYNAMIC_LIBARCHIVE

::archive *(*archive::archive_read_new)() = NULL;
int (*archive::archive_read_support_filter_all)(::archive *) = NULL;
int (*archive::archive_read_support_format_all)(::archive *) = NULL;
int (*archive::archive_read_free)(::archive *) = NULL;
const char *(*archive::archive_error_string)(::archive *) = NULL;
int (*archive::archive_read_next_header)(::archive *, ::archive_entry **) = NULL;
mode_t (*archive::archive_entry_filetype)(::archive_entry *) = NULL;
int (*archive::archive_read_data_skip)(::archive *) = NULL;
const char *(*archive::archive_entry_pathname)(::archive_entry *) = NULL;
int (*archive::archive_read_open2)(::archive *a, void *, archive_open_callback *,
    archive_read_callback *, archive_skip_callback *, archive_close_callback *) = NULL;
int (*archive::archive_read_open_fd)(::archive *, int, size_t) = NULL;
int (*archive::archive_read_data_block)(::archive *, const void **, size_t *,
    la_int64_t *) = NULL;
la_int64_t (*archive::archive_seek_data)(::archive *, la_int64_t, int) = NULL;
int (*archive::archive_read_open_memory)(::archive *, const void *, size_t) = NULL;
int (*archive::archive_read_open_filename)(::archive *, const char *, size_t) = NULL;

bool archive::loaded = false;

const char *archive::load(const char *path) {
    void *libarchive = dlopen(path, RTLD_NOW);

    if (libarchive == NULL) {
        return "no library";
    }

    #define load(symbol) do { \
        *(void **)&symbol = dlsym(libarchive, #symbol); \
        if (symbol == NULL) { \
            dlclose(libarchive); \
            return "missing: " #symbol; \
        } \
    } while (0)

    load(archive_read_new);
    load(archive_read_support_filter_all);
    load(archive_read_support_format_all);
    load(archive_read_free);
    load(archive_error_string);
    load(archive_read_next_header);
    load(archive_entry_filetype);
    load(archive_read_data_skip);
    load(archive_entry_pathname);
    load(archive_read_open2);
    load(archive_read_open_fd);
    load(archive_read_data_block);
    load(archive_seek_data);
    load(archive_read_open_memory);
    load(archive_read_open_filename);

    #undef load

    loaded = true;
    return NULL;
}

#endif

static void fix_japanese(std::string &path) {
    if (path.size() > 8) {
        const char *last8 = path.c_str() + path.size() - 8;
        // hardcoded fix for behaviors.xml
        if (strcmp(last8, "\215s\223\256.xml") == 0) {
            path = path.substr(0, path.size() - 8) + "behaviors.xml";
        }
        // hardcoded fix for actions.xml
        if (strcmp(last8, "\223\256\215\354.xml") == 0) {
            path = path.substr(0, path.size() - 8) + "actions.xml";
        }
    }
}

int archive::archive_open(::archive *ar) {
    if (has_filename()) {
        auto name = filename();
        return archive_read_open_filename(ar, name.c_str(), 102400);
    }
    else {
        // archive_read_open_FILE does not implement seek callback
        return archive_read_open_fd(ar, fileno(open_file()), 102400);
    }
}

void archive::iterate_archive(std::function<void (int, ::archive *, std::string const&)> cb) {
    int idx = 0;

    ::archive *ar = archive_read_new();
    archive_read_support_filter_all(ar);
    archive_read_support_format_all(ar);
    
    int ret = archive_open(ar);

    if (ret != ARCHIVE_OK) {
        auto err = get_error(ar);
        archive_read_free(ar);
        throw std::runtime_error("archive_open() failed: " + err);
    }

    iterate_archive(ar, idx, "", cb);
}

bool archive::read_data(::archive *ar, std::function<bool (long, const void *, size_t)> cb) {
    while (true) {
        const void *buf;
        size_t size;
        la_int64_t offset;

        int ret = archive_read_data_block(ar, &buf, &size, &offset);
        if (ret == ARCHIVE_EOF) {
            return true;
        }
        else if (ret != ARCHIVE_OK) {
            std::cerr << "archive_read_data_block() failed: " << get_error(ar) << std::endl;
            return false;
        }

        if (!cb((long)offset, buf, size)) {
            return false;
        }
    }
}

archive::nested_context::nested_context(::archive *parent): parent(parent), offset(0) {
    // deallocated in iterate_archive()
    ar = archive_read_new();
    archive_read_support_filter_all(ar);
    archive_read_support_format_all(ar);
    int ret = archive_read_open2(ar, this, &open_callback, &read_callback,
        &skip_callback, &close_callback);

    if (ret != ARCHIVE_OK) {
        auto err = get_error(ar);
        archive_read_free(ar);
        throw std::runtime_error("archive_read_open2() failed: " + err);
    }
}

::archive *archive::nested_context::archive() {
    return ar;
}

bool archive::nested_context::read(la_int64_t *size, const void **out_buf) {
    const void *parent_buf;
    la_int64_t parent_offset;
    size_t parent_size;
    int ret = archive_read_data_block(parent, &parent_buf, &parent_size, &parent_offset);
    if (parent_offset < offset) {
        // offset cannot decrease, impossible condition
        archive_read_data_skip(parent);
        return false;
    }
    if (ret == ARCHIVE_EOF) {
        *out_buf = &buf[0];
        *size = 0;
        return true;
    }
    else if (ret != ARCHIVE_OK) {
        archive_read_data_skip(parent);
        return false;
    }
    size_t new_buf_size = parent_offset - offset + parent_size;
    if (buf.size() < new_buf_size) {
        buf.resize(new_buf_size);
    }
    memset(&buf[0], 0, parent_offset - offset);
    memcpy(&buf[parent_offset - offset], parent_buf, parent_size);
    offset = parent_offset + parent_size;
    *out_buf = &buf[0];
    *size = (la_int64_t)parent_size;
    return true;
}

bool archive::read_data(::archive *ar, std::ostream &out, size_t max_size) {
    return read_data(ar, [&out, max_size](long offset, const void *buf, size_t size){
        if ((offset + size) > max_size) {
            return false;
        }
        out.seekp(offset);
        out.write((const char *)buf, size);
        return true;
    });
}

int archive::nested_context::open_callback(::archive *sender, void *data) {
    (void)sender;
    (void)data;
    return 0;
}

la_ssize_t archive::nested_context::read_callback(::archive *sender, void *data,
    const void **buf)
{
    (void)sender;
    auto ctx = (archive::nested_context *)data;
    la_int64_t size;
    bool success = ctx->read(&size, buf);
    return success ? (la_ssize_t)size : 0;
}

la_int64_t archive::nested_context::skip_callback(::archive *sender, void *data, la_int64_t skip) {
    (void)sender;
    (void)data;
    (void)skip;
    return 0;
}

int archive::nested_context::close_callback(::archive *sender, void *data) {
    (void)sender;
    (void)data;
    return 0;
}

bool archive::try_recurse(int &idx, ::archive *parent, ::archive_entry *entry, std::string const& pathname,
    std::function<void (int, ::archive *, std::string const&)> &cb)
{
    auto ext = to_lower(file_extension(pathname));
    (void)entry;
    size_t size_without_ext = pathname.size() - ext.size() - 1;
    if (ext == "zip") {
        auto filename = to_lower(last_component(pathname));
        if (filename == "src.zip") {
            // ignore src.zip, it usually contains the unmodified default shimeji
            return false;
        }
        auto new_root = pathname.substr(0, size_without_ext) + "/";
        try {
            // try extracting nested archive without extracting whole archive into memory
            nested_context ctx { parent };
            auto ar = ctx.archive();
            iterate_archive(ar, idx, new_root, cb);
            return true;
        }
        catch (std::exception &ex) {
            std::cerr << "failed to extract nested archive: " << ex.what() << std::endl;
        }
    }
    else if (ext == "7z" || ext == "rar") {
        auto new_root = pathname.substr(0, size_without_ext) + "/";
        try {
            // try extracting nested archive into memory first
            std::ostringstream ss;
            bool read = read_data(parent, ss, 50 * 1024 * 1024);
            if (read) {
                auto str = ss.str();
                ss.str("");
                auto ar = archive_read_new();
                archive_read_support_filter_all(ar);
                archive_read_support_format_all(ar);
                int ret = archive_read_open_memory(ar, &str[0], str.size());
                if (ret != ARCHIVE_OK) {
                    auto err = get_error(ar);
                    archive_read_free(ar);
                    throw std::runtime_error("archive_read_open_memory() failed: " + err);
                }
                iterate_archive(ar, idx, new_root, cb);
                return true;
            }
            else {
                std::cerr << "cannot read nested archive into buffer" << std::endl;
            }
        }
        catch (std::exception &ex) {
            std::cerr << "failed to extract nested archive: " << ex.what() << std::endl;
        }
    }
    return false;
}

std::string archive::get_error(::archive *ar) {
    const char *err = archive_error_string(ar);
    if (err == nullptr) err = "(null)";
    return std::string { err };
}

void archive::iterate_archive(::archive *ar, int &idx, std::string const& root,
    std::function<void (int, ::archive *, std::string const&)> &cb)
{
    #if SHIMEJIFINDER_DYNAMIC_LIBARCHIVE
    if (!loaded) {
        throw std::runtime_error("libarchive not loaded");
    }
    #endif
    ::archive_entry *entry;
    int ret;

    while ((ret = archive_read_next_header(ar, &entry)) == ARCHIVE_OK) {
        mode_t type = archive_entry_filetype(entry);
        if (type == AE_IFREG) {
            const char *c_pathname = archive_entry_pathname(entry);
            std::string pathname;
            bool did_recurse = false;
            if (c_pathname != nullptr) {
                pathname = c_pathname;
                #if SHIMEJIFINDER_HAS_UTF8_CONVERT
                    if (!is_valid_utf8(pathname)) {
                        shift_jis_to_utf8(pathname);
                    }
                #endif
                pathname = root + pathname; 
                did_recurse = try_recurse(idx, ar, entry, pathname, cb);
            }
            if (!did_recurse) {
                cb(idx, ar, pathname);
                ++idx;
            }
        }
    }
    if (ret != ARCHIVE_EOF) {
        auto err = get_error(ar);
        archive_read_free(ar);
        throw std::runtime_error("archive_read_next_header() failed: " + err);
    }
    ret = archive_read_free(ar);
    if (ret != ARCHIVE_OK) {
        auto err = get_error(ar);
        throw std::runtime_error("archive_read_free() failed: " + err);
    }
}

void archive::fill_entries() {
    iterate_archive([this](int idx, ::archive *ar, std::string const& pathname){
        (void)ar;
        if (pathname.empty()) {
            return;
        }
        auto fixed_name = pathname;
        fix_japanese(fixed_name);
        add_entry({ idx, fixed_name });
    });
}

void archive::extract() {
    size_t stored_idx = 0;
    iterate_archive([this, &stored_idx](int idx, ::archive *ar, std::string const& pathname){
        (void)pathname;
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
        read_data(ar, [this](long offset, const void *buf, size_t size){
            write_next(offset, buf, size);
            return true;
        });
        end_write();
    });
}

}
}

#endif
