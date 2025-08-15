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

#include "analyze.hpp"
#include "libunarr/archive.hpp"
#include "libarchive/archive.hpp"
#include "archive.hpp"
#include "archive_entry.hpp"
#include "archive_folder.hpp"
#include "extract_target.hpp"
#include "memory_extractor.hpp"
#include "utils.hpp"
#include <exception>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>
#include <array>
#include <pugixml.hpp>
#include <string.h>

namespace shimejifinder {

template<typename T>
static std::unique_ptr<archive> open_archive(T const& input) {
    #if !SHIMEJIFINDER_NO_LIBARCHIVE
    try {
        auto ar = std::make_unique<libarchive::archive>();
        ar->open(input);
        return ar;
    }
    catch (std::exception &ex) {
        std::cerr << "libarchive: open(): " << ex.what() << std::endl;
    }
    #endif
    #if !SHIMEJIFINDER_NO_LIBUNARR
    try {
        auto ar = std::make_unique<libunarr::archive>();
        ar->open(input);
        return ar;
    }
    catch (std::exception &ex) {
        std::cerr << "libunarr: open(): " << ex.what() << std::endl;
    }
    #endif
    throw std::runtime_error("failed to open archive");
}

class analyzer {
private:
    std::string m_name;
    archive *m_ar;
    analyze_config m_config;

    struct unparsed_xml_pair {
        archive_entry *actions;
        archive_entry *behaviors;
        archive_folder *root;
    };

    static std::set<std::string> find_paths(
        std::string const& actions_xml);
    void analyze();
public:
    void analyze(std::string const& name, archive *ar,
        analyze_config const& config);
};

static archive_entry *find_file(archive_folder *folder,
    std::vector<std::string> const& names)
{
    archive_entry *entry = nullptr;
    for (size_t i=0; entry == nullptr && i<names.size(); ++i) {
        entry = folder->entry_named(names[i]);
    }
    return entry;
}

std::set<std::string> analyzer::find_paths(
    std::string const& actions_xml)
{
    try {
        pugi::xml_document doc;
        doc.load_string(actions_xml.c_str(), pugi::parse_default);
        auto mascot = doc.child("Mascot");
        if (mascot == nullptr)
            mascot = doc.child("マスコット");
        if (mascot == nullptr) {
            std::cerr << "shimejifinder: not a mascot file" << std::endl;
            return {};
        }
        
        // find file names for referenced images and sounds
        std::set<std::string> paths;
        std::vector<pugi::xml_node> search_next = { mascot };
        while (!search_next.empty()) {
            size_t size = search_next.size();
            for (size_t i=0; i<size; ++i) {
                pugi::xml_node node = search_next[i];
                auto name = node.name();
                
                if (name != NULL && (strcmp(name, "Pose") == 0 ||
                    strcmp(name, "ポーズ") == 0))
                {
                    auto attr = node.attribute("画像");
                    if (!attr.empty())
                        paths.insert(attr.as_string());

                    attr = node.attribute("Image");
                    if (!attr.empty())
                        paths.insert(attr.as_string());

                    attr = node.attribute("ImageRight");
                    if (!attr.empty())
                        paths.insert(attr.as_string());

                    attr = node.attribute("Sound");
                    if (!attr.empty())
                        paths.insert(attr.as_string());
                }
                else {
                    // breadth-first search
                    for (auto child : node.children()) {
                        if (child.type() == pugi::xml_node_type::node_element) {
                            search_next.push_back(child);
                        }
                    }
                }
            }
        }

        return paths;
    }
    catch (std::exception &ex) {
        std::cerr << "shimejifinder: find_paths() failed: " << ex.what() << std::endl;
    }
    catch (...) {
        std::cerr << "shimejifinder: find_paths() failed" << std::endl;
    }
    return {};
}

void analyzer::analyze() {
    std::vector<unparsed_xml_pair> unparsed;
    archive_folder root { *m_ar };
    std::vector<archive_folder *> search_next = { &root };

    // find actions/behaviors pairs
    while (!search_next.empty()) {
        size_t size = search_next.size();
        for (size_t i=0; i<size; ++i) {
            auto folder = search_next[i];

            // breadth-first search
            for (auto subfolder_pair : folder->folders()) {
                search_next.push_back(&subfolder_pair.second);
            }

            // find behaviors file
            static const std::vector<std::string> behaviours_names =
                { "行動.xml", "behaviors.xml", "behavior.xml" };
            archive_entry *behaviors = find_file(folder, behaviours_names);
            if (behaviors == nullptr) {
                continue;
            }
            
            // find actions file
            static const std::vector<std::string> behaviours_names =
                { "動作.xml", "actions.xml", "action.xml" };
            archive_entry *actions = find_file(folder, behaviours_names);
            if (actions == nullptr) {
                continue;
            }

            // there is a shimeji here, actions file will be extracted in
            // the next step
            unparsed_xml_pair pair;
            pair.actions = actions;
            pair.behaviors = behaviors;
            pair.root = folder;
            unparsed.push_back(pair);
        }
        search_next.erase(search_next.begin(), search_next.begin() + size);
    }

    // extract actions
    memory_extractor extractor;
    for (size_t i=0; i<unparsed.size(); ++i) {
        unparsed[i].actions->add_target({ std::to_string(i) });
    }
    m_ar->extract(&extractor);

    // determine which shimeji to extract
    for (size_t i=0; i<unparsed.size(); ++i) {
        auto &unparsed_pair = unparsed[i];
        unparsed_pair.actions->clear_targets();
        auto &actions_xml = extractor.data(std::to_string(i));

        // find paths referenced in the xml
        auto paths = find_paths(actions_xml);
        if (paths.size() == 0) {
            continue;
        }
        
        // associate these xml files with image files and shimeji
        //TODO
    }
}

void analyzer::analyze(std::string const& name, archive *ar,
    analyze_config const& config)
{
    m_name = name;
    m_ar = ar;
    m_config = config;

    analyze();
}

std::unique_ptr<archive> analyze(std::string const& name, std::string const& filename,
    analyze_config const& config)
{
    auto ar = open_archive(filename);
    analyzer{}.analyze(name, ar.get(), config);
    return ar;
}

std::unique_ptr<archive> analyze(std::string const& filename,
    analyze_config const& config)
{
    return analyze(std::filesystem::path(filename)
        .replace_extension().filename().string(), filename, config);
}

std::unique_ptr<archive> analyze(std::string const& name, std::function<FILE *()> file_open,
    analyze_config const& config)
{
    auto ar = open_archive(file_open);
    analyzer{}.analyze(name, ar.get(), config);
    return ar;
}

std::unique_ptr<archive> analyze(std::string const& name, std::function<int ()> file_open,
    analyze_config const& config)
{
    return analyze(name, [file_open](){
        return fdopen(file_open(), "rb");
    }, config);
}

}
