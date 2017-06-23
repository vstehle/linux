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

#ifndef __IPU3_UAPI_H
#define __IPU3_UAPI_H

#include <linux/types.h>
#include <linux/v4l2-controls.h>

#define IPU3_UAPI_ISP_VEC_ELEMS				64

/******************* V4L2 control ids *******************/

#define IPU3_CID_USER_BASE		(V4L2_CID_USER_BASE + 0xf100)
#define V4L2_CID_IPU3_AE_GRID		(IPU3_CID_USER_BASE + 0)

#define IMGU_ABI_PAD	__aligned(IPU3_UAPI_ISP_WORD_BYTES)
#define IPU3_ALIGN	__attribute__((aligned(IPU3_UAPI_ISP_WORD_BYTES)))

#define IPU3_UAPI_ISP_WORD_BYTES			32
#define IPU3_UAPI_MAX_STRIPES				2

/******************* ipu3_uapi_stats_3a *******************/

#define IPU3_UAPI_MAX_BUBBLE_SIZE			10

#define IPU3_UAPI_AE_COLORS				4
#define IPU3_UAPI_AE_BINS				256

#define IPU3_UAPI_AWB_MD_ITEM_SIZE			8
#define IPU3_UAPI_AWB_MAX_SETS				60
#define IPU3_UAPI_AWB_SET_SIZE				0x500
#define IPU3_UAPI_AWB_SPARE_FOR_BUBBLES \
		(IPU3_UAPI_MAX_BUBBLE_SIZE * IPU3_UAPI_MAX_STRIPES * \
		 IPU3_UAPI_AWB_MD_ITEM_SIZE)
#define IPU3_UAPI_AWB_MAX_BUFFER_SIZE \
		(IPU3_UAPI_AWB_MAX_SETS * \
		 (IPU3_UAPI_AWB_SET_SIZE + IPU3_UAPI_AWB_SPARE_FOR_BUBBLES))

#define IPU3_UAPI_AF_MAX_SETS				24
#define IPU3_UAPI_AF_MD_ITEM_SIZE			4
#define IPU3_UAPI_AF_SPARE_FOR_BUBBLES \
		(IPU3_UAPI_MAX_BUBBLE_SIZE * IPU3_UAPI_MAX_STRIPES * \
		 IPU3_UAPI_AF_MD_ITEM_SIZE)
#define IPU3_UAPI_AF_Y_TABLE_SET_SIZE			0x80
#define IPU3_UAPI_AF_Y_TABLE_MAX_SIZE \
	(IPU3_UAPI_AF_MAX_SETS * \
	 (IPU3_UAPI_AF_Y_TABLE_SET_SIZE + IPU3_UAPI_AF_SPARE_FOR_BUBBLES) * \
	 IPU3_UAPI_MAX_STRIPES)

#define IPU3_UAPI_AWB_FR_MAX_SETS			24
#define IPU3_UAPI_AWB_FR_MD_ITEM_SIZE			8
#define IPU3_UAPI_AWB_FR_BAYER_TBL_SIZE			0x100
#define IPU3_UAPI_AWB_FR_SPARE_FOR_BUBBLES \
		(IPU3_UAPI_MAX_BUBBLE_SIZE * IPU3_UAPI_MAX_STRIPES * \
		IPU3_UAPI_AWB_FR_MD_ITEM_SIZE)
#define IPU3_UAPI_AWB_FR_BAYER_TABLE_MAX_SIZE \
	(IPU3_UAPI_AWB_FR_MAX_SETS * \
	(IPU3_UAPI_AWB_FR_BAYER_TBL_SIZE + \
	 IPU3_UAPI_AWB_FR_SPARE_FOR_BUBBLES) * \
	IPU3_UAPI_MAX_STRIPES)

struct ipu3_uapi_grid_config {
	__u8 width;				/* 6 or 7 (rgbs_grd_cfg) bits */
	__u8 height;
	__u16 block_width_log2:3;
	__u16 block_height_log2:3;
	__u16 height_per_slice:8;			/* default value 1 */
	__u16 x_start;					/* 12 bits */
	__u16 y_start;
#define IPU3_UAPI_GRID_START_MASK			((1 << 12) - 1)
#define IPU3_UAPI_GRID_Y_START_EN			(1 << 15)
	__u16 x_end;					/* 12 bits */
	__u16 y_end;
};

struct ipu3_uapi_awb_meta_data {
	__u8 meta_data_buffer[IPU3_UAPI_AWB_MAX_BUFFER_SIZE];
};

struct ipu3_uapi_awb_raw_buffer {
	struct ipu3_uapi_awb_meta_data meta_data;
};

struct IPU3_ALIGN ipu3_uapi_awb_config_s {
	__u16 rgbs_thr_gr;
	__u16 rgbs_thr_r;
	__u16 rgbs_thr_gb;
	__u16 rgbs_thr_b;
/* controls generation of meta_data (like FF enable/disable) */
#define IPU3_UAPI_AWB_RGBS_THR_B_EN		(1 << 14)
#define IPU3_UAPI_AWB_RGBS_THR_B_INCL_SAT	(1 << 15)

	struct ipu3_uapi_grid_config grid;
};

struct ipu3_uapi_ae_raw_buffer {
	__u32 vals[IPU3_UAPI_AE_BINS * IPU3_UAPI_AE_COLORS];
};

struct ipu3_uapi_ae_raw_buffer_aligned {
	struct ipu3_uapi_ae_raw_buffer buff IPU3_ALIGN;
};

struct ipu3_uapi_ae_grid_config {
	__u8 width;
	__u8 height;
	__u8 block_width_log2:4;
	__u8 block_height_log2:4;
	__u8 __reserved0:5;
	__u8 ae_en:1;
	__u8 rst_hist_array:1;
	__u8 done_rst_hist_array:1;
	__u16 x_start;			/* 12 bits */
	__u16 y_start;
	__u16 x_end;
	__u16 y_end;
};

struct ipu3_uapi_af_filter_config {
	struct {
		__u8 a1;
		__u8 a2;
		__u8 a3;
		__u8 a4;
	} y1_coeff_0;
	struct {
		__u8 a5;
		__u8 a6;
		__u8 a7;
		__u8 a8;
	} y1_coeff_1;
	struct {
		__u8 a9;
		__u8 a10;
		__u8 a11;
		__u8 a12;
	} y1_coeff_2;

	__u32 y1_sign_vec;

	struct {
		__u8 a1;
		__u8 a2;
		__u8 a3;
		__u8 a4;
	} y2_coeff_0;
	struct {
		__u8 a5;
		__u8 a6;
		__u8 a7;
		__u8 a8;
	} y2_coeff_1;
	struct {
		__u8 a9;
		__u8 a10;
		__u8 a11;
		__u8 a12;
	} y2_coeff_2;

	__u32 y2_sign_vec;

	struct {
		__u8 y_gen_rate_gr;	/* 6 bits */
		__u8 y_gen_rate_r;
		__u8 y_gen_rate_b;
		__u8 y_gen_rate_gb;
	} y_calc;

	struct {
		__u32 __reserved0:8;
		__u32 y1_nf:4;
		__u32 __reserved1:4;
		__u32 y2_nf:4;
		__u32 __reserved2:12;
	} nf;
};

struct ipu3_uapi_af_meta_data {
	__u8 y_table[IPU3_UAPI_AF_Y_TABLE_MAX_SIZE] IPU3_ALIGN;
};

struct ipu3_uapi_af_raw_buffer {
	struct ipu3_uapi_af_meta_data meta_data IPU3_ALIGN;
};

struct ipu3_uapi_af_frame_size {
	__u16 width;
	__u16 height;
};

struct ipu3_uapi_af_config_s {
	struct ipu3_uapi_af_filter_config filter_config IPU3_ALIGN;
	struct ipu3_uapi_af_frame_size frame_size;
	struct ipu3_uapi_grid_config grid_cfg IPU3_ALIGN;
};

struct ipu3_uapi_awb_fr_meta_data {
	__u8 bayer_table[IPU3_UAPI_AWB_FR_BAYER_TABLE_MAX_SIZE] IPU3_ALIGN;
};

struct ipu3_uapi_awb_fr_raw_buffer {
	struct ipu3_uapi_awb_fr_meta_data meta_data;
};

struct IPU3_ALIGN ipu3_uapi_awb_fr_config_s {
	struct ipu3_uapi_grid_config grid_cfg;
	__u8 bayer_coeff[6];
	__u16 __reserved1;
	__u32 bayer_sign;		/* 11 bits */
	__u8 bayer_nf;			/* 4 bits */
	__u8 __reserved2[3];
};

struct ipu3_uapi_4a_config {
	struct ipu3_uapi_awb_config_s awb_config IPU3_ALIGN;
	struct ipu3_uapi_ae_grid_config ae_grd_config IPU3_ALIGN;
	struct ipu3_uapi_af_config_s af_config IPU3_ALIGN;
	struct ipu3_uapi_awb_fr_config_s awb_fr_config IPU3_ALIGN;
};

struct ipu3_uapi_bubble_info {
	__u32 num_of_stripes IPU3_ALIGN;
	__u32 num_sets IPU3_ALIGN;
	__u32 size_of_set IPU3_ALIGN;
	__u32 bubble_size IPU3_ALIGN;
};

struct ipu3_uapi_stats_3a_bubble_info_per_stripe {
	struct ipu3_uapi_bubble_info awb[IPU3_UAPI_MAX_STRIPES];
	struct ipu3_uapi_bubble_info af[IPU3_UAPI_MAX_STRIPES];
	struct ipu3_uapi_bubble_info awb_fr[IPU3_UAPI_MAX_STRIPES];
};

struct ipu3_uapi_ff_status {
	__u32 awb_en IPU3_ALIGN;
	__u32 ae_en IPU3_ALIGN;
	__u32 af_en IPU3_ALIGN;
	__u32 awb_fr_en IPU3_ALIGN;
};

struct ipu3_uapi_stats_3a {
	struct ipu3_uapi_awb_raw_buffer awb_raw_buffer IPU3_ALIGN;
	struct ipu3_uapi_ae_raw_buffer_aligned
			ae_raw_buffer[IPU3_UAPI_MAX_STRIPES] IPU3_ALIGN;
	struct ipu3_uapi_af_raw_buffer af_raw_buffer IPU3_ALIGN;
	struct ipu3_uapi_awb_fr_raw_buffer awb_fr_raw_buffer IPU3_ALIGN;
	struct ipu3_uapi_4a_config stats_4a_config IPU3_ALIGN;
	__u32 ae_join_buffers IPU3_ALIGN;
	struct ipu3_uapi_stats_3a_bubble_info_per_stripe
			stats_3a_bubble_per_stripe IPU3_ALIGN;
	struct ipu3_uapi_ff_status stats_3a_status IPU3_ALIGN;
};

/******************* ipu3_uapi_stats_dvs *******************/

