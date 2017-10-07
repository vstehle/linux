/*
 * cros_ec_pd_update - Chrome OS EC Power Delivery Device FW Update Driver
 *
 * Copyright (C) 2014 Google, Inc
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * This driver communicates with a Chrome OS PD device and performs tasks
 * related to auto-updating its firmware.
 */

#include <linux/acpi.h>
#include <linux/firmware.h>
#include <linux/kernel.h>
#include <linux/kobject.h>
#include <linux/mfd/cros_ec.h>
#include <linux/mfd/cros_ec_commands.h>
#include <linux/mfd/cros_ec_pd_update.h>
#include <linux/module.h>

/* Store our PD device pointer so we can send update-related commands. */
static struct cros_ec_dev *pd_ec;

/**
 * firmware_images - Keep this updated with the latest RW FW + hash for each
 * PD device. Entries should be primary sorted by id_major and secondary
 * sorted by id_minor.
 */
static const struct cros_ec_pd_firmware_image firmware_images[] = {
	/* PD_DEVICE_TYPE_ZINGER */
	{
		.id_major = PD_DEVICE_TYPE_ZINGER,
		.id_minor = 1,
		.filename = "cros-pd/zinger_000002.bin",
		.hash = { 0x9e, 0x28, 0xfb, 0x69, 0x9c,
			  0xf9, 0xc3, 0x3c, 0x47, 0x26,
			  0x10, 0x26, 0x48, 0x6a, 0xe1,
			  0xaf, 0x71, 0x44, 0x95, 0xc6 },
	},
};

static const int firmware_image_count = ARRAY_SIZE(firmware_images);

/**
 * cros_ec_pd_get_status - Get info about a possible PD device attached to a
 * given port. Returns 0 on success, <0 on failure.
 *
 * @dev: PD device
 * @pd_dev: EC PD device
 * @port: Port # on device
 * @result: Stores received EC command result, on success
 * @hash_entry: Stores received PD device RW FW info, on success
 */
static int cros_ec_pd_get_status(struct device *dev,
				 struct cros_ec_dev *pd_dev,
				 int port,
				 uint32_t *result,
				 struct ec_params_usb_pd_rw_hash_entry
					*hash_entry)
{
	int ret;
	struct cros_ec_command *msg;
	struct ec_params_usb_pd_info_request *info_request;

	msg = kzalloc(sizeof(*msg) + sizeof(*info_request), GFP_KERNEL);
	if (!msg)
		return -ENOMEM;

	info_request = (struct ec_params_usb_pd_info_request *)msg->data;
	info_request->port = port;

	msg->command = EC_CMD_USB_PD_DEV_INFO | pd_dev->cmd_offset;
	msg->insize = sizeof(*hash_entry);
	msg->outsize = sizeof(*info_request);

	ret = cros_ec_cmd_xfer(pd_dev->ec_dev, msg);
	if (ret < 0) {
		dev_err(dev, "Unable to get device status (err:%d)\n", ret);
		goto error;
	}

	memcpy(hash_entry, msg->data, sizeof(*hash_entry));
	*result = msg->result;
	ret = 0;
error:
	kfree(msg);
	return ret;
}

/**
 * cros_ec_pd_send_hash_entry - Inform the EC of a PD devices for which we
 * have firmware available. EC typically will not store more than four hashes.
 * Returns 0 on success, <0 on failure.
 *
 * @dev: device
 * @pd_dev: EC PD device
 * @fw: FW update image to inform the EC of
 */
static int cros_ec_pd_send_hash_entry(struct device *dev,
				      struct cros_ec_dev *pd_dev,
				      const struct cros_ec_pd_firmware_image
						   *fw)
{
	int ret;
	struct cros_ec_command *msg;
	struct ec_params_usb_pd_rw_hash_entry *hash_entry;

	msg = kzalloc(sizeof(*msg) + sizeof(*hash_entry), GFP_KERNEL);
	if (!msg)
		return -ENOMEM;

	hash_entry = (struct ec_params_usb_pd_rw_hash_entry *)msg->data;

	msg->command = EC_CMD_USB_PD_RW_HASH_ENTRY | pd_dev->cmd_offset;
	msg->insize = 0;
	msg->outsize = sizeof(*hash_entry);

	hash_entry->dev_id = MAJOR_MINOR_TO_DEV_ID(fw->id_major, fw->id_minor);
	memcpy(hash_entry->dev_rw_hash.b, fw->hash, PD_RW_HASH_SIZE);

	ret = cros_ec_cmd_xfer(pd_dev->ec_dev, msg);
	if (ret < 0)
		dev_err(dev, "Unable to send device hash (err:%d)\n", ret);

	kfree(msg);
	return ret;
}

/**
 * cros_ec_pd_send_fw_update_cmd - Calls cros_ec_cmd_xfer to send update-
 * related EC cmmand.
 *
 * @pd_dev: EC PD device
 * @msg: EC command message with non-size parameters already set
 * @cmd: fw_update command
 * @size: pd command payload size in bytes
 */
