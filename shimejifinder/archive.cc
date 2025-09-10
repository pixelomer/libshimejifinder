// 
// libshimejifinder - library for finding and extracting shimeji from archives
// Copyright (C) 2025 pixelomer
// 
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <https://www.gnu.org/licenses/>.
// 

#include "archive.hpp"
#include <filesystem>
#include <stdexcept>
#include <fcntl.h>
#include <string>
#include <unistd.h>
#include <fstream>
#include <cstdint>
#include "fs_extractor.hpp"

#include "default_actions.cc"
#include "default_behaviors.cc"
#include "shimejifinder/extract_target.hpp"

namespace shimejifinder {

void archive::add_entry(archive_entry const& entry) {
    static const std::set<std::string> allowed_extensions =
        { "wav", "png", "xml" };
    if (allowed_extensions.count(entry.lower_extension()) == 1) {
        m_entries.push_back(std::make_shared<archive_entry>(entry));
    }
}

void archive::begin_write(extract_target const& entry) {
    m_extractor->begin_write(entry);
}

void archive::write_next(size_t offset, const void *buf, size_t size) {
    m_extractor->write_next(offset, buf, size);
}

void archive::end_write() {
    m_extractor->end_write();
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

void archive::extract(extractor *extractor) {
    if (m_entries.size() == 0) {
        return;
    }
    m_extractor = extractor;
    try {
        extract();
        close_opened_file();
        extract_internal_targets();
        m_extractor->finalize();
        m_extractor = nullptr;
    }
    catch (...) {
        close_opened_file();
        m_extractor->finalize();
        m_extractor = nullptr;
        throw;
    }
}

void archive::extract(std::filesystem::path output) {
    fs_extractor extractor { output };
    extract(&extractor);
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

archive::archive(): m_file_open(nullptr), m_opened_file(nullptr),
    m_extractor(nullptr) {}

}