#define IPU3_UAPI_DVS_STAT_LEVELS			3
#define IPU3_UAPI_DVS_STAT_L0_MV_VEC_PER_SET		12
#define IPU3_UAPI_DVS_STAT_L1_MV_VEC_PER_SET		11
#define IPU3_UAPI_DVS_STAT_L2_MV_VEC_PER_SET		9
#define IPU3_UAPI_DVS_STAT_STRIPE_ALIGN_GAP		IPU3_UAPI_MAX_STRIPES
#define IPU3_UAPI_DVS_STAT_MAX_VERTICAL_FEATURES		16

struct ipu3_uapi_dvs_stat_mv {
	__u16 vec_fe_x_pos;		/* 12 bits */
	__u16 vec_fe_y_pos;
	__u16 vec_fm_x_pos;		/* 12 bits */
	__u16 vec_fm_y_pos;
	__u32 harris_grade;		/* 28 bits */
	__u16 match_grade;		/* 15 bits */
	__u16 level;			/* 3 bits */
};

struct ipu3_uapi_dvs_stat_mv_single_set_l0 {
	struct ipu3_uapi_dvs_stat_mv
		mv_entry[IPU3_UAPI_DVS_STAT_L0_MV_VEC_PER_SET +
		IPU3_UAPI_DVS_STAT_STRIPE_ALIGN_GAP] IPU3_ALIGN;
};

struct ipu3_uapi_dvs_stat_mv_single_set_l1 {
	struct ipu3_uapi_dvs_stat_mv
		mv_entry[IPU3_UAPI_DVS_STAT_L1_MV_VEC_PER_SET +
		IPU3_UAPI_DVS_STAT_STRIPE_ALIGN_GAP] IPU3_ALIGN;
};

struct ipu3_uapi_dvs_stat_mv_single_set_l2 {
	struct ipu3_uapi_dvs_stat_mv
		mv_entry[IPU3_UAPI_DVS_STAT_L2_MV_VEC_PER_SET +
		IPU3_UAPI_DVS_STAT_STRIPE_ALIGN_GAP] IPU3_ALIGN;
};

struct ipu3_uapi_dvs_stat_motion_vec {
	struct ipu3_uapi_dvs_stat_mv_single_set_l0
		dvs_mv_output_l0[IPU3_UAPI_DVS_STAT_MAX_VERTICAL_FEATURES]
		IPU3_ALIGN;
	struct ipu3_uapi_dvs_stat_mv_single_set_l1
		dvs_mv_output_l1[IPU3_UAPI_DVS_STAT_MAX_VERTICAL_FEATURES]
		IPU3_ALIGN;
	struct ipu3_uapi_dvs_stat_mv_single_set_l2
		dvs_mv_output_l2[IPU3_UAPI_DVS_STAT_MAX_VERTICAL_FEATURES]
		IPU3_ALIGN;
};

struct ipu3_uapi_dvs_stat_stripe_data {
	__u8 grid_width[IPU3_UAPI_MAX_STRIPES][IPU3_UAPI_DVS_STAT_LEVELS];
	__u16 stripe_offset;
};

struct ipu3_uapi_dvs_stat_gbl_config {
	__u8 kappa;					/* 4 bits */
	__u8 match_shift:4;
	__u8 ybin_mode:1;
	__u16 __reserved1;
};

struct ipu3_uapi_dvs_stat_grd_config {
	__u8 grid_width;				/* 5 bits */
	__u8 grid_height;
	__u8 block_width;				/* 8 bits */
	__u8 block_height;
	__u16 x_start;					/* 12 bits */
	__u16 y_start;
	__u16 enable;
	__u16 x_end;					/* 12 bits */
	__u16 y_end;
};

struct ipu3_uapi_dvs_stat_fe_roi_cfg {
	__u8 x_start;
	__u8 y_start;
	__u8 x_end;
	__u8 y_end;
};

struct ipu3_uapi_dvs_stat_cfg {
	struct ipu3_uapi_dvs_stat_gbl_config gbl_cfg;
	struct ipu3_uapi_dvs_stat_grd_config
					grd_config[IPU3_UAPI_DVS_STAT_LEVELS];
	struct ipu3_uapi_dvs_stat_fe_roi_cfg
					fe_roi_cfg[IPU3_UAPI_DVS_STAT_LEVELS];
	__u8 __reserved[IPU3_UAPI_ISP_WORD_BYTES -
		 (sizeof(struct ipu3_uapi_dvs_stat_gbl_config) +
		  (sizeof(struct ipu3_uapi_dvs_stat_grd_config) +
		   sizeof(struct ipu3_uapi_dvs_stat_fe_roi_cfg)) *
		  IPU3_UAPI_DVS_STAT_LEVELS) % IPU3_UAPI_ISP_WORD_BYTES];
};

struct ipu3_uapi_stats_dvs {
	struct ipu3_uapi_dvs_stat_motion_vec motion_vec IPU3_ALIGN;
	struct ipu3_uapi_dvs_stat_cfg cfg IPU3_ALIGN;
	struct ipu3_uapi_dvs_stat_stripe_data stripe_data IPU3_ALIGN;
};

/******************* ipu3_uapi_stats_lace *******************/

#define IPU3_UAPI_LACE_STAT_REGS_PER_SET		320
#define IPU3_UAPI_LACE_STAT_MAX_OPERATIONS		41

struct ipu3_uapi_lace_stat_stats_regs {
	__u8 bin[4];					/* the bins 0-3 */
};

struct ipu3_uapi_lace_stat_hist_single_set {
	struct ipu3_uapi_lace_stat_stats_regs
		lace_hist_set[IPU3_UAPI_LACE_STAT_REGS_PER_SET] IPU3_ALIGN;
};

struct ipu3_uapi_lace_stat_hist_vec {
	struct ipu3_uapi_lace_stat_hist_single_set
	       lace_hist_output[IPU3_UAPI_LACE_STAT_MAX_OPERATIONS] IPU3_ALIGN;
};

struct ipu3_uapi_lace_stat_gbl_cfg {
	__u32 lh_mode:3;
	__u32 __reserved:3;
	__u32 y_ds_mode:2;
	__u32 uv_ds_mode_unsupported:1;
	__u32 uv_input_unsupported:1;
	__u32 __reserved1:10;
	__u32 rst_loc_hist:1;
	__u32 done_rst_loc_hist:1;
	__u32 __reserved2:10;
};

struct ipu3_uapi_lace_stat_y_grd_hor_cfg {
	__u32 grid_width:6;
	__u32 __reserved:10;
	__u32 block_width:4;
	__u32 __reserved1:12;
};

struct ipu3_uapi_lace_stat_y_grd_hor_roi {
	__u32 x_start:12;
	__u32 __reserved:4;
	__u32 x_end:12;
	__u32 __reserved1:4;
};

struct ipu3_uapi_lace_stat_uv_grd_hor_cfg {
	__u32 not_supported;
};

struct ipu3_uapi_lace_stat_uv_grd_hor_roi {
	__u32 not_supported;
};

struct ipu3_uapi_lace_stat_grd_vrt_cfg {
	__u32 __reserved:8;
	__u32 grid_h:6;
	__u32 __reserved1:6;
	__u32 block_h:4;
	__u32 grid_h_per_slice:7;
	__u32 __reserved2:1;
};

struct ipu3_uapi_lace_stat_grd_vrt_roi {
	__u32 y_start:12;
	__u32 __reserved:4;
	__u32 y_end:12;
	__u32 __reserved1:4;
};

struct ipu3_uapi_lace_stat_cfg {
	struct ipu3_uapi_lace_stat_gbl_cfg lace_stat_gbl_cfg;
	struct ipu3_uapi_lace_stat_y_grd_hor_cfg lace_stat_y_grd_hor_cfg;
	struct ipu3_uapi_lace_stat_y_grd_hor_roi lace_stat_y_grd_hor_roi;
	struct ipu3_uapi_lace_stat_uv_grd_hor_cfg lace_stat_uv_grd_hor_cfg;
	struct ipu3_uapi_lace_stat_uv_grd_hor_roi lace_stat_uv_grd_hor_roi;
	struct ipu3_uapi_lace_stat_grd_vrt_cfg lace_stat_grd_vrt_cfg;
	struct ipu3_uapi_lace_stat_grd_vrt_roi lace_stat_grd_vrt_roi;
};

struct ipu3_uapi_stats_lace {
	struct ipu3_uapi_lace_stat_hist_vec lace_hist_vec IPU3_ALIGN;
	struct ipu3_uapi_lace_stat_cfg lace_stat_cfg IPU3_ALIGN;
};

/******************* ipu3_uapi_acc_param *******************/

#define IPU3_UAPI_BNR_LUT_SIZE				32

/* number of elements in gamma correction LUT */
#define IPU3_UAPI_GAMMA_CORR_LUT_ENTRIES		256

#define IPU3_UAPI_SHD_MAX_OPERATIONS \
		(IPU3_UAPI_SHD_MAX_PROCESS_LINES + IPU3_UAPI_SHD_MAX_TRANSFERS)
#define IPU3_UAPI_SHD_MAX_PROCESS_LINES			31
#define IPU3_UAPI_SHD_MAX_TRANSFERS			31
#define IPU3_UAPI_SHD_MAX_CELLS_PER_SET			146
/* largest grid is 73x56 */
#define IPU3_UAPI_SHD_MAX_CFG_SETS			28

#define IPU3_UAPI_DVS_STAT_L0_MD_ENTRIES		84
#define IPU3_UAPI_DVS_STAT_PARTS_IN_MD_ENTRY		10
#define IPU3_UAPI_DVS_STAT_L1_MD_ENTRIES		66
#define IPU3_UAPI_DVS_STAT_L2_MD_ENTRIES		45
#define IPU3_UAPI_DVS_STAT_MAX_OPERATIONS		100
#define IPU3_UAPI_DVS_STAT_MAX_PROCESS_LINES		52
#define IPU3_UAPI_DVS_STAT_MAX_TRANSFERS		52

#define IPU3_UAPI_YUVP2_YTM_LUT_ENTRIES			256
#define IPU3_UAPI_YUVP2_TCC_MACC_TABLE_ELEMENTS		16
#define IPU3_UAPI_YUVP2_TCC_INV_Y_LUT_ELEMENTS		14
#define IPU3_UAPI_YUVP2_TCC_GAIN_PCWL_LUT_ELEMENTS	258
#define IPU3_UAPI_YUVP2_TCC_R_SQR_LUT_ELEMENTS		24

#define IPU3_UAPI_DPC_COMMANDS_PER_TRANSFER		2
#define IPU3_UAPI_DPC_MAX_SUPPORTED_HEIGHT		3840
#define IPU3_UAPI_DPC_STRIPE_SIZE			50
#define IPU3_UAPI_DPC_MAX_OPERATIONS \
	(IPU3_UAPI_DPC_COMMANDS_PER_TRANSFER * IPU3_UAPI_DPC_MAX_CFG_SETS)
#define IPU3_UAPI_DPC_MAX_PROCESS_LINES		IPU3_UAPI_DPC_MAX_CFG_SETS
#define IPU3_UAPI_DPC_MAX_TRANSFERS		IPU3_UAPI_DPC_MAX_CFG_SETS
#define IPU3_UAPI_DPC_MAX_DP_FIRST_LINES_PAIR		70
#define IPU3_UAPI_DPC_MAX_DP_PER_SET			192
#define IPU3_UAPI_DPC_MAX_CFG_SETS \
	((IPU3_UAPI_DPC_MAX_SUPPORTED_HEIGHT + IPU3_UAPI_DPC_STRIPE_SIZE - 1) \
	/ IPU3_UAPI_DPC_STRIPE_SIZE)

