/*
 * Copyright (c) 2017 Intel Corporation.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License version
 * 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * Purpose of this module:
 *
 * 1) Create any needed extra devices from ACPI nodes which were not created
 *    by the kernel (primarily VCM and EEPROM devices)
 *
 * 2) Return connection information on given MIPI-camera device specified
 *    by ACPI handle:
 *    - Return i2c bus information (address) for the device driver
 *    - Return CSI2 bus information for the bridge driver
 *
 */

#include <linux/module.h>
#include <media/intel-acpi-camera.h>

struct __packed intel_acpi_camera_ssdb {
	u8 version;
	u8 sensor_card_sku;
	u8 csi2_data_stream_interface[16];
	u16 bdf_value;
	u32 dphy_link_en_fuses;
	u32 lanes_clock_division;
	u8 link_used;
	u8 lanes_used;
	u32 csi_rx_dly_cnt_termen_clane;
	u32 csi_rx_dly_cnt_settle_clane;
	u32 csi_rx_dly_cnt_termen_dlane0;
	u32 csi_rx_dly_cnt_settle_dlane0;
	u32 csi_rx_dly_cnt_termen_dlane1;
	u32 csi_rx_dly_cnt_settle_dlane1;
	u32 csi_rx_dly_cnt_termen_dlane2;
	u32 csi_rx_dly_cnt_settle_dlane2;
	u32 csi_rx_dly_cnt_termen_dlane3;
	u32 csi_rx_dly_cnt_settle_dlane3;
	u32 max_lane_speed;
	u8 sensor_calibration_file_index;
	u8 sensor_calibration_file_index_mbz[3];
	u8 rom_type;
	u8 vcm_type;
	u8 platform;
	u8 platform_sub;
	u8 flash_support;
	u8 privacy_led;
	u8 degree;		/* Camera module rotation */
	u8 mipi_define;
	u32 mclk;
	u8 control_logic_id;
	u8 mipi_data_format;
	u8 silicon_version;
	u8 customer_id;
};

struct __packed intel_acpi_camera_cldb {
	u8 version;
	u8 control_logic_type;
	u8 control_logic_id;
	u8 sensor_card_sku;
	u8 reserved[28];
};

struct intel_acpi_camera_pwr_entry tps68470_imx135[] = {
	{
	 .name = "VCM",
	 .value = 2797400,
	 .entry_type = INTEL_ACPI_CAMERA_REGULATOR,
	 },
	{
	 .name = "VIO",
	 .value = 1800600,
	 .entry_type = INTEL_ACPI_CAMERA_REGULATOR,
	 },
	{
	 .name = "VSIO",
	 .value = 1800600,
	 .entry_type = INTEL_ACPI_CAMERA_REGULATOR,
	 },
	{
	 .name = "ANA",
	 .value = 2708400,
	 .entry_type = INTEL_ACPI_CAMERA_REGULATOR,
	 },
	{
	 .name = "tps68470-a",
	 .value = 24000000,
	 .entry_type = INTEL_ACPI_CAMERA_CLK,
	 },
	{
	 .name = "CORE",
	 .value = 1050000,
	 .entry_type = INTEL_ACPI_CAMERA_REGULATOR,
	 .delay = 3000,
	 },
	{
	 .name = "gpio.5",
	 .value = 1,
	 .entry_type = INTEL_ACPI_CAMERA_GPIO,
	 },
	{
	 .name = "gpio.6",
	 .value = 1,
	 .entry_type = INTEL_ACPI_CAMERA_GPIO,
	 },
	{
	 .name = "s_resetn",
	 .value = 1,
	 .entry_type = INTEL_ACPI_CAMERA_GPIO,
	 .delay = 3000,
	 },
	{
	 .name = "daisy-chain",
	 .value = 1,
	 .entry_type = INTEL_ACPI_CAMERA_GPIO,
	 },
};

