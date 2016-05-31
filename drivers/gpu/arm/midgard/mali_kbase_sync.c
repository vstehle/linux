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



#ifdef CONFIG_SYNC

#include <linux/mutex.h>
#include <linux/seq_file.h>
#include "sync.h"
#include <mali_kbase.h>
#include <mali_kbase_sync.h>

#define TIMELINE_DRV_NAME "Mali"

struct mali_sync_timeline {
	struct sync_timeline timeline;
	int counter;
	struct mutex counter_lock;
};

struct mali_sync_pt {
	struct fence fence;
	int result;
};

static struct mali_sync_timeline *to_mali_sync_timeline(struct sync_timeline *timeline)
{
	return container_of(timeline, struct mali_sync_timeline, timeline);
}

static struct mali_sync_pt *to_mali_sync_pt(struct fence *fence)
{
	return container_of(fence, struct mali_sync_pt, fence);
}

int kbase_sync_timeline_is_ours(struct sync_timeline *timeline)
{
	if (!timeline)
		return false;
	return strncmp(TIMELINE_DRV_NAME, timeline->drv_name, sizeof(timeline->drv_name)) == 0;
}

struct sync_timeline *kbase_sync_timeline_alloc(const char *name)
{
	struct sync_timeline *tl;
	struct mali_sync_timeline *mtl;

	tl = sync_timeline_create(sizeof(struct mali_sync_timeline), TIMELINE_DRV_NAME, name);
	if (!tl)
		return NULL;

	/* Set the counter in our private struct */
	mtl = to_mali_sync_timeline(tl);
	/* mtl->counter set to 0 by kzalloc */;
	mutex_init(&mtl->counter_lock);

	return tl;
}

struct fence *kbase_fence_alloc(struct sync_timeline *parent)
{
	struct fence *fence;
	struct mali_sync_timeline *mtl = to_mali_sync_timeline(parent);
	struct mali_sync_pt *mpt;

	/* Counter has to be incremented only if fence create succeeds.. */
	mutex_lock(&mtl->counter_lock);
	fence = sync_pt_create(parent, sizeof(struct mali_sync_pt), mtl->counter + 1);

	if (!fence) {
		mutex_unlock(&mtl->counter_lock);
		return NULL;
	}

	mtl->counter++;
	mutex_unlock(&mtl->counter_lock);
	mpt = to_mali_sync_pt(fence);
	mpt->result = 0;

	return fence;
}

void kbase_sync_signal_fence(struct fence *fence, int result)
{
	struct mali_sync_pt *mpt = to_mali_sync_pt(fence);
	struct mali_sync_timeline *mtl = to_mali_sync_timeline(fence_parent(fence));
	unsigned long flags;
	int diff;

	mpt->result = result;

	/* timeline.value is protected by child_list_lock */
	spin_lock_irqsave(&mtl->timeline.child_list_lock, flags);

	diff = mtl->timeline.value - (int)mpt->fence.seqno;

	if (diff > 0) {
		/* The timeline is already at or ahead of this point.
		 * This should not happen unless userspace has been
		 * signalling fences out of order, so warn but don't
		 * violate the sync_pt API.
		 * The warning is only in debug builds to prevent
		 * a malicious user being able to spam dmesg.
		 */
#ifdef CONFIG_MALI_DEBUG
		pr_err("Fences were triggered in a different order to allocation!");
#endif				/* CONFIG_MALI_DEBUG */
		spin_unlock_irqrestore(&mtl->timeline.child_list_lock, flags);
		sync_timeline_signal(&mtl->timeline, 0);
		return;
	}

	/* We set timeline value ourselves and just use sync_signal_timeline to
	 * remove fences from the list
	 */
	mtl->timeline.value = (int)mpt->fence.seqno;
	spin_unlock_irqrestore(&mtl->timeline.child_list_lock, flags);
	sync_timeline_signal(&mtl->timeline, 0);
}

#endif				/* CONFIG_SYNC */
