#include <memory>

#include "remote_fs_connection.h"
#include "common/config.h"
#include <grpc/grpc.h>
#include <grpc++/channel.h>
#include <grpc++/client_context.h>
#include <grpc++/create_channel.h>
#include <grpc++/security/credentials.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include "common/fs_utils.h"
#include "common/proto/filesystem.pb.h"

using namespace remotefs;
using grpc::ClientContext;
using grpc::Status;
using grpc::StatusCode;

RemoteFSConnection::RemoteFSConnection(const std::string &server_address) {
    auto channel = grpc::CreateChannel(server_address, grpc::InsecureChannelCredentials());
    stub = RemoteFS::NewStub(channel);
}

int RemoteFSConnection::get_checksum(const std::string &filename, std::string *checksum) {
    ClientContext context;
    FileRequest request;
    request.set_path(filename);
    ChecksumResponse response;
    Status status = stub->GetChecksum(&context, request, &response);
    if (status.ok()) {
        *checksum = response.checksum();
        return 0;
    }
    errno = status_to_errno(status);
    return status.error_code();
}

int RemoteFSConnection::get_stat(const std::string &filename, filesystem::Stat *st) {
    ClientContext context;
    FileRequest request;
    request.set_path(filename);
    Status status = stub->GetStat(&context, request, st);
    errno = status_to_errno(status);
    return status.error_code();
}


int RemoteFSConnection::get_stat(const std::string &filename, struct stat *st) {
    filesystem::Stat response;
    int res = get_stat(filename, &response);
    if (res == 0) {
        protoToStat(response, st);
    }
    return res;
}


int RemoteFSConnection::get_dir(const std::string &path, filesystem::Node *node) {
    ClientContext context;
    FileRequest request;
    request.set_path(path);
    FileListResponse response;
    Status status = stub->ListDirectory(&context, request, &response);
    if (!status.ok()) {
        return status_to_errno(status);
    }
    auto &children = *(node->mutable_children());
    for (auto &item : response.item()) {
        debug_print("Adding item %s to %s\n", item.name().c_str(), path.c_str());
        auto n = filesystem::Node();
        n.set_partial(true);
        if (!item.target().empty()) {
            n.set_target(item.target());
        }
        *n.mutable_stat() = item.stat();
        children[item.name()] = std::move(n);
    }
    return 0;
}

bool RemoteFSConnection::create_base_dir(const std::string &filename) {
    size_t pos = 0;
    while (pos < std::string::npos) {
        size_t next = filename.find('/', pos + 1);
        if (pos + 1 < next && next < std::string::npos) {
            std::string path = filename.substr(0, next);
            int res = mkdir(path.c_str(), S_IRWXU);
            if (res != 0 && errno != EEXIST) {
                return false;
            }
        }
        pos = next;
    }
    return true;
}

int RemoteFSConnection::fetch_file(const std::string &filename, const std::string &save_as) {
    LockMapGuard<std::string> guard(lock_map, filename);
    if (access(save_as.c_str(), F_OK) == 0) {
        return 0;
    }

    if (!create_base_dir(save_as)) {
        debug_print("Unable to create base directory for file %s errno: %d\n", save_as.c_str(), errno);
        return -1;
    }

    int fd = open(save_as.c_str(), O_WRONLY | O_CREAT | O_EXCL, S_IRWXU);
    if (fd == -1) {
        debug_print("Unable to create file %s errno: %d\n", save_as.c_str(), errno);
        return -1;
    }

    bool error = false;

    ClientContext context;
    FileRequest request;
    request.set_path(filename);
    DataChunkResponse response;
    auto reader = stub->GetData(&context, request);

    ssize_t res;
    while (reader->Read(&response) && !error) {
        size_t offset = 0;
        do {
            res = write(fd, response.data().data() + offset, response.data().size() - offset);
            if (res < 0 || (static_cast<unsigned long int>(res) < response.data().size() &&
                            errno != EINTR)) {
                debug_print("Error writing to file %s errno: %d\n", save_as.c_str(), errno);
                context.TryCancel();
                error = true;
                break;
            }
            offset += res;
        } while (offset < response.data().size() && errno == EINTR);
    }

    if (close(fd)) {
        error = true;
        debug_print("Error closing file %s errno: %d\n", save_as.c_str(), errno);
    }
    Status status = reader->Finish();
    if (!status.ok()) {
        error = true;
        debug_print("rpc error: %s: %s\n", status.error_message().c_str(), status.error_details().c_str());
    }
    if (error) {
        if (unlink(save_as.c_str())) {
            debug_print("Error removing file %s errno: %d\n", save_as.c_str(), errno);
        }
        return -1;
    }
    return 0;
}

int RemoteFSConnection::status_to_errno(const Status &status) {
    if (status.error_code() == StatusCode::OK) {
        return 0;
    } else if (status.error_code() == StatusCode::NOT_FOUND) {
        return ENOENT;
    } else if (status.error_code() == StatusCode::INVALID_ARGUMENT) {
        return ENOTDIR;
    } else if (status.error_code() == StatusCode::PERMISSION_DENIED) {
        return EACCES;
    } else if (status.error_code() == StatusCode::INTERNAL) {
        return EACCES;
    } else {
        return EACCES;
    }
}
