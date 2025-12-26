#!/usr/bin/env python3
"""
S3ClFS Data Extractor

Extracts filesystem data from S3ClFS bucket dumps into a regular directory structure.
Parses Qt QDataStream format used by the original C++ implementation.
"""

import os
import sys
import struct
import argparse
from pathlib import Path
from dataclasses import dataclass
from typing import Dict, Any, Optional, Tuple, List
import stat as stat_module


class QDataStreamReader:
    """Reader for Qt's QDataStream binary format (big-endian, Qt 5.x)"""

    # Qt QVariant type IDs (from qmetatype.h)
    QVARIANT_INVALID = 0
    QVARIANT_BOOL = 1
    QVARIANT_INT = 2
    QVARIANT_UINT = 3
    QVARIANT_LONGLONG = 4
    QVARIANT_ULONGLONG = 5
    QVARIANT_DOUBLE = 6
    QVARIANT_STRING = 10
    QVARIANT_BYTEARRAY = 12
    QVARIANT_MAP = 8
    QVARIANT_LIST = 9

    def __init__(self, data: bytes):
        self.data = data
        self.pos = 0

    def remaining(self) -> int:
        return len(self.data) - self.pos

    def read_bytes(self, n: int) -> bytes:
        if self.pos + n > len(self.data):
            raise ValueError(f"Not enough data: need {n}, have {self.remaining()}")
        result = self.data[self.pos:self.pos + n]
        self.pos += n
        return result

    def read_uint8(self) -> int:
        return struct.unpack('>B', self.read_bytes(1))[0]

    def read_int32(self) -> int:
        return struct.unpack('>i', self.read_bytes(4))[0]

    def read_uint32(self) -> int:
        return struct.unpack('>I', self.read_bytes(4))[0]

    def read_int64(self) -> int:
        return struct.unpack('>q', self.read_bytes(8))[0]

    def read_uint64(self) -> int:
        return struct.unpack('>Q', self.read_bytes(8))[0]

    def read_double(self) -> float:
        return struct.unpack('>d', self.read_bytes(8))[0]

    def read_qstring(self) -> str:
        """Read a QString (4-byte length in bytes, then UTF-16BE data)"""
        length = self.read_uint32()
        if length == 0xFFFFFFFF:  # Null string
            return ""
        if length == 0:
            return ""
        data = self.read_bytes(length)
        # QDataStream serializes QString in big-endian UTF-16
        return data.decode('utf-16-be')

    def read_qbytearray(self) -> bytes:
        """Read a QByteArray (4-byte length, then raw bytes)"""
        length = self.read_uint32()
        if length == 0xFFFFFFFF:  # Null array
            return b""
        if length == 0:
            return b""
        return self.read_bytes(length)

    def read_qvariant(self) -> Any:
        """Read a QVariant value"""
        type_id = self.read_uint32()
        is_null = self.read_uint8()

        if is_null:
            return None

        if type_id == self.QVARIANT_INVALID:
            return None
        elif type_id == self.QVARIANT_BOOL:
            return self.read_uint8() != 0
        elif type_id == self.QVARIANT_INT:
            return self.read_int32()
        elif type_id == self.QVARIANT_UINT:
            return self.read_uint32()
        elif type_id == self.QVARIANT_LONGLONG:
            return self.read_int64()
        elif type_id == self.QVARIANT_ULONGLONG:
            return self.read_uint64()
        elif type_id == self.QVARIANT_DOUBLE:
            return self.read_double()
        elif type_id == self.QVARIANT_STRING:
            return self.read_qstring()
        elif type_id == self.QVARIANT_BYTEARRAY:
            return self.read_qbytearray()
        elif type_id == self.QVARIANT_MAP:
            return self.read_qvariantmap()
        elif type_id == self.QVARIANT_LIST:
            return self.read_qvariantlist()
        else:
            raise ValueError(f"Unknown QVariant type: {type_id} at pos {self.pos}")

    def read_qvariantmap(self) -> Dict[str, Any]:
        """Read a QVariantMap (QMap<QString, QVariant>)"""
        count = self.read_uint32()
        result = {}
        for _ in range(count):
            key = self.read_qstring()
            value = self.read_qvariant()
            result[key] = value
        return result

    def read_qvariantlist(self) -> List[Any]:
        """Read a QVariantList (QList<QVariant>)"""
        count = self.read_uint32()
        return [self.read_qvariant() for _ in range(count)]


