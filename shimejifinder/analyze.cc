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
#include <cstring>
#include <algorithm>

namespace shimejifinder {

static const std::vector<std::string> k_behaviors_names =
    { "行動.xml", "behaviors.xml", "behavior.xml", "two.xml", "2.xml" };
static const std::vector<std::string> k_actions_names =
    { "動作.xml", "actions.xml", "action.xml", "one.xml", "1.xml" };

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
        const archive_folder *root;
    };

    static std::set<std::string> find_paths(
        std::string const& actions_xml);
    static void add_search_paths(std::vector<const archive_folder *>
        &search_paths, const archive_folder *base);
    bool register_shimeji(const archive_folder *base,
        archive_entry *actions, archive_entry *behaviors,
        std::set<std::string> const& paths,
        const archive_folder *alternative_base = nullptr);
    size_t discover_shimejiee(const archive_folder *img,
        archive_entry *actions, archive_entry *behaviors,
        std::set<std::string> const& paths);
    std::string shimeji_name(const archive_folder *base);
    void analyze();
public:
    void analyze(std::string const& name, archive *ar,
        analyze_config const& config);
};

static archive_entry *find_file(const archive_folder *folder,
    std::vector<std::string> const& names)
{
    archive_entry *entry = nullptr;
    for (size_t i=0; entry == nullptr && i<names.size(); ++i) {
        entry = folder->entry_named(names[i]);
    }
    return entry;
}

size_t analyzer::discover_shimejiee(const archive_folder *img,
    archive_entry *actions, archive_entry *behaviors,
    std::set<std::string> const& paths)
{
    size_t associated = 0;
    for (auto const& pair : img->folders()) {
        auto folder = &pair.second;
        if (folder->lower_name() == "unused") {
            // unused/ folder is ignored
            continue;
        }
        if (find_file(folder, k_actions_names) != nullptr ||
            find_file(folder, k_behaviors_names) != nullptr)
        {
            // this shimeji has its own configuration files
            continue;
        }
        
        bool found = register_shimeji(folder, actions, behaviors,
            paths, img);
        if (found) {
            ++associated;
        }
    }
    return associated;
}

static std::string normalize_filename(std::string name) {
    // "///my/images/shime1.png" ==> "my_images_shime1.png"
    size_t i=0;
    while (i < name.size() && name[i] == '/')
        ++i;
    name = name.substr(i);
    i = 0;
    while ((i = name.find('/', i)) != std::string::npos)
        name[i++] = '_';
    return to_lower(name);
}

void analyzer::add_search_paths(std::vector<const archive_folder *>
    &search_paths, const archive_folder *base)
{
    search_paths.push_back( base );
    search_paths.push_back( base->folder_named("img") );
    search_paths.push_back( base->folder_named("sound") );
    search_paths.push_back( base->parent() );
    search_paths.push_back( base->parent()->folder_named("img") );
    search_paths.push_back( base->parent()->folder_named("sound") );
}

bool analyzer::register_shimeji(const archive_folder *base,
    archive_entry *actions, archive_entry *behaviors,
    std::set<std::string> const& paths,
    const archive_folder *alternative_base)
{
    auto name = shimeji_name(base);
    std::vector<const archive_folder *> search_paths;
    add_search_paths(search_paths, base);
    if (alternative_base != nullptr) {
        add_search_paths(search_paths, alternative_base);
    }
    std::vector<std::pair<archive_entry *, std::string>> targets(paths.size(),
        { nullptr, "" });
    bool has_images = false;
    for (auto search : search_paths) {
        if (search == nullptr) {
            continue;
        }
        auto path_iter = paths.cbegin();
        for (size_t j=0; j<targets.size(); ++j, ++path_iter) {
            auto &file = targets[j];
            if (file.first != nullptr) {
                continue;
            }
            auto &path = *path_iter;
            auto entry = search->relative_file(path);
            if (entry != nullptr) {
                file.first = entry;
                file.second = normalize_filename(path);
                if (file.first->lower_extension() == "png") {
                    has_images = true;
                }
            }
        }
    }
    if (!has_images) {
        return false;
    }
    for (auto target : targets) {
        if (target.first == nullptr) {
            continue;
        }
        extract_target::extract_type type;
        auto entry = target.first;
        auto &normalized_path = target.second;
        if (entry->lower_extension() == "png") {
            type = extract_target::extract_type::IMAGE;
        }
        else /* if (entry->lower_extension() == "wav") */ {
            type = extract_target::extract_type::SOUND;
        }
        entry->add_target({ name, normalized_path, type });
    }
    actions->add_target({ name, "actions.xml",
        extract_target::extract_type::XML });
    behaviors->add_target({ name, "behaviors.xml",
        extract_target::extract_type::XML });
    m_ar->add_shimeji(name);
    return true;
}

