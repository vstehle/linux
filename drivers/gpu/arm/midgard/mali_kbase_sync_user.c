/*
 *
 * (C) COPYRIGHT 2012-2015 ARM Limited. All rights reserved.
 *
 * This program is free software and is provided to you under the terms of the
 * GNU General Public License version 2 as published by the Free Software
 * Foundation, and any use by you of this program is subject to the terms
 * of such GNU licence.
 *
 * A copy of the licence is included with the program, and can also be obtained
 * from Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA  02110-1301, USA.
 *
 */





/**
 * @file mali_kbase_sync_user.c
 *
 */

#ifdef CONFIG_SYNC

#include <linux/sched.h>
#include <linux/fdtable.h>
#include <linux/file.h>
#include <linux/fs.h>
#include <linux/module.h>
#include <linux/anon_inodes.h>
#include <linux/version.h>
#include <linux/uaccess.h>
#include <mali_kbase_sync.h>
#include <mali_base_kernel_sync.h>

static int kbase_stream_close(struct inode *inode, struct file *file)
{
	struct sync_timeline *tl;

	tl = (struct sync_timeline *)file->private_data;
	BUG_ON(!tl);
	sync_timeline_destroy(tl);
	return 0;
}

static const struct file_operations stream_fops = {
	.owner = THIS_MODULE,
	.release = kbase_stream_close,
};

int kbase_stream_create(const char *name, int *const out_fd)
{
	struct sync_timeline *tl;

	BUG_ON(!out_fd);

	tl = kbase_sync_timeline_alloc(name);
	if (!tl)
		return -EINVAL;

	*out_fd = anon_inode_getfd(name, &stream_fops, tl, O_RDONLY | O_CLOEXEC);

	if (*out_fd < 0) {
		sync_timeline_destroy(tl);
		return -EINVAL;
	}

	return 0;
}

int kbase_stream_create_fence(int tl_fd)
{
	struct sync_timeline *tl;
	struct fence *fence;
	struct sync_file *sfile;

	int fd;
	struct file *tl_file;

	tl_file = fget(tl_fd);
	if (tl_file == NULL)
		return -EBADF;

	if (tl_file->f_op != &stream_fops) {
		fd = -EBADF;
		goto out;
	}

	tl = tl_file->private_data;

	fence = kbase_fence_alloc(tl);
	if (!fence) {
		fd = -EFAULT;
		goto out;
	}

	sfile = sync_file_create(fence);
	if (!sfile) {
		fence_put(fence);
		fd = -EFAULT;
		goto out;
	}

	/* from here the sync_fole owns the fence */

	/* create a fd representing the fence */
	fd = get_unused_fd_flags(O_RDWR | O_CLOEXEC);
	if (fd < 0) {
		fput(sfile->file);
		goto out;
	}

	/* bind fence to the new fd */
	fd_install(fd, sfile->file);

 out:
	fput(tl_file);

	return fd;
}

/* sync_file_fdget is static now, so implement it ourselves.
 * We cannot access static fops either, so to verify that this is actual sync
 * call file info ioctl and verify there is a fence attached.
 */
struct sync_file *kbase_sync_file_fdget(int fd)
{
	struct file *file = fget(fd);
	mm_segment_t fs = get_fs();
	struct sync_file_info info = { { 0 } };
	int ret;

	if (!file)
		return NULL;

	if (!file->f_op->unlocked_ioctl)
		goto err;

	set_fs(get_ds());
	ret = file->f_op->unlocked_ioctl(file, SYNC_IOC_FILE_INFO, (unsigned long)(uintptr_t)&info);
	set_fs(fs);
	if (ret != 0 && info.num_fences == 0)
		goto err;

	return file->private_data;

err:
	fput(file);
	return NULL;
}

int kbase_fence_validate(int fd)
{
	struct sync_file *sfile;

	sfile = kbase_sync_file_fdget(fd);
	if (!sfile)
		return -EINVAL;

	fput(sfile->file);
	return 0;
}

#endif				/* CONFIG_SYNC */
