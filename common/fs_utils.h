#ifndef CLOUDRUN_FS_FS_UTILS_H
#define CLOUDRUN_FS_FS_UTILS_H

#include "common/proto/filesystem.pb.h"
#include <sys/stat.h>

void protoToStat(const filesystem::Stat &proto, struct stat *st);

#endif //CLOUDRUN_FS_FS_UTILS_H