static int cros_ec_pd_send_fw_update_cmd(
	struct cros_ec_device *pd_dev,
	struct cros_ec_command *msg,
	uint8_t cmd,
	uint32_t size)
{
	struct ec_params_usb_pd_fw_update *pd_cmd =
		(struct ec_params_usb_pd_fw_update *)msg->data;

	pd_cmd->cmd = cmd;
	pd_cmd->size = size;
	msg->outsize = pd_cmd->size + sizeof(*pd_cmd);

	return cros_ec_cmd_xfer(pd_dev, msg);
}


/**
 * cros_ec_pd_fw_update - Send EC_CMD_USB_PD_FW_UPDATE command to perform
 * update-related operation.
 *
 * @dev: PD device
 * @fw: RW FW update file
 * @pd_dev: EC PD device
 * @port: Port# to which update device is attached
 */
static int cros_ec_pd_fw_update(struct device *dev,
				const struct firmware *fw,
				struct cros_ec_dev *pd_dev,
				uint8_t port)
{
	struct cros_ec_command *msg;
	int i, ret;

	struct ec_params_usb_pd_fw_update *pd_cmd;
	uint8_t *pd_cmd_data;

	msg = kzalloc(sizeof(*msg) + sizeof(*pd_cmd) + PD_FLASH_WRITE_STEP,
		      GFP_KERNEL);
	if (!msg)
		return -ENOMEM;

	pd_cmd = (struct ec_params_usb_pd_fw_update *)msg->data;
	pd_cmd_data = (uint8_t *)(pd_cmd + 1);

	/* Common host command */
	msg->command = EC_CMD_USB_PD_FW_UPDATE | pd_dev->cmd_offset;
	msg->insize = 0;

	/* Common port */
	pd_cmd->port = port;

	/* Erase signature */
	ret = cros_ec_pd_send_fw_update_cmd(pd_dev->ec_dev, msg,
					    USB_PD_FW_ERASE_SIG, 0);
	if (ret < 0) {
		dev_err(dev, "Unable to clear PD signature (err:%d)\n", ret);
		goto error;
	}

	/* Reboot PD */
	ret = cros_ec_pd_send_fw_update_cmd(pd_dev->ec_dev, msg,
					    USB_PD_FW_REBOOT, 0);
	if (ret < 0) {
		dev_err(dev, "Unable to reboot PD (err:%d)\n", ret);
		goto error;
	}

	/* Erase RW flash */
	ret = cros_ec_pd_send_fw_update_cmd(pd_dev->ec_dev, msg,
					    USB_PD_FW_FLASH_ERASE, 0);
	if (ret < 0) {
		dev_err(dev, "Unable to erase PD RW flash (err:%d)\n", ret);
		goto error;
	}

	/* Write RW flash */
	for (i = 0; i < fw->size; i += PD_FLASH_WRITE_STEP) {
		int size = min(fw->size - i, (size_t)PD_FLASH_WRITE_STEP);
		memcpy(pd_cmd_data, fw->data + i, size);
		ret = cros_ec_pd_send_fw_update_cmd(pd_dev->ec_dev,
						    msg,
						    USB_PD_FW_FLASH_WRITE,
						    size);
		if (ret < 0) {
			dev_err(dev, "Unable to write PD RW flash (err:%d)\n",
				ret);
			goto error;
		}
	}

	/* Reboot PD into new RW */
	ret = cros_ec_pd_send_fw_update_cmd(pd_dev->ec_dev, msg,
					    USB_PD_FW_REBOOT, 0);
	if (ret < 0) {
		dev_err(dev, "Unable to reboot PD post-update (err:%d)\n", ret);
		goto error;
	}

	ret = 0;
error:
	kfree(msg);
	return ret;
}

/**
 * find_firmware_image - Search firmware image table for an image matching
 * the passed PD device id. Returns matching index if id is found and
 * PD_NO_IMAGE if id is not found in table.
 *
 * @dev_id: Target PD device id
 */
static int find_firmware_image(uint16_t dev_id)
{
	/*
	 * TODO(shawnn): Replace sequential table search with modified binary
	 * search on major / minor.
	 */
	int i;

	for (i = 0; i < firmware_image_count; ++i)
		if (MAJOR_MINOR_TO_DEV_ID(firmware_images[i].id_major,
					  firmware_images[i].id_minor)
					  == dev_id)
			return i;

	return PD_NO_IMAGE;
}

/**
 * acpi_cros_ec_pd_notify - Upon receiving a notification host event from the
 * EC, probe the status of attached PD devices and kick off an RW firmware
 * update if needed.
 */
