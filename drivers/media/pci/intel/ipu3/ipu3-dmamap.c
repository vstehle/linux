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
#include <linux/types.h>
#include <linux/dma-iommu.h>
#include <linux/dma-mapping.h>
#include <linux/highmem.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/version.h>
#include <linux/vmalloc.h>
#include "ipu3-mmu.h"

/*
 * Based on arch/arm64/mm/dma-mapping.c, with simplifications possible due
 * to driver-specific character of this file.
 */

static pgprot_t __get_dma_pgprot(struct dma_attrs *attrs, pgprot_t prot)
{
	if (dma_get_attr(DMA_ATTR_NON_CONSISTENT, attrs))
		return prot;
	return pgprot_writecombine(prot);
}

static void flush_page(struct device *dev, const void *virt, phys_addr_t phys)
{
	/*
	 * FIXME: Yes, casting to override the const specifier is ugly.
	 * However, for some reason, this callback is intended to flush cache
	 * for a page pointed to by a const pointer, even though the cach
	 * flush operation by definition does not keep the affected memory
	 * constant...
	 */
	clflush_cache_range((void *)virt, PAGE_SIZE);
}

static void *ipu3_dmamap_alloc(struct device *dev, size_t size,
			       dma_addr_t *handle, gfp_t gfp,
			       struct dma_attrs *attrs)
{
	int ioprot = dma_direction_to_prot(DMA_BIDIRECTIONAL, false);
	size_t iosize = size;
	struct page **pages;
	pgprot_t prot;
	void *addr;

	if (WARN(!dev, "cannot create IOMMU mapping for unknown device\n"))
		return NULL;

	if (WARN(!gfpflags_allow_blocking(gfp),
		 "atomic allocations not supported\n") ||
	    WARN(dma_get_attr(DMA_ATTR_FORCE_CONTIGUOUS, attrs),
	         "contiguous allocations not supported\n"))
		return NULL;

	size = PAGE_ALIGN(size);

	dev_dbg(dev, "%s: allocating %zu\n", __func__, size);

	/*
	 * Some drivers rely on this, and we probably don't want the
	 * possibility of stale kernel data being read by devices anyway.
	 */
	gfp |= __GFP_ZERO;

	/*
	 * On x86, __GFP_DMA or __GFP_DMA32 might be added implicitly, based
	 * on device DMA mask. However the mask does not apply to the IOMMU,
	 * which is expected to be able to map any physical page.
	 */
	gfp &= ~(__GFP_DMA | __GFP_DMA32);

	pages = iommu_dma_alloc(dev, iosize, gfp, attrs, ioprot,
				handle, flush_page);
	if (!pages)
		return NULL;

	prot = __get_dma_pgprot(attrs, PAGE_KERNEL);
	addr = dma_common_pages_remap(pages, size, VM_USERMAP, prot,
				      __builtin_return_address(0));
	if (!addr)
		iommu_dma_free(dev, pages, iosize, handle);

	dev_dbg(dev, "%s: allocated %zu @ IOVA %pad @ VA %p\n",
		__func__, size, handle, addr);

	return addr;
}

static void ipu3_dmamap_free(struct device *dev, size_t size, void *cpu_addr,
			     dma_addr_t handle, struct dma_attrs *attrs)
{
	struct page **pages;
	size_t iosize = size;

	size = PAGE_ALIGN(size);

	pages = dma_common_get_mapped_pages(cpu_addr, VM_USERMAP);
	if (WARN_ON(!pages))
		return;

	dev_dbg(dev, "%s: freeing %zu @ IOVA %pad @ VA %p\n",
		__func__, size, &handle, cpu_addr);

	iommu_dma_free(dev, pages, iosize, &handle);

	dma_common_free_remap(cpu_addr, size, VM_USERMAP);
}

static int ipu3_dmamap_mmap(struct device *dev, struct vm_area_struct *vma,
			    void *cpu_addr, dma_addr_t dma_addr, size_t size,
			    struct dma_attrs *attrs)
{
	struct page **pages;

	vma->vm_page_prot = __get_dma_pgprot(attrs, vma->vm_page_prot);

