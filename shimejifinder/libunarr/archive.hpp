#pragma once

#if !SHIMEJIFINDER_NO_LIBUNARR

#include "../archive.hpp"

namespace shimejifinder {
namespace libunarr {

class archive : public shimejifinder::archive {
protected:
    void fill_entries(FILE *file) override;
    void extract(FILE *file) override;
};

}
}

#endif
