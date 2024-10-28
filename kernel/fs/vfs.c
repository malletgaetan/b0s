#include "kernel/lib/list/list.h"
#include "kernel/lib/string/string.h"

static struct list_head mountpoints = LIST_HEAD_INIT(mountpoints);

struct vfs_mnt *vfs_mnt(char *type, char *mountpoint) {
	struct vfs_mnt *mnt = kmalloc(sizeof(struct vfs_mnt));
	strcpy(&mnt->type, type);
	strcpy(&mnt->mountpoint, mountpoint);
    list_add(&mnt->list, &mountpoints);
	return mnt;
}

void vfs_umnt(struct vfs_mnt *mnt) {
	list_del(&mnt->list);
	kfree(mnt);
}

struct vfs_mnt *vfs_get_mnt(char *path) {
	u64 max = 0;
	struct vfs_mnt *tmp = NULL;
	list_for_each(mnt, head) {
		u64 matchlen = strmatch(mnt->mountpoint, path);
		if (matchlen > max)
			tmp = mnt;
	}
	return tmp;
}