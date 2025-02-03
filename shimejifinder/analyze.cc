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
            target.set_extract_name(entry->lower_name());
            entry->add_target(target);
            ++count;
        }
    }
    return count;
}

void extractShimejiEE(std::string const& root_path, archive &ar, std::string const& default_name) {
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
        auto image_count = extractAll(&folder, "png",
            { shimeji_name, "", extract_target::extract_type::IMAGE });
        if (image_count >= 2) {
            behaviors->add_target({ shimeji_name, "behaviors.xml",
                extract_target::extract_type::XML });
            actions->add_target({ shimeji_name, "actions.xml",
                extract_target::extract_type::XML });
            extractAll(sound, "wav",
                { shimeji_name, "", extract_target::extract_type::SOUND });
            ar.add_shimeji(shimeji_name);
        }
    }
}

void extractShimeji(std::string const& root_path, archive &ar, std::string const& default_name) {
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

    auto image_count = extractAll(img, "png",
        { name, "", extract_target::extract_type::IMAGE });
    if (image_count >= 2) {
        behaviors->add_target({ name, "behaviors.xml",
            extract_target::extract_type::XML });
        actions->add_target({ name, "actions.xml",
            extract_target::extract_type::XML });
        extractAll(sound, "wav",
            { name, "", extract_target::extract_type::SOUND });
        ar.add_shimeji(name);
    }
}

void extractImgFolder(std::string const& root_path, archive &ar, std::string const& default_name) {
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
    for (size_t i=0; i<entries.size(); ++i) {
        entries[i]->add_target({ name, entries[i]->lower_name(),
            extract_target::extract_type::IMAGE});
    }
    ar.add_default_xml_targets(name);
}

void analyze(std::string const& name, archive &ar) {
    // look for jar files
    for (size_t i=0; i<ar.size(); ++i) {
        auto entry = ar[i];
        if (entry->lower_name() == "shimeji-ee.jar") {
            extractShimejiEE(entry->dirname(), ar, name);
        }
        else if (entry->lower_name() == "shimeji.jar") {
            extractShimeji(entry->dirname(), ar, name);
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
            extractShimeji(dirname.substr(0, dirname.rfind('/')), ar, name);
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
            extractImgFolder(dirname, ar, name);
        }
    }
}

std::unique_ptr<archive> analyze(std::string const& name, std::string const& filename) {
    auto ar = open_archive(filename);
    analyze(name, *ar);
    return ar;
}

std::unique_ptr<archive> analyze(std::string const& filename) {
    return analyze(std::filesystem::path(filename)
        .replace_extension().filename().string(), filename);
}

std::unique_ptr<archive> analyze(std::string const& name, std::function<FILE *()> file_open) {
    auto ar = open_archive(file_open);
    analyze(name, *ar);   
    return ar;
}

std::unique_ptr<archive> analyze(std::string const& name, std::function<int ()> file_open) {
    return analyze(name, [file_open](){
        return fdopen(file_open(), "rb");
    });
}

}