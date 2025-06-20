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
#include "utils.hpp"
#include <exception>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>
#include <array>

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

bool extractFirst(archive_folder *folder, std::string const& lower_extension,
    extract_target tmpl)
{
    if (folder == nullptr) {
        return false;
    }
    std::shared_ptr<archive_entry> first_entry = nullptr;
    for (auto &pair : folder->files()) {
        auto &entry = pair.second;
        if (entry->lower_extension() == lower_extension) {
            if (entry->lower_name() == "icon.png") {
                // skip icon.png
                continue;
            }
            if (first_entry == nullptr || entry->lower_name() < first_entry->lower_name()) {
                first_entry = entry;
            }
        }
    }
    if (first_entry == nullptr) {
        return false;
    }
    auto target = tmpl;
    target.set_extract_name(first_entry->lower_name());
    first_entry->add_target(target);
    return true;
}

int extractAll(archive_folder *folder, std::string const& lower_extension,
    extract_target tmpl)
{
    if (folder == nullptr) {
        return 0;
    }
    int count = 0;
    for (auto &pair : folder->files()) {
        auto &entry = pair.second;
        if (entry->lower_extension() == lower_extension) {
            if (entry->lower_name() == "icon.png") {
                // skip icon.png
                continue;
            }
            auto target = tmpl;
            target.set_extract_name(tmpl.extract_name() + entry->lower_name());
            entry->add_target(target);
            ++count;
        }
    }
    return count;
}

void extractNestedImages(std::string const& shimeji_name, std::string &prefix,
    archive_folder &subfolder)
{
    auto orig_size = prefix.size();
    if (prefix.empty()) prefix = subfolder.lower_name();
    else prefix += "_" + subfolder.lower_name();

    // determine if this is an image folder
    bool is_image_folder = true;
    for (auto &file_pair : subfolder.files()) {
        // only allow txt and png
        if (file_pair.second->lower_extension() != "png" &&
            file_pair.second->lower_extension() != "txt")
        {
            is_image_folder = false;
            break;
        }
    }

    if (is_image_folder) {
        // extract all images in this folder
        extractAll(&subfolder, "png", { shimeji_name,
            prefix + "_", extract_target::extract_type::IMAGE });

        // explore subfolders
        for (auto &child_pair : subfolder.folders()) {
            extractNestedImages(shimeji_name, prefix, child_pair.second);
        }
    }

    prefix = prefix.substr(0, orig_size);
}


void extractNestedImages(std::string const& shimeji_name,
    archive_folder &root)
{
    std::string prefix;
    for (auto &pair : root.folders()) {
        extractNestedImages(shimeji_name, prefix, pair.second);
    }
}

void extractShimejiEE(std::string const& root_path, archive &ar, std::string const& default_name,
    analyze_config const& config)
{
    archive_folder root { ar, root_path };
    auto default_conf = root.folder_named("conf");
    auto default_sound = root.folder_named("sound");
    (void)default_sound;
    archive_entry *default_behaviors = nullptr;
    archive_entry *default_actions = nullptr;
    if (default_conf != nullptr) {
        default_behaviors = default_conf->entry_named("behaviors.xml");
        if (default_behaviors == nullptr)
            default_behaviors = default_conf->entry_named("behavior.xml");
        default_actions = default_conf->entry_named("actions.xml");
        if (default_actions == nullptr)
            default_actions = default_conf->entry_named("action.xml");
    }
    auto img = root.folder_named("img");
    if (img == nullptr) {
        std::cerr << "warning: no img folder" << std::endl;
        return;
    }
    auto find_xmls = [default_behaviors, default_actions](archive_entry *&actions,
        archive_entry *&behaviors, archive_folder &shimeji_folder)
    {
        auto conf = shimeji_folder.folder_named("conf");
        if (conf != nullptr) {
            behaviors = conf->entry_named("behaviors.xml");
            if (behaviors == nullptr)
                behaviors = conf->entry_named("behavior.xml");
            actions = conf->entry_named("actions.xml");
            if (actions == nullptr)
                actions = conf->entry_named("action.xml");
        }
        if (actions == nullptr)
            actions = default_actions;
        if (behaviors == nullptr)
            behaviors = default_behaviors;
        if (actions == nullptr || behaviors == nullptr) {
            std::cerr << "warning: missing actions or behaviors" << std::endl;
            return false;
        }
        return true;
    };
    int count = 0;
    for (auto &pair : img->folders()) {
        auto &folder = pair.second;
        if (folder.name() == "unused") {
            continue;
        }
        archive_entry *actions = nullptr, *behaviors = nullptr;
        if (!find_xmls(actions, behaviors, folder)) {
            continue;
        }
        // valid shimeji
        ++count;
    }

    for (auto &pair : img->folders()) {
        auto &folder = pair.second;
        if (folder.name() == "unused") {
            continue;
        }
        std::string shimeji_name;
        if (folder.name() == "Shimeji" && count == 1) {
            shimeji_name = default_name;
        }
        else {
            shimeji_name = folder.name();
        }
        archive_entry *actions = nullptr, *behaviors = nullptr;
        if (!find_xmls(actions, behaviors, folder)) {
            continue;
        }
        auto sound = folder.folder_named("sound");
        if (sound == nullptr)
            sound = default_sound;
        if (config.only_thumbnails) {
            extractFirst(&folder, "png", { shimeji_name, "",
                extract_target::extract_type::IMAGE });
        }
        else {
            extractAll(&folder, "png",{ shimeji_name, "",
                extract_target::extract_type::IMAGE });
            extractNestedImages(shimeji_name, folder);
            behaviors->add_target({ shimeji_name, "behaviors.xml",
                extract_target::extract_type::XML });
            actions->add_target({ shimeji_name, "actions.xml",
                extract_target::extract_type::XML });
            extractAll(sound, "wav",
                { shimeji_name, "", extract_target::extract_type::SOUND });
        }
        ar.add_shimeji(shimeji_name);
    }
}

