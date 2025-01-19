#include "extract_target.hpp"

namespace shimejifinder {

std::string const& extract_target::shimeji_name() const {
    return m_shimeji_name;
}

std::string const& extract_target::extract_name() const {
    return m_extract_name;
}

extract_target::extract_type extract_target::type() const {
    return m_type;
}

void extract_target::set_extract_name(std::string const& name) {
    m_extract_name = name;
}

extract_target::extract_target(std::string const& shimeji_name, std::string const& filename,
    extract_type type): m_shimeji_name(shimeji_name), m_extract_name(filename),
    m_type(type) {}

}