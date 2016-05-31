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

#include <linux/seq_file.h>
#include <linux/slab.h>
#include "sync.h"
#include <mali_kbase.h>
#include <mali_kbase_sync.h>

#define TIMELINE_DRV_NAME "Mali"

struct mali_sync_pt {
	struct sync_pt sync_pt;
	int result;
};

static struct mali_sync_pt *to_mali_sync_pt(struct sync_pt *sync_pt)
{
	return container_of(sync_pt, struct mali_sync_pt, sync_pt);
}

int kbase_sync_timeline_is_ours(struct sync_timeline *timeline)
{
	if (!timeline)
		return false;
	return strncmp(TIMELINE_DRV_NAME, timeline->drv_name, sizeof(timeline->drv_name)) == 0;
}

struct mali_sync_timeline *kbase_sync_timeline_alloc(const char *name)
{
	struct mali_sync_timeline *mtl;

	mtl = kzalloc(sizeof(*mtl), GFP_KERNEL);
	if (!mtl)
		return NULL;

	mtl->timeline = sync_timeline_create(TIMELINE_DRV_NAME, name);
	if (!mtl->timeline) {
		kfree(mtl);
		return NULL;
	}

	/* mtl->counter set to 0 by kzalloc */;
	mutex_init(&mtl->counter_lock);

	return mtl;
}

void kbase_sync_timeline_free(struct mali_sync_timeline *mtl)
{
	mutex_destroy(&mtl->counter_lock);
	sync_timeline_destroy(mtl->timeline);
	kfree(mtl);
}

struct fence *kbase_fence_alloc(struct mali_sync_timeline *mtl)
{
	struct sync_pt *sync_pt;
	struct mali_sync_pt *mpt;

	/* Counter has to be incremented only if fence create succeeds.. */
	mutex_lock(&mtl->counter_lock);
	sync_pt = sync_pt_create(mtl->timeline, sizeof(struct mali_sync_pt), mtl->counter + 1);

	if (!sync_pt) {
		mutex_unlock(&mtl->counter_lock);
		return NULL;
	}

	mtl->counter++;
	mutex_unlock(&mtl->counter_lock);
	mpt = to_mali_sync_pt(sync_pt);
	mpt->result = 0;

	return &sync_pt->base;
}

void kbase_sync_signal_fence(struct fence *fence, int result)
{
	struct mali_sync_pt *mpt = to_mali_sync_pt(fence_to_sync_pt(fence));
	struct sync_timeline *tl = fence_parent(fence);
	unsigned long flags;
	int diff;

	mpt->result = result;

	/* timeline.value is protected by child_list_lock */
	spin_lock_irqsave(&tl->child_list_lock, flags);

	diff = tl->value - (int)mpt->sync_pt.base.seqno;

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
		spin_unlock_irqrestore(&tl->child_list_lock, flags);
		sync_timeline_signal(tl, 0);
		return;
	}

	/* We set timeline value ourselves and just use sync_signal_timeline to
	 * remove fences from the list
	 */
	tl->value = (int)mpt->sync_pt.base.seqno;
	spin_unlock_irqrestore(&tl->child_list_lock, flags);
	sync_timeline_signal(tl, 0);
}

#endif				/* CONFIG_SYNC */
