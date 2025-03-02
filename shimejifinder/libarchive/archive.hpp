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

#if !SHIMEJIFINDER_NO_LIBARCHIVE

#include "../archive.hpp"
#include <archive.h>

#if SHIMEJIFINDER_DYNAMIC_LIBARCHIVE
#include <archive_entry.h>
#else
struct archive_entry;
struct archive;
#endif

namespace shimejifinder {
namespace libarchive {

class archive : public shimejifinder::archive {
#if SHIMEJIFINDER_DYNAMIC_LIBARCHIVE
public:
    static ::archive *(*archive_read_new)();
    static int (*archive_read_support_filter_all)(::archive *);
    static int (*archive_read_support_format_all)(::archive *);
    static int (*archive_read_free)(::archive *);
    static const char *(*archive_error_string)(::archive *);
    static int (*archive_read_next_header)(::archive *, ::archive_entry **);
    static mode_t (*archive_entry_filetype)(::archive_entry *);
    static int (*archive_read_data_skip)(::archive *);
    static const char *(*archive_entry_pathname)(::archive_entry *);
    static int (*archive_read_open2)(::archive *a, void *, archive_open_callback *,
        archive_read_callback *, archive_skip_callback *, archive_close_callback *);
    static int (*archive_read_open_fd)(::archive *, int, size_t);
    static int (*archive_read_data_block)(::archive *, const void **, size_t *,
        la_int64_t *);
    static la_int64_t (*archive_seek_data)(::archive *, la_int64_t, int);
    static int (*archive_read_open_memory)(::archive *, const void *, size_t);
    static int (*archive_read_open_filename)(::archive *, const char *, size_t);

    static bool loaded;

    static const char *load(const char *path);
#endif
private:
    class nested_context {
    private:
        ::archive *parent;
        la_ssize_t offset;
        ::archive *ar;
        std::vector<uint8_t> buf;
        bool read(la_int64_t *size, const void **out_buf);
        static int close_callback(::archive *ar, void *data);
        static la_int64_t skip_callback(::archive *ar, void *data, la_int64_t skip);
        static la_ssize_t read_callback(::archive *ar, void *data, const void **buf);
        static int open_callback(::archive *ar, void *data);
    public:
        nested_context(::archive *parent);
        ::archive *archive();
    };

    static std::string get_error(::archive *ar);
    bool read_data(::archive *ar, std::function<bool (long, const void *, size_t)> cb);
    bool read_data(::archive *ar, std::ostream &out, size_t max_size = SIZE_MAX);
    bool try_recurse(int &idx, ::archive *, ::archive_entry *, std::string const& pathname,
        std::function<void (int, ::archive *, std::string const&)> &cb);
    void iterate_archive(std::function<void (int, ::archive *,
        std::string const&)> cb);
    void iterate_archive(::archive *ar, int &idx, std::string const& root,
        std::function<void (int, ::archive *, std::string const&)> &cb);
    int archive_open(::archive *ar);
protected:
    void fill_entries() override;
    void extract() override;
};

}
}

#endif
