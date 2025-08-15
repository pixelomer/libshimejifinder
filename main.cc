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

#include <shimejifinder/archive.hpp>
#include <shimejifinder/analyze.hpp>
#include <shimejifinder/archive_folder.hpp>
#include <cstdlib>
#include <filesystem>
#include <shimejifinder/libarchive/archive.hpp>
#include <unistd.h>
#include <iostream>

int main(int argc, char **argv) {
    setlocale(LC_ALL, "C.UTF-8"); // this is required for 7z
    if (argc <= 1) {
        std::cerr << "usage: shimejifinder-test <path-to-archive>" << std::endl;
        return EXIT_FAILURE;
    }
    std::string path = argv[1];

    auto ar = shimejifinder::analyze(path);
    for (auto &found : ar->shimejis()) {
        std::cout << "found: " << found << std::endl;
    }
    shimejifinder::archive_folder root { *ar };
    //root.print(std::cout);
    ar->extract(std::filesystem::current_path() / "out");
}