#define IPU3_UAPI_BDS_SAMPLE_PATTERN_ARRAY_SIZE		8
#define IPU3_UAPI_BDS_PHASE_COEFFS_ARRAY_SIZE		32

#define IPU3_UAPI_ANR_LUT_SIZE				26
#define IPU3_UAPI_ANR_PYRAMID_SIZE			22

#define IPU3_UAPI_AWB_FR_MAX_TRANSFERS			30
#define IPU3_UAPI_AWB_FR_MAX_PROCESS_LINES		30
#define IPU3_UAPI_AWB_FR_MAX_OPERATIONS \
	(IPU3_UAPI_AWB_FR_MAX_TRANSFERS + IPU3_UAPI_AWB_FR_MAX_PROCESS_LINES)

#define IPU3_UAPI_AE_WEIGHTS				96

#define IPU3_UAPI_AF_MAX_TRANSFERS			30
#define IPU3_UAPI_AF_MAX_PROCESS_LINES			30
#define IPU3_UAPI_AF_MAX_OPERATIONS \
		(IPU3_UAPI_AF_MAX_TRANSFERS + IPU3_UAPI_AF_MAX_PROCESS_LINES)

#define IPU3_UAPI_AWB_MAX_PROCESS_LINES			68
#define IPU3_UAPI_AWB_MAX_TRANSFERS			68
#define IPU3_UAPI_AWB_MAX_OPERATIONS \
		(IPU3_UAPI_AWB_MAX_PROCESS_LINES + IPU3_UAPI_AWB_MAX_TRANSFERS)

#define IPU3_UAPI_OSYS_PIN_VF				0
#define IPU3_UAPI_OSYS_PIN_OUT				1
#define IPU3_UAPI_OSYS_PINS				2

typedef __u32 imgu_addr_t;

struct ipu3_uapi_stripe_input_frame_resolution {
	__u16 width;
	__u16 height;
	__u32 bayer_order;		/* enum ipu3_uapi_bayer_order */
	__u32 raw_bit_depth;
};

struct ipu3_uapi_acc_operation {
	/*
	 * zero means on init,
	 * others mean upon receiving an ack signal from the BC acc.
	 */
	__u8 op_indicator;
	__u8 op_type;
};

struct ipu3_uapi_acc_process_lines_cmd_data {
	__u16 lines;
	__u8 cfg_set;
	__u8 __reserved;		/* Align to 4 bytes */
};

struct ipu3_uapi_stripes {
	/* offset from start of frame - measured in pixels */
	__u16 offset;
	/* stripe width - measured in pixels */
	__u16 width;
	/* stripe width - measured in pixels */
	__u16 height;
};

struct ipu3_uapi_stripe_data {
	/*
	 * number of stripes for current processing source
	 * - VLIW binary parameter we currently support 1 or 2 stripes
	 */
	__u16 num_of_stripes;

	/*
	 * the following data is derived from resolution-related
	 * pipe config and from num_of_stripes
	 */

	/*
	 *'input-stripes' - before input cropping
	 * used by input feeder
	 */
	struct ipu3_uapi_stripe_input_frame_resolution input_frame;

	/*'effective-stripes' - after input cropping used dpc, bds */
	struct ipu3_uapi_stripes effective_stripes[IPU3_UAPI_MAX_STRIPES];

	/* 'down-scaled-stripes' - after down-scaling ONLY. used by BDS */
	struct ipu3_uapi_stripes down_scaled_stripes[IPU3_UAPI_MAX_STRIPES];

	/*
	 *'bds-out-stripes' - after bayer down-scaling and padding.
	 * used by all algos starting with norm up to the ref-frame for GDC
	 * (currently up to the output kernel)
	 */
	struct ipu3_uapi_stripes bds_out_stripes[IPU3_UAPI_MAX_STRIPES];

	/* 'bds-out-stripes (no overlap)' - used for ref kernel */
	struct ipu3_uapi_stripes
			bds_out_stripes_no_overlap[IPU3_UAPI_MAX_STRIPES];

	/*
	 * input resolution for output system (equal to bds_out - envelope)
	 * output-system input frame width as configured by user
	 */
	__u16 output_system_in_frame_width;
	/* output-system input frame height as configured by user */
	__u16 output_system_in_frame_height;

	/*
	 * 'output-stripes' - accounts for stiching on the output (no overlap)
	 * used by the output kernel
	 */
	struct ipu3_uapi_stripes output_stripes[IPU3_UAPI_MAX_STRIPES];

	/*
	 * 'block-stripes' - accounts for stiching by the output system
	 * (1 or more blocks overlap)
	 * used by DVS, TNR and the output system kernel
	 */
	struct ipu3_uapi_stripes block_stripes[IPU3_UAPI_MAX_STRIPES];

	__u16 effective_frame_width;	/* Needed for vertical cropping */
	__u16 bds_frame_width;
	__u16 out_frame_width;	/* Output frame width as configured by user */
	__u16 out_frame_height;	/* Output frame height as configured by user */

	/* GDC in buffer (A.K.A delay frame,ref buffer) info */
	__u16 gdc_in_buffer_width;	/* GDC in buffer width  */
	__u16 gdc_in_buffer_height;	/* GDC in buffer height */
	/* GDC in buffer first valid pixel x offset */
	__u16 gdc_in_buffer_offset_x;
	/* GDC in buffer first valid pixel y offset */
	__u16 gdc_in_buffer_offset_y;

	/* Display frame width as configured by user */
	__u16 display_frame_width;
	/* Display frame height as configured by user */
	__u16 display_frame_height;
	__u16 bds_aligned_frame_width;
	/* Number of vectors to left-crop when writing stripes (not stripe 0) */
	__u16 half_overlap_vectors;
	/* Decimate ISP and fixed func resolutions after BDS (ir_extraction) */
	__u16 ir_ext_decimation;
};

struct ipu3_uapi_input_feeder_data {
	__u32 row_stride;				/* row stride */
	__u32 start_row_address;			/* start row address */
	__u32 start_pixel;				/* start pixel */
};

struct ipu3_uapi_input_feeder_data_aligned {
	struct ipu3_uapi_input_feeder_data data IPU3_ALIGN;
};

struct ipu3_uapi_input_feeder_data_per_stripe {
	struct ipu3_uapi_input_feeder_data_aligned
		input_feeder_data[IPU3_UAPI_MAX_STRIPES];
};

struct ipu3_uapi_input_feeder_config {
	struct ipu3_uapi_input_feeder_data data;
	struct ipu3_uapi_input_feeder_data_per_stripe data_per_stripe
		IPU3_ALIGN;
};

struct ipu3_uapi_bnr_static_config_wb_gains_config {
	__u16 gr;
	__u16 r;
	__u16 b;
	__u16 gb;
};

struct ipu3_uapi_bnr_static_config_wb_gains_thr_config {
	__u8 gr;
	__u8 r;
	__u8 b;
	__u8 gb;
};

struct ipu3_uapi_bnr_static_config_thr_coeffs_config {
	__u32 cf:13;
	__u32 __reserved0:3;
	__u32 cg:5;
	__u32 ci:5;
	__u32 __reserved1:1;
	__u32 r_nf:5;
};

struct ipu3_uapi_bnr_static_config_thr_ctrl_shd_config {
	__u8 gr;
	__u8 r;
	__u8 b;
	__u8 gb;
};

struct ipu3_uapi_bnr_static_config_opt_center_config {
	__s32 x_reset:13;
	__u32 __reserved0:3;
	__s32 y_reset:13;
	__u32 __reserved2:3;
};

struct ipu3_uapi_bnr_static_config_lut_config {
	__u8 values[IPU3_UAPI_BNR_LUT_SIZE];
};

struct ipu3_uapi_bnr_static_config_bp_ctrl_config {
	__u32 bp_thr_gain:5;
	__u32 __reserved0:2;
	__u32 defect_mode:1;
	__u32 bp_gain:6;
	__u32 __reserved1:18;
	__u32 w0_coeff:4;
	__u32 __reserved2:4;
	__u32 w1_coeff:4;
	__u32 __reserved3:20;
};

struct ipu3_uapi_bnr_static_config_dn_detect_ctrl_config {
	__u32 alpha:4;
	__u32 beta:4;
	__u32 gamma:4;
	__u32 __reserved0:4;
	__u32 max_inf:4;
	__u32 __reserved1:7;
	/* aka 'green disparity enable' */
	__u32 gd_enable:1;
	__u32 bpc_enable:1;
	__u32 bnr_enable:1;
	__u32 ff_enable:1;
	__u32 __reserved2:1;
};

struct ipu3_uapi_bnr_static_config_opt_center_sqr_config {
	__u32 x_sqr_reset;
	__u32 y_sqr_reset;
};

struct ipu3_uapi_bnr_static_config {
	struct ipu3_uapi_bnr_static_config_wb_gains_config wb_gains;
	struct ipu3_uapi_bnr_static_config_wb_gains_thr_config wb_gains_thr;
	struct ipu3_uapi_bnr_static_config_thr_coeffs_config thr_coeffs;
	struct ipu3_uapi_bnr_static_config_thr_ctrl_shd_config thr_ctrl_shd;
	struct ipu3_uapi_bnr_static_config_opt_center_config opt_center;
	struct ipu3_uapi_bnr_static_config_lut_config lut;
	struct ipu3_uapi_bnr_static_config_bp_ctrl_config bp_ctrl;
	struct ipu3_uapi_bnr_static_config_dn_detect_ctrl_config dn_detect_ctrl;
	__u32 column_size;				/* 0x44 */
	struct ipu3_uapi_bnr_static_config_opt_center_sqr_config opt_center_sqr;
};

struct ipu3_uapi_bnr_static_config_green_disparity {
	__u32 gd_red:6;
	__u32 __reserved0:2;
	__u32 gd_green:6;
	__u32 __reserved1:2;
	__u32 gd_blue:6;
	__u32 __reserved2:10;
	__u32 gd_black:14;
	__u32 __reserved3:2;
	__u32 gd_shading:7;
	__u32 __reserved4:1;
	__u32 gd_support:2;
	__u32 __reserved5:1;
	__u32 gd_clip:1;			/* central weights variables */
	__u32 gd_central_weight:4;
};

struct ipu3_uapi_dm_config {
	/* DWORD0 */
	__u32 dm_en:1;
	__u32 ch_ar_en:1;
	__u32 fcc_en:1;
	__u32 __reserved0:13;
	__u32 frame_width:16;

	/* DWORD1 */
	__u32 gamma_sc:5;
	__u32 __reserved1:3;
	__u32 lc_ctrl:5;
	__u32 __reserved2:3;
	__u32 cr_param1:5;
	__u32 __reserved3:3;
	__u32 cr_param2:5;
	__u32 __reserved4:3;

