#pragma once
#include "../archive.hpp"

namespace shimejifinder {
namespace libarchive {

class archive : public shimejifinder::archive {
protected:
    void fill_entries(int fd) override;
    void extract(int fd) override;
};

}
}