static void acpi_cros_ec_pd_notify(struct acpi_device *acpi_device, u32 event)
{
	const struct firmware *fw;
	struct ec_params_usb_pd_rw_hash_entry hash_entry;
	struct device *dev = &acpi_device->dev;
	char *file;
	uint32_t result;
	int ret, port, i;

	if (!pd_ec) {
		dev_err(dev, "No pd_ec device found\n");
		return;
	}

	/*
	 * If there is an EC based charger, send a notification to it to
	 * trigger a refresh of the power supply state.
	 */
	if (pd_ec->ec_dev->charger) {
		power_supply_changed(pd_ec->ec_dev->charger);
	}

	/* Received notification, send command to check on PD status. */
	for (port = 0; port < PD_MAX_PORTS; ++port) {
		ret = cros_ec_pd_get_status(dev, pd_ec, port, &result,
					    &hash_entry);
		if (ret < 0) {
			dev_err(dev, "Can't get device status (err:%d)\n",
				ret);
			return;
		} else if (result == EC_RES_SUCCESS) {
			if (hash_entry.dev_id == PD_DEVICE_TYPE_NONE)
				i = PD_NO_IMAGE;
			else
				i = find_firmware_image(hash_entry.dev_id);

			/* Device found, should we update firmware? */
			if (i != PD_NO_IMAGE &&
			    memcmp(hash_entry.dev_rw_hash.b,
				   firmware_images[i].hash,
				   PD_RW_HASH_SIZE) != 0) {
				file = firmware_images[i].filename;
				ret = request_firmware(&fw, file, dev);
				if (ret) {
					dev_err(dev, "Error, can't load file %s\n",
						file);
					continue;
				}

				if (fw->size > PD_RW_IMAGE_SIZE) {
					dev_err(dev, "Firmware file %s is too large\n",
						file);
					goto done;
				}

				/* Update firmware */
				cros_ec_pd_fw_update(dev, fw, pd_ec, port);
done:
				release_firmware(fw);
			} else if (i != PD_NO_IMAGE) {
				/**
				 * Device already has latest firmare. Send
				 * hash entry to EC so we don't get subsequent
				 * FW update requests.
				 */
				cros_ec_pd_send_hash_entry(dev,
							   pd_ec,
							   &firmware_images[i]);
			} else {
				/* Unknown PD device -- don't update FW */
			}
		}
		/* Non-success status, we've probed every port that exists. */
		else
			break;
	}

}

static int acpi_cros_ec_pd_add(struct acpi_device *acpi_device)
{
	return 0;
}

static int acpi_cros_ec_pd_remove(struct acpi_device *acpi_device)
{
	return 0;
}

static umode_t cros_ec_pd_attrs_are_visible(struct kobject *kobj,
					    struct attribute *a, int n)
{
	struct device *dev = container_of(kobj, struct device, kobj);
	struct cros_ec_dev *ec = container_of(dev, struct cros_ec_dev,
					      class_dev);
	uint32_t result;
	struct ec_params_usb_pd_rw_hash_entry hash_entry;

	/* Check if a PD MCU is present */
	if (cros_ec_pd_get_status(dev, ec, 0, &result, &hash_entry) == 0 &&
	    result == EC_RES_SUCCESS) {
		/*
		 * Save our ec pointer so we can conduct transactions.
		 * TODO(shawnn): Find a better way to access the ec pointer.
		 */
		if (!pd_ec)
			pd_ec = ec;
		return a->mode;
	}

	return 0;
}

static ssize_t show_firmware_images(struct device *dev,
				    struct device_attribute *attr, char *buf) {
	int size = 0;
	int i;

	for (i = 0; i < firmware_image_count; ++i) {
		if (firmware_images[i].filename == NULL)
			size += scnprintf(buf + size, PAGE_SIZE, "%d: NONE\n",
					  i);
		else
			size += scnprintf(buf + size, PAGE_SIZE, "%d: %s\n", i,
					  firmware_images[i].filename);
	}

	return size;
}


static DEVICE_ATTR(firmware_images, S_IRUGO, show_firmware_images, NULL);

static struct attribute *__pd_attrs[] = {
	&dev_attr_firmware_images.attr,
	NULL,
};

struct attribute_group cros_ec_pd_attr_group = {
	.name = "pd_update",
	.attrs = __pd_attrs,
	.is_visible = cros_ec_pd_attrs_are_visible,
};

/**
 * TODO(shawnn): Find an alternative notification method for devices which
 * don't use ACPI.
 */
static const struct acpi_device_id pd_device_ids[] = {
	{ "GOOG0003", 0 },
	{ }
};

MODULE_DEVICE_TABLE(acpi, pd_device_ids);

static struct acpi_driver acpi_cros_ec_pd_driver = {
	.name = "cros_ec_pd_update",
	.class = "cros_ec_pd_update",
	.ids = pd_device_ids,
	.ops = {
		.add = acpi_cros_ec_pd_add,
		.remove = acpi_cros_ec_pd_remove,
		.notify = acpi_cros_ec_pd_notify,
	},
};

module_acpi_driver(acpi_cros_ec_pd_driver);

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("ChromeOS power device FW update driver");
