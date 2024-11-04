#ifndef VFS_H
#define VFS_H

#define VFS_TYPE_LENGTH 32
#define VFS_PATH_LENGTH 4096
#define VFS_FILE_LENGTH VFS_PATH_LENGTH

#include "kernel/lib/list/list.h"

// TODO: how should we let userspace get theses values?
// O_RDONLY it opens a file in read only mode
// O_RDWR it opens a file for reading and writing
// O_WRONLY it opens a file only for writing.

enum {
	VFS_OK,		 // shouldn't be used
	VFS_ENOENT,	 // no such file or directory
	VFS_EBADF,	 // bad file descriptor
	VFS_EACCESS, // permission denied
	VFS_EMFILE,	 // too much opened file
};

struct fdesc {
	u64 fs_file_id;
	u64 head;
};

struct vfs_ops {
	i32 (*open)(const char *path, i32 flags, ...);
	i32 (*close)(i32 fs_id);
	i64 (*read)(i32 fs_id, char *read_buffer, u64 offset, u64 nbyte);
	i64 (*write)(i32 fs_id, const void *write_buffer, u64 nbyte);
	u64 (*seek)(i32 fs_id, u64 offset, i32 whence);
};

struct vfs_mnt {
	char type[VFS_TYPE_LENGTH];
	char mountpoint[VFS_TYPE_LENGTH];
	// struct storage_device	*device; // your time will come
	struct vfs_ops ops;
	struct list_head list;
};

struct vfs_mnt *vfs_mnt(char *type, char *mountpoint);
void vfs_umnt(struct vfs_mnt *mnt);
struct vfs_mnt *vfs_get_mnt(char *path);

#endif