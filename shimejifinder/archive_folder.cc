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

#include "archive_folder.hpp"
#include "utils.hpp"
#include <iostream>
#include <ostream>
#include <sstream>

static std::ostream &indent(std::ostream &out, int depth) {
    for (int i=0; i<depth; ++i) {
        out << "  ";
    }
    return out;
}

namespace shimejifinder {

archive_folder::archive_folder() {}

archive_folder *archive_folder::folder_named(std::string const& name) {
    return m_folders.count(name) == 1 ? &m_folders.at(name) : nullptr;
}

archive_entry *archive_folder::entry_named(std::string const& name) {
    return m_entries.count(name) == 1 ? m_entries.at(name).get() : nullptr;
}

std::map<std::string, archive_folder> &archive_folder::folders() {
    return m_folders;
}

std::map<std::string, std::shared_ptr<archive_entry>> &archive_folder::files() {
    return m_entries;
}

void archive_folder::print(std::ostream &out) const {
    out << "[" << m_name << "]" << std::endl;
    print(out, 1);
}

std::string const& archive_folder::name() const {
    return m_name;
}

std::string archive_folder::lower_name() const {
    return to_lower(m_name);
}

void archive_folder::print(std::ostream &out, int depth) const {
    for (auto &folder : m_folders) {
        indent(out, depth) << "[" << folder.second.m_name << "]" << std::endl;
        folder.second.print(out, depth+1);
    }
    for (auto &pair : m_entries) {
        auto &entry = pair.second;
        auto &path = entry->path();
        auto name = path.substr(path.rfind('/') + 1);
        indent(out, depth) << name;
        if (!entry->extract_targets().empty()) {
            out << "*";
        }
        out << std::endl;
    }
}

archive_folder::archive_folder(archive const& ar, std::string const& root): m_name("/") {
    for (size_t i=0; i<ar.size(); ++i) {
        auto entry = ar[i];
        if (!entry->valid()) {
            continue;
        }
        auto path = entry->path();
        if (!root.empty() && path.find(root) != 0) {
            continue;
        }
        path = path.substr(root.size());
        if (entry->path().empty() ||
            (entry->path().size() == 1 && entry->path()[0] == '/'))
        {
            continue;
        }
        if (path[0] == '/') {
            path = path.substr(1);
        }
        std::stringstream ss { path };
        std::string component;
        auto folder = this;
        while (std::getline(ss, component, '/')) {
            if (!ss.eof()) {
                // folder
                folder = &folder->m_folders[to_lower(component)];
                folder->m_name = component;
            }
            else {
                // file
                folder->m_entries[to_lower(component)] = entry;
            }
        }
    }
}

}
