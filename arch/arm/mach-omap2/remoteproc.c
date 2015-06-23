/*
 * Remote processor machine-specific module for OMAP4
 *
 * Copyright (C) 2011 Texas Instruments, Inc.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#define pr_fmt(fmt)    "%s: " fmt, __func__

#include <linux/kernel.h>
#include <linux/err.h>
#include <linux/remoteproc.h>
#include <linux/dma-contiguous.h>
#include <linux/dma-mapping.h>

#include <../arch/arm/mach-omap2/omap_device.h>
#include <../arch/arm/mach-omap2/omap_hwmod.h>
#include <linux/platform_data/remoteproc-omap.h>
#include <linux/iommu.h>
#include <../arch/arm/mach-omap2/omap-pm.h>
#include <linux/platform_data/iommu-omap.h>

#include "cm1_44xx.h"
#include "cm2_44xx.h"
#include "cm-regbits-44xx.h"

int ducati_device_enable(struct platform_device *pdev)
{
	int ret;

	printk("ducati_device_enable\n");

	printk("  deassert reset for cpu0\n");
	ret = omap_device_deassert_hardreset(pdev, "cpu0");
	if (ret < 0)
		return ret;

//	printk("  deassert reset for cpu1\n");
//	ret = omap_device_deassert_hardreset(pdev, "cpu1");
//	if (ret < 0)
//		return ret;

	return omap_device_enable(pdev);
}

int ducati_device_shutdown(struct platform_device *pdev)
{
	int ret;

	printk("ducati_device_shutdown\n");

	printk("  assert reset for cpu0\n");
	ret = omap_device_assert_hardreset(pdev, "cpu0");
	if (ret < 0)
		return ret;

//	printk("  assert reset for cpu1\n");
//	ret = omap_device_assert_hardreset(pdev, "cpu1");
//	if (ret < 0)
//		return ret;

	return omap_device_idle(pdev);
}

/*
 * These data structures define platform-specific information
 * needed for each supported remote processor.
 *
 * At this point we only support the remote dual M3 "Ducati" imaging
 * subsystem (aka "ipu").
 */
static struct omap_rproc_pdata omap4_rproc_data[] = {
	{
		.name		= "ipu",
		.firmware	= "ducati-m3-core0.xem3",
		.mbox_name	= "mbox_ipu",
		.oh_name	= "ipu",
//		.oh_name_opt	= "ipu",
		.device_enable	= ducati_device_enable,
		.device_shutdown = ducati_device_shutdown,
	},
};

static struct omap_iommu_arch_data omap4_rproc_iommu[] = {
	{ .name = "55082000.mmu" },
};

static struct platform_device omap4_ducati = {
	.name	= "omap-rproc",
	.id	= 1,
};

static struct platform_device *omap4_rproc_devs[] __initdata = {
	&omap4_ducati,
};

static int __init omap_rproc_init(void)
{
	struct omap_hwmod *oh[2];
	struct omap_device *od;
	int i, ret = 0, oh_count;

	printk("omap_rproc_init\n");

	/* build the remote proc devices */
	for (i = 0; i < ARRAY_SIZE(omap4_rproc_data); i++) {
		const char *oh_name = omap4_rproc_data[i].oh_name;
		const char *oh_name_opt = omap4_rproc_data[i].oh_name_opt;
		struct platform_device *pdev = omap4_rproc_devs[i];
		oh_count = 0;

		printk("  name: %s\n", oh_name);

		oh[0] = omap_hwmod_lookup(oh_name);
		if (!oh[0]) {
			pr_err("could not look up %s\n", oh_name);
			continue;
		}
		oh_count++;

		printk("  looked up\n");

		/*
		 * ipu might have a secondary hwmod entry (for configurations
		 * where we want both M3 cores to be represented by a single
		 * device).
		 */
		if (oh_name_opt) {
			printk("  opt name: %s\n", oh_name_opt);

			oh[1] = omap_hwmod_lookup(oh_name_opt);
			if (!oh[1]) {
				pr_err("could not look up %s\n", oh_name_opt);
				continue;
			}
			oh_count++;
			printk("  looked up opt\n");
		}

		device_initialize(&pdev->dev);
		printk("  dev initialized\n");

		/* Set dev_name early to allow dev_xxx in omap_device_alloc */
		dev_set_name(&pdev->dev, "%s.%d", pdev->name,  pdev->id);

		od = omap_device_alloc(pdev, oh, oh_count);
		if (!od) {
			dev_err(&pdev->dev, "omap_device_alloc failed\n");
			put_device(&pdev->dev);
			ret = PTR_ERR(od);
			continue;
		}

		printk("  omap dev allocated\n");

		ret = platform_device_add_data(pdev,
					&omap4_rproc_data[i],
					sizeof(struct omap_rproc_pdata));
		if (ret) {
			dev_err(&pdev->dev, "can't add pdata\n");
			omap_device_delete(od);
			put_device(&pdev->dev);
			continue;
		}

		printk("  platform dev data added\n");

		/* attach the remote processor to its iommu device */
		pdev->dev.archdata.iommu = &omap4_rproc_iommu[i];

		ret = omap_device_register(pdev);
		if (ret) {
			dev_err(&pdev->dev, "omap_device_register failed\n");
			omap_device_delete(od);
			put_device(&pdev->dev);
			continue;
		}

		printk("  omap dev registered\n");
	}

	printk("  return %i\n", ret);
	return ret;
}
device_initcall(omap_rproc_init);
