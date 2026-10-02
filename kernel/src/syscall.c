#include "syscall.h"
#include "pit.h"
#include "uart.h"
#include "vfs.h"
#include "memory.h"
#include <stdint.h>
#define SYS_READ 0
#define SYS_WRITE 1
#define SYS_OPEN 2
#define SYS_CLOSE 3
#define SYS_STAT 4
#define SYS_LSEEK 8
#define SYS_USLEEP 35
#define SYS_MKDIR 83
#define SYS_UNLINK 87
#define SYS_SYMLINK 88
#define SYS_READLINK 89
#define EBADF 9
#define EFAULT 14
#define EINVAL 22
#define ENOSYS 38
#define USER_MAX 0x0000800000000000ULL
typedef struct linux_timespec { int64_t seconds; int64_t nanoseconds; } linux_timespec_t;
typedef struct linux_stat { uint64_t dev; uint64_t ino; uint32_t mode; uint32_t nlink; uint32_t uid; uint32_t gid; uint32_t pad; uint64_t size; uint64_t blocks; uint64_t atime; uint64_t mtime; uint64_t ctime; } linux_stat_t;
static int valid_user(const void *pointer, uint64_t size) { uint64_t address = (uint64_t)(uintptr_t)pointer; return pointer != 0 && address < USER_MAX && size <= USER_MAX - address; }
static int64_t sys_write(uint64_t fd, const char *buffer, uint64_t count) { if (fd != 1 && fd != 2) return -EBADF; if (!valid_user(buffer, count) || count > 4096) return -EFAULT; for (uint64_t i = 0; i < count; i++) uart_putc(buffer[i]); return count; }
static int64_t sys_read(int fd, void *buffer, uint64_t count) { if (!valid_user(buffer, count)) return -EFAULT; return vfs_read(fd, buffer, count); }
static int64_t sys_usleep(const linux_timespec_t *request) { if (!valid_user(request, sizeof(*request)) || request->seconds < 0 || request->nanoseconds < 0 || request->nanoseconds >= 1000000000) return -EINVAL; uint64_t start = pit_get_ticks(); uint64_t ns = (uint64_t)request->seconds * 1000000000ULL + (uint64_t)request->nanoseconds; uint64_t ticks = (ns + 9999999) / 10000000; if (ticks == 0 && ns != 0) ticks = 1; __asm__ volatile ("sti"); while (pit_get_ticks() - start < ticks) __asm__ volatile ("hlt"); __asm__ volatile ("cli"); return 0; }
static int64_t sys_stat(const char *path, linux_stat_t *output) { sbfs_inode_t inode; if (!valid_user(path, 1) || !valid_user(output, sizeof(*output)) || vfs_stat(path, &inode) != 0) return -EFAULT; memset(output, 0, sizeof(*output)); output->mode = inode.mode; output->nlink = inode.links; output->uid = inode.uid; output->gid = inode.gid; output->size = inode.size; output->blocks = (inode.size + 511) / 512; return 0; }
void syscall_init(void) { uart_puts("[ OK ] SYSCALL initialized, filesystem ABI active\n"); }
void syscall_dispatch(syscall_frame_t *frame) { switch (frame->rax) { case SYS_READ: frame->rax = (uint64_t)sys_read((int)frame->rdi, (void *)(uintptr_t)frame->rsi, frame->rdx); break; case SYS_WRITE: frame->rax = (uint64_t)sys_write(frame->rdi, (const char *)(uintptr_t)frame->rsi, frame->rdx); break; case SYS_OPEN: frame->rax = (uint64_t)vfs_open((const char *)(uintptr_t)frame->rdi, (int)frame->rsi, frame->rdx); break; case SYS_CLOSE: frame->rax = (uint64_t)vfs_close((int)frame->rdi); break; case SYS_STAT: frame->rax = (uint64_t)sys_stat((const char *)(uintptr_t)frame->rdi, (linux_stat_t *)(uintptr_t)frame->rsi); break; case SYS_LSEEK: frame->rax = (uint64_t)vfs_lseek((int)frame->rdi, (int64_t)frame->rsi, (int)frame->rdx); break; case SYS_USLEEP: frame->rax = (uint64_t)sys_usleep((const linux_timespec_t *)(uintptr_t)frame->rdi); break; case SYS_MKDIR: frame->rax = (uint64_t)vfs_mkdir((const char *)(uintptr_t)frame->rdi, frame->rsi); break; case SYS_UNLINK: frame->rax = (uint64_t)vfs_unlink((const char *)(uintptr_t)frame->rdi); break; case SYS_SYMLINK: frame->rax = (uint64_t)vfs_symlink((const char *)(uintptr_t)frame->rdi, (const char *)(uintptr_t)frame->rsi); break; case SYS_READLINK: frame->rax = (uint64_t)vfs_readlink((const char *)(uintptr_t)frame->rdi, (char *)(uintptr_t)frame->rsi, frame->rdx); break; default: frame->rax = (uint64_t)-ENOSYS; break; } }
