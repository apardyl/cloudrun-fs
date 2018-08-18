#include <memory>

#include "remotefsconnection.h"
#include "config.h"
#include <grpc/grpc.h>
#include <grpcpp/channel.h>
#include <grpcpp/client_context.h>
#include <grpcpp/create_channel.h>
#include <grpcpp/security/credentials.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>

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
    {
        std::unique_lock<std::mutex> lock(download_mutex);
        if (concurrent_downloads.count(filename) == 0) {
            concurrent_downloads[filename] = std::make_unique<std::condition_variable>();
        } else {
            do {
                concurrent_downloads[filename]->wait(lock);
            } while (concurrent_downloads.count(filename) != 0);
        }
    }

    // Check if the file already exists before attempting download.
    if (access(save_as.c_str(), F_OK) == 0) {
        return 0;
    }

    if (!create_base_dir(save_as)) {
        debug_print("Unable to base directory for file %s errno: %d\n", save_as.c_str(), errno);
        return -1;
    }

    std::string tmp_filename = save_as + ".part";
    int fd = open(tmp_filename.c_str(), O_WRONLY | O_CREAT | O_EXCL, S_IRWXU);
    if (fd == -1) {
        debug_print("Unable to create file %s errno: %d\n", tmp_filename.c_str(), errno);
        return -1;
    }

    ClientContext context;
    FileRequest request;
    request.set_path(filename);
    DataChunkResponse response;
    auto reader = stub->GetData(&context, request);
    bool error = false;
    ssize_t res;
    size_t offset = 0;
    while (reader->Read(&response)) {
        do {
            res = write(fd, response.data().data() + offset, response.data().size() - offset);
            if (res == -1 || (res < response.data().size() && errno != EINTR)) {
                debug_print("Error writing to file %s errno: %d\n", tmp_filename.c_str(), errno);
                context.TryCancel();
                error = true;
                break;
            }
            offset += res;
        } while (offset < response.data().size() && errno == EINTR);
    }

    if (close(fd)) {
        error = true;
        debug_print("Error closing file %s errno: %d\n", tmp_filename.c_str(), errno);
    }
    Status status = reader->Finish();
    if (!status.ok()) {
        error = true;
        if (!status.ok()) {
            debug_print("rpc error: %s: %s\n", status.error_message().c_str(), status.error_details().c_str());
        }
    }
    if (error) {
        if (unlink(tmp_filename.c_str())) {
            error = true;
            debug_print("Error removing file %s errno: %d\n", tmp_filename.c_str(), errno);
        }
    } else {
        if (rename(tmp_filename.c_str(), save_as.c_str())) {
            error = true;
            debug_print("Error moving file %s to %s errno: %d\n", tmp_filename.c_str(), save_as.c_str(), errno);
        }
    }

    {
        std::unique_lock<std::mutex> lock(download_mutex);
        concurrent_downloads[filename]->notify_all();
        concurrent_downloads.erase(filename);
    }
    return error ? -1 : 0;
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
