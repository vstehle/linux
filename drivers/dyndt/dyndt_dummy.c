// SPDX-License-Identifier: GPL-2.0

#define DEBUG

#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/of.h>

static int dyndt_dummy_probe(struct platform_device *pdev)
{
	dev_dbg(&pdev->dev, "%s\n", __func__);
	return 0;
}

static int dyndt_dummy_remove(struct platform_device *pdev)
{
	dev_dbg(&pdev->dev, "%s\n", __func__);
	return 0;
}

static const struct of_device_id dyndt_dummy_of_match[] = {
	{ .compatible = "dyndt-dummy", },
	{},
};
MODULE_DEVICE_TABLE(of, dyndt_dummy_of_match);

struct platform_driver dyndt_dummy_pdrv = {
	.probe = dyndt_dummy_probe,
	.remove = dyndt_dummy_remove,
	.driver = {
		.name = "dyndt_dummy",
		.of_match_table = of_match_ptr(dyndt_dummy_of_match),
	},
};

static int __init dyndt_dummy_init(void)
{
	int ret;

	pr_debug("%s\n", __func__);

	ret = platform_driver_register(&dyndt_dummy_pdrv);
	if (ret) {
		pr_err("%s: platform_driver_register: %d\n", __func__, ret);
		return ret;
	}

	return 0;
}

static void __exit dyndt_dummy_exit(void)
{
	pr_debug("%s\n", __func__);
}

module_init(dyndt_dummy_init);
module_exit(dyndt_dummy_exit);

MODULE_AUTHOR("Vincent Stehlé <vincent.stehle@laposte.net>");
MODULE_DESCRIPTION("Dyndt dummy.");
MODULE_LICENSE("GPL");
