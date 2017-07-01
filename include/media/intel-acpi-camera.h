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
 *
 */

#ifndef INTEL_ACPI_CAMERA_H
#define INTEL_ACPI_CAMERA_H

#include <linux/acpi.h>

enum intel_acpi_camera_pwr_type {
	INTEL_ACPI_CAMERA_REGULATOR = 0,
	INTEL_ACPI_CAMERA_CLK,
	INTEL_ACPI_CAMERA_GPIO,
};

struct intel_acpi_camera_pwdb {
	char name[32];
	u32 value;
	u32 entry_type;
	u32 delay;
} __packed;

struct intel_acpi_camera_pwr_entry {
	char name[16];
	u32 value;
	enum intel_acpi_camera_pwr_type entry_type;
	u32 delay;
};

struct intel_acpi_camera_pmic_config {
	char *sensor;
	char *pmic;
	/*
	 * control logic type
	 * 0: UNKNOWN
	 * 1: DISCRETE(CRD-D)
	 * 2: PMIC TPS68470
	 * 3: PMIC uP6641
	 */
	int pmic_type;
	int pwr_entry_num;
	struct intel_acpi_camera_pwr_entry *pwr_entry;
};

/* define acpi camera device type */
enum intel_acpi_camera_device_type {
	INTEL_ACPI_CAMERA_CIO2 = 0,
	INTEL_ACPI_CAMERA_IMGU,
	INTEL_ACPI_CAMERA_SENSOR,
	INTEL_ACPI_CAMERA_VCM,
	INTEL_ACPI_CAMERA_UNKNOWN,
};

struct intel_acpi_camera_i2c {
	int mclk;		/* In Hz */
	int lanes;		/* Lanes physically connected */
	int xshutdown;		/* Reset GPIO or -1 if none */
	const char *hid;	/* Sensor hardware ID */
	struct intel_acpi_camera_pmic_config *pmic_config;
};

struct intel_acpi_camera_csi2 {
	int port;		/* CSI2 receiver port number */
	int lanes;		/* Lanes physically connected */
};

int intel_acpi_camera_csi2(acpi_handle ahandle,
			   struct intel_acpi_camera_csi2 *info);

int intel_acpi_camera_i2c(struct device *dev,
			  struct intel_acpi_camera_i2c *info);

u64 intel_acpi_camera_camd(acpi_handle ahandle);
#endif
