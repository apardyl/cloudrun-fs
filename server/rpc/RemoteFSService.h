#ifndef CLOUDRUN_CLIENT_REMOTEFSSERVICE_H
#define CLOUDRUN_CLIENT_REMOTEFSSERVICE_H

#include "common/proto/remotefs.grpc.pb.h"

class RemoteFSService : public remotefs::RemoteFS::Service {
    grpc::Status errno_to_status(int code);
public:
    grpc::Status GetLink(::grpc::ServerContext *context, const ::remotefs::FileRequest *request,
                         ::remotefs::LinkResponse *response) override;

    grpc::Status GetChecksum(::grpc::ServerContext *context, const ::remotefs::FileRequest *request,
                             ::remotefs::ChecksumResponse *response) override;

    grpc::Status GetStat(::grpc::ServerContext *context, const ::remotefs::FileRequest *request,
                         ::filesystem::Stat *response) override;

    grpc::Status GetData(::grpc::ServerContext *context, const ::remotefs::FileRequest *request,
                         ::grpc::ServerWriter<::remotefs::DataChunkResponse> *writer) override;

    grpc::Status ListDirectory(::grpc::ServerContext *context, const ::remotefs::FileRequest *request,
                               ::remotefs::FileListResponse *response) override;

    grpc::Status PutFile(::grpc::ServerContext *context, ::grpc::ServerReader<::remotefs::DataChunkRequest> *reader,
                         ::remotefs::Empty *response) override;
};

#endif //CLOUDRUN_CLIENT_REMOTEFSSERVICE_H
