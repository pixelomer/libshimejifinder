#include "archive.hpp"
#include <filesystem>
#include <stdexcept>
#include <fcntl.h>
#include <string>
#include <unistd.h>
#include <fstream>
#include <cstdint>

#include "default_actions.cc"
#include "default_behaviors.cc"
#include "shimejifinder/extract_target.hpp"

namespace shimejifinder {

void archive::add_entry(archive_entry const& entry) {
    m_entries.push_back(std::make_shared<archive_entry>(entry));
}

void archive::begin_write(extract_target const& target) {
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

void archive::write_next(size_t offset, const void *buf, size_t size) {
    for (auto &stream : m_active_writes) {
        stream.seekp(offset);
        stream.write((const char *)buf, size);
    }
}

void archive::end_write() {
    for (auto &stream : m_active_writes) {
        stream.close();
    }
    m_active_writes.clear();
}

void archive::write_target(extract_target const& target, uint8_t *buf, size_t size) {
    begin_write(target);
    write_next(0, buf, size);
    end_write();
}

void archive::add_default_xml_targets(std::string const& shimeji_name) {
    m_default_xml_targets.push_back(shimeji_name);
}

void archive::fill_entries() {
    throw std::runtime_error("not implemented");
}

void archive::revert_to_index(int idx) {
    m_entries.resize(idx);
}

void archive::extract() {
    throw std::runtime_error("not implemented");
}

size_t archive::size() const {
    return m_entries.size();
}

std::shared_ptr<archive_entry> archive::operator[](size_t i) {
    return m_entries[i];
}

std::shared_ptr<archive_entry> archive::operator[](size_t i) const {
    return m_entries[i];
}

std::shared_ptr<archive_entry> archive::at(size_t i) {
    return this->operator[](i);
}

std::shared_ptr<archive_entry> archive::at(size_t i) const {
    return this->operator[](i);
}

bool archive::has_filename() const {
    return !m_filename.empty();
}

std::string archive::filename() const {
    return m_filename;
}

FILE *archive::open_file() {
    FILE *&file = m_opened_file;
    if (file != nullptr) {
        throw std::runtime_error("open_file() may only be called once in fill_entries() or extract()");
    }
    if (has_filename()) {
        file = fopen(m_filename.c_str(), "rb");
    }
    else {
        file = m_file_open();
    }
    if (file == nullptr) {
        throw std::runtime_error("fopen() failed");
    }
    return file;
}

void archive::close_opened_file() {
    if (m_opened_file != nullptr) {
        fclose(m_opened_file);
        m_opened_file = nullptr;
    }
}

void archive::init() {
    try {
        fill_entries();
        close_opened_file();
    }
    catch (...) {
        close_opened_file();
        close();
        throw;
    }
}

void archive::open(std::string const& filename) {
    close();
    m_filename = filename;
    m_file_open = nullptr;
    init();
}

void archive::open(std::function<FILE *()> file_open) {
    close();
    m_filename = "";
    m_file_open = file_open;
    init();
}

void archive::extract_internal_targets(std::string const& filename,
    const char *buf, size_t size)
{
    for (auto &shimeji : m_default_xml_targets) {
        begin_write({ shimeji, filename,
            extract_target::extract_type::XML });
    }
    write_next(0, buf, size);
    end_write();
}

void archive::extract_internal_targets() {
    extract_internal_targets("actions.xml", default_actions,
        default_actions_len);
    extract_internal_targets("behaviors.xml", default_behaviors,
        default_behaviors_len);
}

void archive::extract(std::filesystem::path output) {
    m_output_path = output;
    try {
        extract();
        close_opened_file();
        extract_internal_targets();
        m_output_path.clear();
    }
    catch (...) {
        close_opened_file();
        m_output_path.clear();
        throw;
    }
}

void archive::close() {
    m_file_open = nullptr;
    m_entries.clear();
}

archive::~archive() {
    close();
}

std::set<std::string> const& archive::shimejis() {
    return m_shimejis;
}

void archive::add_shimeji(std::string const& shimeji) {
    m_shimejis.insert(shimeji);
}

archive::archive(): m_file_open(nullptr), m_opened_file(nullptr) {}

}