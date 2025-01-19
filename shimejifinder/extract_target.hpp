#pragma once
#include <string>

namespace shimejifinder {

class extract_target {
public:
    enum class extract_type {
        INVALID = 0,
        IMAGE,
        SOUND,
        XML
    };
private:
    std::string m_shimeji_name;
    std::string m_extract_name;
    extract_type m_type;
public:
    std::string const& shimeji_name() const;
    std::string const& extract_name() const;
    extract_type type() const;
    void set_extract_name(std::string const& name);
    extract_target(std::string const& shimeji_name, std::string const& filename,
        extract_type type);
};

}