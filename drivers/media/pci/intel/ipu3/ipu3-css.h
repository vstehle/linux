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

#ifndef __IPU3_CSS_H
#define __IPU3_CSS_H

#include <linux/videodev2.h>
#include <linux/types.h>
#include "ipu3-abi.h"
#include "ipu3-css-pool.h"

/* 2 stages for split isp pipeline, 1 for scaling */
#define IMGU_NUM_SP			2
#define IMGU_MAX_PIPELINE_NUM		20

/* For DVS etc., format FRAME_FMT_YUV420_16 */
#define IPU3_CSS_AUX_FRAME_REF		0
/* For temporal noise reduction DVS etc., format FRAME_FMT_YUV_LINE */
#define IPU3_CSS_AUX_FRAME_TNR		1
#define IPU3_CSS_AUX_FRAME_TYPES	2	/* REF and TNR */
#define IPU3_CSS_AUX_FRAMES		2	/* 2 for REF and 2 for TNR */

#define IPU3_CSS_QUEUE_IN		0
#define IPU3_CSS_QUEUE_PARAMS		1
#define IPU3_CSS_QUEUE_OUT		2
#define IPU3_CSS_QUEUE_VF		3
#define IPU3_CSS_QUEUE_STAT_3A		4
#define IPU3_CSS_QUEUE_STAT_DVS		5
#define IPU3_CSS_QUEUE_STAT_LACE	6
#define IPU3_CSS_QUEUES			7

#define IPU3_CSS_RECT_EFFECTIVE		0       /* Effective resolution */
#define IPU3_CSS_RECT_BDS		1       /* Resolution after BDS */
#define IPU3_CSS_RECT_ENVELOPE		2       /* DVS envelope size */
#define IPU3_CSS_RECTS			3

#define IA_CSS_BINARY_MODE_PRIMARY	2
#define IA_CSS_BINARY_MODE_VIDEO	3

/*
 * The pipe id type, distinguishes the kind of pipes that
 * can be run in parallel.
 */
enum ipu3_css_pipe_id {
	IPU3_CSS_PIPE_ID_PREVIEW,
	IPU3_CSS_PIPE_ID_COPY,
	IPU3_CSS_PIPE_ID_VIDEO,
	IPU3_CSS_PIPE_ID_CAPTURE,
	IPU3_CSS_PIPE_ID_YUVPP,
	IPU3_CSS_PIPE_ID_ACC,
	IPU3_CSS_PIPE_ID_NUM
};

struct ipu3_css_format {
	u32 pixelformat;
	enum v4l2_colorspace colorspace;
	enum imgu_abi_frame_format frame_format;
	enum imgu_abi_bayer_order bayer_order;
	enum imgu_abi_osys_format osys_format;
	enum imgu_abi_osys_tiling osys_tiling;
	u32 bytesperpixel_num;	/* Bytes per pixel in first plane * 50 */
	u8 bit_depth;		/* Effective bits per pixel */
	u8 chroma_decim;	/* Chroma plane decimation, 0=no chroma plane */
	u8 width_align;		/* Alignment requirement for width_pad */
	u8 flags;
#define IPU3_CSS_QUEUE_TO_FLAGS(q)	(1 << (q))
#define IPU3_CSS_FORMAT_FL_IN		\
			IPU3_CSS_QUEUE_TO_FLAGS(IPU3_CSS_QUEUE_IN)
#define IPU3_CSS_FORMAT_FL_PARAMS		\
			IPU3_CSS_QUEUE_TO_FLAGS(IPU3_CSS_QUEUE_PARAMS)
#define IPU3_CSS_FORMAT_FL_OUT		\
			IPU3_CSS_QUEUE_TO_FLAGS(IPU3_CSS_QUEUE_OUT)
#define IPU3_CSS_FORMAT_FL_VF		\
			IPU3_CSS_QUEUE_TO_FLAGS(IPU3_CSS_QUEUE_VF)
#define IPU3_CSS_FORMAT_FL_STAT_3A	\
			IPU3_CSS_QUEUE_TO_FLAGS(IPU3_CSS_QUEUE_STAT_3A)
#define IPU3_CSS_FORMAT_FL_STAT_DVS	\
			IPU3_CSS_QUEUE_TO_FLAGS(IPU3_CSS_QUEUE_STAT_DVS)
#define IPU3_CSS_FORMAT_FL_STAT_LACE	\
			IPU3_CSS_QUEUE_TO_FLAGS(IPU3_CSS_QUEUE_STAT_LACE)
#define IPU3_CSS_FORMAT_FL_PSEUDO	(IPU3_CSS_FORMAT_FL_PARAMS | \
					 IPU3_CSS_FORMAT_FL_STAT_3A | \
					 IPU3_CSS_FORMAT_FL_STAT_DVS | \
					 IPU3_CSS_FORMAT_FL_STAT_LACE)
};

struct ipu3_css_queue {
	struct v4l2_pix_format pix_fmt;
	const struct ipu3_css_format *css_fmt;
	unsigned int width_pad;	/* bytesperline / byp */
	struct list_head bufs;
};

/* IPU3 Camera Sub System structure */
struct ipu3_css {
	struct device *dev;
	void __iomem *base;
	struct device *dma_dev;
	const struct firmware *fw;
	struct imgu_fw_header *fwp;
	int iomem_length;
	int fw_bl, fw_sp[IMGU_NUM_SP];	/* Indices of bl and SP binaries */
	struct ipu3_css_map *binary;	/* fw binaries mapped to device */
	int current_binary;	/* Currently selected binary or -1 */
	bool streaming;		/* true when streaming is enabled */
	long frame;	/* Latest frame not yet processed */
	enum ipu3_css_pipe_id pipe_id;  /* CSS pipe ID. */

	/* Data structures shared with IMGU and driver, always allocated */
	struct ipu3_css_map xmem_sp_stage_ptrs[IPU3_CSS_PIPE_ID_NUM]
					    [IMGU_ABI_MAX_STAGES];
	struct ipu3_css_map xmem_isp_stage_ptrs[IPU3_CSS_PIPE_ID_NUM]
					    [IMGU_ABI_MAX_STAGES];
	struct ipu3_css_map sp_ddr_ptrs;
	struct ipu3_css_map xmem_sp_group_ptrs;
	struct ipu3_css_map dvs_meta_data[IMGU_MAX_PIPELINE_NUM]
					[IPU3_UAPI_MAX_STRIPES];

	/* Data structures shared with IMGU and driver, binary specific */
	/* PARAM_CLASS_CONFIG and PARAM_CLASS_STATE parameters */
	struct ipu3_css_map binary_params_cs[IMGU_ABI_PARAM_CLASS_NUM - 1]
					    [IMGU_ABI_NUM_MEMORIES];

	struct {
		struct ipu3_css_map mem[IPU3_CSS_AUX_FRAMES];
		unsigned int width;
		unsigned int height;
		unsigned int bytesperline;
		unsigned int bytesperpixel;
	} aux_frames[IPU3_CSS_AUX_FRAME_TYPES];

	struct ipu3_css_queue queue[IPU3_CSS_QUEUES];
	struct v4l2_rect rect[IPU3_CSS_RECTS];
};

#endif