@dataclass
class Inode:
    """Represents a filesystem inode"""
    ino: int
    mode: int
    uid: int
    gid: int
    size: int
    ctime: int
    mtime: int
    atime: int
    rdev: int = 0

    # Type-specific data
    children: Optional[Dict[str, Tuple[int, int]]] = None  # For directories: name -> (inode, type)
    blocks: Optional[Dict[int, bytes]] = None  # For files: offset -> block_hash
    symlink_target: Optional[str] = None  # For symlinks

    def is_dir(self) -> bool:
        return stat_module.S_ISDIR(self.mode)

    def is_file(self) -> bool:
        return stat_module.S_ISREG(self.mode)

    def is_symlink(self) -> bool:
        return stat_module.S_ISLNK(self.mode)

    def type_str(self) -> str:
        if self.is_dir():
            return "dir"
        elif self.is_file():
            return "file"
        elif self.is_symlink():
            return "symlink"
        elif stat_module.S_ISFIFO(self.mode):
            return "fifo"
        elif stat_module.S_ISCHR(self.mode):
            return "chardev"
        elif stat_module.S_ISBLK(self.mode):
            return "blockdev"
        elif stat_module.S_ISSOCK(self.mode):
            return "socket"
        return "unknown"


class S3ClFSExtractor:
    """Extracts S3ClFS filesystem data"""

    BLOCK_SIZE = 65536  # S3FUSE_BLOCK_SIZE

    def __init__(self, source_dir: str, verbose: bool = False):
        self.source_dir = Path(source_dir)
        self.metadata_dir = self.source_dir / "metadata"
        self.data_dir = self.source_dir / "data"
        self.verbose = verbose
        self.inodes: Dict[int, Inode] = {}
        self.format_config: Dict[str, Any] = {}

    def log(self, msg: str):
        if self.verbose:
            print(msg)

    def read_format_file(self) -> bool:
        """Read metadata/format.dat"""
        format_path = self.metadata_dir / "format.dat"
        if not format_path.exists():
            print(f"Warning: format.dat not found at {format_path}")
            return False

        try:
            data = format_path.read_bytes()
            reader = QDataStreamReader(data)
            # Data is wrapped in a QVariant
            self.format_config = reader.read_qvariant()
            self.log(f"Format config: {self.format_config}")
            return True
        except Exception as e:
            print(f"Error reading format.dat: {e}")
            return False

    def find_metadata_files(self) -> List[Path]:
        """Find all metadata .dat files (excluding format.dat)"""
        files = []
        for path in self.metadata_dir.rglob("*.dat"):
            if path.name != "format.dat":
                files.append(path)
        return sorted(files)

    def parse_inode_path(self, path: Path) -> Optional[Tuple[int, int]]:
        """Parse inode and revision from metadata path"""
        # Path format: metadata/X/XY/XXXXXXXXXXXXXXXX/YYYYYYYYYYYYYYYY.dat
        try:
            parts = path.relative_to(self.metadata_dir).parts
            if len(parts) != 4:
                return None
            inode_hex = parts[2]
            revision_hex = parts[3].replace('.dat', '')
            return (int(inode_hex, 16), int(revision_hex, 16))
        except (ValueError, IndexError):
            return None

    def parse_metadata_file(self, path: Path) -> Optional[Inode]:
        """Parse a metadata file and return an Inode"""
        parsed = self.parse_inode_path(path)
        if not parsed:
            self.log(f"Skipping unparseable path: {path}")
            return None

        ino_from_path, revision = parsed

        try:
            data = path.read_bytes()
            if len(data) < 16:  # Minimum: 8-byte header + some map data
                self.log(f"Skipping truncated file: {path} ({len(data)} bytes)")
                return None

            reader = QDataStreamReader(data)

            # Inode files have an 8-byte header (size indicator) before the QMap data
            # Skip this header
            header = reader.read_uint64()
            self.log(f"Inode header value: {header}")

            # Read the props map directly (not wrapped in QVariant)
            props = reader.read_qvariantmap()
            if not isinstance(props, dict):
                self.log(f"Expected dict, got {type(props)} in {path}")
                return None
            attrs = props.get('attrs', {})

            if not attrs:
                self.log(f"No attrs in {path}")
                return None

            inode = Inode(
                ino=attrs.get('ino', ino_from_path),
                mode=attrs.get('mode', 0),
                uid=attrs.get('uid', 0),
                gid=attrs.get('gid', 0),
                size=attrs.get('size', 0),
                ctime=attrs.get('ctime', 0),
                mtime=attrs.get('mtime', 0),
                atime=attrs.get('atime', 0),
                rdev=attrs.get('rdev', 0),
            )

            # Read type-specific content - stored as sequential QByteArray pairs (not a QMap)
            if reader.remaining() > 4:
                try:
                    meta_content: Dict[bytes, bytes] = {}
                    # Read pairs of QByteArray until we run out of data
                    while reader.remaining() >= 8:  # Need at least 4+4 bytes for two lengths
                        key = reader.read_qbytearray()
                        if reader.remaining() < 4:
                            break
                        value = reader.read_qbytearray()
                        meta_content[key] = value

                    if inode.is_dir():
                        # Directory: filename -> (inode, type)
                        inode.children = {}
                        for name_bytes, value in meta_content.items():
                            name = name_bytes.decode('utf-8', errors='replace')
                            if len(value) >= 12:  # uint64 inode + uint32 type
                                child_ino = struct.unpack('>Q', value[:8])[0]
                                child_type = struct.unpack('>I', value[8:12])[0]
                                inode.children[name] = (child_ino, child_type)

                    elif inode.is_file():
                        # File: offset -> block_hash
                        inode.blocks = {}
                        for offset_bytes, hash_bytes in meta_content.items():
                            if len(offset_bytes) == 8:
                                offset = struct.unpack('>q', offset_bytes)[0]
                                inode.blocks[offset] = hash_bytes

                    elif inode.is_symlink():
                        # Symlink: "\x00" -> target
                        target = meta_content.get(b'\x00', b'')
                        inode.symlink_target = target.decode('utf-8', errors='replace')

                except Exception as e:
                    self.log(f"Error reading meta content for {path}: {e}")

            return inode

        except Exception as e:
            # Only log errors for verbose mode - many files may be truncated/corrupt
            self.log(f"Error parsing {path}: {e}")
            return None

    def load_all_inodes(self):
        """Load all inodes from metadata files"""
        metadata_files = self.find_metadata_files()
        print(f"Found {len(metadata_files)} metadata files")

        # Group by inode, keep latest revision
        inode_revisions: Dict[int, Tuple[int, Path]] = {}

        for path in metadata_files:
            parsed = self.parse_inode_path(path)
            if parsed:
                ino, rev = parsed
                if ino not in inode_revisions or rev > inode_revisions[ino][0]:
                    inode_revisions[ino] = (rev, path)

        print(f"Found {len(inode_revisions)} unique inodes")

        skipped = 0
        for ino, (rev, path) in inode_revisions.items():
            inode = self.parse_metadata_file(path)
            if inode:
                self.inodes[ino] = inode
                self.log(f"Loaded inode {ino}: {inode.type_str()}")
            else:
                skipped += 1

        print(f"Successfully loaded {len(self.inodes)} inodes ({skipped} skipped/corrupt)")

    def read_block(self, block_hash: bytes) -> Optional[bytes]:
        """Read a data block by its hash"""
        hash_hex = block_hash.hex()
        # Path: data/X/XY/HASH.dat
        block_path = self.data_dir / hash_hex[-1] / hash_hex[-2:] / f"{hash_hex}.dat"

        if block_path.exists():
            return block_path.read_bytes()

        # Try without extension
        block_path_noext = self.data_dir / hash_hex[-1] / hash_hex[-2:] / hash_hex
        if block_path_noext.exists():
            return block_path_noext.read_bytes()

        self.log(f"Block not found: {hash_hex}")
        return None

    def reconstruct_file(self, inode: Inode) -> bytes:
        """Reconstruct a file's contents from its blocks"""
        if not inode.blocks:
            return b""

        # Sort blocks by offset
        sorted_blocks = sorted(inode.blocks.items())

        result = bytearray()
        expected_offset = 0

        for offset, block_hash in sorted_blocks:
            # Fill gap with zeros if needed
            if offset > expected_offset:
                result.extend(b'\x00' * (offset - expected_offset))

            block_data = self.read_block(block_hash)
            if block_data:
                result.extend(block_data)
                expected_offset = offset + len(block_data)
            else:
                # Missing block - fill with zeros
                result.extend(b'\x00' * self.BLOCK_SIZE)
                expected_offset = offset + self.BLOCK_SIZE

        # Truncate to actual size
        return bytes(result[:inode.size])

    def extract_to_directory(self, output_dir: str):
        """Extract the filesystem to a regular directory"""
        output_path = Path(output_dir)
        output_path.mkdir(parents=True, exist_ok=True)

        # Build path map starting from root (inode 1)
        if 1 not in self.inodes:
            print("Error: Root inode (1) not found!")
            return

        def extract_inode(ino: int, dest_path: Path, name: str = ""):
            if ino not in self.inodes:
                print(f"Warning: Inode {ino} not found")
                return

            inode = self.inodes[ino]

            if inode.is_dir():
                dest_path.mkdir(parents=True, exist_ok=True)
                self.log(f"Created directory: {dest_path}")

                if inode.children:
                    for child_name, (child_ino, child_type) in inode.children.items():
                        if child_name in ('.', '..'):
                            continue
                        child_path = dest_path / child_name
                        extract_inode(child_ino, child_path, child_name)

            elif inode.is_file():
                content = self.reconstruct_file(inode)
                dest_path.write_bytes(content)
                self.log(f"Extracted file: {dest_path} ({len(content)} bytes)")

            elif inode.is_symlink():
                if inode.symlink_target:
                    try:
                        if dest_path.exists() or dest_path.is_symlink():
                            dest_path.unlink()
                        dest_path.symlink_to(inode.symlink_target)
                        self.log(f"Created symlink: {dest_path} -> {inode.symlink_target}")
                    except OSError as e:
                        print(f"Warning: Could not create symlink {dest_path}: {e}")

            else:
                self.log(f"Skipping special file: {dest_path} ({inode.type_str()})")

        print(f"Extracting to {output_path}...")
        extract_inode(1, output_path)
        print("Extraction complete!")

    def list_contents(self, show_blocks: bool = False):
        """List filesystem contents"""
        if 1 not in self.inodes:
            print("Error: Root inode (1) not found!")
            return

        def list_inode(ino: int, path: str = "/", indent: int = 0):
            if ino not in self.inodes:
                print(f"{'  ' * indent}[missing inode {ino}]")
                return

            inode = self.inodes[ino]
            prefix = "  " * indent
            mode_str = stat_module.filemode(inode.mode)

            if inode.is_dir():
                print(f"{prefix}{mode_str} {inode.uid}:{inode.gid} {path}")
                if inode.children:
                    for child_name, (child_ino, child_type) in sorted(inode.children.items()):
                        if child_name in ('.', '..'):
                            continue
                        child_path = f"{path}{child_name}/" if child_type & 0o40000 else f"{path}{child_name}"
                        list_inode(child_ino, child_path, indent + 1)

            elif inode.is_file():
                size_str = f"{inode.size:>10}"
                print(f"{prefix}{mode_str} {inode.uid}:{inode.gid} {size_str} {path}")
                if show_blocks and inode.blocks:
                    for offset, block_hash in sorted(inode.blocks.items()):
                        print(f"{prefix}    block @{offset}: {block_hash.hex()}")

            elif inode.is_symlink():
                target = inode.symlink_target or "?"
                print(f"{prefix}{mode_str} {inode.uid}:{inode.gid} {path} -> {target}")

            else:
                print(f"{prefix}{mode_str} {inode.uid}:{inode.gid} {path} ({inode.type_str()})")

        list_inode(1)

    def dump_raw(self):
        """Dump raw inode information"""
        for ino, inode in sorted(self.inodes.items()):
            print(f"\nInode {ino}:")
            print(f"  Type: {inode.type_str()}")
            print(f"  Mode: {oct(inode.mode)} ({stat_module.filemode(inode.mode)})")
            print(f"  UID/GID: {inode.uid}/{inode.gid}")
            print(f"  Size: {inode.size}")

            if inode.children:
                print(f"  Children: {len(inode.children)}")
                for name, (child_ino, child_type) in inode.children.items():
                    print(f"    {name}: inode={child_ino}, type={oct(child_type)}")

            if inode.blocks:
                print(f"  Blocks: {len(inode.blocks)}")
                for offset, block_hash in sorted(inode.blocks.items()):
                    print(f"    @{offset}: {block_hash.hex()}")

            if inode.symlink_target:
                print(f"  Target: {inode.symlink_target}")