	/* DWORD2 */
	__u32 coring_param:5;
	__u32 __reserved5:27;
};

struct ipu3_uapi_ccm_mat_config {
	__s16 coeff_m11;
	__s16 coeff_m12;
	__s16 coeff_m13;
	__s16 coeff_o_r;
	__s16 coeff_m21;
	__s16 coeff_m22;
	__s16 coeff_m23;
	__s16 coeff_o_g;
	__s16 coeff_m31;
	__s16 coeff_m32;
	__s16 coeff_m33;
	__s16 coeff_o_b;
};

struct ipu3_uapi_gamma_corr_ctrl {
	__u32 enable:1;
	__u32 __reserved:31;
};

struct ipu3_uapi_gamma_corr_lut {
	__u16 lut[IPU3_UAPI_GAMMA_CORR_LUT_ENTRIES];
};

struct ipu3_uapi_gamma_config {
	struct ipu3_uapi_gamma_corr_ctrl gc_ctrl IPU3_ALIGN;
	struct ipu3_uapi_gamma_corr_lut gc_lut IPU3_ALIGN;
};

struct ipu3_uapi_csc_mat_config {
	__s16 coeff_c11;
	__s16 coeff_c12;
	__s16 coeff_c13;
	__s16 coeff_b1;
	__s16 coeff_c21;
	__s16 coeff_c22;
	__s16 coeff_c23;
	__s16 coeff_b2;
	__s16 coeff_c31;
	__s16 coeff_c32;
	__s16 coeff_c33;
	__s16 coeff_b3;
};

struct ipu3_uapi_cds_params {
	__u32 ds_c00:2;
	__u32 ds_c01:2;
	__u32 ds_c02:2;
	__u32 ds_c03:2;
	__u32 ds_c10:2;
	__u32 ds_c11:2;
	__u32 ds_c12:2;
	__u32 ds_c13:2;
	__u32 ds_nf:5;
	__u32 __reserved0:3;
	__u32 csc_en:1;
	__u32 uv_bin_output:1;
	__u32 __reserved1:6;
};

struct ipu3_uapi_shd_grid_config {
	/* reg 0 */
	__u8 width;
	__u8 height;
	__u8 block_width_log2:3;
	__u8 __reserved0:1;
	__u8 block_height_log2:3;
	__u8 __reserved1:1;
	__u8 grid_height_per_slice;
	/* reg 1 */
	__s16 x_start;			/* 13 bits */
	__s16 y_start;
};

struct ipu3_uapi_shd_general_config {
	__u32 init_set_vrt_offst_ul:8;
	__u32 shd_enable:1;
	/* aka 'gf' */
	__u32 gain_factor:2;
	__u32 __reserved:21;
};

struct ipu3_uapi_shd_black_level_config {
	__s16 bl_r;			/* 12 bits */
	__s16 bl_gr;
#define IPU3_UAPI_SHD_BLGR_NF_SHIFT	13	/* Normalization shift aka nf */
#define IPU3_UAPI_SHD_BLGR_NF_MASK	0x7
	__s16 bl_gb;			/* 12 bits */
	__s16 bl_b;
};

struct ipu3_uapi_shd_config_static {
	/* B0: Fixed order: one transfer to GAC */
	struct ipu3_uapi_shd_grid_config grid;
	struct ipu3_uapi_shd_general_config general;
	struct ipu3_uapi_shd_black_level_config black_level;
};

struct ipu3_uapi_shd_transfer_luts_set_data {
	__u8 set_number;
	imgu_addr_t rg_lut_ddr_addr;
	imgu_addr_t bg_lut_ddr_addr;
	__u32 align_dummy;
};

struct ipu3_uapi_shd_intra_frame_operations_data {
	struct ipu3_uapi_acc_operation
		operation_list[IPU3_UAPI_SHD_MAX_OPERATIONS] IPU3_ALIGN;
	struct ipu3_uapi_acc_process_lines_cmd_data
		process_lines_data[IPU3_UAPI_SHD_MAX_PROCESS_LINES] IPU3_ALIGN;
	struct ipu3_uapi_shd_transfer_luts_set_data
		transfer_data[IPU3_UAPI_SHD_MAX_TRANSFERS] IPU3_ALIGN;
};

struct ipu3_uapi_shd_lut_set {
};

struct ipu3_uapi_shd_lut {
	struct {
		struct {
			__u16 r;
			__u16 gr;
		} r_and_gr[IPU3_UAPI_SHD_MAX_CELLS_PER_SET];
		__u8 __reserved1[24];
		struct {
			__u16 gb;
			__u16 b;
		} gb_and_b[IPU3_UAPI_SHD_MAX_CELLS_PER_SET];
		__u8 __reserved2[24];
	} sets[IPU3_UAPI_SHD_MAX_CFG_SETS];
};

struct ipu3_uapi_shd_config {
	struct ipu3_uapi_shd_config_static shd IPU3_ALIGN;
	struct ipu3_uapi_shd_intra_frame_operations_data shd_ops IPU3_ALIGN;
	struct ipu3_uapi_shd_lut shd_lut IPU3_ALIGN;
};

struct ipu3_uapi_dvs_stat_stripe_cfg {
	struct ipu3_uapi_dvs_stat_cfg stripe_cfg[IPU3_UAPI_MAX_STRIPES];
};

struct ipu3_uapi_dvs_stat_transfer_op_data {
	__u8 set_number;
};

struct ipu3_uapi_dvs_stat_intra_frame_operations_data {
	struct ipu3_uapi_acc_operation
		ops[IPU3_UAPI_DVS_STAT_MAX_OPERATIONS] IPU3_ALIGN;
	struct ipu3_uapi_acc_process_lines_cmd_data
		process_lines_data[IPU3_UAPI_DVS_STAT_MAX_PROCESS_LINES]
		IPU3_ALIGN;
	struct ipu3_uapi_dvs_stat_transfer_op_data
		transfer_data[IPU3_UAPI_DVS_STAT_MAX_TRANSFERS] IPU3_ALIGN;
};

struct ipu3_uapi_dvs_stat_meta_data_align_p {
	imgu_addr_t p_meta_data IPU3_ALIGN;
};

struct ipu3_uapi_dvs_stat_config {
	struct ipu3_uapi_dvs_stat_cfg cfg IPU3_ALIGN;
	struct ipu3_uapi_dvs_stat_stripe_cfg stripe IPU3_ALIGN;
	struct ipu3_uapi_dvs_stat_intra_frame_operations_data
		operations_data IPU3_ALIGN;
	struct ipu3_uapi_dvs_stat_meta_data_align_p
		meta_data[IPU3_UAPI_MAX_STRIPES];
};

struct ipu3_uapi_lace_stat_operation {
	__u8 op_indicator;
	__u16 lines;
};

struct ipu3_uapi_lace_stat_intra_frame_op_data {
	struct ipu3_uapi_lace_stat_operation
		ops[IPU3_UAPI_LACE_STAT_MAX_OPERATIONS] IPU3_ALIGN;
};

struct ipu3_uapi_lace_stat_config {
	struct ipu3_uapi_lace_stat_cfg lace_stat_cfg IPU3_ALIGN;
	struct ipu3_uapi_lace_stat_intra_frame_op_data operations_data
		IPU3_ALIGN;
};

struct ipu3_uapi_iefd_cux2 {
	__u32 x0:9;
	__u32 x1:9;
	__u32 a01:9;
	__u32 b01:5;				/* NOTE: hardcoded to zero */
};

struct ipu3_uapi_iefd_cux6_ed {
	__u32 x0:9;
	__u32 x1:9;
	__u32 x2:9;
	__u32 __reserved0:5;

	__u32 x3:9;
	__u32 x4:9;
	__u32 x5:9;
	__u32 __reserved1:5;

	__u32 a01:9;
	__u32 a12:9;
	__u32 a23:9;
	__u32 __reserved2:5;

	__u32 a34:9;
	__u32 a45:9;
	__u32 __reserved3:14;

	__u32 b01:9;
	__u32 b12:9;
	__u32 b23:9;
	__u32 __reserved4:5;

	__u32 b34:9;
	__u32 b45:9;
	__u32 __reserved5:14;
};

struct ipu3_uapi_iefd_cux2_1 {
	__u32 x0:9;
	__u32 x1:9;
	__u32 a01:9;
	__u32 __reserved1:5;

	__u32 b01:8;
	__u32 __reserved2:24;
};

struct ipu3_uapi_iefd_cux4 {
	__u32 x0:9;
	__u32 x1:9;
	__u32 x2:9;
	__u32 __reserved0:5;

	__u32 x3:9;
	__u32 a01:9;
	__u32 a12:9;
	__u32 __reserved1:5;

	__u32 a23:9;
	__u32 b01:8;
	__u32 b12:8;
	__u32 __reserved2:7;

	__u32 b23:8;
	__u32 __reserved3:24;
};

struct ipu3_uapi_iefd_cux6_rad {
	__u32 x0:8;
	__u32 x1:8;
	__u32 x2:8;
	__u32 x3:8;

	__u32 x4:8;
	__u32 x5:8;
	__u32 __reserved1:16;

	__u32 a01:16;
	__u32 a12:16;

	__u32 a23:16;
	__u32 a34:16;

	__u32 a45:16;
	__u32 __reserved2:16;

	__u32 b01:10;
	__u32 b12:10;
	__u32 b23:10;
	__u32 __reserved4:2;

	__u32 b34:10;
	__u32 b45:10;
	__u32 __reserved5:12;
};

struct ipu3_uapi_yuvp1_iefd_cfg_units {
	struct ipu3_uapi_iefd_cux2 cu_1;
	struct ipu3_uapi_iefd_cux6_ed cu_ed;
	struct ipu3_uapi_iefd_cux2 cu_3;
	struct ipu3_uapi_iefd_cux2_1 cu_5;
	struct ipu3_uapi_iefd_cux4 cu_6;
	struct ipu3_uapi_iefd_cux2 cu_7;
	struct ipu3_uapi_iefd_cux4 cu_unsharp;
	struct ipu3_uapi_iefd_cux6_rad cu_radial;
	struct ipu3_uapi_iefd_cux2 cu_vssnlm;
};

struct ipu3_uapi_yuvp1_iefd_config_s {
	__u32 horver_diag_coeff:7;	/* Gradiant compensation */
	__u32 __reserved0:1;
	__u32 clamp_stitch:6;		/* Slope to stitch edge */
	__u32 __reserved1:2;
	__u32 direct_metric_update:5;	/* Update coeff for direction metric */
	__u32 __reserved2:3;
	__u32 ed_horver_diag_coeff:7;
	__u32 __reserved3:1;
};

struct ipu3_uapi_yuvp1_iefd_control {
	__u32 iefd_en:1;		/* Enable IEFD */
	__u32 denoise_en:1;		/* Enable denoise */
	__u32 direct_smooth_en:1;	/* Enable directional smooth */
	__u32 rad_en:1;			/* Enable radial update */
	__u32 vssnlm_en:1;		/* Enable VSSNLM output filter */
	__u32 __reserved:27;
};

