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

#ifdef CONFIG_SW_SYNC

#include <linux/sched.h>
#include <linux/fdtable.h>
#include <linux/file.h>
#include <linux/fs.h>
#include <linux/module.h>
#include <linux/anon_inodes.h>
#include <linux/version.h>
#include <linux/uaccess.h>
#include <linux/slab.h>
#include <mali_kbase_sync.h>
#include <mali_base_kernel_sync.h>

static int kbase_stream_close(struct inode *inode, struct file *file)
{
	struct mali_sync_context *msc;

	msc = (struct mali_sync_context *)file->private_data;
	BUG_ON(!msc);
	kbase_sync_context_free(msc);
	return 0;
}

static const struct file_operations stream_fops = {
	.owner = THIS_MODULE,
	.release = kbase_stream_close,
};

int kbase_stream_create(const char *name, int *const out_fd)
{
	struct mali_sync_context *msc;

	BUG_ON(!out_fd);

	msc = kbase_sync_context_alloc(name);
	if (!msc)
		return -EINVAL;

	*out_fd = anon_inode_getfd(name, &stream_fops, msc, O_RDONLY | O_CLOEXEC);

	if (*out_fd < 0) {
		kbase_sync_context_free(msc);
		return -EINVAL;
	}

	return 0;
}

int kbase_stream_create_fence(int tl_fd, struct sync_file **rsfile)
{
	struct mali_sync_context *msc;
	struct dma_fence *fence;
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

	msc = tl_file->private_data;

	fence = kbase_fence_alloc(msc);
	if (!fence) {
		fd = -EFAULT;
		goto out;
	}

	sfile = sync_file_create(fence);
	if (!sfile) {
		dma_fence_put(fence);
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

	/* Return newly created sync_file to caller. */
	fget(fd);
	*rsfile = sfile;

 out:
	fput(tl_file);

	return fd;
}

int kbase_fence_validate(int fd)
{
	struct dma_fence *fence = sync_file_get_fence(fd);
	if (!fence)
		return -EINVAL;

	dma_fence_put(fence);
	return 0;
}

#endif				/* CONFIG_SW_SYNC */
