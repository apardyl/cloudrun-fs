#include "remotefsconnection.h"
#include <grpc/grpc.h>
#include <grpcpp/channel.h>
#include <grpcpp/client_context.h>
#include <grpcpp/create_channel.h>
#include <grpcpp/security/credentials.h>

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

int RemoteFSConnection::fetch_file(const std::string &filename, const std::string &save_as) {
    return -1;
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
