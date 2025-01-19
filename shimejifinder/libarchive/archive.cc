#include "archive.hpp"
#include <cstring>
#include <stdexcept>
#include <stdlib.h>
#include <cstdio>
#include <archive.h>
#include <archive_entry.h>
#include <iostream>
#include <functional>

static void iterate_archive(int fd, std::function<void (int, struct archive *, struct archive_entry *)> cb) {
    struct archive *ar;
    struct archive_entry *entry;
    int ret;
    int idx = 0;

    ar = archive_read_new();
    archive_read_support_filter_all(ar);
    archive_read_support_format_all(ar);
    ret = archive_read_open_fd(ar, fd, 10240);
    if (ret != ARCHIVE_OK) {
        archive_read_free(ar);
        throw std::runtime_error("archive_read_open_filename() failed: " +
            std::string(archive_error_string(ar)));
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
            std::string(archive_error_string(ar)));
    }
    ret = archive_read_free(ar);
    if (ret != ARCHIVE_OK) {
        throw std::runtime_error("archive_read_free() failed: " +
            std::string(archive_error_string(ar)));
    }
}

namespace shimejifinder {
namespace libarchive {

void archive::fill_entries(int fd) {
    iterate_archive(fd, [this](int idx, ::archive *ar, ::archive_entry *entry){
        (void)ar;
        char *pathnameAlloc = NULL;
        const char *pathname = archive_entry_pathname(entry);
        if (pathname == NULL) {
            add_entry({ idx });
            return;
        }
        auto pathnameLen = strlen(pathname);
        if (pathnameLen > 8) {
            const char *last8 = pathname + pathnameLen - 8;
            // hardcoded fix for behaviors.xml
            if (strcmp(last8, "\215s\223\256.xml") == 0) {
                pathnameAlloc = new char[pathnameLen + 6];
                strcpy(pathnameAlloc, pathname);
                strcpy(pathnameAlloc + pathnameLen - 8, "behaviors.xml");
            }
            // hardcoded fix for actions.xml
            if (strcmp(last8, "\223\256\215\354.xml") == 0) {
                pathnameAlloc = new char[pathnameLen + 4];
                strcpy(pathnameAlloc, pathname);
                strcpy(pathnameAlloc + pathnameLen - 8, "actions.xml");
            }
        }
        std::string name;
        if (pathnameAlloc != NULL) {
            name = pathnameAlloc;
            delete[] pathnameAlloc;
        }
        else {
            name = pathname;
        }
        add_entry({ idx, name });
    });
}

void archive::extract(int fd) {
    std::vector<uint8_t> data;
    iterate_archive(fd, [this, &data](int idx, ::archive *ar, ::archive_entry *ar_entry) {
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