std::string analyzer::shimeji_name(const archive_folder *base) {
    static std::set<std::string> blacklist = {
        "img", "conf", "shimeji", "unused", "shimeji-ee",
        "src", "/" };
    const archive_folder *cwd = base;
    while (blacklist.count(cwd->lower_name()) == 1) {
        const archive_folder *parent = cwd->parent();
        if (parent == cwd) {
            // could not find a unique name
            return m_name;
        }
        cwd = parent;
    }
    return cwd->name();
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
                    static const std::vector<std::string> attr_names =
                        { "画像", "Image", "ImageRight", "Sound" };
                    for (auto &attr_name : attr_names) {
                        auto attr = node.attribute(attr_name);
                        if (!attr.empty()) {
                            paths.insert(to_lower(attr.as_string()));
                        }
                    }
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
            search_next.erase(search_next.begin(),
                search_next.begin() + size);
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
    std::vector<const archive_folder *> search_next = { &root };
    std::vector<const archive_folder *> shime1_roots;

    // find actions/behaviors pairs and shime1.png files
    while (!search_next.empty()) {
        size_t size = search_next.size();
        for (size_t i=0; i<size; ++i) {
            auto folder = search_next[i];

            // breadth-first search
            for (auto &subfolder_pair : folder->folders()) {
                search_next.push_back(&subfolder_pair.second);
            }

            // find shime1.png
            archive_entry *shime1 = folder->entry_named("shime1.png");
            if (shime1 != nullptr) {
                shime1_roots.push_back(folder);
            }

            // find behaviors file
            archive_entry *behaviors = find_file(folder, k_behaviors_names);
            if (behaviors == nullptr) {
                continue;
            }
            
            // find actions file
            archive_entry *actions = find_file(folder, k_actions_names);
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

    // determine which shimeji to extract based on these actions
    for (size_t i=0; i<unparsed.size(); ++i) {
        auto &unparsed_pair = unparsed[i];
        unparsed_pair.actions->clear_targets();
        auto &actions_xml = extractor.data(std::to_string(i));

        // find paths referenced in the xml
        auto paths = find_paths(actions_xml);
        if (paths.size() == 0) {
            continue;
        }

        // if this folder is named conf and ../img exists, then
        // this configuration may belong to multiple shimeji
        size_t has_associated = 0;
        if (unparsed_pair.root->lower_name() == "conf") {
            auto img = unparsed_pair.root->parent()->folder_named("img");
            if (img != nullptr) {
                has_associated = discover_shimejiee(img,
                    unparsed_pair.actions, unparsed_pair.behaviors, paths);
            }
        }

        if (has_associated == 0) {
            register_shimeji(unparsed_pair.root,
                unparsed_pair.actions, unparsed_pair.behaviors,
                paths);
        }
    }

    // check shime1.png files to discover shimeji that do not have
    // associated configuration files
    std::array<archive_entry *, 46> shimes;
    for (auto shime1_root : shime1_roots) {
        size_t i;
        for (i=0; i<46; ++i) {
            auto shime = shime1_root->entry_named("shime" +
                std::to_string(i+1) + ".png");
            if (shime == nullptr) {
                break;
            }
            if (shime->extract_targets().size() > 0) {
                break;
            }
            shimes[i] = shime;
        }
        if (i != 46) {
            continue;
        }
        if (shime1_root->entry_named("shime47.png") != nullptr) {
            continue;
        }
        auto name = shimeji_name(shime1_root);
        for (i=0; i<46; ++i) {
            shimes[i]->add_target({ name, "shime" + std::to_string(i+1) + ".png",
                extract_target::extract_type::IMAGE });
            m_ar->add_default_xml_targets(name);
        }
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
