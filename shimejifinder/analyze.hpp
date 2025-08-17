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

struct analyze_config {
    bool unused = false;
};

/// Analyzes the specified archive file and returns an archive object ready
/// to be extracted, or null if an error occurred.
/// @param filename Path to archive.
/// @param config Analyzer configuration.
std::unique_ptr<archive> analyze(std::string const& filename, analyze_config const& config = {});

/// Analyzes the specified archive file and returns an archive object ready
/// to be extracted, or null if an error occurred.
/// @param name User-friendly name of the archive. Usually the archive's
///             filename without its extension. Will be used as fallbac
///             when a shimeji's name cannot be determined.
/// @param filename Path to archive.
/// @param config Analyzer configuration.
std::unique_ptr<archive> analyze(std::string const& name, std::string const& filename,
    analyze_config const& config = {});

/// Analyzes the specified archive file and returns an archive object ready
/// to be extracted, or null if an error occurred.
/// @param name User-friendly name of the archive. Usually the archive's
///             filename without its extension. Will be used as fallbac
///             when a shimeji's name cannot be determined.
/// @param file_open Callback to open the archive file. Will be called
///                  multiple times.
/// @param config Analyzer configuration.
std::unique_ptr<archive> analyze(std::string const& name, std::function<FILE *()> file_open,
    analyze_config const& config = {});

/// Analyzes the specified archive file and returns an archive object ready
/// to be extracted, or null if an error occurred.
/// @param name User-friendly name of the archive. Usually the archive's
///             filename without its extension. Will be used as fallbac
///             when a shimeji's name cannot be determined.
/// @param file_open Callback to open the archive file. Should return a
///                  file descriptor. Will be called multiple times.
/// @param config Analyzer configuration.
std::unique_ptr<archive> analyze(std::string const& name, std::function<int ()> file_open,
    analyze_config const& config = {});

}
