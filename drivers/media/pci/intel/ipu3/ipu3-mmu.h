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

#ifndef __IPU3_MMU_H
#define __IPU3_MMU_H

struct ipu3_mmu;

struct ipu3_mmu *ipu3_mmu_init(struct device *parent, void __iomem *base,
			       struct bus_type *bus);
void ipu3_mmu_exit(struct ipu3_mmu *mmu);
void ipu3_mmu_suspend(struct ipu3_mmu *mmu);
void ipu3_mmu_resume(struct ipu3_mmu *mmu);

#endif
