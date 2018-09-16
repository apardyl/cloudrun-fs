# cloudrun-hashfs
Read-only caching file system based on FUSE for cloudrun project.

## Requirements
* libfuse 2.9.x
* bazel (build-only)

## Build
* Build the client:
```
bazel build client:cloudrun-fs
```
* Build the server:
```
bazel build server:cloudrun-fs-server
```
Binaries are placed in the `bazel-bin/` directory.
## Usage
* Mount file system: 
```
cloudrun-fs <filesystem meta file> <hash store path> <private store path> <remote address:remote port>
```
* Run server:
```
cloudrun-fs-server serve <IP:PORT OR unix://<path>>
```
* Scan filesystem:
```
cloudrun-fs-server scan <save file name> <list of  directories to scan>
```
(works best with cloudrun-client and cloudrun-worker)
