#include "archive_entry.hpp"
#include "utils.hpp"
#include <cstring>

namespace shimejifinder {

archive_entry::archive_entry(): m_valid(false), m_index(-1) {}
archive_entry::archive_entry(int index): m_valid(false), m_index(index) {}
archive_entry::archive_entry(int index, std::string const& path): m_valid(true),
    m_index(index), m_path(path),
    m_lowername(to_lower(path.substr(path.rfind('/')+1))) {}

bool archive_entry::valid() const {
    return m_valid;
}

std::string const& archive_entry::path() const {
    return m_path;
}

int archive_entry::index() const {
    return m_index;
}

std::vector<extract_target> const& archive_entry::extract_targets() const {
    return m_extract_targets;
}

std::string const& archive_entry::lower_name() const {
    return m_lowername;
}

std::string archive_entry::dirname() const {
    auto pos = m_path.rfind('/');
    if (pos == std::string::npos) {
        return "";
    }
    return m_path.substr(0, pos);
}

std::string archive_entry::lower_extension() const {
    auto pos = m_lowername.rfind('.');
    if (pos == std::string::npos) {
        return "";
    }
    return m_lowername.substr(pos+1);
}

void archive_entry::add_target(extract_target const& target) {
    m_extract_targets.push_back(target);
}

}