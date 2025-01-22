#pragma once
#include <vector>
#include "archive_entry.hpp"
#include "extract_target.hpp"
#include <functional>
#include <set>
#include <memory>
#include <filesystem>

namespace shimejifinder {

/*

#import "SHPArchiveEntry.h"

@interface SHPArchive : NSObject {
@protected
    NSURL *_url;
    NSArray<SHPArchiveEntry *> *_entries;
}
@property (getter=url, readonly) NSURL *url;
@property (getter=entries, readonly) NSArray<SHPArchiveEntry *> *entries;
+ (instancetype)archiveWithURL:(NSURL *)url;
- (NSArray<SHPArchiveEntry *> *)entries;
- (NSURL *)url;
- (instancetype)initWithURL:(NSURL *)url; // Do not call directly
- (void)extract;
@end

*/

class archive {
private:
    std::function<FILE *()> m_file_open;
    std::vector<std::shared_ptr<archive_entry>> m_entries;
    std::set<std::string> m_shimejis;
    std::filesystem::path m_output_path;
    std::vector<std::ofstream> m_active_writes;
    void init();
protected:
    void begin_write(extract_target const& entry);
    void write_next(size_t offset, const void *buf, size_t size);
    void end_write();
    void revert_to_index(int idx);
    void add_entry(archive_entry const& entry);
    void write_target(extract_target const& target, uint8_t *buf, size_t size);
    virtual void fill_entries(FILE *file);
    virtual void extract(FILE *file);
public:
    archive();
    size_t size() const;
    std::shared_ptr<archive_entry> operator[](size_t i);
    std::shared_ptr<archive_entry> operator[](size_t i) const;
    std::shared_ptr<archive_entry> at(size_t i);
    std::shared_ptr<archive_entry> at(size_t i) const;
    std::set<std::string> const& shimejis();
    void add_shimeji(std::string const& shimeji);
    void open(std::function<FILE *()> file_open);
    void open(std::string const& filename);
    void extract(std::filesystem::path output);
    void close();
    virtual ~archive();
};

}