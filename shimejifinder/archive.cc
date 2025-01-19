#include "archive.hpp"
#include <filesystem>
#include <stdexcept>
#include <fcntl.h>
#include <unistd.h>
#include <fstream>
#include <cstdint>

namespace shimejifinder {

void archive::add_entry(archive_entry const& entry) {
    m_entries.push_back(std::make_shared<archive_entry>(entry));
}

void archive::write_target(extract_target const& target, uint8_t *buf, size_t size) {
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
    out.open(output_path);
    out.write((const char *)buf, size);
    out.close();
}

void archive::fill_entries(int fd) {
    (void)fd;
    throw std::runtime_error("not implemented");
}

void archive::extract(int fd) {
    (void)fd;
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

void archive::init() {
    int fd = m_file_open();
    try {
        fill_entries(fd);
        ::close(fd);
    }
    catch (...) {
        ::close(fd);
        close();
        throw;
    }
}

void archive::open(std::string const& filename) {
    close();
    m_file_open = [filename]() {
        return ::open(filename.c_str(), O_RDONLY);
    };
    init();
}

void archive::open(std::function<int ()> file_open) {
    close();
    m_file_open = file_open;
    init();
}

void archive::extract(std::filesystem::path output) {
    int fd = m_file_open();
    m_output_path = output;
    try {
        extract(fd);
        ::close(fd);
        m_output_path.clear();
    }
    catch (...) {
        ::close(fd);
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

archive::archive(): m_file_open(nullptr) {}

}