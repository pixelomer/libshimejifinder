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

#include "archive.hpp"
#include <memory>
#include <functional>

namespace shimejifinder {

std::unique_ptr<archive> analyze(std::string const& filename);
std::unique_ptr<archive> analyze(std::string const& name, std::string const& filename);
std::unique_ptr<archive> analyze(std::string const& name, std::function<FILE *()> file_open);
std::unique_ptr<archive> analyze(std::string const& name, std::function<int ()> file_open);

}
