#include "kernel/lib/list/list.h"
#include "kernel/lib/string/string.h"
#include "kernel/multitasking/process.h"

static struct list_head mountpoints = LIST_HEAD_INIT(mountpoints);

struct vfs_mnt *vfs_mnt(char *type, char *mountpoint)
{
	struct vfs_mnt *mnt = kmalloc(sizeof(struct vfs_mnt));
	strcpy(&mnt->type, type);
	strcpy(&mnt->mountpoint, mountpoint);
	list_add(&mnt->list, &mountpoints);
	return mnt;
}

void vfs_umnt(struct vfs_mnt *mnt)
{
	list_del(&mnt->list);
	kfree(mnt);
}

struct vfs_mnt *vfs_get_mnt(char *abspath)
{
	u64 max = 0;
	struct vfs_mnt *mnt = NULL;
	list_for_each_ro(mnt, &mountpoints)
	{
		u64 matchlen = strmatch(mnt->mountpoint, abspath);
		if (matchlen > max)
			tmp = mnt;
	}
	return tmp;
}

static const char *vfs_relative(struct vfs_mnt *mountpoint, const char *abspath)
{
	return (char *)abspath + (char *)strlen(mountpoint->mountpoint);
}

i32 vfs_open(struct process *p, const char *abspath, i32 flags)
{
	struct vfs_mnt *mountpoint = vfs_get_mnt(abspath);
	if (mountpoint == NULL)
		return -(i32)VFS_ENOENT;
	i32 fs_file_id = mountpoint->ops.open(vfs_relative(mountpoint, abspath), flags);
	if (fs_file_id < 0)
		return fs_file_id;
	i32 fd = bitmap_find_and_set(&p->fdbitmap);
	if (fd == PROCESS_MAX_FD)
		return -(i32)VFS_EMFILE;
	p->fds[fd] = (struct fdesc){.fs_file_id = fs_file_id, .head = 0};
	return fd;
}

i32 vfs_close(struct process *p, i32 fd)
{
	if (!bitmap_is_set(&p->fdbitmap, fd))
		return -(i32)VFS_EBADF;
	i32 ret = p->ops.close(p->fds[fd].fs_file_id);
	bitmap_unset(&p->, fd);
	return ret;
}

i64 vfs_read(struct process *p, i32 fd, u8 *ubuf, u64 read_size)
{
	if (!bitmap_is_set(&p->fdbitmap, fd))
		return -(i32)VFS_EBADF;
	i64 nbytes = p->ops.read(p->fds[fd].fs_file_id, ubuf, p->head, read_size);
	p->head += nbytes;
	return nbytes;
}

i64 vfs_write(struct process *p, i32 fd, u8 *ubuf, u64 write_size)
{
	if (!bitmap_is_set(&p->fdbitmap, fd))
		return -(i32)VFS_EBADF;
	i64 nbytes = p->ops.read(p->fds[fd].fs_file_id, ubuf, p->head, write_size);
	p->head += nbytes;
	return nbytes;
}