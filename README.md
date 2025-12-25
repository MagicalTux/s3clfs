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
