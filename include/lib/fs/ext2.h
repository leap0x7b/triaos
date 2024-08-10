#ifndef LIB__FS__EXT2_H
#define LIB__FS__EXT2_H

#include <stdint.h>

#define EXT2_MAGIC_SIGNATURE 0xEF53

typedef struct {
    uint32_t inodes_count;
    uint32_t blocks_count;
    uint32_t reserved_blocks_count;
    uint32_t free_blocks_count;
    uint32_t free_inodes_count;
    uint32_t first_data_block;
    uint32_t log_block_size;
    uint32_t log_fragment_size;
    uint32_t blocks_per_group;
    uint32_t fragments_per_group;
    uint32_t inodes_per_group;
    uint32_t last_mount_time;
    uint32_t last_written_time;

    uint16_t mount_count;
    uint16_t max_mount_count;
    uint16_t signature; // 0xEF53
    uint16_t state;
    uint16_t error_handling_methods;
    uint16_t version_minor;

    uint32_t last_checked;
    uint32_t check_interval;
    uint32_t creator_os;
    uint32_t version_major;

    uint16_t reserved_blocks_uid;
    uint16_t reserved_blocks_gid;

    // Extended superblock
    uint32_t first_inode;

    uint16_t inode_size;
    uint16_t block_group;

    uint32_t optional_features;
    uint32_t required_features;
    uint32_t unsupported_features;

    uint64_t uuid[2];
    char volume_name[16];
    char last_mounted_path[64];

    uint32_t compression_algorithm;

    uint8_t prealloc_blocks_count;
    uint8_t prealloc_dir_blocks_count;

    uint16_t _unused1;

    uint64_t journal_uuid[2];
    uint32_t journal_inode;
    uint32_t journal_device;
    uint32_t last_orphan;

    uint32_t hash_seed[4];
    uint8_t default_hash_version;

    uint8_t _unused2[3];

    uint32_t default_mount_opts;
    uint32_t first_meta_block_group;
} __attribute__((packed)) TiFsExt2Superblock;

typedef enum {
    EXT2_FILE_SYSTEM_IS_CLEAN = 1,
    EXT2_FILE_SYSTEM_HAS_ERRORS = 2,
} TiFsExt2States;

typedef enum {
    EXT2_ERRORS_IGNORE = 1,
    EXT2_ERRORS_READ_ONLY = 2,
    EXT2_ERRORS_KERNEL_PANIC = 3,
} TiFsExt2ErrorHandlingMethods;

typedef enum {
    EXT2_OS_LINUX = 0,
    EXT2_OS_HURD = 1,
    EXT2_OS_MASIX = 2,
    EXT2_OS_FREEBSD = 3,
    EXT2_OS_OTHER_BSD = 4,
    EXT2_OS_TRIAOS = 5, // custom value
} TiFsExt2CreatorOS;

typedef enum {
    EXT2_DIR_PREALLOC = 1 << 0,
    EXT2_HAS_AFS_SERVER_INODES = 1 << 1,
    EXT2_HAS_EXT3_JOURNAL = 1 << 2,
    EXT2_HAS_EXTENDED_ATTRIBUTES = 1 << 3,
    EXT2_IS_RESIZEABLE = 1 << 4,
    EXT2_HASH_INDEX = 1 << 5,
} TiFsExt2OptionalFeatures;

typedef enum {
    EXT2_USES_COMPRESSION = 1 << 0,
    EXT2_DIRECTORY_TYPE_FIELD = 1 << 1,
    EXT2_JOURNAL_REPLAY = 1 << 2,
    EXT2_USES_JOURNAL_DEVICE = 1 << 3,
} TiFsExt2RequiredFeatures;

typedef enum {
    EXT2_SPARSE_SUPERBLOCK = 1 << 0,
    EXT2_LARGE_FILE_SIZE = 1 << 2,
    EXT2_BINARY_TREE_DIRS = 1 << 3,
} TiFsExt2UnsupportedFeatures;

typedef struct {
    uint32_t block_bitmap;
    uint32_t inode_bitmap;
    uint32_t inode_table;

    uint16_t free_blocks_count;
    uint16_t free_inodes_count;
    uint16_t used_directories_count;

    uint16_t _reserved[7];
} __attribute__((packed)) TiFsExt2BlockGroupDescriptor;

typedef struct {
    uint8_t fragment_number;
    uint8_t frament_size;
    uint16_t _reserved1;
    uint16_t user_id;
    uint16_t group_id;
    uint32_t _reserved2;
} __attribute__((packed)) TiFsExt2Osd2;

typedef struct {
    uint16_t mode;
    uint16_t user_id;

    uint32_t size;
    uint32_t last_accessed_time;
    uint32_t creation_time;
    uint32_t last_modified_time;
    uint32_t deletion_time;

    uint16_t group_id;
    uint16_t hard_links_count;

    uint32_t blocks_count;
    uint32_t flags;
    uint32_t osd1;
    uint32_t blocks[15];
    uint32_t generation_number;

    // Ext2 version >= 1.0
    uint32_t file_acl_block;
    uint32_t directory_acl_block;

    uint32_t fragment_block;
    TiFsExt2Osd2 osd2;
} __attribute__((packed)) TiFsExt2Inode;

typedef enum {
    EXT2_INODE_FIFO = 0x1000,
    EXT2_INODE_CHARACTER_DEVICE = 0x2000,
    EXT2_INODE_DIRECTORY = 0x4000,
    EXT2_INODE_BLOCK_DEVICE = 0x6000,
    EXT2_INODE_REGULAR_FILE = 0x8000,
    EXT2_INODE_SYMBOLIC_LINK = 0xA000,
    EXT2_INODE_UNIX_SOCKET = 0xC000,
} TiFsExt2InodeType;

typedef struct {
    uint32_t inode;
    uint16_t entry_size;
    uint8_t filename_length;
    uint8_t type;
    char *filename;
} __attribute__((packed)) TiFsExt2DirectoryEntry;

#endif