void extractShimeji(std::string const& root_path, archive &ar, std::string const& default_name,
    analyze_config const& config)
{
    std::string name = last_component(root_path);
    if (name.empty() || name == "Shimeji") {
        name = default_name;
    }
    archive_folder root { ar, root_path };
    auto img = root.folder_named("img");
    if (img == nullptr) {
        img = &root;
    }
    auto sound = root.folder_named("sound");
    auto conf = root.folder_named("conf");

    if (conf == nullptr) {
        std::cerr << "missing conf" << std::endl;
        return;
    }

    auto behaviors = conf->entry_named("行動.xml");
    if (behaviors == nullptr)
        behaviors = conf->entry_named("behaviors.xml");
    if (behaviors == nullptr)
        behaviors = conf->entry_named("behavior.xml");
    
    auto actions = conf->entry_named("動作.xml");
    if (actions == nullptr)
        actions = conf->entry_named("actions.xml");
    if (actions == nullptr)
        actions = conf->entry_named("action.xml");

    if (behaviors == nullptr || actions == nullptr) {
        std::cerr << "missing actions or behaviors" << std::endl;
        return;
    }

    if (config.only_thumbnails) {
        extractFirst(img, "png", { name, "",
            extract_target::extract_type::IMAGE });
    }
    else {
        extractAll(img, "png", { name, "", extract_target::extract_type::IMAGE });
        extractNestedImages(name, *img);
        behaviors->add_target({ name, "behaviors.xml",
            extract_target::extract_type::XML });
        actions->add_target({ name, "actions.xml",
            extract_target::extract_type::XML });
        extractAll(sound, "wav",
            { name, "", extract_target::extract_type::SOUND });
    }
    ar.add_shimeji(name);
}

void extractImgFolder(std::string const& root_path, archive &ar, std::string const& default_name,
    analyze_config const& config)
{
    std::string name = last_component(root_path);
    if (name.empty() || name == "Shimeji") {
        name = default_name;
    }
    archive_folder root { ar, root_path };
    std::array<archive_entry *, 46> entries;
    if (root.entry_named("shime47.png") != nullptr) {
        // shime47.png is not expected to exist
        return;
    }
    for (size_t i=1; i<=entries.size(); ++i) {
        auto shime = "shime" + std::to_string(i) + ".png";
        auto entry = root.entry_named(shime);
        if (entry == nullptr) {
            return;
        }
        entries[i-1] = entry;
    }
    ar.add_shimeji(name);
    for (size_t i=0, count=(config.only_thumbnails ? 1 : entries.size());
        i<count; ++i)
    {
        entries[i]->add_target({ name, entries[i]->lower_name(),
            extract_target::extract_type::IMAGE});
    }
    ar.add_default_xml_targets(name);
}

void analyze(std::string const& name, archive &ar, analyze_config const& config) {
    // look for jar files
    for (size_t i=0; i<ar.size(); ++i) {
        auto entry = ar[i];
        if (entry->lower_name() == "shimeji-ee.jar") {
            extractShimejiEE(entry->dirname(), ar, name, config);
        }
        else if (entry->lower_name() == "shimeji.jar") {
            extractShimeji(entry->dirname(), ar, name, config);
        }
    }

    // look for xml files
    for (size_t i=0; i<ar.size(); ++i) {
        auto entry = ar[i];
        if (!entry->extract_targets().empty()) {
            continue;
        }
        if (entry->lower_name() == "actions.xml") {
            auto dirname = entry->dirname();
            auto last_comp = dirname.substr(dirname.rfind('/')+1);
            if (last_comp != "conf") {
                continue;
            }
            extractShimeji(dirname.substr(0, dirname.rfind('/')), ar, name, config);
        }
    }

    // look for shime1.png
    for (size_t i=0; i<ar.size(); ++i) {
        auto entry = ar[i];
        if (!entry->extract_targets().empty()) {
            continue;
        }
        if (entry->lower_name() == "shime1.png") {
            auto dirname = entry->dirname();
            extractImgFolder(dirname, ar, name, config);
        }
    }
}

std::unique_ptr<archive> analyze(std::string const& name, std::string const& filename,
    analyze_config const& config)
{
    auto ar = open_archive(filename);
    analyze(name, *ar, config);
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
    analyze(name, *ar, config);
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
