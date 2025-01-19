#pragma once
#include <string>
#include <vector>
#include "extract_target.hpp"

namespace shimejifinder {

/*

@interface SHPArchiveEntry : NSObject
@property (nonatomic, assign, readonly) int index;
@property (nonatomic, strong, readonly) NSString *name;
@property (nonatomic, strong) NSMutableArray<NSString *> *extractPaths;
- (void)extractToFolder:(NSString *)folderPath lowercaseFilename:(BOOL)lowercase;
- (instancetype)initWithName:(NSString *)name index:(int)index;
- (void)extractTo:(NSString *)filePath;
- (BOOL)willExtract;
@end

*/

class archive_entry {
private:
    bool m_valid;
    int m_index;
    std::string m_path;
    std::string m_lowername;
    std::string m_dirname;
    std::vector<extract_target> m_extract_targets;
public:
    archive_entry();
    archive_entry(int index);
    archive_entry(int index, std::string const& path);
    bool valid() const;
    int index() const;
    std::vector<extract_target> const& extract_targets() const;
    std::string const& path() const;
    std::string const& lower_name() const;
    std::string dirname() const;
    std::string lower_extension() const;
    void add_target(extract_target const& target);
};

}