static struct intel_acpi_camera_pwr_entry tps68470_ov5670[] = {
	{
	 .name = "VSIO",
	 .value = 1800600,
	 .entry_type = INTEL_ACPI_CAMERA_REGULATOR,
	 },
	{
	 .name = "AUX2",
	 .value = 1800600,
	 .entry_type = INTEL_ACPI_CAMERA_REGULATOR,
	 },
	{
	 .name = "tps68470-b",
	 .value = 19200000,
	 .entry_type = INTEL_ACPI_CAMERA_CLK,
	 },
	{
	 .name = "gpio.4",
	 .value = 1,
	 .entry_type = INTEL_ACPI_CAMERA_GPIO,
	 },
	{
	 .name = "gpio.5",
	 .value = 1,
	 .entry_type = INTEL_ACPI_CAMERA_GPIO,
	 },
	{
	 .name = "AUX1",
	 .value = 1213200,
	 .entry_type = INTEL_ACPI_CAMERA_REGULATOR,
	 .delay = 300000,
	 },
};

static struct intel_acpi_camera_pwr_entry tps68470_ov13850[] = {
	{
	 .name = "VSIO",
	 .value = 1800600,
	 .entry_type = INTEL_ACPI_CAMERA_REGULATOR,
	 },
	{
	 .name = "tps68470-a",
	 .value = 19200000,
	 .entry_type = INTEL_ACPI_CAMERA_CLK,
	 },
	{
	 .name = "ANA",
	 .value = 2815200,
	 .entry_type = INTEL_ACPI_CAMERA_REGULATOR,
	 },
	{
	 .name = "VCM",
	 .value = 2815200,
	 .entry_type = INTEL_ACPI_CAMERA_REGULATOR,
	 },
	{
	 .name = "s_resetn",
	 .value = 1,
	 .entry_type = INTEL_ACPI_CAMERA_GPIO,
	 },
	{
	 .name = "CORE",
	 .value = 1200000,
	 .entry_type = INTEL_ACPI_CAMERA_REGULATOR,
	 },
	{
	 .name = "daisy-chain",
	 .value = 1,
	 .entry_type = INTEL_ACPI_CAMERA_GPIO,
	 .delay = 3000,
	 },
};

static struct intel_acpi_camera_pmic_config supported_sensors_pmic[] = {
	{
	 .sensor = "INT3471",
	 .pmic = "INT3472",
	 .pmic_type = 2,	/* TPS68470 */
	 .pwr_entry_num = ARRAY_SIZE(tps68470_imx135),
	 .pwr_entry = tps68470_imx135,
	 },
	{
	 .sensor = "OVTID850",
	 .pmic = "INT3472",
	 .pmic_type = 2,	/* TPS68470 */
	 .pwr_entry_num = ARRAY_SIZE(tps68470_ov13850),
	 .pwr_entry = tps68470_ov13850,
	 },
	{
	 .sensor = "INT3479",
	 .pmic = "INT3472",
	 .pmic_type = 2,	/* TPS68470 */
	 .pwr_entry_num = ARRAY_SIZE(tps68470_ov5670),
	 .pwr_entry = tps68470_ov5670,
	 },
};

static int intel_acpi_camera_ssdb(acpi_handle ahandle,
				  struct intel_acpi_camera_ssdb *ssdb)
{
	struct acpi_buffer buf = { ACPI_ALLOCATE_BUFFER, NULL };
	union acpi_object *obj;
	struct intel_acpi_camera_ssdb *s;
	int r;

	if (!ahandle || !ssdb)
		return -EINVAL;

	r = acpi_evaluate_object(ahandle, "SSDB", NULL, &buf);
	obj = buf.pointer;
	if (ACPI_FAILURE(r) || !obj)
		return -EINVAL;
	if (obj->type != ACPI_TYPE_BUFFER)
		goto error;
	s = (struct intel_acpi_camera_ssdb *)obj->buffer.pointer;
	if (!s || obj->buffer.length < sizeof(*s))
		goto error;
	if (s->mipi_define != 0x01)
		goto error;

	*ssdb = *s;
	kfree(obj);
	return 0;

error:
	kfree(obj);
	return -EINVAL;
}

