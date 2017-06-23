/*
 * Copyright (c) 2017 Intel Corporation.
 * Copyright (C) 2017 Google, Inc.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License version
 * 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */

#include <linux/dma-iommu.h>
#include <linux/iova.h>
#include <linux/iommu.h>
#include <linux/iopoll.h>
#include <linux/module.h>
#include <linux/pm_runtime.h>
#include <linux/slab.h>
#include <linux/vmalloc.h>

#include <asm/cacheflush.h>

#include "ipu3-mmu.h"

#define IPU3_PAGE_SHIFT		12
#define IPU3_PAGE_SIZE		(1UL << IPU3_PAGE_SHIFT)

#define IPU3_PT_BITS		10
#define IPU3_PT_PTES		(1UL << IPU3_PT_BITS)

#define IPU3_ADDR2PTE(addr)	((addr) >> IPU3_PAGE_SHIFT)
#define IPU3_PTE2ADDR(pte)	((phys_addr_t)(pte) << IPU3_PAGE_SHIFT)

#define IPU3_L2PT_SHIFT		IPU3_PT_BITS
#define IPU3_L2PT_MASK		((1UL << IPU3_L2PT_SHIFT) - 1)

#define IPU3_L1PT_SHIFT		IPU3_PT_BITS
#define IPU3_L1PT_MASK		((1UL << IPU3_L1PT_SHIFT) - 1)

#define IPU3_MMU_ADDRESS_BITS	(IPU3_PAGE_SHIFT + \
				 IPU3_L2PT_SHIFT + \
				 IPU3_L1PT_SHIFT)

#define IMGU_REG_BASE		0x4000
#define REG_TLB_INVALIDATE	(IMGU_REG_BASE + 0x300)
#define TLB_INVALIDATE		1
#define REG_L1_PHYS		(IMGU_REG_BASE + 0x304) /* 27-bit pfn */
#define REG_GP_HALT		(IMGU_REG_BASE + 0x5dc)
#define REG_GP_HALTED		(IMGU_REG_BASE + 0x5e0)

struct ipu3_mmu_domain {
	struct iommu_domain domain;

	struct ipu3_mmu *mmu;
	spinlock_t lock;

	void *dummy_page;
	u32 dummy_page_pteval;

	u32 *dummy_l2pt;
	u32 dummy_l2pt_pteval;

	u32 **l2pts;
};

struct ipu3_mmu {
	struct device *dev;
	struct bus_type *bus;
	void __iomem *base;
	struct iommu_group *group;
	struct iommu_ops ops;
	struct ipu3_mmu_domain *domain;

	u32 *l1pt;
};

static inline struct ipu3_mmu *to_ipu3_mmu(struct device *dev)
{
	const struct iommu_ops *iommu_ops = dev->bus->iommu_ops;

	return container_of(iommu_ops, struct ipu3_mmu, ops);
}

static inline struct ipu3_mmu_domain *
to_ipu3_mmu_domain(struct iommu_domain *domain)
{
	return container_of(domain, struct ipu3_mmu_domain, domain);
}

/**
 * ipu3_mmu_tlb_invalidate - invalidate translation look-aside buffer
 * @mmu: MMU to perform the invalidate operation on
 *
 * This function invalidates the whole TLB. Must be called when the hardware
 * is powered on.
 */
static void ipu3_mmu_tlb_invalidate(struct ipu3_mmu *mmu)
{
	writel(TLB_INVALIDATE, mmu->base + REG_TLB_INVALIDATE);
}

static void call_if_ipu3_is_powered(struct ipu3_mmu *mmu,
				    void (*func)(struct ipu3_mmu *mmu))
{
	pm_runtime_get_noresume(mmu->dev);
	if (pm_runtime_active(mmu->dev))
		func(mmu);
	pm_runtime_put(mmu->dev);
}

/**
 * ipu3_mmu_set_halt - set CIO gate halt bit
 * @mmu: MMU to set the CIO gate bit in.
 * @halt: Desired state of the gate bit.
 *
 * This function sets the CIO gate bit that controls whether external memory
 * accesses are allowed. Must be called when the hardware is powered on.
 */
static void ipu3_mmu_set_halt(struct ipu3_mmu *mmu, bool halt)
{
	int ret;
	u32 val;

	writel(halt, mmu->base + REG_GP_HALT);
	ret = readl_poll_timeout(mmu->base + REG_GP_HALTED,
				 val, (val & 1) == halt, 1000, 100000);

	if (ret)
		dev_err(mmu->dev, "failed to %s CIO gate halt\n",
			halt ? "set" : "clear");
}

