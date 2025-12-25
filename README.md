# S3ClFS - S3 Cluster FileSystem

> **Historical Archive**: This repository contains code from 2015-2016 and is preserved for archeological purposes only. It is not maintained and should not be used in production.

A distributed, cache-enabled file system that uses Amazon S3 as backend storage, with support for multiple hosts accessing simultaneously via SNS/SQS cluster synchronization.

Licensed under GPLv3. See attached LICENSE file for details.

## Overview

S3ClFS was developed between February 2015 and November 2016 as a FUSE-based filesystem that allows mounting an S3 bucket as a local directory. Key features included:

- **Distributed access**: Multiple hosts could simultaneously access the same S3 bucket as a shared filesystem
- **Cluster synchronization**: Used AWS SNS/SQS for real-time notification of changes between nodes
- **Intelligent caching**: Dual-layer caching with LMDB for metadata and optional block-level data cache
- **Decentralized inode allocation**: Used microsecond-precision timestamps as inode numbers to avoid collisions
- **Built-in fsck**: Consistency checker for detecting orphaned inodes and unused data blocks

## Architecture

### Core Components

| Component | Files | Description |
|-----------|-------|-------------|
| FUSE Integration | `QtFuse.*`, `S3Fuse.*` | Custom FUSE bindings with Qt event loop integration |
| Filesystem Operations | `S3FS.*` | Main orchestrator for file/directory operations |
| Storage Layer | `S3FS_Store.*`, `S3FS_Obj.*` | Local cache and S3 storage management |
| AWS Integration | `S3FS_Aws*.*` | Hand-rolled S3, SNS, SQS clients with AWS Signature V2/V4 |
| Local Cache | `Keyval.*` | LMDB wrapper for metadata and block caching |
| Control Interface | `S3FS_Control.*` | Unix socket management interface with JSON protocol |

### Technical Details

- **Block size**: 65KB (`S3FUSE_BLOCK_SIZE`)
- **Cache database**: Up to 2GB LMDB storage (configurable)
- **Cluster nodes**: Support for 1-100 nodes with unique node IDs
- **Metadata storage**: Hierarchical S3 paths with versioned entries, merged on read

## S3 Storage Structure

The filesystem stores data in S3 using two prefixes:

### Format Configuration

```
metadata/format.dat
```

Contains filesystem format information serialized via `QDataStream`:
- `block_size`: Block size (65536 bytes)
- `hash_algo`: Hash algorithm for data blocks (SHA3-256)

### Metadata Storage

Inodes are stored using a hierarchical path structure based on the inode number (a 64-bit microsecond-precision timestamp):

```
metadata/{last-hex-char}/{last-2-hex-chars}/{inode-hex}/{revision-hex}.dat
```

**Examples:**
```
metadata/1/01/0000000000000001/0000816909a2b79a.dat   # Root directory (inode 1)
metadata/a/9a/0000816909a2b79a/00050e9d947721b8.dat   # File with inode 142288133207962
```

Each metadata file contains (serialized via `QDataStream`):

1. **Inode attributes** (`S3FS_Obj`):
   - `ino`: Inode number (uint64)
   - `mode`: File type and permissions
   - `uid`, `gid`: Owner/group
   - `size`: File size
   - `ctime`, `mtime`, `atime`: Timestamps with nanosecond precision
   - `rdev`: Device number (for special files)

2. **Type-specific content**:
   - **Directories**: Map of `filename → (inode, type)`
   - **Regular files**: Map of `block_offset → data_hash`
   - **Symlinks**: Target path stored as `"\x00" → target`

3. **Change history**: Recent modifications within 1 hour for conflict resolution

**Conflict Resolution:** When multiple revision files exist for an inode (from concurrent writes on different cluster nodes), all revisions are read and merged based on their change logs, then consolidated into a single file.

### Data Block Storage

File content is stored as content-addressed blocks:

```
data/{last-hex-char}/{last-2-hex-chars}/{hash-hex}.dat
```

**Example:**
```
data/9/89/abcdef0123456789abcdef0123456789abcdef0123456789abcdef0123456789.dat
```

- Blocks are 65KB (`S3FUSE_BLOCK_SIZE`)
- Named by SHA3-256 hash of content
- Content-addressable: identical blocks are stored once
- Referenced by file metadata via offset → hash mapping

### Local Cache Structure (LMDB)

The local cache uses key prefixes to organize data:

| Prefix | Purpose |
|--------|---------|
| `0x01` | Cached metadata (inode data) |
| `0x02` | Cached data blocks |
| `0x03` | Metadata revision info (latest known revision per inode) |
| `0x11` | Inode last access time (for cache eviction) |
| `0x12` | Data block last access time (for cache eviction) |
| `0xff` | Full sync completion marker |

## Historical Context

This project represents an early approach to S3-backed filesystems, predating the maturity of alternatives like s3fs-fuse. Notable historical aspects:

- **Pre-AWS SDK era**: Implements AWS authentication from scratch (before boto3/official SDKs were standard)
- **Qt5.4 on Ubuntu 14.04 (Trusty)**: Reflects 2015-era Linux ecosystem
- **GCC 4.8**: Target compiler as shown in Travis CI configuration
- **No containerization**: Pre-Docker ubiquity design

## Technologies Used

- **C++11** with Qt5 framework (QtCore, QtNetwork, QtXmlPatterns)
- **FUSE 2.9+** for userspace filesystem
- **LMDB** (embedded via git submodule) for local key-value storage
- **OpenSSL** for AWS request signing
- **AWS S3/SNS/SQS** for storage and cluster coordination

## Original Build Instructions

S3ClFS required Qt5.4+ and Fuse 2.9+:

```
qmake -qt=qt5
make
```

## Original Configuration

AWS credentials via `~/.config/aws/credentials`:

```ini
[default]
aws_access_key_id = (your key id)
aws_secret_access_key = (secret value)
```

### AWS Setup (for cluster mode)

The full cluster setup required:
- An S3 bucket with appropriate IAM policy (see `doc/aws_rule.txt`)
- An SNS topic with bucket event notifications
- SQS queues per machine (20 second receive wait time)
- SNS subscription to each SQS queue

Single-machine mode could skip the SNS/SQS setup entirely.

## Code Statistics

- ~4,500 lines of C++ source code
- Well-structured separation of concerns
- Qt event loop with thread-safe FUSE integration
- Compact implementation for a complete distributed filesystem