struct ipu3_uapi_sharp_cfg {
	__u32 nega_lmt_txt:13;
	__u32 __reserved0:19;
	__u32 posi_lmt_txt:13;
	__u32 __reserved1:19;
	__u32 nega_lmt_dir:13;
	__u32 __reserved2:19;
	__u32 posi_lmt_dir:13;
	__u32 __reserved3:19;
};

struct ipu3_uapi_far_w {
	__u32 dir_shrp:7;
	__u32 __reserved0:1;
	__u32 dir_dns:7;
	__u32 __reserved1:1;
	__u32 ndir_dns_powr:7;
	__u32 __reserved2:9;
};

struct ipu3_uapi_unsharp_cfg {
	__u32 unsharp_weight:7;
	__u32 __reserved0:1;
	__u32 unsharp_amount:9;
	__u32 __reserved1:15;
};

struct ipu3_uapi_yuvp1_iefd_shrp_cfg {
	struct ipu3_uapi_sharp_cfg cfg;
	struct ipu3_uapi_far_w far_w;
	struct ipu3_uapi_unsharp_cfg unshrp_cfg;
};

struct ipu3_uapi_unsharp_coef0 {
	__u32 c00:9;			/* Coeff11 */
	__u32 c01:9;			/* Coeff12 */
	__u32 c02:9;			/* Coeff13 */
	__u32 __reserved:5;
};

struct ipu3_uapi_unsharp_coef1 {
	__u32 c11:9;			/* Coeff22 */
	__u32 c12:9;			/* Coeff23 */
	__u32 c22:9;			/* Coeff33 */
	__u32 __reserved:5;
};

struct ipu3_uapi_yuvp1_iefd_unshrp_cfg {
	struct ipu3_uapi_unsharp_coef0 unsharp_coef0;
	struct ipu3_uapi_unsharp_coef1 unsharp_coef1;
};

struct ipu3_uapi_radial_reset_xy {
	__s32 x:13;
	__u32 __reserved0:3;
	__s32 y:13;
	__u32 __reserved1:3;
};

struct ipu3_uapi_radial_reset_x2 {
	__u32 x2:24;
	__u32 __reserved:8;
};

struct ipu3_uapi_radial_reset_y2 {
	__u32 y2:24;
	__u32 __reserved:8;
};

struct ipu3_uapi_radial_cfg {
	__u32 rad_nf:4;
	__u32 __reserved0:4;
	__u32 rad_inv_r2:7;
	__u32 __reserved1:17;
};

struct ipu3_uapi_rad_far_w {
	__u32 rad_dir_far_sharp_w:8;
	__u32 rad_dir_far_dns_w:8;
	__u32 rad_ndir_far_dns_power:8;
	__u32 __reserved:8;
};

struct ipu3_uapi_cu_cfg0 {
	__u32 cu6_pow:7;
	__u32 __reserved0:1;
	__u32 cu_unsharp_pow:7;
	__u32 __reserved1:1;
	__u32 rad_cu6_pow:7;
	__u32 __reserved2:1;
	__u32 rad_cu_unsharp_pow:6;
	__u32 __reserved3:2;
};

struct ipu3_uapi_cu_cfg1 {
	__u32 rad_cu6_x1:9;
	__u32 __reserved0:1;
	__u32 rad_cu_unsharp_x1:9;
	__u32 __reserved1:13;
};

struct ipu3_uapi_yuvp1_iefd_rad_cfg {
	struct ipu3_uapi_radial_reset_xy reset_xy;
	struct ipu3_uapi_radial_reset_x2 reset_x2;
	struct ipu3_uapi_radial_reset_y2 reset_y2;
	struct ipu3_uapi_radial_cfg cfg;
	struct ipu3_uapi_rad_far_w rad_far_w;
	struct ipu3_uapi_cu_cfg0 cu_cfg0;
	struct ipu3_uapi_cu_cfg1 cu_cfg1;
};

struct ipu3_uapi_vss_lut_x {
	__u32 vs_x0:8;
	__u32 vs_x1:8;
	__u32 vs_x2:8;
	__u32 __reserved2:8;
};

struct ipu3_uapi_vss_lut_y {
	__u32 vs_y1:4;
	__u32 __reserved0:4;
	__u32 vs_y2:4;
	__u32 __reserved1:4;
	__u32 vs_y3:4;
	__u32 __reserved2:12;
};

struct ipu3_uapi_yuvp1_iefd_vssnlm_cfg {
	struct ipu3_uapi_vss_lut_x vss_lut_x;
	struct ipu3_uapi_vss_lut_y vss_lut_y;
};

struct ipu3_uapi_yuvp1_iefd_config {
	struct ipu3_uapi_yuvp1_iefd_cfg_units units;
	struct ipu3_uapi_yuvp1_iefd_config_s config;
	struct ipu3_uapi_yuvp1_iefd_control control;
	struct ipu3_uapi_yuvp1_iefd_shrp_cfg sharp;
	struct ipu3_uapi_yuvp1_iefd_unshrp_cfg unsharp;
	struct ipu3_uapi_yuvp1_iefd_rad_cfg rad;
	struct ipu3_uapi_yuvp1_iefd_vssnlm_cfg vsslnm;
};

struct ipu3_uapi_yuvp1_yds_config {
	__u32 c00:2;
	__u32 c01:2;
	__u32 c02:2;
	__u32 c03:2;
	__u32 c10:2;
	__u32 c11:2;
	__u32 c12:2;
	__u32 c13:2;
	__u32 norm_factor:5;
	__u32 __reserved0:4;
	__u32 bin_output:1;
	__u32 __reserved1:6;
};

struct ipu3_uapi_yuvp1_chnr_enable_config {
	__u32 enable:1;
	__u32 yuv_mode:1;
	__u32 __reserved0:14;
	__u32 col_size:12;
	__u32 __reserved1:4;
};

struct ipu3_uapi_yuvp1_chnr_coring_config {
	__u32 u:13;
	__u32 __reserved0:3;
	__u32 v:13;
	__u32 __reserved1:3;
};

struct ipu3_uapi_yuvp1_chnr_sense_gain_config {
	__u32 vy:8;
	__u32 vu:8;
	__u32 vv:8;
	__u32 __reserved0:8;

	__u32 hy:8;
	__u32 hu:8;
	__u32 hv:8;
	__u32 __reserved1:8;
};

struct ipu3_uapi_yuvp1_chnr_iir_fir_config {
	__u32 fir_0h:6;
	__u32 __reserved0:2;
	__u32 fir_1h:6;
	__u32 __reserved1:2;
	__u32 fir_2h:6;
	__u32 dalpha_clip_val:9;
	__u32 __reserved2:1;
};

struct ipu3_uapi_yuvp1_chnr_config {
	struct ipu3_uapi_yuvp1_chnr_enable_config enable;
	struct ipu3_uapi_yuvp1_chnr_coring_config coring;
	struct ipu3_uapi_yuvp1_chnr_sense_gain_config sense_gain;
	struct ipu3_uapi_yuvp1_chnr_iir_fir_config iir_fir;
};

struct ipu3_uapi_yuvp1_y_ee_nr_lpf_config {
	__u32 a_diag:5;
	__u32 __reserved0:3;
	__u32 a_periph:5;
	__u32 __reserved1:3;
	__u32 a_cent:5;
	__u32 __reserved2:9;
	__u32 enable:1;
};

struct ipu3_uapi_yuvp1_y_ee_nr_sense_config {
	__u32 edge_sense_0:13;
	__u32 __reserved0:3;
	__u32 delta_edge_sense:13;
	__u32 __reserved1:3;
	__u32 corner_sense_0:13;
	__u32 __reserved2:3;
	__u32 delta_corner_sense:13;
	__u32 __reserved3:3;
};

struct ipu3_uapi_yuvp1_y_ee_nr_gain_config {
	__u32 gain_pos_0:5;
	__u32 __reserved0:3;
	__u32 delta_gain_posi:5;
	__u32 __reserved1:3;
	__u32 gain_neg_0:5;
	__u32 __reserved2:3;
	__u32 delta_gain_neg:5;
	__u32 __reserved3:3;
};

struct ipu3_uapi_yuvp1_y_ee_nr_clip_config {
	__u32 clip_pos_0:5;
	__u32 __reserved0:3;
	__u32 delta_clip_posi:5;
	__u32 __reserved1:3;
	__u32 clip_neg_0:5;
	__u32 __reserved2:3;
	__u32 delta_clip_neg:5;
	__u32 __reserved3:3;
};

struct ipu3_uapi_yuvp1_y_ee_nr_frng_config {
	__u32 gain_exp:4;
	__u32 __reserved0:28;
	__u32 min_edge:13;
	__u32 __reserved1:3;
	__u32 lin_seg_param:4;
	__u32 __reserved2:4;
	__u32 t1:1;
	__u32 t2:1;
	__u32 __reserved3:6;
};

struct ipu3_uapi_yuvp1_y_ee_nr_diag_config {
	__u32 diag_disc_g:4;
	__u32 __reserved0:4;
	__u32 hvw_hor:4;
	__u32 dw_hor:4;
	__u32 hvw_diag:4;
	__u32 dw_diag:4;
	__u32 __reserved1:8;
};

struct ipu3_uapi_yuvp1_y_ee_nr_fc_coring_config {
	__u32 pos_0:13;
	__u32 __reserved0:3;
	__u32 pos_delta:13;
	__u32 __reserved1:3;
	__u32 neg_0:13;
	__u32 __reserved2:3;
	__u32 neg_delta:13;
	__u32 __reserved3:3;
};

struct ipu3_uapi_yuvp1_y_ee_nr_config {
	struct ipu3_uapi_yuvp1_y_ee_nr_lpf_config lpf;
	struct ipu3_uapi_yuvp1_y_ee_nr_sense_config sense;
	struct ipu3_uapi_yuvp1_y_ee_nr_gain_config gain;
	struct ipu3_uapi_yuvp1_y_ee_nr_clip_config clip;
	struct ipu3_uapi_yuvp1_y_ee_nr_frng_config frng;
	struct ipu3_uapi_yuvp1_y_ee_nr_diag_config diag;
	struct ipu3_uapi_yuvp1_y_ee_nr_fc_coring_config fc_coring;
};

struct ipu3_uapi_yuvp2_y_tm_lut_static_config {
	__u16 entries[IPU3_UAPI_YUVP2_YTM_LUT_ENTRIES]; /* 13 significand bits*/
	__u32 enable;
};

struct ipu3_uapi_yuvp2_tcc_gen_control_static_config {
	__u32 en:1;
	__u32 blend_shift:3;
	__u32 gain_according_to_y_only:1;
	__u32 __reserved0:11;
	__s32 gamma:5;
	__u32 __reserved1:3;
	__s32 delta:5;
	__u32 __reserved2:3;
};

struct ipu3_uapi_yuvp2_tcc_macc_elem_static_config {
	__s32 a:12;
	__u32 __reserved0:4;
	__s32 b:12;
	__u32 __reserved1:4;
	__s32 c:12;
	__u32 __reserved2:4;
	__s32 d:12;
	__u32 __reserved3:4;
};

