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
    auto ar = shimejifinder::analyze(path, path);
    for (auto &found : ar->shimejis()) {
        std::cout << "found: " << found << std::endl;
    }
    //shimejifinder::archive_folder root { *ar };
    //root.print(std::cout);
    ar->extract(std::filesystem::current_path() / "out");
}