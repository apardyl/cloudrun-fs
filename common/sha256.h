#ifndef CLOUDRUN_FS_UTILS_SHA256_H
#define CLOUDRUN_FS_UTILS_SHA256_H

#include <string>

int sha256_file(const std::string& path, std::string* checksum);

#endif //CLOUDRUN_FS_UTILS_SHA256_H