/**
 * ipu3_mmu_alloc_page_table - allocate a pre-filled page table
 * @pteval: Value to initialize for page table entries with.
 *
 * Return: Pointer to allocated page table or NULL on failure.
 */
static u32 *ipu3_mmu_alloc_page_table(u32 pteval)
{
	u32 *pt;
	int pte;

	pt = kmalloc(IPU3_PT_PTES * sizeof(*pt), GFP_KERNEL);
	if (!pt)
		return NULL;

	for (pte = 0; pte < IPU3_PT_PTES; pte++)
		pt[pte] = pteval;

	clflush_cache_range(pt, IPU3_PT_PTES * sizeof(*pt));

	return pt;
}

/**
 * ipu3_mmu_free_page_table - free page table
 * @pt: Page table to free.
 */
static void ipu3_mmu_free_page_table(u32 *pt)
{
	kfree(pt);
}

/**
 * address_to_pte_idx - split IOVA into L1 and L2 page table indices
 * @iova: IOVA to split.
 * @l1pt_idx: Output for the L1 page table index.
 * @l2pt_idx: Output for the L2 page index.
 */
static void address_to_pte_idx(unsigned long iova, u32 *l1pt_idx,
			       u32 *l2pt_idx)
{
	iova >>= IPU3_PAGE_SHIFT;

	if (l2pt_idx)
		*l2pt_idx = iova & IPU3_L2PT_MASK;

	iova >>= IPU3_L2PT_SHIFT;

	if (l1pt_idx)
		*l1pt_idx = iova & IPU3_L1PT_MASK;
}

static struct iommu_domain *ipu3_mmu_domain_alloc(unsigned int type)
{
	struct ipu3_mmu_domain *mmu_dom;
	u32 pteval;

	if (WARN(type != IOMMU_DOMAIN_DMA,
		 "IPU3 MMU only supports DMA domains\n"))
		return NULL;

	mmu_dom = kzalloc(sizeof(*mmu_dom), GFP_KERNEL);
	if (!mmu_dom)
		return NULL;

	if (iommu_get_dma_cookie(&mmu_dom->domain))
		goto fail_domain;

	mmu_dom->domain.geometry.aperture_start = 0;
	mmu_dom->domain.geometry.aperture_end =
		DMA_BIT_MASK(IPU3_MMU_ADDRESS_BITS);
	mmu_dom->domain.geometry.force_aperture = true;

	/*
	 * The MMU does not have a "valid" bit, so we have to use a dummy
	 * page for invalid entries.
	 */
	mmu_dom->dummy_page = kzalloc(IPU3_PAGE_SIZE, GFP_KERNEL);
	if (!mmu_dom->dummy_page)
		goto fail_cookie;
	pteval = IPU3_ADDR2PTE(virt_to_phys(mmu_dom->dummy_page));
	mmu_dom->dummy_page_pteval = pteval;

	/*
	 * Allocate a dummy L2 page table with all entries pointing to
	 * the dummy page.
	 */
	mmu_dom->dummy_l2pt = ipu3_mmu_alloc_page_table(pteval);
	if (!mmu_dom->dummy_l2pt)
		goto fail_page;
	pteval = IPU3_ADDR2PTE(virt_to_phys(mmu_dom->dummy_l2pt));
	mmu_dom->dummy_l2pt_pteval = pteval;

	/*
	 * Allocate the array of L2PT CPU pointers, initialized to zero,
	 * which means the dummy L2PT allocated above.
	 */
	mmu_dom->l2pts = vzalloc(IPU3_PT_PTES * sizeof(*mmu_dom->l2pts));
	if (!mmu_dom->l2pts)
		goto fail_l2pt;

	spin_lock_init(&mmu_dom->lock);
	return &mmu_dom->domain;

fail_l2pt:
	ipu3_mmu_free_page_table(mmu_dom->dummy_l2pt);
fail_page:
	kfree(mmu_dom->dummy_page);
fail_cookie:
	iommu_put_dma_cookie(&mmu_dom->domain);
fail_domain:
	kfree(mmu_dom);
	return NULL;
}

static void ipu3_mmu_domain_free(struct iommu_domain *domain)
{
	struct ipu3_mmu_domain *mmu_dom = to_ipu3_mmu_domain(domain);
	int pte;

	/* We expect the domain to be detached already. */
	WARN_ON(mmu_dom->mmu);

	for (pte = 0; pte < IPU3_PT_PTES; ++pte)
		ipu3_mmu_free_page_table(mmu_dom->l2pts[pte]); /* NULL-safe */
	vfree(mmu_dom->l2pts);

	ipu3_mmu_free_page_table(mmu_dom->dummy_l2pt);
	kfree(mmu_dom->dummy_page);
	iommu_put_dma_cookie(domain);

	kfree(mmu_dom);
}

