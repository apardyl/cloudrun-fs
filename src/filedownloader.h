#ifndef CLOUDRUN_HASHFS_FILEDOWNLOADER_H
#define CLOUDRUN_HASHFS_FILEDOWNLOADER_H

#include <string>

class FileDownloader {
public:
    int fetchFile(std::string filename, std::string saveAs);
};


#endif //CLOUDRUN_HASHFS_FILEDOWNLOADER_H
