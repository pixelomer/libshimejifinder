#pragma once

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

#include <vector>
#include "archive_entry.hpp"
#include "extract_target.hpp"
#include <functional>
#include <set>
#include <memory>
#include <filesystem>

namespace shimejifinder {

class archive {
private:
    std::function<FILE *()> m_file_open;
    FILE *m_opened_file;
    std::string m_filename;
    std::vector<std::shared_ptr<archive_entry>> m_entries;
    std::set<std::string> m_shimejis;
    std::filesystem::path m_output_path;
    std::vector<std::ofstream> m_active_writes;
    std::vector<std::string> m_default_xml_targets;
    void init();
    void extract_internal_targets(std::string const& filename,
        const char *buf, size_t size);
    void extract_internal_targets();
    void close_opened_file();
protected:
    void begin_write(extract_target const& entry);
    void write_next(size_t offset, const void *buf, size_t size);
    void end_write();
    void revert_to_index(int idx);
    void add_entry(archive_entry const& entry);
    void write_target(extract_target const& target, uint8_t *buf, size_t size);
    FILE *open_file();
    bool has_filename() const;
    std::string filename() const;
    virtual void fill_entries();
    virtual void extract();
public:
    archive();
    size_t size() const;
    std::shared_ptr<archive_entry> operator[](size_t i);
    std::shared_ptr<archive_entry> operator[](size_t i) const;
    std::shared_ptr<archive_entry> at(size_t i);
    std::shared_ptr<archive_entry> at(size_t i) const;
    std::set<std::string> const& shimejis();
    void add_shimeji(std::string const& shimeji);
    void open(std::function<FILE *()> file_open);
    void open(std::string const& filename);
    void extract(std::filesystem::path output);
    void close();
    void add_default_xml_targets(std::string const& shimeji_name);
    virtual ~archive();
};

}
