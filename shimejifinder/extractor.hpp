#pragma once
#include "extract_target.hpp"

namespace shimejifinder {

class extractor {
public:
    virtual void begin_write(extract_target const& entry) = 0;
    virtual void write_next(size_t offset, const void *buf, size_t size) = 0;
    virtual void end_write() = 0;
};

}
