#pragma once
#include "extractor.hpp"
#include <filesystem>
#include <vector>

namespace shimejifinder {

class fs_extractor : public extractor {
public:
    fs_extractor(std::filesystem::path output);
    virtual void begin_write(extract_target const& target);
    virtual void write_next(size_t offset, const void *buf, size_t size);
    virtual void end_write();
private:
    std::filesystem::path m_output_path;
    std::vector<std::ofstream> m_active_writes;
};

}