static void ipu3_mmu_disable(struct ipu3_mmu *mmu)
{
	ipu3_mmu_set_halt(mmu, true);
	ipu3_mmu_tlb_invalidate(mmu);
}

static void ipu3_mmu_detach_dev(struct iommu_domain *domain,
				struct device *dev)
{
	struct ipu3_mmu_domain *mmu_dom = to_ipu3_mmu_domain(domain);
	struct ipu3_mmu *mmu = to_ipu3_mmu(dev);
	unsigned long flags;

	if (mmu->domain != mmu_dom)
		return;

	/* Disallow external memory access when having no valid page tables. */
	call_if_ipu3_is_powered(mmu, ipu3_mmu_disable);

	spin_lock_irqsave(&mmu_dom->lock, flags);

	mmu->domain = NULL;
	mmu_dom->mmu = NULL;

	dev_dbg(dev, "%s: Detached from domain %p\n", __func__, mmu_dom);

	spin_unlock_irqrestore(&mmu_dom->lock, flags);

	memset(mmu->l1pt, 0, IPU3_PT_PTES * sizeof(*mmu->l1pt));
	clflush_cache_range(mmu->l1pt, IPU3_PT_PTES * sizeof(*mmu->l1pt));
}

static void ipu3_mmu_enable(struct ipu3_mmu *mmu)
{
	ipu3_mmu_tlb_invalidate(mmu);
	ipu3_mmu_set_halt(mmu, false);
}

static int ipu3_mmu_attach_dev(struct iommu_domain *domain,
			       struct device *dev)
{
	struct ipu3_mmu_domain *mmu_dom = to_ipu3_mmu_domain(domain);
	struct ipu3_mmu *mmu = to_ipu3_mmu(dev);
	unsigned long flags;
	unsigned int pte;

	if (mmu->domain == mmu_dom)
		return 0;

	if (mmu->domain)
		ipu3_mmu_detach_dev(&mmu->domain->domain, dev);

	spin_lock_irqsave(&mmu_dom->lock, flags);

	for (pte = 0; pte < IPU3_PT_PTES; ++pte) {
		u32 *l2pt = mmu_dom->l2pts[pte];
		u32 pteval = mmu_dom->dummy_l2pt_pteval;

		if (l2pt)
			pteval = IPU3_ADDR2PTE(virt_to_phys(l2pt));

		mmu->l1pt[pte] = pteval;
	}

	clflush_cache_range(mmu->l1pt, IPU3_PT_PTES * sizeof(*mmu->l1pt));

	mmu->domain = mmu_dom;
	mmu_dom->mmu = mmu;

	dev_dbg(dev, "%s: Attached to domain %p\n", __func__, mmu_dom);

	spin_unlock_irqrestore(&mmu_dom->lock, flags);

	/* We have valid page tables, allow external memory access. */
	call_if_ipu3_is_powered(mmu, ipu3_mmu_enable);

	return 0;
}

static u32 *ipu3_mmu_get_l2pt(struct ipu3_mmu_domain *mmu_dom, u32 l1pt_idx,
			      bool allocate)
{
	unsigned long flags;
	u32 *l2pt, *new_l2pt;

	spin_lock_irqsave(&mmu_dom->lock, flags);

	l2pt = mmu_dom->l2pts[l1pt_idx];
	if (l2pt || !allocate)
		goto done;

	spin_unlock_irqrestore(&mmu_dom->lock, flags);

	new_l2pt = ipu3_mmu_alloc_page_table(mmu_dom->dummy_page_pteval);
	if (!new_l2pt)
		return NULL;

	spin_lock_irqsave(&mmu_dom->lock, flags);

	dev_dbg(mmu_dom->mmu->dev,
		"allocated page table %p for l1pt_idx %u\n",
		new_l2pt, l1pt_idx);

	l2pt = mmu_dom->l2pts[l1pt_idx];
	if (l2pt) {
		ipu3_mmu_free_page_table(new_l2pt);
		goto done;
	}

	l2pt = new_l2pt;
	mmu_dom->l2pts[l1pt_idx] = new_l2pt;

	if (mmu_dom->mmu) {
		u32 pteval;

		pteval = IPU3_ADDR2PTE(virt_to_phys(new_l2pt));
		mmu_dom->mmu->l1pt[l1pt_idx] = pteval;
		clflush_cache_range(&mmu_dom->mmu->l1pt[l1pt_idx],
				    sizeof(*mmu_dom->mmu->l1pt));
	}

done:
	spin_unlock_irqrestore(&mmu_dom->lock, flags);
	return l2pt;
}