	pages = dma_common_get_mapped_pages(cpu_addr, VM_USERMAP);
	if (WARN_ON(!pages))
		return -ENXIO;

	return iommu_dma_mmap(pages, size, vma);
}

static int ipu3_dmamap_get_sgtable(struct device *dev, struct sg_table *sgt,
				   void *cpu_addr, dma_addr_t dma_addr,
				   size_t size, struct dma_attrs *attrs)
{
	unsigned int count = PAGE_ALIGN(size) >> PAGE_SHIFT;
	struct page **pages;

	pages = dma_common_get_mapped_pages(cpu_addr, VM_USERMAP);
	if (WARN_ON(!pages))
		return -ENXIO;

	return sg_alloc_table_from_pages(sgt, pages, count, 0, size,
					 GFP_KERNEL);
}

static dma_addr_t ipu3_dmamap_map_page(struct device *dev, struct page *page,
				   unsigned long offset, size_t size,
				   enum dma_data_direction dir,
				   struct dma_attrs *attrs)
{
	int prot = dma_direction_to_prot(dir, false);

	return iommu_dma_map_page(dev, page, offset, size, prot);
}

static void ipu3_dmamap_unmap_page(struct device *dev, dma_addr_t dev_addr,
			       size_t size, enum dma_data_direction dir,
			       struct dma_attrs *attrs)
{
	iommu_dma_unmap_page(dev, dev_addr, size, dir, attrs);
}

static int ipu3_dmamap_map_sg(struct device *dev, struct scatterlist *sgl,
			      int nents, enum dma_data_direction dir,
			      struct dma_attrs *attrs)
{
	return iommu_dma_map_sg(dev, sgl, nents,
				dma_direction_to_prot(dir, false));
}

static void ipu3_dmamap_unmap_sg(struct device *dev, struct scatterlist *sgl,
				 int nents, enum dma_data_direction dir,
				 struct dma_attrs *attrs)
{
	iommu_dma_unmap_sg(dev, sgl, nents, dir, attrs);
}

static struct dma_map_ops ipu3_dmamap_ops = {
	.alloc = ipu3_dmamap_alloc,
	.free = ipu3_dmamap_free,
	.mmap = ipu3_dmamap_mmap,
	.get_sgtable = ipu3_dmamap_get_sgtable,
	.map_page = ipu3_dmamap_map_page,
	.unmap_page = ipu3_dmamap_unmap_page,
	.map_sg = ipu3_dmamap_map_sg,
	.unmap_sg = ipu3_dmamap_unmap_sg,
	.mapping_error = iommu_dma_mapping_error,
};

int ipu3_dmamap_init(struct device *dev, u64 dma_base, u64 size)
{
	struct iommu_domain *domain;
	int ret;

	ret = iommu_dma_init();
	if (ret)
		return ret;

	/*
	 * The IOMMU core code allocates the default DMA domain, which the
	 * underlying IOMMU driver needs to support via the dma-iommu layer.
	 */
	domain = iommu_get_domain_for_dev(dev);
	if (!domain) {
		pr_warn("Failed to get IOMMU domain for device %s\n",
			dev_name(dev));
		return -ENODEV;
	}

	if (WARN(domain->type != IOMMU_DOMAIN_DMA, "device %s already managed?\n",
		 dev_name(dev)))
		return -EINVAL;

	ret = iommu_dma_init_domain(domain, dma_base, size);
	if (ret) {
		pr_warn("Failed to init IOMMU domain for device %s\n",
			dev_name(dev));
		return ret;
	}

	dev->archdata.dma_ops = &ipu3_dmamap_ops;
	return 0;
}
EXPORT_SYMBOL_GPL(ipu3_dmamap_init);

void ipu3_dmamap_cleanup(struct device *dev)
{
	dev->archdata.dma_ops = &ipu3_dmamap_ops;
	iommu_dma_cleanup();
}
EXPORT_SYMBOL_GPL(ipu3_dmamap_cleanup);

MODULE_AUTHOR("Tomasz Figa <tfiga@chromium.org>");
MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("IPU3 DMA mapping support");