struct ipu3_uapi_yuvp2_tcc_macc_table_static_config {
	struct ipu3_uapi_yuvp2_tcc_macc_elem_static_config
		entries[IPU3_UAPI_YUVP2_TCC_MACC_TABLE_ELEMENTS];
};

struct ipu3_uapi_yuvp2_tcc_inv_y_lut_static_config {
	__u16 entries[IPU3_UAPI_YUVP2_TCC_INV_Y_LUT_ELEMENTS];	/* 10 bits */
};

struct ipu3_uapi_yuvp2_tcc_gain_pcwl_lut_static_config {
	__u16 entries[IPU3_UAPI_YUVP2_TCC_GAIN_PCWL_LUT_ELEMENTS];/* 12 bits */
};

struct ipu3_uapi_yuvp2_tcc_r_sqr_lut_static_config {
	__s16 entries[IPU3_UAPI_YUVP2_TCC_R_SQR_LUT_ELEMENTS];	/* 11 bits */
};

struct ipu3_uapi_yuvp2_tcc_static_config {
	struct ipu3_uapi_yuvp2_tcc_gen_control_static_config gen_control;
	struct ipu3_uapi_yuvp2_tcc_macc_table_static_config macc_table;
	struct ipu3_uapi_yuvp2_tcc_inv_y_lut_static_config inv_y_lut;
	struct ipu3_uapi_yuvp2_tcc_gain_pcwl_lut_static_config gain_pcwl;
	struct ipu3_uapi_yuvp2_tcc_r_sqr_lut_static_config r_sqr_lut;
};

struct ipu3_uapi_dpc_num_of_dp {
	__u8 dp_gr;
	__u8 dp_bg;
	__u16 __reserved;
};

struct ipu3_uapi_dpc_params {
	__u16 enable;
	__u16 grad_threshold;		/* 13 bits */
	struct ipu3_uapi_dpc_num_of_dp set[2];
	struct ipu3_uapi_dpc_num_of_dp first_line_pair;
};

struct ipu3_uapi_dpc_transfer_luts_set_data {
	__u8 set_number;
	__u8 num_of_dp_gr;
	__u8 num_of_dp_bg;
	__u8 align_dummy;

};

struct ipu3_uapi_dpc_intra_frame_operations_data {
	struct ipu3_uapi_acc_operation
		operation_list[IPU3_UAPI_DPC_MAX_OPERATIONS] IPU3_ALIGN;
	struct ipu3_uapi_acc_process_lines_cmd_data
		process_lines_data[IPU3_UAPI_DPC_MAX_PROCESS_LINES] IPU3_ALIGN;
	struct ipu3_uapi_dpc_transfer_luts_set_data
		transfer_data[IPU3_UAPI_DPC_MAX_TRANSFERS] IPU3_ALIGN;
};

struct ipu3_uapi_dpc_1st_pair_of_lines_lut_elem {
	__u32 column:13;
	__u32 nghbr_sts:5;
	__u32 p0:14;
	__u32 p1:14;
	__u32 __reserved0:2;
	__u32 p2:14;
	__u32 nghbr_order:1;
	__u32 __reserved1:1;
};

struct ipu3_uapi_dpc_1st_pair_of_lines_lut {
	struct ipu3_uapi_dpc_1st_pair_of_lines_lut_elem
		entries[IPU3_UAPI_DPC_MAX_DP_FIRST_LINES_PAIR];

};

struct ipu3_uapi_dpc_lut_elem {
	__u32 nghbr_sts:5;
	__u32 skip:1;
	__u32 nghbr_order:1;
	__u32 column:13;
	__u32 row_pair_delta:4;
	__u32 __reserved0:8;
};

struct ipu3_uapi_dpc_lut_set {
	struct ipu3_uapi_dpc_lut_elem
		elems[IPU3_UAPI_DPC_MAX_DP_PER_SET] IPU3_ALIGN;
};

struct ipu3_uapi_dpc_lut {
	struct ipu3_uapi_dpc_lut_set sets[IPU3_UAPI_DPC_MAX_CFG_SETS];
};

struct ipu3_uapi_dpc_stripe_config {
	struct ipu3_uapi_dpc_params params IPU3_ALIGN;
	struct ipu3_uapi_dpc_intra_frame_operations_data ops IPU3_ALIGN;
	struct ipu3_uapi_dpc_1st_pair_of_lines_lut first_lines_lut_gr
		IPU3_ALIGN;
	struct ipu3_uapi_dpc_1st_pair_of_lines_lut first_lines_lut_bg
		IPU3_ALIGN;
	struct ipu3_uapi_dpc_lut lut_bg IPU3_ALIGN;
	struct ipu3_uapi_dpc_lut lut_gr IPU3_ALIGN;
};

struct ipu3_uapi_dpc_config_per_stripe {
	struct ipu3_uapi_dpc_stripe_config
		dpc_config[IPU3_UAPI_MAX_STRIPES] IPU3_ALIGN;
};

struct ipu3_uapi_dpc_config {
	struct ipu3_uapi_dpc_config_per_stripe config_per_stripe
		IPU3_ALIGN;
};

struct ipu3_uapi_bds_hor_ctrl0 {
	__u32 sample_patrn_length:9;
	__u32 __reserved0:3;
	__u32 hor_ds_en:1;
	__u32 min_clip_val:1;
	__u32 max_clip_val:2;
	__u32 out_frame_width:13;
	__u32 __reserved1:3;
};

struct ipu3_uapi_bds_ptrn_arr {
	__u32 elems[IPU3_UAPI_BDS_SAMPLE_PATTERN_ARRAY_SIZE];
};

struct ipu3_uapi_bds_phase_entry {
	__s8 coeff_min2;
	__s8 coeff_min1;
	__s8 coeff_0;
	__s8 nf;
	__s8 coeff_pls1;
	__s8 coeff_pls2;
	__s8 coeff_pls3;
	__u8 __reserved;
};

struct ipu3_uapi_bds_phase_arr {
	struct ipu3_uapi_bds_phase_entry
		even[IPU3_UAPI_BDS_PHASE_COEFFS_ARRAY_SIZE];
	struct ipu3_uapi_bds_phase_entry
		odd[IPU3_UAPI_BDS_PHASE_COEFFS_ARRAY_SIZE];
};

struct ipu3_uapi_bds_hor_ctrl1 {
	__u32 hor_crop_start:13;
	__u32 __reserved0:3;
	__u32 hor_crop_end:13;
	__u32 __reserved1:1;
	__u32 hor_crop_en:1;
	__u32 __reserved2:1;
};

struct ipu3_uapi_bds_hor_ctrl2 {
	__u32 input_frame_height:13;
	__u32 __reserved0:19;
};

struct ipu3_uapi_bds_hor {
	struct ipu3_uapi_bds_hor_ctrl0 hor_ctrl0;
	struct ipu3_uapi_bds_ptrn_arr hor_ptrn_arr;
	struct ipu3_uapi_bds_phase_arr hor_phase_arr;
	struct ipu3_uapi_bds_hor_ctrl1 hor_ctrl1;
	struct ipu3_uapi_bds_hor_ctrl2 hor_ctrl2;
};

struct ipu3_uapi_bds_ver_ctrl0 {
	__u32 sample_patrn_length:9;
	__u32 __reserved0:3;
	__u32 ver_ds_en:1;
	__u32 min_clip_val:1;
	__u32 max_clip_val:2;
	__u32 __reserved1:16;
};

struct ipu3_uapi_bds_ver_ctrl1 {
	__u32 out_frame_width:13;
	__u32 __reserved0:3;
	__u32 out_frame_height:13;
	__u32 __reserved1:3;
};

struct ipu3_uapi_bds_ver {
	struct ipu3_uapi_bds_ver_ctrl0 ver_ctrl0;
	struct ipu3_uapi_bds_ptrn_arr ver_ptrn_arr;
	struct ipu3_uapi_bds_phase_arr ver_phase_arr;
	struct ipu3_uapi_bds_ver_ctrl1 ver_ctrl1;

};

struct ipu3_uapi_bds_per_stripe_data {
	struct ipu3_uapi_bds_hor_ctrl0 hor_ctrl0;
	struct ipu3_uapi_bds_ver_ctrl1 ver_ctrl1;
	struct ipu3_uapi_bds_hor_ctrl1 crop;
};

struct ipu3_uapi_ipu3_uapi_bds_per_stripe_data_aligned {
	struct ipu3_uapi_bds_per_stripe_data data IPU3_ALIGN;
};

struct ipu3_uapi_bds_per_stripe {
	struct ipu3_uapi_ipu3_uapi_bds_per_stripe_data_aligned
		aligned_data[IPU3_UAPI_MAX_STRIPES];
};

struct ipu3_uapi_bds_config {
	struct ipu3_uapi_bds_hor hor IPU3_ALIGN;
	struct ipu3_uapi_bds_ver ver IPU3_ALIGN;
	struct ipu3_uapi_bds_per_stripe per_stripe IPU3_ALIGN;
	__u32 enabled;
};

struct ipu3_uapi_anr_search_config {
	__u32 enable;
	__u16 frame_width;
	__u16 frame_height;
};

struct ipu3_uapi_anr_alpha {
	__u16 gr;					/* 9 bits */
	__u16 r;
	__u16 b;
	__u16 gb;
	__u16 dc_gr;
	__u16 dc_r;
	__u16 dc_b;
	__u16 dc_gb;
};

struct ipu3_uapi_anr_beta {
	__u16 beta_gr;					/* 11 bits */
	__u16 beta_r;
	__u16 beta_b;
	__u16 beta_gb;
};

struct ipu3_uapi_anr_plain_color {
	__u16 reg_w_gr[16];				/* 12 bits */
	__u16 reg_w_r[16];
	__u16 reg_w_b[16];
	__u16 reg_w_gb[16];
};

struct ipu3_uapi_anr_transform_config {
	__u32 enable:1;			/* 0 or 1, disabled or enabled */
	__u32 adaptive_treshhold_en:1;	/* On IPU3, always enabled */

	__u32 __reserved1:30;
	__u8 __reserved2[40+4];

	struct ipu3_uapi_anr_alpha alpha[3];
	struct ipu3_uapi_anr_beta beta[3];
	struct ipu3_uapi_anr_plain_color color[3];

	__u16 sqrt_lut[IPU3_UAPI_ANR_LUT_SIZE];	/* 11 bits per element */

	__s16 xreset:13;
#define IPU3_UAPI_ANR_MAX_XRESET		((1 << 12) - 1)
	__u16 __reserved3:3;
	__s16 yreset:13;
	__u16 __reserved4:3;

	__u32 x_sqr_reset:24;
	__u32 r_normfactor:5;
	__u32 __reserved5:3;

	__u32 y_sqr_reset:24;
	__u32 gain_scale:8;
};

struct ipu3_uapi_anr_stitch_pyramid {
	__u32 entry0:6;
	__u32 entry1:6;
	__u32 entry2:6;
	__u32 __reserved:14;
};

struct ipu3_uapi_anr_stitch_config {
	__u32 anr_stitch_en;
	__u16 frame_width;
	__u16 frame_height;
	__u8 __reserved[40];
	struct ipu3_uapi_anr_stitch_pyramid pyramid[IPU3_UAPI_ANR_PYRAMID_SIZE];
};

