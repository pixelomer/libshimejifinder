#pragma once
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
    std::map<std::string, archive_folder> m_folders;
    std::map<std::string, std::shared_ptr<archive_entry>> m_entries;
    void print(std::ostream &out, int depth) const;
public:
    archive_folder();
    archive_folder(archive const& ar, std::string const& root = "");
    std::map<std::string, archive_folder> &folders();
    std::map<std::string, std::shared_ptr<archive_entry>> &files();
    archive_folder *folder_named(std::string const& name);
    archive_entry *entry_named(std::string const& name);
    void print(std::ostream &out = std::cout) const;
    std::string const& name() const;
    std::string lower_name() const;
};

}