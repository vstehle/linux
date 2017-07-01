#
#  Copyright (c) 2017, Intel Corporation.
#
#  This program is free software; you can redistribute it and/or modify it
#  under the terms and conditions of the GNU General Public License,
#  version 2, as published by the Free Software Foundation.
#
#  This program is distributed in the hope it will be useful, but WITHOUT
#  ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
#  FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
#  more details.
#

obj-$(CONFIG_VIDEO_IPU3_ACPI) += intel-acpi-camera.o
obj-$(CONFIG_VIDEO_IPU3_CIO2) += ipu3-cio2.o
obj-$(CONFIG_INTEL_IPU3_MMU) += ipu3-mmu.o
obj-$(CONFIG_INTEL_IPU3_DMAMAP) += ipu3-dmamap.o
ipu3-imgu-objs += \
		ipu3-tables.o ipu3-css-pool.o \
		ipu3-css-fw.o ipu3-css-params.o \
		ipu3-css.o ipu3-v4l2.o  ipu3.o

obj-$(CONFIG_VIDEO_IPU3_IMGU) += ipu3-imgu.o