struct ipu3_uapi_anr_tile2strm_config {
	__u32 enable;
	__u16 frame_width;
	__u16 frame_height;
};

struct ipu3_uapi_anr_config {
	struct ipu3_uapi_anr_search_config search IPU3_ALIGN;
	struct ipu3_uapi_anr_transform_config transform IPU3_ALIGN;
	struct ipu3_uapi_anr_stitch_config stitch IPU3_ALIGN;
	struct ipu3_uapi_anr_tile2strm_config tile2strm IPU3_ALIGN;
};

struct ipu3_uapi_awb_fr_intra_frame_operations_data {
	struct ipu3_uapi_acc_operation ops[IPU3_UAPI_AWB_FR_MAX_OPERATIONS]
								IPU3_ALIGN;
	struct ipu3_uapi_acc_process_lines_cmd_data
	      process_lines_data[IPU3_UAPI_AWB_FR_MAX_PROCESS_LINES] IPU3_ALIGN;
};

struct ipu3_uapi_awb_fr_config {
	struct ipu3_uapi_awb_fr_config_s config;
	struct ipu3_uapi_awb_fr_intra_frame_operations_data operations_data;
	struct ipu3_uapi_awb_fr_config_s stripes[IPU3_UAPI_MAX_STRIPES];
};

struct ipu3_uapi_ae_weight_elem {
	__u32 cell0:4;
	__u32 cell1:4;
	__u32 cell2:4;
	__u32 cell3:4;
	__u32 cell4:4;
	__u32 cell5:4;
	__u32 cell6:4;
	__u32 cell7:4;
};

struct ipu3_uapi_ae_ccm {
	__u16 gain_gr;			/* 11 bits */
	__u16 gain_r;
	__u16 gain_b;
	__u16 gain_gb;
	__s16 mat[16];
};

struct ipu3_uapi_ae_config {
	struct ipu3_uapi_ae_grid_config grid_cfg IPU3_ALIGN;
	struct ipu3_uapi_ae_weight_elem weights[IPU3_UAPI_AE_WEIGHTS]
								IPU3_ALIGN;
	struct ipu3_uapi_ae_ccm ae_ccm IPU3_ALIGN;
	struct {
		struct ipu3_uapi_ae_grid_config grid IPU3_ALIGN;
	} stripes[IPU3_UAPI_MAX_STRIPES];
};

struct ipu3_uapi_af_intra_frame_operations_data {
	struct ipu3_uapi_acc_operation ops[IPU3_UAPI_AF_MAX_OPERATIONS]
		IPU3_ALIGN;
	struct ipu3_uapi_acc_process_lines_cmd_data
		process_lines_data[IPU3_UAPI_AF_MAX_PROCESS_LINES] IPU3_ALIGN;
};

struct ipu3_uapi_af_stripe_config {
	struct ipu3_uapi_af_frame_size frame_size IPU3_ALIGN;
	struct ipu3_uapi_grid_config grid_cfg IPU3_ALIGN;
};

struct ipu3_uapi_af_config {
	struct ipu3_uapi_af_config_s config;
	struct ipu3_uapi_af_intra_frame_operations_data operations_data;
	struct ipu3_uapi_af_stripe_config stripes[IPU3_UAPI_MAX_STRIPES];
};

struct ipu3_uapi_acc_transfer_op_data {
	__u8 set_number;
};

struct IPU3_ALIGN ipu3_uapi_awb_intra_frame_operations_data {
	struct ipu3_uapi_acc_operation ops[IPU3_UAPI_AWB_MAX_OPERATIONS]
		IPU3_ALIGN;
	struct ipu3_uapi_acc_process_lines_cmd_data
		process_lines_data[IPU3_UAPI_AWB_MAX_PROCESS_LINES] IPU3_ALIGN;
	struct ipu3_uapi_acc_transfer_op_data
		transfer_data[IPU3_UAPI_AWB_MAX_TRANSFERS] IPU3_ALIGN;
};

struct ipu3_uapi_awb_config {
	struct ipu3_uapi_awb_config_s config IPU3_ALIGN;
	struct ipu3_uapi_awb_intra_frame_operations_data operations_data;
	struct ipu3_uapi_awb_config_s stripes[IPU3_UAPI_MAX_STRIPES];
};

struct ipu3_uapi_osys_formatter_params {
	__u32 format;
	__u32 flip;
	__u32 mirror;
	__u32 tiling;
	__u32 reduce_range;
	__u32 alpha_blending;	/* FIXME: To figure out the unknown register */
	__u32 release_inp_addr;
	__u32 release_inp_en;
	__u32 process_out_buf_addr;
	__u32 image_width_vecs;
	__u32 image_height_lines;
	__u32 inp_buff_y_st_addr;
	__u32 inp_buff_y_line_stride;
	__u32 inp_buff_y_buffer_stride;
	__u32 int_buff_u_st_addr;
	__u32 int_buff_v_st_addr;
	__u32 inp_buff_uv_line_stride;
	__u32 inp_buff_uv_buffer_stride;
	__u32 out_buff_level;
	__u32 out_buff_nr_y_lines;
	__u32 out_buff_u_st_offset;
	__u32 out_buff_v_st_offset;
	__u32 out_buff_y_line_stride;
	__u32 out_buff_uv_line_stride;
	__u32 hist_buff_st_addr;
	__u32 hist_buff_line_stride;
	__u32 hist_buff_nr_lines;
};

struct ipu3_uapi_osys_formatter {
	struct ipu3_uapi_osys_formatter_params param IPU3_ALIGN;
};

struct ipu3_uapi_osys_scaler_params {
	__u32 inp_buf_y_st_addr;
	__u32 inp_buf_y_line_stride;
	__u32 inp_buf_y_buffer_stride;
	__u32 inp_buf_u_st_addr;
	__u32 inp_buf_v_st_addr;
	__u32 inp_buf_uv_line_stride;
	__u32 inp_buf_uv_buffer_stride;
	__u32 inp_buf_chunk_width;
	__u32 inp_buf_nr_buffers;
	/* Output buffers */
	__u32 out_buf_y_st_addr;
	__u32 out_buf_y_line_stride;
	__u32 out_buf_y_buffer_stride;
	__u32 out_buf_u_st_addr;
	__u32 out_buf_v_st_addr;
	__u32 out_buf_uv_line_stride;
	__u32 out_buf_uv_buffer_stride;
	__u32 out_buf_nr_buffers;
	/* Intermediate buffers */
	__u32 int_buf_y_st_addr;
	__u32 int_buf_y_line_stride;
	__u32 int_buf_u_st_addr;
	__u32 int_buf_v_st_addr;
	__u32 int_buf_uv_line_stride;
	__u32 int_buf_height;
	__u32 int_buf_chunk_width;
	__u32 int_buf_chunk_height;
	/* Context buffers */
	__u32 ctx_buf_hor_y_st_addr;
	__u32 ctx_buf_hor_u_st_addr;
	__u32 ctx_buf_hor_v_st_addr;
	__u32 ctx_buf_ver_y_st_addr;
	__u32 ctx_buf_ver_u_st_addr;
	__u32 ctx_buf_ver_v_st_addr;
	/* Addresses for release-input and process-output tokens */
	__u32 release_inp_buf_addr;
	__u32 release_inp_buf_en;
	__u32 release_out_buf_en;
	__u32 process_out_buf_addr;
	/* Settings dimensions, padding, cropping */
	__u32 input_image_y_width;
	__u32 input_image_y_height;
	__u32 input_image_y_start_column;
	__u32 input_image_uv_start_column;
	__u32 input_image_y_left_pad;
	__u32 input_image_uv_left_pad;
	__u32 input_image_y_right_pad;
	__u32 input_image_uv_right_pad;
	__u32 input_image_y_top_pad;
	__u32 input_image_uv_top_pad;
	__u32 input_image_y_bottom_pad;
	__u32 input_image_uv_bottom_pad;
	__u32 processing_mode;
#define IPU3_UAPI_OSYS_PROCMODE_BYPASS		0
#define IPU3_UAPI_OSYS_PROCMODE_UPSCALE		1
#define IPU3_UAPI_OSYS_PROCMODE_DOWNSCALE	2
	__u32 scaling_ratio;
	__u32 y_left_phase_init;
	__u32 uv_left_phase_init;
	__u32 y_top_phase_init;
	__u32 uv_top_phase_init;
	__u32 coeffs_exp_shift;
	__u32 out_y_left_crop;
	__u32 out_uv_left_crop;
	__u32 out_y_top_crop;
	__u32 out_uv_top_crop;
};

struct ipu3_uapi_osys_scaler {
	struct ipu3_uapi_osys_scaler_params param IPU3_ALIGN;
};

struct ipu3_uapi_osys_frame_params {
	/* Output pins */
	__u32 enable;
	__u32 format;			/* enum ipu3_uapi_osys_format */
	__u32 flip;
	__u32 mirror;
	__u32 tiling;			/* enum ipu3_uapi_osys_tiling */
	__u32 width;
	__u32 height;
	__u32 stride;
	__u32 scaled;
};

struct ipu3_uapi_osys_frame {
	struct ipu3_uapi_osys_frame_params param IPU3_ALIGN;
};

struct ipu3_uapi_osys_stripe {
	/* Input resolution */
	__u32 input_width;
	__u32 input_height;
	/* Output Stripe */
	__u32 output_width[IPU3_UAPI_OSYS_PINS];
	__u32 output_height[IPU3_UAPI_OSYS_PINS];
	__u32 output_offset[IPU3_UAPI_OSYS_PINS];
	__u32 buf_stride[IPU3_UAPI_OSYS_PINS];
	/* Scaler params */
	__u32 block_width;
	__u32 block_height;
	/* Output Crop factor */
	__u32 crop_top[IPU3_UAPI_OSYS_PINS];
	__u32 crop_left[IPU3_UAPI_OSYS_PINS];
};

struct ipu3_uapi_osys_config {
	struct ipu3_uapi_osys_formatter
		formatter[IPU3_UAPI_MAX_STRIPES][IPU3_UAPI_OSYS_PINS];
	struct ipu3_uapi_osys_scaler scaler[IPU3_UAPI_MAX_STRIPES];
	struct ipu3_uapi_osys_frame frame[IPU3_UAPI_OSYS_PINS];
	struct ipu3_uapi_osys_stripe stripe[IPU3_UAPI_MAX_STRIPES];
	/* 32 packed coefficients for luma and chroma */
	__s8 scaler_coeffs_chroma[128];
	__s8 scaler_coeffs_luma[128];
};

