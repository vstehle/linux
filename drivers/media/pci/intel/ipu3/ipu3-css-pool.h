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

#ifndef __IPU3_UTIL_H
#define __IPU3_UTIL_H

#include <linux/device.h>

#define sqr(x)				((x) * (x))
#define DIV_ROUND_CLOSEST_DOWN(a, b)	(((a) + (b / 2) - 1) / (b))
#define roundclosest_down(a, b)		(DIV_ROUND_CLOSEST_DOWN(a, b) * (b))
#define roundclosest(n, di)				\
	({ typeof(n) __n = (n); typeof(di) __di = (di); \
	DIV_ROUND_CLOSEST(__n, __di) * __di; })

#define IPU3_CSS_POOL_SIZE		4

struct ipu3_css_map {
	size_t size;
	void *vaddr;
	dma_addr_t daddr;
};

struct ipu3_css_pool {
	struct {
		struct ipu3_css_map param;
		long framenum;
	} entry[IPU3_CSS_POOL_SIZE];
	unsigned int last; /* Latest entry */
};

int ipu3_css_dma_alloc(struct device *dma_dev, struct ipu3_css_map *map,
			size_t size);
void ipu3_css_dma_free(struct device *dma_dev, struct ipu3_css_map *map);
void ipu3_css_pool_cleanup(struct device *dma_dev, struct ipu3_css_pool *pool);
int ipu3_css_pool_init(struct device *dma_dev, struct ipu3_css_pool *pool,
			int size);
int ipu3_css_pool_get(struct ipu3_css_pool *pool, long framenum);
void ipu3_css_pool_put(struct ipu3_css_pool *pool);
const struct ipu3_css_map *ipu3_css_pool_last(struct ipu3_css_pool *pool,
						unsigned int last);

#endif
