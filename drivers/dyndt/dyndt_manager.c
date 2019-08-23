// SPDX-License-Identifier: GPL-2.0

#define DEBUG

#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/of.h>
#include <linux/timer.h>
#include <linux/kernel.h>
#include <linux/jiffies.h>

// Timer period, in jiffies
#define PERIOD (msecs_to_jiffies(3000) + 1)

/*
 * prop_dis, prop_ok: pre-allocated properties
 * ok: true when status is ok, false when disabled
 */
struct dyndt_manager_data {
	struct timer_list timer;
	struct platform_device *pdev;
	struct device_node *dummy_node;
	struct property *prop_dis;
	struct property *prop_ok;
	bool ok;
};

// TODO! notify ?

static void dyndt_manager_toggle(struct platform_device *pdev)
{
	struct dyndt_manager_data *data = platform_get_drvdata(pdev);
	struct device *dev = &pdev->dev;
	int ret;

	dev_dbg(dev, "%s: %d\n", __func__, data->ok);

	ret = of_update_property(data->dummy_node,
				 data->ok ? data->prop_dis : data->prop_ok);
	if (ret) {
		dev_warn(dev, "%s: could not update property: %d\n", __func__,
			 ret);
	}

	data->ok = !data->ok;
}

// Returns an error pointer
static struct property *dyndt_manager_alloc_prop(struct platform_device *pdev,
						 bool ok)
{
	struct device *dev = &pdev->dev;
	struct property *prop;

	prop = devm_kzalloc(dev, sizeof(*prop), GFP_KERNEL);
	if (!prop) {
		dev_err(dev, "%s: cannot allocate prop!\n", __func__);
		return ERR_PTR(-ENOMEM);
	}

	prop->name = "status";
	prop->value = ok ? "ok" : "disabled";
	prop->length = strlen(prop->value) + 1;
	return prop;
}

static void dyndt_manager_timer_callback(struct timer_list *t)
{
	struct dyndt_manager_data *data =
			container_of(t, struct dyndt_manager_data, timer);
	struct platform_device *pdev = data->pdev;

	dev_dbg(&pdev->dev, "%s\n", __func__);

	dyndt_manager_toggle(pdev);
	mod_timer(t, jiffies + PERIOD);
}

static int dyndt_manager_probe(struct platform_device *pdev)
{
	int ret;
	struct device *dev = &pdev->dev;
	struct dyndt_manager_data *data;
	struct property *status_prop;

	dev_dbg(dev, "%s\n", __func__);

	data = devm_kzalloc(dev, sizeof(*data), GFP_KERNEL);
	if (!data) {
		dev_err(dev, "%s: cannot allocate data!\n", __func__);
		ret = -ENOMEM;
		goto err_data;
	}
	platform_set_drvdata(pdev, data);
	data->pdev = pdev;

	data->prop_ok = dyndt_manager_alloc_prop(pdev, true);
	if (IS_ERR(data->prop_ok)) {
		ret = PTR_ERR(data->prop_ok);
		goto err_stok;
	}

	data->prop_dis = dyndt_manager_alloc_prop(pdev, false);
	if (IS_ERR(data->prop_dis)) {
		ret = PTR_ERR(data->prop_dis);
		goto err_stdis;
	}

	data->dummy_node = of_find_compatible_node(NULL, NULL, "dyndt-dummy");
	if (!data->dummy_node) {
		dev_err(dev, "%s: no dyndt dummy node!\n", __func__);
		ret = -ENODEV;
		goto err_node;
	}

	// Sanity
	status_prop = of_find_property(data->dummy_node, "status", NULL);
	if (!status_prop) {
		dev_err(dev, "%s: no status property!\n", __func__);
		ret = -EINVAL;
		goto err_prop;
	}

	timer_setup(&data->timer, dyndt_manager_timer_callback, 0);
	mod_timer(&data->timer, jiffies + PERIOD);
	return 0;

err_prop:
	of_node_put(data->dummy_node);
err_node:
	devm_kfree(dev, data->prop_dis);
err_stdis:
	devm_kfree(dev, data->prop_ok);
err_stok:
	platform_set_drvdata(pdev, NULL);
	devm_kfree(dev, data);
err_data:
	return ret;
}

static const struct of_device_id dyndt_manager_of_match[] = {
	{ .compatible = "dyndt-manager", },
	{},
};
MODULE_DEVICE_TABLE(of, dyndt_manager_of_match);

struct platform_driver dyndt_manager_pdrv = {
	.probe = dyndt_manager_probe,
	.driver = {
		.name = "dyndt_manager",
		.of_match_table = of_match_ptr(dyndt_manager_of_match),
	},
};

static int __init dyndt_manager_init(void)
{
	int ret;

	pr_debug("%s\n", __func__);

	ret = platform_driver_register(&dyndt_manager_pdrv);
	if (ret) {
		pr_err("%s: platform_driver_register: %d\n", __func__, ret);
		return ret;
	}

	return 0;
}

static void __exit dyndt_manager_exit(void)
{
	pr_debug("%s\n", __func__);
}

module_init(dyndt_manager_init);
module_exit(dyndt_manager_exit);

MODULE_AUTHOR("Vincent Stehlé <vincent.stehle@laposte.net>");
MODULE_DESCRIPTION("Dyndt manager.");
MODULE_LICENSE("GPL");