struct ipu3_uapi_acc_param {
	struct ipu3_uapi_stripe_data stripe;
	struct ipu3_uapi_input_feeder_config input_feeder;
	struct ipu3_uapi_bnr_static_config bnr;
	struct ipu3_uapi_bnr_static_config_green_disparity green_disparity
		IPU3_ALIGN;
	struct ipu3_uapi_dm_config dm IPU3_ALIGN;
	struct ipu3_uapi_ccm_mat_config ccm IPU3_ALIGN;
	struct ipu3_uapi_gamma_config gamma IPU3_ALIGN;
	struct ipu3_uapi_csc_mat_config csc IPU3_ALIGN;
	struct ipu3_uapi_cds_params cds IPU3_ALIGN;
	struct ipu3_uapi_shd_config shd IPU3_ALIGN;
	struct ipu3_uapi_dvs_stat_config dvs_stat IPU3_ALIGN;
	struct ipu3_uapi_lace_stat_config lace_stat IPU3_ALIGN;
	struct ipu3_uapi_yuvp1_iefd_config iefd IPU3_ALIGN;
	struct ipu3_uapi_yuvp1_yds_config yds_c0 IPU3_ALIGN;
	struct ipu3_uapi_yuvp1_chnr_config chnr_c0 IPU3_ALIGN;
	struct ipu3_uapi_yuvp1_y_ee_nr_config y_ee_nr IPU3_ALIGN;
	struct ipu3_uapi_yuvp1_yds_config yds IPU3_ALIGN;
	struct ipu3_uapi_yuvp1_chnr_config chnr IPU3_ALIGN;
	struct ipu3_uapi_yuvp2_y_tm_lut_static_config ytm IPU3_ALIGN;
	struct ipu3_uapi_yuvp1_yds_config yds2 IPU3_ALIGN;
	struct ipu3_uapi_yuvp2_tcc_static_config tcc IPU3_ALIGN;
	struct ipu3_uapi_dpc_config dpc IPU3_ALIGN;
	struct ipu3_uapi_bds_config bds IPU3_ALIGN;
	struct ipu3_uapi_anr_config anr IPU3_ALIGN;
	struct ipu3_uapi_awb_fr_config awb_fr IPU3_ALIGN;
	struct ipu3_uapi_ae_config ae IPU3_ALIGN;
	struct ipu3_uapi_af_config af IPU3_ALIGN;
	struct ipu3_uapi_awb_config awb IPU3_ALIGN;
	struct ipu3_uapi_osys_config osys IPU3_ALIGN;
};

/* Linearization parameters */

#define IPU3_UAPI_LIN_LUT_SIZE			64

struct ipu3_uapi_isp_lin_vmem_params {
	__s16 lin_lutlow_gr[IPU3_UAPI_LIN_LUT_SIZE];
	__s16 lin_lutlow_r[IPU3_UAPI_LIN_LUT_SIZE];
	__s16 lin_lutlow_b[IPU3_UAPI_LIN_LUT_SIZE];
	__s16 lin_lutlow_gb[IPU3_UAPI_LIN_LUT_SIZE];
	__s16 lin_lutdif_gr[IPU3_UAPI_LIN_LUT_SIZE];
	__s16 lin_lutdif_r[IPU3_UAPI_LIN_LUT_SIZE];
	__s16 lin_lutdif_b[IPU3_UAPI_LIN_LUT_SIZE];
	__s16 lin_lutdif_gb[IPU3_UAPI_LIN_LUT_SIZE];
};

/* TNR3 VMEM parameters */

#define IPU3_UAPI_ISP_TNR3_VMEM_LEN	9

struct ipu3_uapi_isp_tnr3_vmem_params {
	__u16 slope[IPU3_UAPI_ISP_TNR3_VMEM_LEN];
	__u16 __reserved1[IPU3_UAPI_ISP_VEC_ELEMS
						- IPU3_UAPI_ISP_TNR3_VMEM_LEN];
	__u16 sigma[IPU3_UAPI_ISP_TNR3_VMEM_LEN];
	__u16 __reserved2[IPU3_UAPI_ISP_VEC_ELEMS
						- IPU3_UAPI_ISP_TNR3_VMEM_LEN];
};

/* XNR3 VMEM parameters */

struct ipu3_uapi_isp_xnr3_vmem_params {
	__u16 x[IPU3_UAPI_ISP_VEC_ELEMS];
	__u16 a[IPU3_UAPI_ISP_VEC_ELEMS];
	__u16 b[IPU3_UAPI_ISP_VEC_ELEMS];
	__u16 c[IPU3_UAPI_ISP_VEC_ELEMS];
};

/* TNR3 DMEM parameters */

struct ipu3_uapi_isp_tnr3_params {
	__u32 knee_y1;
	__u32 knee_y2;
	__u32 maxfb_y;
	__u32 maxfb_u;
	__u32 maxfb_v;
	__u32 round_adj_y;
	__u32 round_adj_u;
	__u32 round_adj_v;
	__u32 ref_buf_select;
};

/* XNR3 DMEM parameters */

struct ipu3_uapi_xnr3_alpha_params {
	__u32 y0;
	__u32 u0;
	__u32 v0;
	__u32 ydiff;
	__u32 udiff;
	__u32 vdiff;
};

struct ipu3_uapi_xnr3_coring_params {
	__u32 u0;
	__u32 v0;
	__u32 udiff;
	__u32 vdiff;
};

struct ipu3_uapi_xnr3_blending_params {
	__u32 strength;
};

struct ipu3_uapi_isp_xnr3_params {
	struct ipu3_uapi_xnr3_alpha_params alpha;
	struct ipu3_uapi_xnr3_coring_params coring;
	struct ipu3_uapi_xnr3_blending_params blending;
};

/* RGBIR DMEM parameters */

#define IPU3_UAPI_RGBIR_LUT_WIDTH	17
#define IPU3_UAPI_RGBIR_LUT_HEIGHT	10
#define IPU3_UAPI_RGBIR_LUT_SIZE	(IPU3_UAPI_RGBIR_LUT_WIDTH * \
					 IPU3_UAPI_RGBIR_LUT_HEIGHT)

struct ipu3_uapi_isp_rgbir_params {
	__u16 ob;					/* optical black level*/
	__u16 ir_height;				/* lut table height */
	__u16 ir_width;					/* lut table width */
	__u16 ir_weights_r[IPU3_UAPI_RGBIR_LUT_SIZE];	/* lut values for red */
	__u16 ir_weights_g[IPU3_UAPI_RGBIR_LUT_SIZE];	/* lut for green */
	__u16 ir_weights_b[IPU3_UAPI_RGBIR_LUT_SIZE];	/* lut for blue */
	__u16 ir_gain;					/* digital gain */
};

/***** Morphing table entry *****/

#define IPU3_UAPI_GDC_FRAC_BITS		8

struct ipu3_uapi_gdc_warp_param {
	__u32 origin_x;
	__u32 origin_y;
	__u32 in_addr_offset;
	__u32 in_block_width;
	__u32 in_block_height;
	__u32 p0_x;
	__u32 p0_y;
	__u32 p1_x;
	__u32 p1_y;
	__u32 p2_x;
	__u32 p2_y;
	__u32 p3_x;
	__u32 p3_y;
	__u32 in_block_width_a;
	__u32 in_block_width_b;
	__u32 padding;			/* struct size multiple of DDR word */
};

/***** Obgrid (optical black level compensation) table entry *****/

struct ipu3_uapi_obgrid_param {
	__u16 gr;
	__u16 r;
	__u16 b;
	__u16 gb;
};

/******************* V4L2_PIX_FMT_IPU3_PARAMS *******************/

/*
 * The video queue "parameters" is of format V4L2_PIX_FMT_IPU3_PARAMS.
 * It is a multiplanar output queue with three planes and type
 * V4L2_BUF_TYPE_VIDEO_OUTPUT_MPLANE. User may also configure the
 * video queue as V4L2_BUF_TYPE_VIDEO_OUTPUT with a single plane, in which
 * case GDC and Obgrid tables can not be set.
 *
 * Plane 0: Defined below in struct ipu3_params, size 288064 bytes.
 *          This contains a lot of parameters and flags selecting which
 *          parameters to apply. Its size and resolution (1x1) are fixed.
 *
 * Plane 1: Contains geometric distortion correction grid coordinates.
 *          Each entry in the grid is defined in
 *          struct ipu3_uapi_gdc_warp_param.
 *          The plane size is the grid entry size times the number of entries,
 *          which depends on the main output image resolution and block size.
 *
 * Plane 2: Contains Obgrid grid. Each entry in the grid is 8 bytes.
 *          The plane size depends on user parameters (internally, on chosen
 *          firmware binary which depends on user parameters).
 */

struct ipu3_uapi_flags {
	/* Flags which of the settings below are to be applied */
	__u32 gdc:1;		/* Whether to apply GDC and */
	__u32 obgrid:1;		/* Obgrid planes */
	__u32 __reserved1:30;

	__u32 __acc_stripe:1;	/* Whether to apply these fields from */
	__u32 __acc_input_feeder:1;	/* acc_param. Fields beginning with */
	__u32 acc_bnr:1;		/* two underscores are reserved and */
	__u32 acc_green_disparity:1;/* must not be enabled */
	__u32 acc_dm:1;
	__u32 acc_ccm:1;
	__u32 acc_gamma:1;
	__u32 acc_csc:1;
	__u32 acc_cds:1;
	__u32 acc_shd:1;
	__u32 acc_dvs_stat:1;
	__u32 acc_lace_stat:1;
	__u32 acc_iefd:1;
	__u32 acc_yds_c0:1;
	__u32 acc_chnr_c0:1;
	__u32 acc_y_ee_nr:1;
	__u32 acc_yds:1;
	__u32 acc_chnr:1;
	__u32 acc_ytm:1;
	__u32 acc_yds2:1;
	__u32 acc_tcc:1;
	__u32 acc_dpc:1;
	__u32 acc_bds:1;
	__u32 acc_anr:1;
	__u32 acc_awb_fr:1;
	__u32 acc_ae:1;
	__u32 acc_af:1;
	__u32 acc_awb:1;
	__u32 __acc_osys:1;
	__u32 __reserved2:3;

	__u32 lin_vmem_params:1;	/* Whether to apply these structs */
	__u32 tnr3_vmem_params:1;
	__u32 xnr3_vmem_params:1;
	__u32 tnr3_dmem_params:1;
	__u32 xnr3_dmem_params:1;
	__u32 __rgbir_dmem_params:1;
	__u32 obgrid_param:1;
	__u32 __reserved3:25;
};

struct ipu3_uapi_params {
	__u32 fourcc;			/* V4L2_PIX_FMT_IPU3_PARAMS */
	__u32 version;			/* Must be 0x100 */

	struct ipu3_uapi_flags use;

	__u8 __reserved4[32 - 4 * 5];	/* Must be zero */

	/* Acceleration cluster parameters */
	struct ipu3_uapi_acc_param acc_param;

	/* VMEM parameters */
	struct ipu3_uapi_isp_lin_vmem_params lin_vmem_params;
	struct ipu3_uapi_isp_tnr3_vmem_params tnr3_vmem_params;
	struct ipu3_uapi_isp_xnr3_vmem_params xnr3_vmem_params;

	/* DMEM parameters */
	struct ipu3_uapi_isp_tnr3_params tnr3_dmem_params;
	struct ipu3_uapi_isp_xnr3_params xnr3_dmem_params;
	struct ipu3_uapi_isp_rgbir_params rgbir_dmem_params;

	struct ipu3_uapi_obgrid_param obgrid_param;
};

#endif