u64 intel_acpi_camera_camd(acpi_handle ahandle)
{
	struct acpi_buffer buf = { ACPI_ALLOCATE_BUFFER, NULL };
	union acpi_object *obj;
	int r;
	u64 value = INTEL_ACPI_CAMERA_UNKNOWN;

	if (!ahandle)
		return value;

	r = acpi_evaluate_object(ahandle, "CAMD", NULL, &buf);
	obj = buf.pointer;
	if (ACPI_FAILURE(r) || !obj)
		return value;

	if (obj->type != ACPI_TYPE_INTEGER) {
		kfree(obj);
		return value;
	}

	value = obj->integer.value;

	kfree(obj);
	return value;
}

EXPORT_SYMBOL_GPL(intel_acpi_camera_camd);

static acpi_status crlmodule_acpi_pmic_walk(acpi_handle ahandle, u32 lvl,
					    void *info, void **rv)
{
	struct acpi_buffer buf = { ACPI_ALLOCATE_BUFFER, NULL };
	struct intel_acpi_camera_cldb *cldb;
	struct acpi_device *adev = NULL;
	struct intel_acpi_camera_i2c *i2c_info = info;
	union acpi_object *obj;
	int ret, i;

	ret = acpi_bus_get_device(ahandle, &adev);
	if (ret || !adev || !adev->status.present)
		return AE_OK;

	ret = acpi_evaluate_object(ahandle, "CLDB", NULL, &buf);
	if (ACPI_FAILURE(ret) || !buf.pointer)
		return AE_OK;

	obj = buf.pointer;
	if (obj->type != ACPI_TYPE_BUFFER || obj->buffer.length < sizeof(*cldb))
		return AE_OK;

	cldb = (struct intel_acpi_camera_cldb *)obj->buffer.pointer;

	/* match against sensor hid, pmic hid and type */
	for (i = 0; i < ARRAY_SIZE(supported_sensors_pmic); i++) {
		if (!strcmp(supported_sensors_pmic[i].pmic,
			    acpi_device_hid(adev)) &&
		    !strcmp(supported_sensors_pmic[i].sensor, i2c_info->hid) &&
		    supported_sensors_pmic[i].pmic_type ==
		    cldb->control_logic_type)
			break;
	}
	if (i < ARRAY_SIZE(supported_sensors_pmic))
		i2c_info->pmic_config = &supported_sensors_pmic[i];

	ACPI_FREE(obj);

	return AE_OK;
}

int intel_acpi_camera_i2c(struct device *dev,
			  struct intel_acpi_camera_i2c *info)
{
	struct intel_acpi_camera_ssdb ssdb;
	int ret = intel_acpi_camera_ssdb(ACPI_HANDLE(dev), &ssdb);

	if (ret)
		return -ENODEV;

	memset(info, 0, sizeof(*info));
	info->mclk = ssdb.mclk;
	info->lanes = ssdb.lanes_used;
	info->xshutdown = -1;
	info->hid = acpi_device_hid(ACPI_COMPANION(dev));

	/* look for pmic */
	acpi_walk_namespace(ACPI_TYPE_DEVICE, ACPI_ROOT_OBJECT,
			    ACPI_UINT32_MAX,
			    crlmodule_acpi_pmic_walk, NULL, info, NULL);

	return 0;
}

EXPORT_SYMBOL_GPL(intel_acpi_camera_i2c);

int intel_acpi_camera_csi2(acpi_handle ahandle,
			   struct intel_acpi_camera_csi2 *info)
{
	struct intel_acpi_camera_ssdb ssdb;
	int ret = intel_acpi_camera_ssdb(ahandle, &ssdb);

	if (ret)
		return -ENODEV;

	memset(info, 0, sizeof(*info));
	info->port = ssdb.link_used;
	info->lanes = ssdb.lanes_used;
	return 0;
}

EXPORT_SYMBOL_GPL(intel_acpi_camera_csi2);

MODULE_AUTHOR("Tianshu Qiu <tian.shu.qiu@intel.com>");
MODULE_AUTHOR("Jian Xu Zheng <jian.xu.zheng@intel.com>");
MODULE_AUTHOR("Yuning Pu <yuning.pu@intel.com>");
MODULE_AUTHOR("Tuukka Toivonen <tuukka.toivonen@intel.com>");
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Intel ACPI camera support");