static int ipu3_mmu_map(struct iommu_domain *domain, unsigned long iova,
			phys_addr_t paddr, size_t size, int prot)
{
	struct ipu3_mmu_domain *mmu_dom = to_ipu3_mmu_domain(domain);
	u32 l1pt_idx, l2pt_idx;
	unsigned long flags;
	u32 *l2pt;

	/* We assume a page by page mapping. */
	if (WARN_ON(size != IPU3_PAGE_SIZE))
		return -EINVAL;

	address_to_pte_idx(iova, &l1pt_idx, &l2pt_idx);

	l2pt = ipu3_mmu_get_l2pt(mmu_dom, l1pt_idx, true);
	if (!l2pt)
		return -ENOMEM;

	spin_lock_irqsave(&mmu_dom->lock, flags);

	if (l2pt[l2pt_idx] != mmu_dom->dummy_page_pteval) {
		spin_unlock_irqrestore(&mmu_dom->lock, flags);
		return -EBUSY;
	}

	l2pt[l2pt_idx] = IPU3_ADDR2PTE(paddr);

	clflush_cache_range(&l2pt[l2pt_idx], sizeof(*l2pt));

	if (mmu_dom->mmu)
		call_if_ipu3_is_powered(mmu_dom->mmu, ipu3_mmu_tlb_invalidate);

	spin_unlock_irqrestore(&mmu_dom->lock, flags);

	return 0;
}

static size_t ipu3_mmu_unmap(struct iommu_domain *domain, unsigned long iova,
			     size_t size)
{
	struct ipu3_mmu_domain *mmu_dom = to_ipu3_mmu_domain(domain);
	u32 l1pt_idx, l2pt_idx;
	unsigned long flags;
	u32 *l2pt;

	/* We assume a page by page unmapping. */
	if (WARN_ON(size != IPU3_PAGE_SIZE))
		return 0;

	address_to_pte_idx(iova, &l1pt_idx, &l2pt_idx);

	l2pt = ipu3_mmu_get_l2pt(mmu_dom, l1pt_idx, false);
	if (!l2pt)
		return 0;

	spin_lock_irqsave(&mmu_dom->lock, flags);

	if (l2pt[l2pt_idx] == mmu_dom->dummy_page_pteval)
		size = 0;
	l2pt[l2pt_idx] = mmu_dom->dummy_page_pteval;

	clflush_cache_range(&l2pt[l2pt_idx], sizeof(*l2pt));

	if (mmu_dom->mmu)
		call_if_ipu3_is_powered(mmu_dom->mmu, ipu3_mmu_tlb_invalidate);

	spin_unlock_irqrestore(&mmu_dom->lock, flags);

	return size;
}

static phys_addr_t ipu3_mmu_iova_to_phys(struct iommu_domain *domain,
					 dma_addr_t iova)
{
	struct ipu3_mmu_domain *d = to_ipu3_mmu_domain(domain);
	u32 l1pt_idx, l2pt_idx;
	u32 pteval;
	u32 *l2pt;

	address_to_pte_idx(iova, &l1pt_idx, &l2pt_idx);

	l2pt = ipu3_mmu_get_l2pt(d, l1pt_idx, false);
	if (!l2pt)
		return 0;

	pteval = l2pt[l2pt_idx];
	if (pteval == d->dummy_page_pteval)
		return 0;

	return IPU3_PTE2ADDR(pteval);
}

static struct iommu_group *ipu3_mmu_device_group(struct device *dev)
{
	struct ipu3_mmu *mmu = to_ipu3_mmu(dev);

	return mmu->group;
}

static int ipu3_mmu_add_device(struct device *dev)
{
	struct iommu_group *group;

	group = iommu_group_get_for_dev(dev);
	if (IS_ERR(group))
		return PTR_ERR(group);

	iommu_group_put(group);
	return 0;
}

static void ipu3_mmu_remove_device(struct device *dev)
{
	struct iommu_domain *domain = iommu_get_domain_for_dev(dev);

	if (!domain)
		return;

	ipu3_mmu_detach_dev(domain, dev);
}

