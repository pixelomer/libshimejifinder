#include "fs_extractor.hpp"
#include <filesystem>
#include <fstream>

namespace shimejifinder {

fs_extractor::fs_extractor(std::filesystem::path output):
    m_output_path(output) {}

void fs_extractor::begin_write(extract_target const& target) {
    auto output_path = m_output_path;
    output_path /= target.shimeji_name() + ".mascot";
    switch (target.type()) {
        case extract_target::extract_type::IMAGE:
            output_path /= "img";
            break;
        case extract_target::extract_type::SOUND:
            output_path /= "sound";
            break;
        case extract_target::extract_type::XML:
            break;
        default:
            throw std::runtime_error("invalid extract target");
    }
    std::filesystem::create_directories(output_path);
    output_path /= target.extract_name();
    std::ofstream out;
    out.open(output_path, std::ios::out | std::ios::binary);
    m_active_writes.emplace_back(std::move(out));
}

void fs_extractor::write_next(size_t offset, const void *buf, size_t size) {
    for (auto &stream : m_active_writes) {
        stream.seekp(offset);
        stream.write((const char *)buf, size);
    }
}

void fs_extractor::end_write() {
    for (auto &stream : m_active_writes) {
        stream.close();
    }
    m_active_writes.clear();
}

}