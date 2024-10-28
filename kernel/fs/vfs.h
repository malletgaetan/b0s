#ifndef VFS_H
# define VFS_H

#define VFS_TYPE_LENGTH 32
#define VFS_PATH_LENGTH 4096

struct vfs_ops {
	i32	(*open)(const char *path, i32 flags, ...);
	i32	(*close)(i32 file_descriptor);
	i64	(*read)(i32 file_descriptor, char *read_buffer, u64 nbyte);
	i64	(*write)(i32 file_descriptor, const void *write_buffer, u64 nbyte);
	u64	(*seek)(i32 file_descriptor, u64 offset, i32 whence);
};

struct vfs_mnt {
    char					type[VFS_TYPE_LENGTH];
    char 					mountpoint[VFS_TYPE_LENGTH];
	// struct storage_device	*device; // your time will come
    struct vfs_ops			ops;
	struct list_head		list;
};


struct vfs_mnt	*vfs_mnt(char *type, char *mountpoint);
void			vfs_umnt(struct vfs_mnt *mnt);
struct vfs_mnt	*vfs_get_mnt(char *path);

#endif