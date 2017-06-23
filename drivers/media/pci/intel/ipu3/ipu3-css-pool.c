/*
 * Copyright (c) 2017 Intel Corporation.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License version
 * 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#include <linux/types.h>
#include <linux/dma-mapping.h>

#include "ipu3-css-pool.h"

int ipu3_css_dma_alloc(struct device *dma_dev,
		       struct ipu3_css_map *map, size_t size)
{
	if (size == 0) {
		map->vaddr = NULL;
		return 0;
	}

	map->vaddr = dma_alloc_coherent(dma_dev, size, &map->daddr, GFP_KERNEL);
	if (!map->vaddr)
		return -ENOMEM;
	map->size = size;

	return 0;
}

void ipu3_css_dma_free(struct device *dma_dev, struct ipu3_css_map *map)
{
	if (map->vaddr)
		dma_free_coherent(dma_dev, map->size, map->vaddr, map->daddr);
	map->vaddr = NULL;
}

void ipu3_css_pool_cleanup(struct device *dma_dev, struct ipu3_css_pool *pool)
{
	int i;

	for (i = 0; i < IPU3_CSS_POOL_SIZE; i++)
		ipu3_css_dma_free(dma_dev, &pool->entry[i].param);
}

int ipu3_css_pool_init(struct device *dma_dev, struct ipu3_css_pool *pool,
		       int size)
{
	int i;

	for (i = 0; i < IPU3_CSS_POOL_SIZE; i++) {
		pool->entry[i].framenum = INT_MIN;
		if (ipu3_css_dma_alloc(dma_dev, &pool->entry[i].param, size))
			goto fail;
	}

	pool->last = IPU3_CSS_POOL_SIZE;

	return 0;

fail:
	ipu3_css_pool_cleanup(dma_dev, pool);
	return -ENOMEM;
}

/*
 * Check that the following call to pool_get succeeds.
 * Return negative on error.
 */
static int ipu3_css_pool_check(struct ipu3_css_pool *pool, long framenum)
{
	/* Get the oldest entry */
	int n = (pool->last + 1) % IPU3_CSS_POOL_SIZE;

	/*
	 * pool->entry[n].framenum stores the frame number where that
	 * entry was allocated. If that was allocated more than POOL_SIZE
	 * frames back, it is old enough that we know it is no more in
	 * use by firmware.
	 */
	if (pool->entry[n].framenum + IPU3_CSS_POOL_SIZE > framenum)
		return -ENOSPC;

	return n;
}

/*
 * Allocate a new parameter from pool at frame number `framenum'.
 * Release the oldest entry in the pool to make space for the new entry.
 * Return negative on error.
 */
int ipu3_css_pool_get(struct ipu3_css_pool *pool, long framenum)
{
	int n = ipu3_css_pool_check(pool, framenum);

	if (n < 0)
		return n;

	pool->entry[n].framenum = framenum;
	pool->last = n;

	return n;
}

/*
 * Undo, for all practical purposes, the effect of pool_get().
 */
void ipu3_css_pool_put(struct ipu3_css_pool *pool)
{
	pool->entry[pool->last].framenum = INT_MIN;
	pool->last = (pool->last + IPU3_CSS_POOL_SIZE - 1) % IPU3_CSS_POOL_SIZE;
}

const struct ipu3_css_map *
ipu3_css_pool_last(struct ipu3_css_pool *pool, unsigned int n)
{
	static const struct ipu3_css_map null_map = { 0 };
	int i = (pool->last + IPU3_CSS_POOL_SIZE - n) % IPU3_CSS_POOL_SIZE;

	WARN_ON(n >= IPU3_CSS_POOL_SIZE);

	if (pool->entry[i].framenum < 0)
		return &null_map;

	return &pool->entry[i].param;
}