def main():
    parser = argparse.ArgumentParser(
        description="Extract S3ClFS filesystem data",
        formatter_class=argparse.RawDescriptionHelpFormatter,
        epilog="""
Examples:
  %(prog)s /path/to/bucket --list
  %(prog)s /path/to/bucket --extract /path/to/output
  %(prog)s /path/to/bucket --dump
        """
    )

    parser.add_argument("source", help="Source directory containing metadata/ and data/")
    parser.add_argument("-l", "--list", action="store_true", help="List filesystem contents")
    parser.add_argument("-e", "--extract", metavar="DIR", help="Extract to directory")
    parser.add_argument("-d", "--dump", action="store_true", help="Dump raw inode information")
    parser.add_argument("-b", "--blocks", action="store_true", help="Show block hashes in listing")
    parser.add_argument("-v", "--verbose", action="store_true", help="Verbose output")

    args = parser.parse_args()

    if not any([args.list, args.extract, args.dump]):
        parser.print_help()
        print("\nError: Specify --list, --extract, or --dump")
        sys.exit(1)

    extractor = S3ClFSExtractor(args.source, verbose=args.verbose)

    # Check source directory
    if not extractor.metadata_dir.exists():
        print(f"Error: metadata/ directory not found in {args.source}")
        sys.exit(1)

    # Read format and load inodes
    extractor.read_format_file()
    extractor.load_all_inodes()

    if args.dump:
        extractor.dump_raw()

    if args.list:
        extractor.list_contents(show_blocks=args.blocks)

    if args.extract:
        extractor.extract_to_directory(args.extract)


if __name__ == "__main__":
    main()
