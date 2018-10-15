#include "RemoteFSService.h"
#include "common/sha256.h"

#include <grpc/grpc.h>
#include <sys/stat.h>
#include <dirent.h>
#include <unistd.h>
#include <fcntl.h>
#include "common/config.h"
#include "server/scanfs/scanfs.h"

using grpc::Status;
using grpc::StatusCode;
using namespace remotefs;

grpc::Status RemoteFSService::GetChecksum(::grpc::ServerContext *context, const ::remotefs::FileRequest *request,
                                          ::remotefs::ChecksumResponse *response) {
    debug_print("GetChecksum: %s\n", request->path().c_str());
    int res = sha256_file(request->path(), response->mutable_checksum());
    return errno_to_status(res);
}

Status RemoteFSService::GetLink(::grpc::ServerContext *context, const ::remotefs::FileRequest *request,
                                ::remotefs::LinkResponse *response) {
    debug_print("GetLink: %s\n", request->path().c_str());
    response->set_target(read_link(request->path()));
    return response->target().empty() ? errno_to_status(errno) : Status::OK;
}

grpc::Status RemoteFSService::GetStat(::grpc::ServerContext *context, const ::remotefs::FileRequest *request,
                                      ::filesystem::Stat *response) {
    debug_print("GetStat: %s\n", request->path().c_str());
    struct stat st{};
    if (lstat(request->path().c_str(), &st) != 0) {
        return errno_to_status(errno);
    }
    stat_to_proto_changed(&st, response);
    return Status::OK;
}

grpc::Status RemoteFSService::GetData(::grpc::ServerContext *context, const ::remotefs::FileRequest *request,
                                      ::grpc::ServerWriter<::remotefs::DataChunkResponse> *writer) {
    debug_print("GetData %s\n", request->path().c_str());
    int fd = open(request->path().c_str(), O_RDONLY);
    if (fd == -1) {
        return errno_to_status(fd);
    }
    char buf[DATA_CHUNK_SIZE];
    ssize_t res;
    while ((res = read(fd, buf, DATA_CHUNK_SIZE))) {
        if (res < 0) {
            close(fd);
            return errno_to_status(errno);
        }
        DataChunkResponse response;
        response.set_data(buf, static_cast<size_t>(res));
        if (!writer->Write(response)) {
            debug_print("Broken stream for %s\n", request->path().c_str());
            close(fd);
            return Status::CANCELLED;
        }
    }
    close(fd);
    return Status::OK;
}

grpc::Status RemoteFSService::ListDirectory(::grpc::ServerContext *context, const ::remotefs::FileRequest *request,
                                            ::remotefs::FileListResponse *response) {
    debug_print("ListDirectory: %s\n", request->path().c_str());
    DIR *dir = opendir(request->path().c_str());
    if (dir == nullptr) {
        return errno_to_status(errno);
    }
    dirent *d;
    while ((d = readdir(dir))) {
        if ((strcmp(d->d_name, ".") == 0) || (strcmp(d->d_name, "..") == 0)) {
            continue;
        }
        FileListResponse::DirEntity *entity = response->add_item();
        entity->set_name(d->d_name);
        struct stat st{};
        std::string p = request->path() + "/" + d->d_name;
        if (lstat(p.c_str(), &st) == 0) {
            stat_to_proto_changed(&st, entity->mutable_stat());
            if (S_ISLNK(st.st_mode)) {
                entity->set_target(read_link(p));
            } else if (S_ISREG(st.st_mode)) {
                entity->set_target(p.c_str());
            }
        }
    }
    closedir(dir);
    return Status::OK;
}

grpc::Status
RemoteFSService::PutFile(::grpc::ServerContext *context, ::grpc::ServerReader<::remotefs::DataChunkRequest> *reader,
                         ::remotefs::Empty *response) {
    return Status(StatusCode::UNIMPLEMENTED, "unimplemented");
}

grpc::Status RemoteFSService::errno_to_status(int code) {
    switch (code) {
        case 0:
            return Status::OK;
        case ENOENT:
            return Status(StatusCode::NOT_FOUND, "ENOENT");
        case ENOTDIR:
            return Status(StatusCode::INVALID_ARGUMENT, "ENOTDIR");
        case EACCES:
            return Status(StatusCode::PERMISSION_DENIED, "EACCES");
        default:
            return Status(StatusCode::INTERNAL, "errno: " + std::to_string(code));
    }
}
