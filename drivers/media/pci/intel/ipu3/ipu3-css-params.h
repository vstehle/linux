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

struct ipu3_css_frame_params {
	/* Output pins */
	unsigned int enable;
	unsigned int format;
	unsigned int flip;
	unsigned int mirror;
	unsigned int tiling;
	unsigned int reduce_range;
	unsigned int width;
	unsigned int height;
	unsigned int stride;
	unsigned int scaled;
	unsigned int crop_left;
	unsigned int crop_top;
};

struct ipu3_css_stripe_params {
	unsigned int processing_mode;
	unsigned int phase_step;
	unsigned int exp_shift;
	unsigned int phase_init_left_y;
	unsigned int phase_init_left_uv;
	unsigned int phase_init_top_y;
	unsigned int phase_init_top_uv;
	unsigned int pad_left_y;
	unsigned int pad_left_uv;
	unsigned int pad_right_y;
	unsigned int pad_right_uv;
	unsigned int pad_top_y;
	unsigned int pad_top_uv;
	unsigned int pad_bottom_y;
	unsigned int pad_bottom_uv;
	unsigned int crop_left_y;
	unsigned int crop_top_y;
	unsigned int crop_left_uv;
	unsigned int crop_top_uv;
	unsigned int start_column_y;
	unsigned int start_column_uv;
	unsigned int chunk_width;
	unsigned int chunk_height;
	unsigned int block_width;
	unsigned int block_height;
	unsigned int input_width;
	unsigned int input_height;
	int output_width[IMGU_ABI_OSYS_PINS];
	int output_height[IMGU_ABI_OSYS_PINS];
	int output_offset[IMGU_ABI_OSYS_PINS];
};

struct ipu3_css_reso {
	unsigned int input_width;
	unsigned int input_height;
	enum imgu_abi_frame_format input_format;
	unsigned int pin_width[IMGU_ABI_OSYS_PINS];
	unsigned int pin_height[IMGU_ABI_OSYS_PINS];
	unsigned int pin_stride[IMGU_ABI_OSYS_PINS];
	enum imgu_abi_frame_format pin_format[IMGU_ABI_OSYS_PINS];
	int chunk_width;
	int chunk_height;
	int block_height;
	int block_width;
};

struct ipu3_css_scaler_info {
	unsigned int phase_step;	/* Same for luma/chroma */
	int exp_shift;

	unsigned int phase_init;	/* luma/chroma dependent */
	int pad_left;
	int pad_right;
	int crop_left;
	int crop_top;
};

int ipu3_css_cfg_acc(struct ipu3_css *css, struct ipu3_uapi_flags *use,
			struct ipu3_uapi_acc_param *acc,
			struct ipu3_uapi_acc_param *acc_old,
			struct ipu3_uapi_acc_param *acc_user);

int ipu3_css_cfg_vmem0(struct ipu3_css *css, struct ipu3_uapi_flags *use,
			void *vmem0, void *vmem0_old,
			struct ipu3_uapi_params *user);


int ipu3_css_cfg_dmem0(struct ipu3_css *css, struct ipu3_uapi_flags *use,
			void *dmem0, void *dmem0_old,
			struct ipu3_uapi_params *user);

void ipu3_css_cfg_gdc_table(struct ipu3_uapi_gdc_warp_param *gdc,
				int frame_in_x, int frame_in_y,
				int frame_out_x, int frame_out_y);
