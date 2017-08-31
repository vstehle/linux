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



#ifdef CONFIG_SW_SYNC

#include <linux/seq_file.h>
#include <linux/slab.h>
#include "sync_debug.h"
#include <mali_kbase.h>
#include <mali_kbase_sync.h>

static void kbase_sync_context_put(struct mali_sync_context *msc);

static struct mali_sync_fence *to_mali_fence(struct dma_fence *fence)
{
	return container_of(fence, struct mali_sync_fence, base);
}

static struct mali_sync_context *mali_fence_parent(struct dma_fence *fence)
{
	return container_of(fence->lock, struct mali_sync_context, lock);
}

static const char *mali_fence_get_driver_name(struct dma_fence *fence)
{
	return "mali";
}

static const char *mali_fence_get_timeline_name(struct dma_fence *fence)
{
	return "mali_timeline";
}

static bool mali_fence_enable_signaling(struct dma_fence *fence)
{
	return true;
}

static void mali_fence_release(struct dma_fence *fence)
{
	struct mali_sync_fence *mf = to_mali_fence(fence);

	WARN_ON(!list_empty(&mf->child_list_link));
	kbase_sync_context_put(mali_fence_parent(fence));
	dma_fence_free(fence);
}

struct dma_fence_ops mali_sync_fence_ops = {
	.get_driver_name = mali_fence_get_driver_name,
	.get_timeline_name = mali_fence_get_timeline_name,
	.enable_signaling = mali_fence_enable_signaling,
	.wait = dma_fence_default_wait,
	.release = mali_fence_release
};

int kbase_sync_fence_is_ours(struct dma_fence *fence)
{
	return fence->ops == &mali_sync_fence_ops;
}

static void signal_all_before_fence(struct mali_sync_fence *mf)
{
	struct mali_sync_context *msc = mali_fence_parent(&mf->base);
	unsigned save_next_seqno = msc->next_signal_seqno;
	struct mali_sync_fence *it, *next;

	lockdep_assert_held(mf->base.lock);

	list_for_each_entry_safe(it, next, &msc->child_list_head, child_list_link)
		if (dma_fence_is_later(&mf->base, &it->base)) {
			/* prevent recursion */
			msc->next_signal_seqno = it->base.seqno;
			dma_fence_signal_locked(&it->base);
		}
	msc->next_signal_seqno = save_next_seqno;
}

static void mali_fence_signal_cb(struct dma_fence *fence,
				 struct dma_fence_cb *cb)
{
	struct mali_sync_fence *mf = to_mali_fence(fence);
	struct mali_sync_context *msc = mali_fence_parent(fence);

	lockdep_assert_held(fence->lock);
	/* Callbacks are called with lock held so it is safe to traverse
	 * child list.
	 */
	if (fence->seqno != msc->next_signal_seqno)
		signal_all_before_fence(mf);

	msc->next_signal_seqno = fence->seqno + 1;

	list_del_init(&mf->child_list_link);
	dma_fence_put(&mf->base);
}

struct mali_sync_context *kbase_sync_context_alloc(const char *name)
{
	struct mali_sync_context *msc;

	msc = kzalloc(sizeof(*msc), GFP_KERNEL);
	if (!msc)
		return NULL;

	kref_init(&msc->kref);
	msc->context = dma_fence_context_alloc(1);
	spin_lock_init(&msc->lock);
	INIT_LIST_HEAD(&msc->child_list_head);

	return msc;
}

static void kbase_sync_context_signal_all(struct mali_sync_context *msc)
{
	unsigned long flags;
	struct mali_sync_fence *it, *next;

	spin_lock_irqsave(&msc->lock, flags);

	list_for_each_entry_safe(it, next, &msc->child_list_head, child_list_link) {
		dma_fence_set_error(&it->base, -ENOENT);
		dma_fence_signal_locked(&it->base);
	}

	spin_unlock_irqrestore(&msc->lock, flags);
}

static void kbase_sync_context_release(struct kref *kref)
{
	struct mali_sync_context *msc =
		container_of(kref, struct mali_sync_context, kref);

	WARN_ON(spin_is_locked(&msc->lock));
	WARN_ON(!list_empty(&msc->child_list_head));
	kfree(msc);
}

static void kbase_sync_context_get(struct mali_sync_context *msc)
{
	kref_get(&msc->kref);
}

static void kbase_sync_context_put(struct mali_sync_context *msc)
{
	kref_put(&msc->kref, kbase_sync_context_release);
}

void kbase_sync_context_free(struct mali_sync_context *msc)
{
	kbase_sync_context_signal_all(msc);
	kbase_sync_context_put(msc);
}

struct dma_fence *kbase_fence_alloc(struct mali_sync_context *msc)
{
	struct mali_sync_fence *mf;
	unsigned long flags;
	int ret;

	mf = kzalloc(sizeof(*mf), GFP_KERNEL);

	if (!mf)
		return ERR_PTR(-ENOMEM);

	spin_lock_irqsave(&msc->lock, flags);
	dma_fence_init(&mf->base,
		       &mali_sync_fence_ops,
		       &msc->lock,
		       msc->context,
		       msc->seqno);

	msc->seqno++;

	/* Get a ref so the lock doesn't disappear on us. */
	kbase_sync_context_get(msc);
	INIT_LIST_HEAD(&mf->child_list_link);
	list_add_tail(&mf->child_list_link, &msc->child_list_head);
	/* Get a fence ref while it is on child list */
	dma_fence_get(&mf->base);
	spin_unlock_irqrestore(&msc->lock, flags);

	ret = dma_fence_add_callback(&mf->base, &mf->cb, mali_fence_signal_cb);
	if (ret == -ENOENT) {
		/* Fence was signaled before callback was added. */
		spin_lock_irqsave(&msc->lock, flags);
		signal_all_before_fence(mf); /* Redundant? */
		list_del_init(&mf->child_list_link);
		/* If this fence has been signaled, it means someone has created
		   newer fence and signaled it. So next_signal_seqno has been
		   updated there.
		 */
		spin_unlock_irqrestore(&msc->lock, flags);
		dma_fence_put(&mf->base);
	} else if (ret < 0) {
		goto fail;
	}
	return &mf->base;
fail:
	spin_lock_irqsave(&msc->lock, flags);
	list_del_init(&mf->child_list_link);
	spin_unlock_irqrestore(&msc->lock, flags);
	dma_fence_put(&mf->base);
	kbase_sync_context_put(msc);
	dma_fence_put(&mf->base);
	return NULL;
}

void kbase_sync_signal_fence(struct dma_fence *fence, int result)
{
	unsigned long flags;

	spin_lock_irqsave(fence->lock, flags);

	if (dma_fence_get_status_locked(fence) == 0) {
		if (result < 0)
			dma_fence_set_error(fence, result);
		dma_fence_signal_locked(fence);
	}
	spin_unlock_irqrestore(fence->lock, flags);
}

#endif				/* CONFIG_SW_SYNC */
