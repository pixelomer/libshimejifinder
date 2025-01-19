#pragma once
#include "archive.hpp"
#include <memory>
#include <functional>

namespace shimejifinder {

std::unique_ptr<archive> analyze(std::string const& name, std::string const& filename);
std::unique_ptr<archive> analyze(std::string const& name, std::function<int ()> file_open);

}