# cloudrun-hashfs
Read-only caching file system based on FUSE for cloudrun project.

## Requirements
* libfuse 2.9.x
* protobuf (protoc and protobuf-cpp) >= 3.0
* grpc >=1.13.0
#### For Arch linux install following packages: fuse2, protobuf and (AUR) grpc

## Install
```bash
git clone --recurse-submodules https://github.com/apardyl/cloudrun-fs.git
cd cloudrun-fs
make -j"$(nproc)"
sudo make install
```

## Usage
```
cloudrun-fs <filesystem meta file> <hash store path> <private store path> <remote address:remote port>
```
(works best with cloudrun-client and cloudrun-worker)
