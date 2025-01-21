#if !SHIMEJIFINDER_NO_LIBARCHIVE

#include "archive.hpp"
#include <cstring>
#include <stdexcept>
#include <stdlib.h>
#include <cstdio>
#include <archive.h>
#include <archive_entry.h>
#include <iostream>
#include <functional>

#if SHIMEJIFINDER_DYNAMIC_LIBARCHIVE
#include <dlfcn.h>
#endif

namespace shimejifinder {
namespace libarchive {

#if SHIMEJIFINDER_DYNAMIC_LIBARCHIVE

::archive *(*archive::archive_read_new)() = NULL;
int (*archive::archive_read_support_filter_all)(::archive *) = NULL;
int (*archive::archive_read_support_format_all)(::archive *) = NULL;
int (*archive::archive_read_open_FILE)(::archive *, FILE *) = NULL;
int (*archive::archive_read_free)(::archive *) = NULL;
const char *(*archive::archive_error_string)(::archive *) = NULL;
int (*archive::archive_read_next_header)(::archive *, ::archive_entry **) = NULL;
mode_t (*archive::archive_entry_filetype)(::archive_entry *) = NULL;
int (*archive::archive_read_data_skip)(::archive *) = NULL;
const char *(*archive::archive_entry_pathname)(::archive_entry *) = NULL;
int (*archive::archive_entry_size_is_set)(::archive_entry *) = NULL;
la_int64_t (*archive::archive_entry_size)(::archive_entry *) = NULL;
la_ssize_t (*archive::archive_read_data)(::archive *, void *, size_t) = NULL;
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
    load(archive_read_open_FILE);
    load(archive_read_free);
    load(archive_error_string);
    load(archive_read_next_header);
    load(archive_entry_filetype);
    load(archive_read_data_skip);
    load(archive_entry_pathname);
    load(archive_entry_size_is_set);
    load(archive_entry_size);
    load(archive_read_data);

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

void archive::iterate_archive(FILE *file, std::function<void (int, ::archive *, ::archive_entry *)> cb) {
    #if SHIMEJIFINDER_DYNAMIC_LIBARCHIVE
    if (!loaded) {
        throw std::runtime_error("libarchive not loaded");
    }
    #endif
    ::archive *ar;
    ::archive_entry *entry;
    int ret;
    int idx = 0;

    ar = archive_read_new();
    archive_read_support_filter_all(ar);
    archive_read_support_format_all(ar);
    ret = archive_read_open_FILE(ar, file);
    auto get_error = [&ar](){
        const char *err = archive_error_string(ar);
        if (err == nullptr) err = "(null)";
        return std::string { err };
    };
    if (ret != ARCHIVE_OK) {
        archive_read_free(ar);
        throw std::runtime_error("archive_read_open_filename() failed: " +
            get_error());
    }
    while ((ret = archive_read_next_header(ar, &entry)) == ARCHIVE_OK) {
        mode_t type = archive_entry_filetype(entry);
        if (type == AE_IFREG) {
            cb(idx, ar, entry);
            ++idx;
        }
        archive_read_data_skip(ar);
    }
    if (ret != ARCHIVE_EOF) {
        archive_read_free(ar);
        throw std::runtime_error("archive_read_next_header() failed: " +
            get_error());
    }
    ret = archive_read_free(ar);
    if (ret != ARCHIVE_OK) {
        throw std::runtime_error("archive_read_free() failed: " +
            get_error());
    }
}

void archive::fill_entries(FILE *file) {
    iterate_archive(file, [this](int idx, ::archive *ar, ::archive_entry *entry){
        (void)ar;
        const char *c_pathname = archive_entry_pathname(entry);
        if (c_pathname == NULL) {
            add_entry({ idx });
            return;
        }
        std::string pathname = c_pathname;
        fix_japanese(pathname);
        add_entry({ idx, pathname });
    });
}

void archive::extract(FILE *file) {
    std::vector<uint8_t> data;
    iterate_archive(file, [this, &data](int idx, ::archive *ar, ::archive_entry *ar_entry) {
        auto entry = at(idx);
        if (!entry->valid() || entry->extract_targets().empty()) {
            return;
        }
        if (!archive_entry_size_is_set(ar_entry)) {
            std::cerr << "entry size not set? " << entry->lower_name() << std::endl;
            return;
        }
        int64_t entry_size = archive_entry_size(ar_entry);
        if (entry_size >= 0) {
            if (data.size() < (size_t)entry_size) {
                data.resize(entry_size);
            }
            archive_read_data(ar, &data[0], entry_size);
        }
        else {
            entry_size = 0;
        }
        for (auto &target : entry->extract_targets()) {
            write_target(target, &data[0], (size_t)entry_size);
        }
    });
}

}
}

#endif