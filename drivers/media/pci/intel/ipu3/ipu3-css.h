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

#include "ipu3-abi.h"
#include "ipu3-css-pool.h"

/* 2 stages for split isp pipeline, 1 for scaling */
#define IMGU_NUM_SP			2
#define IMGU_MAX_PIPELINE_NUM		20

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
};

#endif