static struct iommu_ops ipu3_iommu_ops_template = {
	.domain_alloc   = ipu3_mmu_domain_alloc,
	.domain_free    = ipu3_mmu_domain_free,
	.attach_dev	= ipu3_mmu_attach_dev,
	.detach_dev	= ipu3_mmu_detach_dev,
	.map		= ipu3_mmu_map,
	.unmap		= ipu3_mmu_unmap,
	.map_sg		= default_iommu_map_sg,
	.iova_to_phys	= ipu3_mmu_iova_to_phys,
	.device_group	= ipu3_mmu_device_group,
	.add_device	= ipu3_mmu_add_device,
	.remove_device	= ipu3_mmu_remove_device,
	.pgsize_bitmap	= SZ_4K,
};

/**
 * ipu3_mmu_init() - initialize IPU3 MMU block
 * @parent:	Parent IPU device.
 * @base:	IOMEM base of hardware registers.
 * @bus:	Bus on which DMA devices are registered.
 *
 * Return: Pointer to IPU3 MMU private data pointer or ERR_PTR() on error.
 */
struct ipu3_mmu *ipu3_mmu_init(struct device *parent, void __iomem *base,
			       struct bus_type *bus)
{
	struct ipu3_mmu *mmu;
	u32 pteval;
	int ret;

	mmu = kzalloc(sizeof(*mmu), GFP_KERNEL);
	if (!mmu)
		return ERR_PTR(-ENOMEM);
	mmu->base = base;
	mmu->dev = parent;
	mmu->bus = bus;
	mmu->ops = ipu3_iommu_ops_template;

	/* Disallow external memory access when having no valid page tables. */
	ipu3_mmu_set_halt(mmu, true);

	/*
	 * Allocate the L1 page table.
	 *
	 * NOTE that the hardware does not allow changing the L1 page table
	 * at runtime, so we use shadow L1 tables with CPU L2 table pointers
	 * per-domain and update the L1 table on domain attach and detach.
	 */
	mmu->l1pt = ipu3_mmu_alloc_page_table(0);
	if (!mmu->l1pt) {
		ret = -ENOMEM;
		goto fail_mmu;
	}

	mmu->group = iommu_group_alloc();
	if (!mmu->group) {
		ret = -ENOMEM;
		goto fail_l1pt;
	}

	pteval = IPU3_ADDR2PTE(virt_to_phys(mmu->l1pt));
	writel(pteval, mmu->base + REG_L1_PHYS);
	ipu3_mmu_tlb_invalidate(mmu);

	bus_set_iommu(bus, &mmu->ops);

	return mmu;

fail_l1pt:
	ipu3_mmu_free_page_table(mmu->l1pt);
fail_mmu:
	kfree(mmu);

	return ERR_PTR(ret);
}
EXPORT_SYMBOL_GPL(ipu3_mmu_init);

/**
 * ipu3_mmu_exit() - clean up IPU3 MMU block
 * @mmu: IPU3 MMU private data
 */
void ipu3_mmu_exit(struct ipu3_mmu *mmu)
{
	bus_set_iommu(mmu->bus, NULL);

	/* We are going to free our page tables, no more memory access. */
	ipu3_mmu_set_halt(mmu, true);
	ipu3_mmu_tlb_invalidate(mmu);
	ipu3_mmu_free_page_table(mmu->l1pt);
	iommu_group_put(mmu->group);
	kfree(mmu);
}
EXPORT_SYMBOL_GPL(ipu3_mmu_exit);

void ipu3_mmu_suspend(struct ipu3_mmu *mmu)
{
	ipu3_mmu_set_halt(mmu, true);
}
EXPORT_SYMBOL_GPL(ipu3_mmu_suspend);

void ipu3_mmu_resume(struct ipu3_mmu *mmu)
{
	u32 pteval;

	ipu3_mmu_set_halt(mmu, true);

	pteval = IPU3_ADDR2PTE(virt_to_phys(mmu->l1pt));
	writel(pteval, mmu->base + REG_L1_PHYS);

	ipu3_mmu_tlb_invalidate(mmu);

	if (mmu->domain)
		ipu3_mmu_set_halt(mmu, false);
}
EXPORT_SYMBOL_GPL(ipu3_mmu_resume);

MODULE_AUTHOR("Tuukka Toivonen <tuukka.toivonen@intel.com>");
MODULE_AUTHOR("Sakari Ailus <sakari.ailus@linux.intel.com>");
MODULE_AUTHOR("Samu Onkalo <samu.onkalo@intel.com>");
MODULE_AUTHOR("Tomasz Figa <tfiga@chromium.org>");
MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("ipu3 mmu driver");
