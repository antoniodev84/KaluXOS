#ifndef KALUXOS_VFS_H
#define KALUXOS_VFS_H
#include <stdint.h>
#include "sbfs.h"
#define VFS_MAX_FDS 64
void vfs_init(void);
int vfs_open(const char *path, int flags, uint32_t mode);
int vfs_close(int fd);
int64_t vfs_read(int fd, void *buffer, uint64_t count);
int64_t vfs_write(int fd, const void *buffer, uint64_t count);
int64_t vfs_lseek(int fd, int64_t offset, int whence);
int vfs_mkdir(const char *path, uint32_t mode);
int vfs_unlink(const char *path);
int vfs_symlink(const char *target, const char *path);
int vfs_readlink(const char *path, char *buffer, uint64_t size);
int vfs_stat(const char *path, sbfs_inode_t *inode);
#endif
