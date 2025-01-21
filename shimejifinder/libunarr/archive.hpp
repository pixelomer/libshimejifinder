#pragma once

#if !SHIMEJIFINDER_NO_LIBUNARR

#include "../archive.hpp"

namespace shimejifinder {
namespace libunarr {

class archive : public shimejifinder::archive {
protected:
    void fill_entries(int fd) override;
    void extract(int fd) override;
};

}
}

#endif