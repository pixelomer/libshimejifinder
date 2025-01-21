#pragma once

#if !SHIMEJIFINDER_NO_LIBARCHIVE

#include "../archive.hpp"

#if SHIMEJIFINDER_DYNAMIC_LIBARCHIVE
#include <archive.h>
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
    static ::archive_entry *(*archive_read_entry)();
    static int (*archive_read_support_filter_all)(::archive *);
    static int (*archive_read_support_format_all)(::archive *);
    static int (*archive_read_open_fd)(::archive *, int, size_t);
    static int (*archive_read_free)(::archive *);
    static const char *(*archive_error_string)(::archive *);
    static int (*archive_read_next_header)(::archive *, ::archive_entry **);
    static mode_t (*archive_entry_filetype)(::archive_entry *);
    static int (*archive_read_data_skip)(::archive *);
    static const char *(*archive_entry_pathname)(::archive_entry *);
    static int (*archive_entry_size_is_set)(::archive_entry *);
    static la_int64_t (*archive_entry_size)(::archive_entry *);
    static la_ssize_t (*archive_read_data)(::archive *, void *, size_t);

    static bool loaded;

    static const char *load(const char *path);
#endif
private:
    static void iterate_archive(int fd, std::function<void (int, ::archive *, ::archive_entry *)> cb);
protected:
    void fill_entries(int fd) override;
    void extract(int fd) override;
};

}
}

#endif