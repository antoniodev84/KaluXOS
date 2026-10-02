#include "vfs.h"
#include "uart.h"
#include "memory.h"
#include <stdint.h>
#define O_CREAT 0100
#define O_TRUNC 01000
#define O_APPEND 02000
#define SEEK_SET 0
#define SEEK_CUR 1
#define SEEK_END 2
#define EBADF 9
#define EEXIST 17
#define ENOENT 2
#define EISDIR 21
#define EINVAL 22
#define EMFILE 24
#define ENOSPC 28
typedef struct vfs_file { uint32_t used; uint32_t inode_number; uint32_t flags; uint64_t position; sbfs_inode_t inode; } vfs_file_t;
static vfs_file_t files[VFS_MAX_FDS];
void vfs_init(void) { memset(files, 0, sizeof(files)); sbfs_init(); if (sbfs_mounted()) uart_puts("[ OK ] VFS mounted root at /\n"); }
int vfs_open(const char *path, int flags, uint32_t mode) { uint32_t id; sbfs_inode_t inode; if (sbfs_lookup(path, &id, &inode) != 0) { if ((flags & O_CREAT) == 0) return -ENOENT; if (sbfs_create(path, SBFS_MODE_REG | (mode & 0777), &id, &inode) != 0) return -ENOSPC; } else if ((flags & O_CREAT) != 0 && (flags & O_TRUNC) == 0) { return -EEXIST; } if ((inode.mode & 0170000) == SBFS_MODE_DIR) return -EISDIR; for (int i = 0; i < VFS_MAX_FDS; i++) if (!files[i].used) { files[i].used = 1; files[i].inode_number = id; files[i].flags = flags; files[i].position = (flags & O_APPEND) ? inode.size : 0; files[i].inode = inode; if ((flags & O_TRUNC) != 0) { files[i].inode.size = 0; files[i].position = 0; } return i; } return -EMFILE; }
int vfs_close(int fd) { if (fd < 0 || fd >= VFS_MAX_FDS || !files[fd].used) return -EBADF; files[fd].used = 0; return 0; }
int64_t vfs_read(int fd, void *buffer, uint64_t count) { if (fd < 0 || fd >= VFS_MAX_FDS || !files[fd].used) return -EBADF; if ((files[fd].inode.mode & 0170000) == SBFS_MODE_DIR) return -EISDIR; int result = sbfs_read(files[fd].inode_number, &files[fd].inode, files[fd].position, buffer, count); if (result >= 0) files[fd].position += result; return result; }
int64_t vfs_write(int fd, const void *buffer, uint64_t count) { if (fd < 0 || fd >= VFS_MAX_FDS || !files[fd].used) return -EBADF; if ((files[fd].inode.mode & 0170000) == SBFS_MODE_DIR) return -EISDIR; int result = sbfs_write(files[fd].inode_number, &files[fd].inode, files[fd].position, buffer, count); if (result >= 0) files[fd].position += result; return result; }
int64_t vfs_lseek(int fd, int64_t offset, int whence) { if (fd < 0 || fd >= VFS_MAX_FDS || !files[fd].used) return -EBADF; int64_t base; if (whence == SEEK_SET) base = 0; else if (whence == SEEK_CUR) base = (int64_t)files[fd].position; else if (whence == SEEK_END) base = (int64_t)files[fd].inode.size; else return -EINVAL; if (base < 0 || offset < 0 || offset > INT64_MAX - base) return -EINVAL; files[fd].position = (uint64_t)(base + offset); return base + offset; }
int vfs_mkdir(const char *path, uint32_t mode) { return sbfs_mkdir(path, mode); }
int vfs_unlink(const char *path) { return sbfs_unlink(path); }
int vfs_symlink(const char *target, const char *path) { return sbfs_symlink(target, path); }
int vfs_readlink(const char *path, char *buffer, uint64_t size) { return sbfs_readlink(path, buffer, size); }
int vfs_stat(const char *path, sbfs_inode_t *inode) { return sbfs_lookup(path, 0, inode); }
