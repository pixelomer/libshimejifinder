#pragma once

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
