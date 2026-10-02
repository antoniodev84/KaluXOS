#ifndef KALUXOS_SBFS_H
#define KALUXOS_SBFS_H
#include <stdint.h>
#define SBFS_MAGIC 0x53424653U
#define SBFS_VERSION 1U
#define SBFS_BASE_SECTOR 6144ULL
#define SBFS_BLOCK_SIZE 512U
#define SBFS_MAX_FILE_SIZE 0xffffffffULL
#define SBFS_INODE_SIZE 128U
#define SBFS_INODES_PER_SECTOR 4U
#define SBFS_MAX_INODES 8192U
#define SBFS_NAME_MAX 63U
#define SBFS_SYMLINK_MAX 119U
#define SBFS_MODE_DIR 0040000U
#define SBFS_MODE_REG 0100000U
#define SBFS_MODE_LNK 0120000U
#define SBFS_FLAG_USED 1U
#define SBFS_FLAG_INLINE 2U
typedef struct sbfs_superblock { uint32_t magic; uint32_t version; uint32_t block_size; uint32_t inode_size; uint64_t total_sectors; uint64_t inode_table_start; uint64_t inode_table_sectors; uint64_t data_start; uint64_t data_next; uint32_t inode_count; uint32_t root_inode; uint64_t generation; uint8_t reserved[440]; } __attribute__((packed)) sbfs_superblock_t;
typedef struct sbfs_inode { uint32_t mode; uint32_t uid; uint32_t gid; uint32_t flags; uint64_t size; uint64_t start; uint64_t end; uint32_t links; uint32_t parent; uint64_t direct[8]; uint64_t indirect; uint64_t double_indirect; char name[64]; char symlink[120]; } __attribute__((packed)) sbfs_inode_t;
void sbfs_init(void);
int sbfs_mounted(void);
int sbfs_lookup(const char *path, uint32_t *inode_number, sbfs_inode_t *inode);
int sbfs_read(uint32_t inode_number, sbfs_inode_t *inode, uint64_t offset, void *buffer, uint64_t size);
int sbfs_write(uint32_t inode_number, sbfs_inode_t *inode, uint64_t offset, const void *buffer, uint64_t size);
int sbfs_create(const char *path, uint32_t mode, uint32_t *inode_number, sbfs_inode_t *inode);
int sbfs_mkdir(const char *path, uint32_t mode);
int sbfs_unlink(const char *path);
int sbfs_symlink(const char *target, const char *path);
int sbfs_readlink(const char *path, char *buffer, uint64_t size);
#endif
