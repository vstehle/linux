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
 * @file mali_kbase_sync.h
 *
 */

#ifndef MALI_KBASE_SYNC_H
#define MALI_KBASE_SYNC_H

#include <linux/mutex.h>
#include "sync_debug.h"

#if LINUX_VERSION_CODE < KERNEL_VERSION(3, 17, 0)
/* For backwards compatiblility with kernels before 3.17. After 3.17
 * sync_pt_parent is included in the kernel. */
static inline struct sync_timeline *sync_pt_parent(struct sync_pt *pt)
{
	return pt->parent;
}
#endif

struct mali_sync_context {
	struct kref kref;
	u64 context;
	spinlock_t lock;
	unsigned seqno;
	unsigned next_signal_seqno;
	struct list_head child_list_head;
};

struct mali_sync_fence {
	struct dma_fence base;
	struct dma_fence_cb cb;
	struct list_head child_list_link;
};

/*
 * Create a stream object.
 * Built on top of sync context object.
 * Exposed as a file descriptor.
 * Life-time controlled via the file descriptor:
 * - dup to add a ref
 * - close to remove a ref
 */
int kbase_stream_create(const char *name, int *const out_fd);

/*
 * Create a fence in a stream object
 */
int kbase_stream_create_fence(int tl_fd, struct sync_file **rsfile);

/*
 * Validate a fd to be a valid fence
 * No reference is taken.
 *
 * This function is only usable to catch unintentional user errors early,
 * it does not stop malicious code changing the fd after this function returns.
 */
int kbase_fence_validate(int fd);

/* Returns true if the specified fence is allocated by Mali */
int kbase_sync_fence_is_ours(struct dma_fence *fence);

/* Allocates a sync context for Mali
 *
 * One sync context should be allocated per API context.
 */
struct mali_sync_context *kbase_sync_context_alloc(const char *name);

/* Free Mali sync context. Also signals all fences it owns.
 */
void kbase_sync_context_free(struct mali_sync_context *msc);

/* Allocates a fence within sync context.
 *
 * The sync context must be the one allocated by kbase_sync_context_alloc
 *
 * Fence must be triggered in *exactly* the same order as they are allocated.
 */
struct dma_fence *kbase_fence_alloc(struct mali_sync_context *msc);

/* Signals a particular fence
 *
 * Fences must be triggered in *exactly* the same order as they are allocated.
 *
 * If they are signalled in the wrong order then a message will be printed in debug
 * builds and otherwise attempts to signal order sync_pts will be ignored.
 *
 * result can be negative to indicate error, any other value is interpreted as success.
 */
void kbase_sync_signal_fence(struct dma_fence *fence, int result);

#endif
