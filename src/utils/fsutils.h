#ifndef CLOUDRUN_HASHFS_FSUTILS_H
#define CLOUDRUN_HASHFS_FSUTILS_H

#include <proto/filesystem.pb.h>
#include <sys/stat.h>

void protoToStat(const filesystem::Stat &proto, struct stat *st);

#endif //CLOUDRUN_HASHFS_FSUTILS_H
