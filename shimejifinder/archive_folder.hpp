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

#include "archive_entry.hpp"
#include "archive.hpp"
#include <iostream>
#include <map>
#include <ostream>
#include <memory>
#include <string>

namespace shimejifinder {

class archive_folder {
private:
    std::string m_name;
    archive_folder *m_parent;
    std::map<std::string, archive_folder> m_folders;
    std::map<std::string, std::shared_ptr<archive_entry>> m_entries;
    void print(std::ostream &out, int depth) const;
public:
    archive_folder();
    archive_folder(archive const& ar, std::string const& root = "");
    archive_folder *parent();
    archive_entry *relative_file(std::string const& path);
    const std::map<std::string, archive_folder> &folders();
    const std::map<std::string, std::shared_ptr<archive_entry>> &files();
    archive_folder *folder_named(std::string const& name);
    archive_entry *entry_named(std::string const& name);
    void print(std::ostream &out = std::cout) const;
    std::string const& name() const;
    std::string lower_name() const;
};

}
