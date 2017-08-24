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

#ifndef __IPU3_ABI_H
#define __IPU3_ABI_H

#include <uapi/linux/intel-ipu3.h>

/******************* IMGU Hardware information *******************/

#define IMGU_ISP_VMEM_ALIGN			128
#define IMGU_DVS_BLOCK_W			64
#define IMGU_DVS_BLOCK_H			32
#define IMGU_GDC_BUF_X				(2 * IMGU_DVS_BLOCK_W)
#define IMGU_GDC_BUF_Y				IMGU_DVS_BLOCK_H
/* n = 0..1 */
#define IMGU_SP_PMEM_BASE(n)			(0x20000 + (n) * 0x4000)
#define IMGU_MAX_BQ_GRID_WIDTH			80
#define IMGU_MAX_BQ_GRID_HEIGHT			60
#define IMGU_OBGRID_TILE_SIZE			16
#define IMGU_PIXELS_PER_WORD			50
#define IMGU_BYTES_PER_WORD			64
#define IMGU_STRIPE_FIXED_HALF_OVERLAP		2
#define IMGU_SHD_SETS				3
#define IMGU_BDS_MIN_CLIP_VAL			0
#define IMGU_BDS_MAX_CLIP_VAL			2
#define IPU3_ABI_AWB_MAX_CELLS_PER_SET		160
#define IPU3_ABI_AF_MAX_CELLS_PER_SET		32
#define IPU3_ABI_AWB_FR_MAX_CELLS_PER_SET	32

#define IMGU_ABI_ACC_OP_IDLE			0
#define IMGU_ABI_ACC_OP_END_OF_ACK		1
#define IMGU_ABI_ACC_OP_END_OF_OPS		2
#define IMGU_ABI_ACC_OP_NO_OPS			3

#define IMGU_ABI_ACC_OPTYPE_PROCESS_LINES	0
#define IMGU_ABI_ACC_OPTYPE_TRANSFER_DATA	1

#define IMGU_MMU_PADDR_SHIFT			12

/* Register definitions */

/* PM_CTRL_0_5_0_IMGHMMADR */
#define IMGU_REG_PM_CTRL			0x0
#define IMGU_PM_CTRL_START			BIT(0)
#define IMGU_PM_CTRL_CFG_DONE			BIT(1)
#define IMGU_PM_CTRL_RACE_TO_HALT		BIT(2)
#define IMGU_PM_CTRL_NACK_ALL			BIT(3)
#define IMGU_PM_CTRL_CSS_PWRDN			BIT(4)
#define IMGU_PM_CTRL_RST_AT_EOF			BIT(5)
#define IMGU_PM_CTRL_FORCE_HALT			BIT(6)
#define IMGU_PM_CTRL_FORCE_UNHALT		BIT(7)
#define IMGU_PM_CTRL_FORCE_PWRDN		BIT(8)
#define IMGU_PM_CTRL_FORCE_RESET		BIT(9)
#define IMGU_PM_CTRL_RETURN_LICENSE_AT_EOF	BIT(10)
#define IMGU_PM_CTRL_POWER_DOWN_AT_EOF		(IMGU_PM_CTRL_CSS_PWRDN | \
					IMGU_PM_CTRL_RACE_TO_HALT | \
					IMGU_PM_CTRL_RETURN_LICENSE_AT_EOF)
#define IMGU_PM_CTRL_RESET_AT_EOF		(IMGU_PM_CTRL_RST_AT_EOF | \
					IMGU_PM_CTRL_RACE_TO_HALT | \
					IMGU_PM_CTRL_RETURN_LICENSE_AT_EOF)
/* SYSTEM_REQ_0_5_0_IMGHMMADR */
#define IMGU_REG_SYSTEM_REQ			0x18
#define IMGU_SYSTEM_REQ_FREQ_MASK		0x3f
#define IMGU_SYSTEM_REQ_FREQ_DIVIDER		25
#define IMGU_REG_INT_STATUS			0x30
#define IMGU_REG_INT_ENABLE			0x34
#define IMGU_REG_INT_CSS_IRQ			(1 << 31)
/* STATE_0_5_0_IMGHMMADR */
#define IMGU_REG_STATE				0x130
#define IMGU_STATE_HALT_STS			BIT(0)
#define IMGU_STATE_IDLE_STS			BIT(1)
#define IMGU_STATE_POWER_UP			BIT(2)
#define IMGU_STATE_POWER_DOWN			BIT(3)
#define IMGU_STATE_CSS_BUSY_MASK		0xc0
#define IMGU_STATE_PM_FSM_MASK			0x180
#define IMGU_STATE_PWRDNM_FSM_MASK		0x1E00000
/* PM_STS_0_5_0_IMGHMMADR */
#define IMGU_REG_PM_STS				0x140

#define IMGU_REG_BASE				0x4000

#define IMGU_REG_ISP_CTRL			(IMGU_REG_BASE + 0x00)
#define IMGU_CTRL_RST				BIT(0)
#define IMGU_CTRL_START				BIT(1)
#define IMGU_CTRL_BREAK				BIT(2)
#define IMGU_CTRL_RUN				BIT(3)
#define IMGU_CTRL_BROKEN			BIT(4)
#define IMGU_CTRL_IDLE				BIT(5)
#define IMGU_CTRL_SLEEPING			BIT(6)
#define IMGU_CTRL_STALLING			BIT(7)
#define IMGU_CTRL_IRQ_CLEAR			BIT(8)
#define IMGU_CTRL_IRQ_READY			BIT(10)
#define IMGU_CTRL_IRQ_SLEEPING			BIT(11)
#define IMGU_CTRL_ICACHE_INV			BIT(12)
#define IMGU_CTRL_IPREFETCH_EN			BIT(13)
#define IMGU_REG_ISP_START_ADDR			(IMGU_REG_BASE + 0x04)
#define IMGU_REG_ISP_ICACHE_ADDR		(IMGU_REG_BASE + 0x10)
#define IMGU_REG_ISP_PC				(IMGU_REG_BASE + 0x1c)

/* SP Registers, sp = 0:SP0; 1:SP1 */
#define IMGU_REG_SP_CTRL(sp)		(IMGU_REG_BASE + (sp) * 0x100 + 0x100)
	/* For bits in IMGU_REG_SP_CTRL, see IMGU_CTRL_* */
#define IMGU_REG_SP_START_ADDR(sp)	(IMGU_REG_BASE + (sp) * 0x100 + 0x104)
#define IMGU_REG_SP_ICACHE_ADDR(sp)	(IMGU_REG_BASE + (sp) * 0x100 + 0x11c)
#define IMGU_REG_SP_CTRL_SINK(sp)	(IMGU_REG_BASE + (sp) * 0x100 + 0x130)
#define IMGU_REG_SP_PC(sp)		(IMGU_REG_BASE + (sp) * 0x100 + 0x134)

#define IMGU_REG_TLB_INVALIDATE		(IMGU_REG_BASE + 0x300)
#define IMGU_TLB_INVALIDATE			1
#define IMGU_REG_L1_PHYS		(IMGU_REG_BASE + 0x304) /* 27-bit pfn */

#define IMGU_REG_CIO_GATE_BURST_STATE	(IMGU_REG_BASE + 0x404)

#define IMGU_REG_GP_BUSY		(IMGU_REG_BASE + 0x500)
#define IMGU_REG_GP_STARVING		(IMGU_REG_BASE + 0x504)
#define IMGU_REG_GP_WORKLOAD		(IMGU_REG_BASE + 0x508)
#define IMGU_REG_GP_IRQ(n)	(IMGU_REG_BASE + (n) * 4 + 0x50c) /* n = 0..4 */
#define IMGU_REG_GP_SP1_STRMON_STAT	(IMGU_REG_BASE + 0x520)
#define IMGU_REG_GP_SP2_STRMON_STAT	(IMGU_REG_BASE + 0x524)
#define IMGU_REG_GP_ISP_STRMON_STAT	(IMGU_REG_BASE + 0x528)
#define IMGU_REG_GP_MOD_STRMON_STAT	(IMGU_REG_BASE + 0x52c)

/* Port definitions for the streaming monitors. */
/* For each definition there is signal pair : valid [bit 0]- accept [bit 1] */
#define IMGU_GP_STRMON_STAT_SP1_PORT_SP12DMA		BIT(0)
#define IMGU_GP_STRMON_STAT_SP1_PORT_DMA2SP1		BIT(2)
#define IMGU_GP_STRMON_STAT_SP1_PORT_SP12SP2		BIT(4)
#define IMGU_GP_STRMON_STAT_SP1_PORT_SP22SP1		BIT(6)
#define IMGU_GP_STRMON_STAT_SP1_PORT_SP12ISP		BIT(8)
#define IMGU_GP_STRMON_STAT_SP1_PORT_ISP2SP1		BIT(10)
#define IMGU_GP_STRMON_STAT_SP1_PORT_SP12GDC		BIT(12)
#define IMGU_GP_STRMON_STAT_SP1_PORT_GDC2SP1		BIT(14)
#define IMGU_GP_STRMON_STAT_SP1_PORT_SP12DECOMP		BIT(16)
#define IMGU_GP_STRMON_STAT_SP1_PORT_DECOMP2SP1		BIT(18)
#define IMGU_GP_STRMON_STAT_SP1_PORT_OUTFORMACC		BIT(20)
#define IMGU_GP_STRMON_STAT_SP1_PORT_OUTSCALER		BIT(22)

#define IMGU_GP_STRMON_STAT_SP2_PORT_SP22DMA		BIT(0)
#define IMGU_GP_STRMON_STAT_SP2_PORT_DMA2SP2		BIT(2)
#define IMGU_GP_STRMON_STAT_SP2_PORT_SP22SP1		BIT(4)
#define IMGU_GP_STRMON_STAT_SP2_PORT_SP12SP2		BIT(6)
#define IMGU_GP_STRMON_STAT_SP2_PORT_SP22ISP		BIT(8)
#define IMGU_GP_STRMON_STAT_SP2_PORT_ISP2SP2		BIT(10)
#define IMGU_GP_STRMON_STAT_SP2_PORT_SP22GDC		BIT(12)
#define IMGU_GP_STRMON_STAT_SP2_PORT_GDC2SP2		BIT(14)
#define IMGU_GP_STRMON_STAT_SP2_PORT_SP22DECOMP		BIT(16)
#define IMGU_GP_STRMON_STAT_SP2_PORT_DECOMP2SP2		BIT(18)
#define IMGU_GP_STRMON_STAT_SP2_PORT_OUTFORMACC		BIT(20)
#define IMGU_GP_STRMON_STAT_SP2_PORT_OUTSCALER		BIT(22)

#define IMGU_GP_STRMON_STAT_ISP_PORT_ISP2DMA		BIT(0)
#define IMGU_GP_STRMON_STAT_ISP_PORT_DMA2ISP		BIT(2)
#define IMGU_GP_STRMON_STAT_ISP_PORT_ISP2SP1		BIT(4)
#define IMGU_GP_STRMON_STAT_ISP_PORT_SP12ISP		BIT(6)
#define IMGU_GP_STRMON_STAT_ISP_PORT_ISP2SP2		BIT(8)
#define IMGU_GP_STRMON_STAT_ISP_PORT_SP22ISP		BIT(10)
#define IMGU_GP_STRMON_STAT_ISP_PORT_ISP2GDC		BIT(12)
#define IMGU_GP_STRMON_STAT_ISP_PORT_GDC2ISP		BIT(14)
#define IMGU_GP_STRMON_STAT_ISP_PORT_ISP2DECOMP		BIT(16)
#define IMGU_GP_STRMON_STAT_ISP_PORT_DECOMP2ISP		BIT(18)
#define IMGU_GP_STRMON_STAT_ISP_PORT_S2V1		BIT(20)
#define IMGU_GP_STRMON_STAT_ISP_PORT_S2V2		BIT(22)
#define IMGU_GP_STRMON_STAT_ISP_PORT_S2V3		BIT(24)
#define IMGU_GP_STRMON_STAT_ISP_PORT_S2V4		BIT(26)
#define IMGU_GP_STRMON_STAT_ISP_PORT_S2V5		BIT(28)
#define IMGU_GP_STRMON_STAT_ISP_PORT_S2V6		BIT(30)

/* Between the devices and the fifo */
#define IMGU_GP_STRMON_STAT_MOD_PORT_SP12DMA		BIT(0)
#define IMGU_GP_STRMON_STAT_MOD_PORT_DMA2SP1		BIT(2)
#define IMGU_GP_STRMON_STAT_MOD_PORT_SP22DMA		BIT(4)
#define IMGU_GP_STRMON_STAT_MOD_PORT_DMA2SP2		BIT(6)
#define IMGU_GP_STRMON_STAT_MOD_PORT_ISP2DMA		BIT(8)
#define IMGU_GP_STRMON_STAT_MOD_PORT_DMA2ISP		BIT(10)
#define IMGU_GP_STRMON_STAT_MOD_PORT_CELLS2GDC		BIT(12)
#define IMGU_GP_STRMON_STAT_MOD_PORT_GDC2CELLS		BIT(14)
#define IMGU_GP_STRMON_STAT_MOD_PORT_CELLS2DECOMP	BIT(16)
#define IMGU_GP_STRMON_STAT_MOD_PORT_DECOMP2CELLS	BIT(18)
/* n = 1..6 */
#define IMGU_GP_STRMON_STAT_MOD_PORT_S2V(n)	(1 << (((n) - 1) * 2 + 20))

/* n = 1..15 */
#define IMGU_GP_STRMON_STAT_ACCS_PORT_ACC(n)		(1 << (((n) - 1) * 2))

/* After FIFO and demux before SP1, n = 1..15 */
#define IMGU_GP_STRMON_STAT_ACCS2SP1_MON_PORT_ACC(n)	(1 << (((n) - 1) * 2))

/* After FIFO and demux before SP2, n = 1..15 */
#define IMGU_GP_STRMON_STAT_ACCS2SP2_MON_PORT_ACC(n)	(1 << (((n) - 1) * 2))

#define IMGU_REG_GP_MOD_ISP_STRMON_STAT			(IMGU_REG_BASE + 0x530)
#define IMGU_REG_GP_ACCS_STRMON_STAT			(IMGU_REG_BASE + 0x534)
#define IMGU_REG_GP_ACCS_SP1_STRMON_STAT		(IMGU_REG_BASE + 0x538)
#define IMGU_REG_GP_ACCS_SP2_STRMON_STAT		(IMGU_REG_BASE + 0x53c)
#define IMGU_REG_GP_SP1_STRMON_STAT_IRQ_COND		(IMGU_REG_BASE + 0x540)
#define IMGU_REG_GP_SP2_STRMON_STAT_IRQ_COND		(IMGU_REG_BASE + 0x544)
#define IMGU_REG_GP_ISP_STRMON_STAT_IRQ_COND		(IMGU_REG_BASE + 0x548)
#define IMGU_REG_GP_MOD_STRMON_STAT_IRQ_COND		(IMGU_REG_BASE + 0x54c)
#define IMGU_REG_GP_MOD_ISP_STRMON_STAT_IRQ_COND	(IMGU_REG_BASE + 0x550)
#define IMGU_REG_GP_ACCS_STRMON_STAT_IRQ_COND		(IMGU_REG_BASE + 0x554)
#define IMGU_REG_GP_ACCS_SP1_STRMON_STAT_IRQ_COND	(IMGU_REG_BASE + 0x558)
#define IMGU_REG_GP_ACCS_SP2_STRMON_STAT_IRQ_COND	(IMGU_REG_BASE + 0x55c)
#define IMGU_REG_GP_SP1_STRMON_STAT_IRQ_ENABLE		(IMGU_REG_BASE + 0x560)
#define IMGU_REG_GP_SP2_STRMON_STAT_IRQ_ENABLE		(IMGU_REG_BASE + 0x564)
#define IMGU_REG_GP_ISP_STRMON_STAT_IRQ_ENABLE		(IMGU_REG_BASE + 0x568)
#define IMGU_REG_GP_MOD_STRMON_STAT_IRQ_ENABLE		(IMGU_REG_BASE + 0x56c)
#define IMGU_REG_GP_MOD_ISP_STRMON_STAT_IRQ_ENABLE	(IMGU_REG_BASE + 0x570)
#define IMGU_REG_GP_ACCS_STRMON_STAT_IRQ_ENABLE		(IMGU_REG_BASE + 0x574)
#define IMGU_REG_GP_ACCS_SP1_STRMON_STAT_IRQ_ENABLE	(IMGU_REG_BASE + 0x578)
#define IMGU_REG_GP_ACCS_SP2_STRMON_STAT_IRQ_ENABLE_IDX (IMGU_REG_BASE + 0x57c)
#define IMGU_REG_GP_SWITCH_GDC				(IMGU_REG_BASE + 0x580)
#define IMGU_REG_GP_SWITCH_DECOMP			(IMGU_REG_BASE + 0x584)
#define IMGU_REG_GP_SWITCH_MAIN_IRQ_CTRL_MAIN		(IMGU_REG_BASE + 0x588)
#define IMGU_REG_GP_SWITCH_ACC1				(IMGU_REG_BASE + 0x58c)
#define IMGU_REG_GP_SWITCH_ACC2				(IMGU_REG_BASE + 0x590)
#define IMGU_REG_GP_SWITCH_ACC3				(IMGU_REG_BASE + 0x594)
#define IMGU_REG_GP_SWITCH_ACC4				(IMGU_REG_BASE + 0x598)
#define IMGU_REG_GP_SWITCH_ACC5				(IMGU_REG_BASE + 0x59c)
#define IMGU_REG_GP_SWITCH_ACC6				(IMGU_REG_BASE + 0x5a0)
#define IMGU_REG_GP_SWITCH_ACC7				(IMGU_REG_BASE + 0x5a4)
#define IMGU_REG_GP_SWITCH_ACC8				(IMGU_REG_BASE + 0x5a8)
#define IMGU_REG_GP_SWITCH_ACC9				(IMGU_REG_BASE + 0x5ac)
#define IMGU_REG_GP_SWITCH_ACC10			(IMGU_REG_BASE + 0x5b0)
#define IMGU_REG_GP_SWITCH_ACC11			(IMGU_REG_BASE + 0x5b4)
#define IMGU_REG_GP_SWITCH_ACC12			(IMGU_REG_BASE + 0x5b8)
#define IMGU_REG_GP_SWITCH_ACC13			(IMGU_REG_BASE + 0x5bc)
#define IMGU_REG_GP_SWITCH_ACC14			(IMGU_REG_BASE + 0x5c0)
#define IMGU_REG_GP_SWITCH_ACC15			(IMGU_REG_BASE + 0x5c4)
#define IMGU_REG_GP_SWITCH_OUT_FORM_ACC			(IMGU_REG_BASE + 0x5c8)
#define IMGU_REG_GP_SWITCH_OUT_FORM_SCALER		(IMGU_REG_BASE + 0x5cc)
#define IMGU_REG_GP_SRST				(IMGU_REG_BASE + 0x5d0)
#define IMGU_REG_GP_GACC_SRST				(IMGU_REG_BASE + 0x5d4)
#define IMGU_REG_GP_SLV_REG_SRST			(IMGU_REG_BASE + 0x5d8)
#define IMGU_REG_GP_HALT				(IMGU_REG_BASE + 0x5dc)
#define IMGU_REG_GP_HALTED				(IMGU_REG_BASE + 0x5e0)

					/* n = 0..2 (main ctrl, SP0, SP1) */
#define IMGU_REG_IRQCTRL_BASE(n)	(IMGU_REG_BASE + (n) * 0x100 + 0x700)
#define IMGU_IRQCTRL_MAIN			0
#define IMGU_IRQCTRL_SP0			1
#define IMGU_IRQCTRL_SP1			2
#define IMGU_IRQCTRL_NUM			3
#define IMGU_IRQCTRL_IRQ_SP1			BIT(0)
#define IMGU_IRQCTRL_IRQ_SP2			BIT(1)
#define IMGU_IRQCTRL_IRQ_ISP			BIT(2)
#define IMGU_IRQCTRL_IRQ_SP1_STREAM_MON		BIT(3)
#define IMGU_IRQCTRL_IRQ_SP2_STREAM_MON		BIT(4)
#define IMGU_IRQCTRL_IRQ_ISP_STREAM_MON		BIT(5)
#define IMGU_IRQCTRL_IRQ_MOD_STREAM_MON		BIT(6)
#define IMGU_IRQCTRL_IRQ_MOD_ISP_STREAM_MON	BIT(7)
#define IMGU_IRQCTRL_IRQ_ACCS_STREAM_MON	BIT(8)
#define IMGU_IRQCTRL_IRQ_ACCS_SP1_STREAM_MON	BIT(9)
#define IMGU_IRQCTRL_IRQ_ACCS_SP2_STREAM_MON	BIT(10)
#define IMGU_IRQCTRL_IRQ_ISP_PMEM_ERROR		BIT(11)
#define IMGU_IRQCTRL_IRQ_ISP_BAMEM_ERROR	BIT(12)
#define IMGU_IRQCTRL_IRQ_ISP_VMEM_ERROR		BIT(13)
#define IMGU_IRQCTRL_IRQ_ISP_DMEM_ERROR		BIT(14)
#define IMGU_IRQCTRL_IRQ_SP1_ICACHE_MEM_ERROR	BIT(15)
#define IMGU_IRQCTRL_IRQ_SP1_DMEM_ERROR		BIT(16)
#define IMGU_IRQCTRL_IRQ_SP2_ICACHE_MEM_ERROR	BIT(17)
#define IMGU_IRQCTRL_IRQ_SP2_DMEM_ERROR		BIT(18)
#define IMGU_IRQCTRL_IRQ_ACCS_SCRATCH_MEM_ERROR	BIT(19)
#define IMGU_IRQCTRL_IRQ_GP_TIMER(n)		BIT(20 + (n)) /* n=0..1 */
#define IMGU_IRQCTRL_IRQ_DMA			BIT(22)
#define IMGU_IRQCTRL_IRQ_SW_PIN(n)		BIT(23 + (n)) /* n=0..4 */
#define IMGU_IRQCTRL_IRQ_ACC_SYS		BIT(28)
#define IMGU_IRQCTRL_IRQ_OUT_FORM_IRQ_CTRL	BIT(29)
#define IMGU_IRQCTRL_IRQ_SP1_IRQ_CTRL		BIT(30)
#define IMGU_IRQCTRL_IRQ_SP2_IRQ_CTRL		BIT(31)
#define IMGU_REG_IRQCTRL_EDGE(n)	(IMGU_REG_IRQCTRL_BASE(n) + 0x00)
#define IMGU_REG_IRQCTRL_MASK(n)	(IMGU_REG_IRQCTRL_BASE(n) + 0x04)
#define IMGU_REG_IRQCTRL_STATUS(n)	(IMGU_REG_IRQCTRL_BASE(n) + 0x08)
#define IMGU_REG_IRQCTRL_CLEAR(n)	(IMGU_REG_IRQCTRL_BASE(n) + 0x0c)
#define IMGU_REG_IRQCTRL_ENABLE(n)	(IMGU_REG_IRQCTRL_BASE(n) + 0x10)
#define IMGU_REG_IRQCTRL_EDGE_NOT_PULSE(n) (IMGU_REG_IRQCTRL_BASE(n) + 0x14)
#define IMGU_REG_IRQCTRL_STR_OUT_ENABLE(n) (IMGU_REG_IRQCTRL_BASE(n) + 0x18)

#define IMGU_REG_GP_TIMER		(IMGU_REG_BASE + 0xa34)

#define IMGU_REG_SP_DMEM_BASE(n)	(IMGU_REG_BASE + (n) * 0x4000 + 0x4000)
#define IMGU_REG_ISP_DMEM_BASE		(IMGU_REG_BASE + 0xc000)

#define IMGU_REG_GDC_BASE		(IMGU_REG_BASE + 0x18000)
#define IMGU_REG_GDC_LUT_BASE		(IMGU_REG_GDC_BASE + 0x140)
#define IMGU_GDC_LUT_MASK		((1 << 12) - 1) /* Range -1024..+1024 */

#define IMGU_SCALER_PHASES			32
#define IMGU_SCALER_COEFF_BITS			24
#define IMGU_SCALER_PHASE_COUNTER_PREC_REF	6
#define IMGU_SCALER_MAX_EXPONENT_SHIFT		3
#define IMGU_SCALER_FILTER_TAPS			4
#define IMGU_SCALER_TAPS_Y			IMGU_SCALER_FILTER_TAPS
#define IMGU_SCALER_TAPS_UV			(IMGU_SCALER_FILTER_TAPS / 2)
#define IMGU_SCALER_FIR_PHASES \
		(IMGU_SCALER_PHASES << IMGU_SCALER_PHASE_COUNTER_PREC_REF)
#define IMGU_OSYS_BLOCK_WIDTH			(2 * IPU3_UAPI_ISP_VEC_ELEMS)
#define IMGU_OSYS_BLOCK_HEIGHT			32

/******************* imgu_abi_acc_param *******************/

#define IMGU_ABI_BNR_LUT_SIZE			32

/* number of elements in gamma correction LUT */
#define IMGU_ABI_GAMMA_CORR_LUT_ENTRIES		256

#define IMGU_ABI_SHD_MAX_OPERATIONS \
		(IMGU_ABI_SHD_MAX_PROCESS_LINES + IMGU_ABI_SHD_MAX_TRANSFERS)
#define IMGU_ABI_SHD_MAX_PROCESS_LINES		31
#define IMGU_ABI_SHD_MAX_TRANSFERS		31
#define IMGU_ABI_SHD_MAX_CELLS_PER_SET		146
/* largest grid is 73x56 */
#define IMGU_ABI_SHD_MAX_CFG_SETS		(2 * 28)

#define IMGU_ABI_DVS_STAT_L0_MD_ENTRIES		84
#define IMGU_ABI_DVS_STAT_PARTS_IN_MD_ENTRY	10
#define IMGU_ABI_DVS_STAT_L1_MD_ENTRIES		66
#define IMGU_ABI_DVS_STAT_L2_MD_ENTRIES		45
#define IMGU_ABI_DVS_STAT_MAX_OPERATIONS	100
#define IMGU_ABI_DVS_STAT_MAX_PROCESS_LINES	52
#define IMGU_ABI_DVS_STAT_MAX_TRANSFERS		52

#define IMGU_ABI_YUVP2_YTM_LUT_ENTRIES		256
#define IMGU_ABI_YUVP2_TCC_MACC_TABLE_ELEMENTS	16
#define IMGU_ABI_YUVP2_TCC_INV_Y_LUT_ELEMENTS	14
#define IMGU_ABI_YUVP2_TCC_GAIN_PCWL_LUT_ELEMENTS	258
#define IMGU_ABI_YUVP2_TCC_R_SQR_LUT_ELEMENTS		24

#define IMGU_ABI_DPC_COMMANDS_PER_TRANSFER	2
#define IMGU_ABI_DPC_MAX_SUPPORTED_HEIGHT	3840
#define IMGU_ABI_DPC_STRIPE_SIZE		50
#define IMGU_ABI_DPC_MAX_OPERATIONS \
	(IMGU_ABI_DPC_COMMANDS_PER_TRANSFER * IMGU_ABI_DPC_MAX_CFG_SETS)
#define IMGU_ABI_DPC_MAX_PROCESS_LINES		IMGU_ABI_DPC_MAX_CFG_SETS
#define IMGU_ABI_DPC_MAX_TRANSFERS		IMGU_ABI_DPC_MAX_CFG_SETS
#define IMGU_ABI_DPC_MAX_DP_FIRST_LINES_PAIR	70
#define IMGU_ABI_DPC_MAX_DP_PER_SET		192
#define IMGU_ABI_DPC_MAX_CFG_SETS \
	((IMGU_ABI_DPC_MAX_SUPPORTED_HEIGHT + IMGU_ABI_DPC_STRIPE_SIZE - 1) \
	/ IMGU_ABI_DPC_STRIPE_SIZE)

#define IMGU_ABI_BDS_SAMPLE_PATTERN_ARRAY_SIZE	8
#define IMGU_ABI_BDS_PHASE_COEFFS_ARRAY_SIZE	32

#define IMGU_ABI_ANR_LUT_SIZE			26
#define IMGU_ABI_ANR_PYRAMID_SIZE		22

#define IMGU_ABI_AWB_FR_MAX_TRANSFERS		30
#define IMGU_ABI_AWB_FR_MAX_PROCESS_LINES	30
#define IMGU_ABI_AWB_FR_MAX_OPERATIONS \
	(IMGU_ABI_AWB_FR_MAX_TRANSFERS + IMGU_ABI_AWB_FR_MAX_PROCESS_LINES)

#define IMGU_ABI_AE_WEIGHTS			96

#define IMGU_ABI_AF_MAX_TRANSFERS		30
#define IMGU_ABI_AF_MAX_PROCESS_LINES		30
#define IMGU_ABI_AF_MAX_OPERATIONS \
		(IMGU_ABI_AF_MAX_TRANSFERS + IMGU_ABI_AF_MAX_PROCESS_LINES)

#define IMGU_ABI_AWB_MAX_PROCESS_LINES		68
#define IMGU_ABI_AWB_MAX_TRANSFERS		68
#define IMGU_ABI_AWB_MAX_OPERATIONS \
		(IMGU_ABI_AWB_MAX_PROCESS_LINES + IMGU_ABI_AWB_MAX_TRANSFERS)

#define IMGU_ABI_OSYS_PIN_VF			0
#define IMGU_ABI_OSYS_PIN_OUT			1
#define IMGU_ABI_OSYS_PINS			2

enum imgu_abi_frame_format {
	IMGU_ABI_FRAME_FORMAT_NV11,	/* 12 bit YUV 411, Y, UV plane */
	IMGU_ABI_FRAME_FORMAT_NV12,	/* 12 bit YUV 420, Y, UV plane */
	IMGU_ABI_FRAME_FORMAT_NV12_16,	/* 16 bit YUV 420, Y, UV plane */
	IMGU_ABI_FRAME_FORMAT_NV12_TILEY,/* 12 bit YUV 420,Intel tiled format */
	IMGU_ABI_FRAME_FORMAT_NV16,	/* 16 bit YUV 422, Y, UV plane */
	IMGU_ABI_FRAME_FORMAT_NV21,	/* 12 bit YUV 420, Y, VU plane */
	IMGU_ABI_FRAME_FORMAT_NV61,	/* 16 bit YUV 422, Y, VU plane */
	IMGU_ABI_FRAME_FORMAT_YV12,	/* 12 bit YUV 420, Y, V, U plane */
	IMGU_ABI_FRAME_FORMAT_YV16,	/* 16 bit YUV 422, Y, V, U plane */
	IMGU_ABI_FRAME_FORMAT_YUV420,	/* 12 bit YUV 420, Y, U, V plane */
	IMGU_ABI_FRAME_FORMAT_YUV420_16,/* yuv420, 16 bits per subpixel */
	IMGU_ABI_FRAME_FORMAT_YUV422,	/* 16 bit YUV 422, Y, U, V plane */
	IMGU_ABI_FRAME_FORMAT_YUV422_16,/* yuv422, 16 bits per subpixel */
	IMGU_ABI_FRAME_FORMAT_UYVY,	/* 16 bit YUV 422, UYVY interleaved */
	IMGU_ABI_FRAME_FORMAT_YUYV,	/* 16 bit YUV 422, YUYV interleaved */
	IMGU_ABI_FRAME_FORMAT_YUV444,	/* 24 bit YUV 444, Y, U, V plane */
	IMGU_ABI_FRAME_FORMAT_YUV_LINE,	/* Internal format, 2 y lines */
					/* followed by a uv-interleaved line */
	IMGU_ABI_FRAME_FORMAT_RAW,	/* RAW, 1 plane */
	IMGU_ABI_FRAME_FORMAT_RGB565,	/* 16 bit RGB, 1 plane. Each 3 sub
					 * pixels are packed into one 16 bit
					 * value, 5 bits for R, 6 bits for G
					 * and 5 bits for B.
					 */
	IMGU_ABI_FRAME_FORMAT_PLANAR_RGB888, /* 24 bit RGB, 3 planes */
	IMGU_ABI_FRAME_FORMAT_RGBA888,	/* 32 bit RGBA, 1 plane, A=Alpha
					 * (alpha is unused)
					 */
	IMGU_ABI_FRAME_FORMAT_QPLANE6,	/* Internal, for advanced ISP */
	IMGU_ABI_FRAME_FORMAT_BINARY_8,	/* byte stream, used for jpeg. For
					 * frames of this type, we set the
					 * height to 1 and the width to the
					 * number of allocated bytes.
					 */
	IMGU_ABI_FRAME_FORMAT_MIPI,	/* MIPI frame, 1 plane */
	IMGU_ABI_FRAME_FORMAT_RAW_PACKED,	 /* RAW, 1 plane, packed */
	IMGU_ABI_FRAME_FORMAT_CSI_MIPI_YUV420_8, /* 8 bit per Y/U/V. Y odd line
						  * UYVY interleaved even line
						  */
	IMGU_ABI_FRAME_FORMAT_CSI_MIPI_LEGACY_YUV420_8, /* Legacy YUV420.
							 * UY odd line;
							 * VY even line
							 */
	IMGU_ABI_FRAME_FORMAT_CSI_MIPI_YUV420_10,/* 10 bit per Y/U/V. Y odd
						  * line; UYVY interleaved
						  * even line
						  */
	IMGU_ABI_FRAME_FORMAT_YCgCo444_16, /* Internal format for ISP2.7,
					    * 16 bits per plane YUV 444,
					    * Y, U, V plane
					    */
	IMGU_ABI_FRAME_FORMAT_NUM
};

enum imgu_abi_bayer_order {
	IMGU_ABI_BAYER_ORDER_GRBG,
	IMGU_ABI_BAYER_ORDER_RGGB,
	IMGU_ABI_BAYER_ORDER_BGGR,
	IMGU_ABI_BAYER_ORDER_GBRG
};

enum imgu_abi_osys_format {
	IMGU_ABI_OSYS_FORMAT_YUV420,
	IMGU_ABI_OSYS_FORMAT_YV12,
	IMGU_ABI_OSYS_FORMAT_NV12,
	IMGU_ABI_OSYS_FORMAT_NV21,
	IMGU_ABI_OSYS_FORMAT_YUV_LINE,
	IMGU_ABI_OSYS_FORMAT_YUY2,	/* = IMGU_ABI_OSYS_FORMAT_YUYV */
	IMGU_ABI_OSYS_FORMAT_NV16,
	IMGU_ABI_OSYS_FORMAT_RGBA,
	IMGU_ABI_OSYS_FORMAT_BGRA
};

enum imgu_abi_osys_tiling {
	IMGU_ABI_OSYS_TILING_NONE,
	IMGU_ABI_OSYS_TILING_Y,
	IMGU_ABI_OSYS_TILING_YF,
};

struct imgu_abi_shd_grid_config {
	/* reg 0 */
	u32 grid_width:8;
	u32 grid_height:8;
	u32 block_width:3;
	u32 __reserved0:1;
	u32 block_height:3;
	u32 __reserved1:1;
	u32 grid_height_per_slice:8;
	/* reg 1 */
	s32 x_start:13;
	s32 __reserved2:3;
	s32 y_start:13;
	s32 __reserved3:3;
};

struct imgu_abi_shd_general_config {
	u32 init_set_vrt_offst_ul:8;
	u32 shd_enable:1;
	/* aka 'gf' */
	u32 gain_factor:2;
	u32 __reserved:21;
};

struct imgu_abi_shd_black_level_config {
	/* reg 0 */
	s32 bl_r:12;
	s32 __reserved0:4;
	s32 bl_gr:12;
	u32 __reserved1:1;
	/* aka 'nf' */
	u32 normalization_shift:3;
	/* reg 1 */
	s32 bl_gb:12;
	s32 __reserved2:4;
	s32 bl_b:12;
	s32 __reserved3:4;
};

struct imgu_abi_shd_config_static {
	/* B0: Fixed order: one transfer to GAC */
	struct imgu_abi_shd_grid_config grid;
	struct imgu_abi_shd_general_config general;
	struct imgu_abi_shd_black_level_config black_level;
};

struct imgu_abi_shd_lut_elem {
	u16 factors[2];
};

struct imgu_abi_shd_lut_set {
	struct imgu_abi_shd_lut_elem elems[IMGU_ABI_SHD_MAX_CELLS_PER_SET]
		IMGU_ABI_PAD;
};

struct imgu_abi_shd_lut {
	struct imgu_abi_shd_lut_set sets[IMGU_ABI_SHD_MAX_CFG_SETS];
};

struct imgu_abi_shd_config {
	struct imgu_abi_shd_config_static shd IMGU_ABI_PAD;
	struct ipu3_uapi_shd_intra_frame_operations_data shd_ops IMGU_ABI_PAD;
	struct imgu_abi_shd_lut shd_lut IMGU_ABI_PAD;
};

struct imgu_abi_dpc_param0 {
	u32 enable:1;
	u32 __reserved0:15;
	u32 grad_threshold:13;
	u32 __reserved1:3;
};

struct imgu_abi_dpc_num_of_dp {
	u32 num_of_dp_gr:8;
	u32 num_of_dp_bg:8;
	u32 __reserved0:16;
};

struct imgu_abi_dpc_params {
	struct imgu_abi_dpc_param0 enable_and_threshold;
	struct imgu_abi_dpc_num_of_dp num_of_dp_set0;
	struct imgu_abi_dpc_num_of_dp num_of_dp_set1;
	struct imgu_abi_dpc_num_of_dp num_of_dp_first_line_pair;
};

/******************* Firmware ABI definitions *******************/

/***** struct imgu_abi_sp_stage *****/

#define IMGU_ABI_BINARY_MAX_OUTPUT_PORTS 2

enum imgu_abi_queue_id {
	IMGU_ABI_QUEUE_EVENT_ID = -1,
	IMGU_ABI_QUEUE_A_ID = 0,
	IMGU_ABI_QUEUE_B_ID,
	IMGU_ABI_QUEUE_C_ID,
	IMGU_ABI_QUEUE_D_ID,
	IMGU_ABI_QUEUE_E_ID,
	IMGU_ABI_QUEUE_F_ID,
	IMGU_ABI_QUEUE_G_ID,
	IMGU_ABI_QUEUE_H_ID,		/* input frame queue for skycam */
	IMGU_ABI_QUEUE_NUM
};

enum imgu_abi_buffer_type {
	IMGU_ABI_BUFFER_TYPE_INVALID = -1,
	IMGU_ABI_BUFFER_TYPE_3A_STATISTICS = 0,
	IMGU_ABI_BUFFER_TYPE_DIS_STATISTICS,
	IMGU_ABI_BUFFER_TYPE_LACE_STATISTICS,
	IMGU_ABI_BUFFER_TYPE_INPUT_FRAME,
	IMGU_ABI_BUFFER_TYPE_OUTPUT_FRAME,
	IMGU_ABI_BUFFER_TYPE_SEC_OUTPUT_FRAME,
	IMGU_ABI_BUFFER_TYPE_VF_OUTPUT_FRAME,
	IMGU_ABI_BUFFER_TYPE_SEC_VF_OUTPUT_FRAME,
	IMGU_ABI_BUFFER_TYPE_RAW_OUTPUT_FRAME,
	IMGU_ABI_BUFFER_TYPE_CUSTOM_INPUT,
	IMGU_ABI_BUFFER_TYPE_CUSTOM_OUTPUT,
	IMGU_ABI_BUFFER_TYPE_METADATA,
	IMGU_ABI_BUFFER_TYPE_PARAMETER_SET,
	IMGU_ABI_BUFFER_TYPE_PER_FRAME_PARAMETER_SET,
	IMGU_ABI_NUM_DYNAMIC_BUFFER_TYPE,
	IMGU_ABI_NUM_BUFFER_TYPE
};

struct imgu_abi_crop_pos {
	u16 x;
	u16 y;
};

struct imgu_abi_sp_resolution {
	u16 width;			/* Width of valid data in pixels */
	u16 height;			/* Height of valid data in lines */
};

/*
 * Frame info struct. This describes the contents of an image frame buffer.
 */
struct imgu_abi_frame_sp_info {
	struct imgu_abi_sp_resolution res;
	u16 padded_width;		/* stride of line in memory
					 * (in pixels)
					 */
	u8 format;			/* format of the frame data */
	u8 raw_bit_depth;		/* number of valid bits per pixel,
					 * only valid for RAW bayer frames
					 */
	u8 raw_bayer_order;		/* bayer order, only valid
					 * for RAW bayer frames
					 */
	u8 raw_type;		/* To choose the proper raw frame type. for
				 * Legacy SKC pipes/Default is set to
				 * IMGU_ABI_RAW_TYPE_BAYER. For RGB IR sensor -
				 * driver should set it to:
				 * IronGr case - IMGU_ABI_RAW_TYPE_IR_ON_GR
				 * IronGb case - IMGU_ABI_RAW_TYPE_IR_ON_GB
				 */
#define IMGU_ABI_RAW_TYPE_BAYER		0
#define IMGU_ABI_RAW_TYPE_IR_ON_GR	1
#define IMGU_ABI_RAW_TYPE_IR_ON_GB	2
	u8 padding[2];			/* Extend to 32 bit multiple */
};

struct imgu_abi_buffer_sp {
	union {
		imgu_addr_t xmem_addr;
		enum imgu_abi_queue_id queue_id;
	} buf_src;
	enum imgu_abi_buffer_type buf_type;
};

struct imgu_abi_frame_sp_plane {
	u32 offset;		/* offset in bytes to start of frame data */
				/* offset is wrt data in imgu_abi_sp_sp_frame */
};

struct imgu_abi_frame_sp_rgb_planes {
	struct imgu_abi_frame_sp_plane r;
	struct imgu_abi_frame_sp_plane g;
	struct imgu_abi_frame_sp_plane b;
};
struct imgu_abi_frame_sp_yuv_planes {
	struct imgu_abi_frame_sp_plane y;
	struct imgu_abi_frame_sp_plane u;
	struct imgu_abi_frame_sp_plane v;
};
struct imgu_abi_frame_sp_nv_planes {
	struct imgu_abi_frame_sp_plane y;
	struct imgu_abi_frame_sp_plane uv;
};
struct imgu_abi_frame_sp_plane6 {
	struct imgu_abi_frame_sp_plane r;
	struct imgu_abi_frame_sp_plane r_at_b;
	struct imgu_abi_frame_sp_plane gr;
	struct imgu_abi_frame_sp_plane gb;
	struct imgu_abi_frame_sp_plane b;
	struct imgu_abi_frame_sp_plane b_at_r;
};
struct imgu_abi_frame_sp_binary_plane {
	u32 size;
	struct imgu_abi_frame_sp_plane data;
};

struct imgu_abi_frame_sp {
	struct imgu_abi_frame_sp_info info;
	struct imgu_abi_buffer_sp buf_attr;
	union {
		struct imgu_abi_frame_sp_plane raw;
		struct imgu_abi_frame_sp_plane rgb;
		struct imgu_abi_frame_sp_rgb_planes planar_rgb;
		struct imgu_abi_frame_sp_plane yuyv;
		struct imgu_abi_frame_sp_yuv_planes yuv;
		struct imgu_abi_frame_sp_nv_planes nv;
		struct imgu_abi_frame_sp_plane6 plane6;
		struct imgu_abi_frame_sp_binary_plane binary;
	} planes;
};

struct imgu_abi_resolution {
	u32 width;
	u32 height;
};

struct imgu_abi_frames_sp {
	struct imgu_abi_frame_sp in;
	struct imgu_abi_frame_sp out[IMGU_ABI_BINARY_MAX_OUTPUT_PORTS];
	struct imgu_abi_resolution effective_in_res;
	struct imgu_abi_frame_sp out_vf;
	struct imgu_abi_frame_sp_info internal_frame_info;
	struct imgu_abi_buffer_sp s3a_buf;
	struct imgu_abi_buffer_sp dvs_buf;
	struct imgu_abi_buffer_sp lace_buf;
};

struct imgu_abi_uds_info {
	u16 curr_dx;
	u16 curr_dy;
	u16 xc;
	u16 yc;
};

/* Information for a single pipeline stage */
struct imgu_abi_sp_stage {
	/* Multiple boolean flags can be stored in an integer */
	u8 num;				/* Stage number */
	u8 isp_online;
	u8 isp_copy_vf;
	u8 isp_copy_output;
	u8 sp_enable_xnr;
	u8 isp_deci_log_factor;
	u8 isp_vf_downscale_bits;
	u8 deinterleaved;
	/*
	 * NOTE: Programming the input circuit can only be done at the
	 * start of a session. It is illegal to program it during execution
	 * The input circuit defines the connectivity
	 */
	u8 program_input_circuit;
	u8 func;
#define IMGU_ABI_STAGE_FUNC_RAW_COPY	0
#define IMGU_ABI_STAGE_FUNC_BIN_COPY	1
#define IMGU_ABI_STAGE_FUNC_ISYS_COPY	2
#define IMGU_ABI_STAGE_FUNC_NO_FUNC	3
	u8 stage_type;			/* The type of the pipe-stage */
#define IMGU_ABI_STAGE_TYPE_SP		0
#define IMGU_ABI_STAGE_TYPE_ISP		1
	u8 num_stripes;
	u8 isp_pipe_version;
	struct {
		u8 vf_output;
		u8 s3a;
		u8 sdis;
		u8 dvs_stats;
		u8 lace_stats;
	} enable;

	struct imgu_abi_crop_pos sp_out_crop_pos;
	struct imgu_abi_frames_sp frames;
	struct imgu_abi_resolution dvs_envelope;
	struct imgu_abi_uds_info uds;
	imgu_addr_t isp_stage_addr;
	imgu_addr_t xmem_bin_addr;
	imgu_addr_t xmem_map_addr;

	u16 top_cropping;
	u16 row_stripes_height;
	u16 row_stripes_overlap_lines;
	u8 if_config_index;	/* Which should be applied by this stage. */
};

/***** struct imgu_abi_isp_stage *****/

#define IMGU_ABI_MAX_BINARY_NAME  64

enum imgu_abi_memories {
	IMGU_ABI_MEM_ISP_PMEM0 = 0,
	IMGU_ABI_MEM_ISP_DMEM0,
	IMGU_ABI_MEM_ISP_VMEM0,
	IMGU_ABI_MEM_ISP_VAMEM0,
	IMGU_ABI_MEM_ISP_VAMEM1,
	IMGU_ABI_MEM_ISP_VAMEM2,
	IMGU_ABI_MEM_ISP_HMEM0,
	IMGU_ABI_MEM_SP0_DMEM0,
	IMGU_ABI_MEM_SP1_DMEM0,
	IMGU_ABI_MEM_DDR,
	IMGU_ABI_NUM_MEMORIES
};

enum imgu_abi_param_class {
	IMGU_ABI_PARAM_CLASS_PARAM,	/* Late binding parameters, like 3A */
	IMGU_ABI_PARAM_CLASS_CONFIG,	/* Pipe config time parameters */
	IMGU_ABI_PARAM_CLASS_STATE,	/* State parameters, eg. buffer index */
	IMGU_ABI_PARAM_CLASS_NUM
};

struct imgu_abi_isp_param_memory_offsets {
	u32 offsets[IMGU_ABI_PARAM_CLASS_NUM];	/* offset wrt hdr in bytes */
};

/*
 * Blob descriptor.
 * This structure describes an SP or ISP blob.
 * It describes the test, data and bss sections as well as position in a
 * firmware file.
 * For convenience, it contains dynamic data after loading.
 */
struct imgu_abi_blob_info {
	/* Static blob data */
	u32 offset;			/* Blob offset in fw file */
	struct imgu_abi_isp_param_memory_offsets memory_offsets;
					/* offset wrt hdr in bytes */
	u32 prog_name_offset;		/* offset wrt hdr in bytes */
	u32 size;			/* Size of blob */
	u32 padding_size;		/* total cummulative of bytes added
					 * due to section alignment
					 */
	u32 icache_source;		/* Position of icache in blob */
	u32 icache_size;		/* Size of icache section */
	u32 icache_padding;	/* added due to icache section alignment */
	u32 text_source;		/* Position of text in blob */
	u32 text_size;			/* Size of text section */
	u32 text_padding;	/* bytes added due to text section alignment */
	u32 data_source;		/* Position of data in blob */
	u32 data_target;		/* Start of data in SP dmem */
	u32 data_size;			/* Size of text section */
	u32 data_padding;	/* bytes added due to data section alignment */
	u32 bss_target;		/* Start position of bss in SP dmem */
	u32 bss_size;			/* Size of bss section
					 * Dynamic data filled by loader
					 */
	const void *code __aligned(8);	/* Code section absolute pointer */
					/* within fw, code = icache + text */
	const void *data __aligned(8);	/* Data section absolute pointer */
					/* within fw, data = data + bss */
};
struct imgu_abi_binary_pipeline_info {
	u32 mode;
	u32 isp_pipe_version;
	u32 pipelining;
	u32 c_subsampling;
	u32 top_cropping;
	u32 left_cropping;
	u32 variable_resolution;
};

struct imgu_abi_binary_input_info {
	u32 min_width;
	u32 min_height;
	u32 max_width;
	u32 max_height;
	u32 source;			/* memory, sensor, variable */
#define IMGU_ABI_BINARY_INPUT_SOURCE_SENSOR	0
#define IMGU_ABI_BINARY_INPUT_SOURCE_MEMORY	1
#define IMGU_ABI_BINARY_INPUT_SOURCE_VARIABLE	2
};

struct imgu_abi_binary_output_info {
	u32 min_width;
	u32 min_height;
	u32 max_width;
	u32 max_height;
	u32 num_chunks;
	u32 variable_format;
};

struct imgu_abi_binary_internal_info {
	u32 max_width;
	u32 max_height;
};

struct imgu_abi_binary_bds_info {
	u32 supported_bds_factors;
/*
 * enumeration of the bayer downscale factors. When a binary supports multiple
 * factors, the OR of these defines is used to build the mask of supported
 * factors. The BDS factor is used in pre-processor expressions so we cannot
 * use an enum here.
 */
#define IMGU_ABI_BDS_FACTOR_1_00		BIT(0)
#define IMGU_ABI_BDS_FACTOR_1_25		BIT(1)
#define IMGU_ABI_BDS_FACTOR_1_50		BIT(2)
#define IMGU_ABI_BDS_FACTOR_2_00		BIT(3)
#define IMGU_ABI_BDS_FACTOR_2_25		BIT(4)
#define IMGU_ABI_BDS_FACTOR_2_50		BIT(5)
#define IMGU_ABI_BDS_FACTOR_3_00		BIT(6)
#define IMGU_ABI_BDS_FACTOR_4_00		BIT(7)
#define IMGU_ABI_BDS_FACTOR_4_50		BIT(8)
#define IMGU_ABI_BDS_FACTOR_5_00		BIT(9)
#define IMGU_ABI_BDS_FACTOR_6_00		BIT(10)
#define IMGU_ABI_BDS_FACTOR_8_00		BIT(11)
};

struct imgu_abi_binary_dvs_info {
	u32 max_envelope_width;
	u32 max_envelope_height;
};

struct imgu_abi_binary_vf_dec_info {
	u32 is_variable;
	u32 max_log_downscale;
};

struct imgu_abi_binary_s3a_info {
	u32 s3atbl_use_dmem;
	u32 fixed_s3a_deci_log;
};

struct imgu_abi_binary_dpc_info {
	u32 bnr_lite;			/* bnr lite enable flag */
};

struct imgu_abi_binary_iterator_info {
	u32 num_stripes;
	u32 row_stripes_height;
	u32 row_stripes_overlap_lines;
};

struct imgu_abi_binary_address_info {
	u32 isp_addresses;		/* Address in ISP dmem */
	u32 main_entry;			/* Address of entry fct */
	u32 in_frame;			/* Address in ISP dmem */
	u32 out_frame;			/* Address in ISP dmem */
	u32 in_data;			/* Address in ISP dmem */
	u32 out_data;			/* Address in ISP dmem */
	u32 sh_dma_cmd_ptr;		/* In ISP dmem */
};

struct imgu_abi_binary_uds_info {
	u16 bpp;
	u16 use_bci;
	u16 use_str;
	u16 woix;
	u16 woiy;
	u16 extra_out_vecs;
	u16 vectors_per_line_in;
	u16 vectors_per_line_out;
	u16 vectors_c_per_line_in;
	u16 vectors_c_per_line_out;
	u16 vmem_gdc_in_block_height_y;
	u16 vmem_gdc_in_block_height_c;
};

struct imgu_abi_binary_block_info {
	u32 block_width;
	u32 block_height;
	u32 output_block_height;
};

struct imgu_abi_isp_data {
	imgu_addr_t address;		/* ISP address */
	u32 size;			/* Disabled if 0 */
};

struct imgu_abi_isp_param_segments {
	struct imgu_abi_isp_data
			params[IMGU_ABI_PARAM_CLASS_NUM][IMGU_ABI_NUM_MEMORIES];
};

struct imgu_abi_binary_info {
	u32 id __aligned(8);		/* IMGU_ABI_BINARY_ID_* */
	struct imgu_abi_binary_pipeline_info pipeline;
	struct imgu_abi_binary_input_info input;
	struct imgu_abi_binary_output_info output;
	struct imgu_abi_binary_internal_info internal;
	struct imgu_abi_binary_bds_info bds;
	struct imgu_abi_binary_dvs_info dvs;
	struct imgu_abi_binary_vf_dec_info vf_dec;
	struct imgu_abi_binary_s3a_info s3a;
	struct imgu_abi_binary_dpc_info dpc_bnr; /* DPC related binary info */
	struct imgu_abi_binary_iterator_info iterator;
	struct imgu_abi_binary_address_info addresses;
	struct imgu_abi_binary_uds_info uds;
	struct imgu_abi_binary_block_info block;
	struct imgu_abi_isp_param_segments mem_initializers;
	struct {
		u8 input_feeder;
		u8 output_system;
		u8 obgrid;
		u8 lin;
		u8 dpc_acc;
		u8 bds_acc;
		u8 shd_acc;
		u8 shd_ff;
		u8 stats_3a_raw_buffer;
		u8 acc_bayer_denoise;
		u8 bnr_ff;
		u8 awb_acc;
		u8 awb_fr_acc;
		u8 anr_acc;
		u8 rgbpp_acc;
		u8 rgbpp_ff;
		u8 demosaic_acc;
		u8 demosaic_ff;
		u8 dvs_stats;
		u8 lace_stats;
		u8 yuvp1_b0_acc;
		u8 yuvp1_c0_acc;
		u8 yuvp2_acc;
		u8 ae;
		u8 af;
		u8 dergb;
		u8 rgb2yuv;
		u8 high_quality;
		u8 kerneltest;
		u8 routing_shd_to_bnr;		/* connect SHD with BNR ACCs*/
		u8 routing_bnr_to_anr;		/* connect BNR with ANR ACCs*/
		u8 routing_anr_to_de;		/* connect ANR with DE ACCs */
		u8 routing_rgb_to_yuvp1;	/* connect RGB with YUVP1 ACCs*/
		u8 routing_yuvp1_to_yuvp2;    /* connect YUVP1 with YUVP2 ACCs*/
		u8 luma_only;
		u8 input_yuv;
		u8 input_raw;
		u8 reduced_pipe;
		u8 vf_veceven;
		u8 dis;
		u8 dvs_envelope;
		u8 uds;
		u8 dvs_6axis;
		u8 block_output;
		u8 streaming_dma;
		u8 ds;
		u8 bayer_fir_6db;
		u8 raw_binning;
		u8 continuous;
		u8 s3a;
		u8 fpnr;
		u8 sc;
		u8 macc;
		u8 output;
		u8 ref_frame;
		u8 tnr;
		u8 xnr;
		u8 params;
		u8 ca_gdc;
		u8 isp_addresses;
		u8 in_frame;
		u8 out_frame;
		u8 high_speed;
		u8 dpc;
		u8 padding[2];
		u8 rgbir;
	} enable;
	struct {
		/* DMA channel ID: [0,...,IMGU_NUM_DMA_CHANNELS> */
#define IMGU_NUM_DMA_CHANNELS		19
		u8 ref_y_channel;
		u8 ref_c_channel;
		u8 tnr_channel;
		u8 tnr_out_channel;
		u8 dvs_coords_channel;
		u8 output_channel;
		u8 c_channel;
		u8 vfout_channel;
		u8 vfout_c_channel;
		u8 vfdec_bits_per_pixel;
		u8 claimed_by_isp;
		u8 padding[2];
	} dma;
};

struct imgu_abi_isp_stage {
	struct imgu_abi_blob_info blob_info;
	struct imgu_abi_binary_info binary_info;
	char binary_name[IMGU_ABI_MAX_BINARY_NAME];
	struct imgu_abi_isp_param_segments mem_initializers;
};

/***** struct imgu_abi_ddr_address_map and parameter set *****/

#define IMGU_ABI_ISP_DDR_WORD_BITS	256
#define IMGU_ABI_ISP_DDR_WORD_BYTES	(IMGU_ABI_ISP_DDR_WORD_BITS / 8)
#define IMGU_ABI_MAX_STAGES		3

/* xmem address map allocation */
struct imgu_abi_ddr_address_map {
	imgu_addr_t isp_mem_param[IMGU_ABI_MAX_STAGES][IMGU_ABI_NUM_MEMORIES];
	imgu_addr_t obgrid_tbl[IPU3_UAPI_MAX_STRIPES];
	imgu_addr_t acc_cluster_params_for_sp;
	imgu_addr_t dvs_6axis_params_y;
};

struct imgu_abi_parameter_set_info {
	/* Pointers to Parameters in ISP format IMPT */
	struct imgu_abi_ddr_address_map mem_map;
	/* Unique ID to track per-frame configurations */
	u32 isp_parameters_id;
	/* Output frame to which this config has to be applied (optional) */
	imgu_addr_t output_frame_ptr;
};

/***** struct imgu_abi_sp_group *****/

#define IMGU_ABI_MAX_IF_CONFIGS	3

/* SP configuration information */
struct imgu_abi_sp_config {
	u8 no_isp_sync;		/* Signal host immediately after start */
	u8 enable_raw_pool_locking;    /* Enable Raw Buffer Locking for HALv3 */
	u8 lock_all;
	u8 disable_cont_vf;
	u8 disable_preview_on_capture;
};

/* Information for a pipeline */
struct imgu_abi_sp_pipeline {
	u32 pipe_id;			/* the pipe ID */
	u32 pipe_num;			/* the dynamic pipe number */
	u32 thread_id;			/* the sp thread ID */
	u32 pipe_config;		/* the pipe config */
#define IMGU_ABI_PIPE_CONFIG_ACQUIRE_ISP	(1 << 31)
	u32 pipe_qos_config;		/* Bitmap of multiple QOS extension fw
					 * state, 0xffffffff indicates non
					 * QOS pipe.
					 */
	u32 inout_port_config;
#define IMGU_ABI_PORT_CONFIG_TYPE_INPUT_HOST		(1 << 0)
#define IMGU_ABI_PORT_CONFIG_TYPE_INPUT_COPYSINK	(1 << 1)
#define IMGU_ABI_PORT_CONFIG_TYPE_INPUT_TAGGERSINK	(1 << 2)
#define IMGU_ABI_PORT_CONFIG_TYPE_OUTPUT_HOST		(1 << 4)
#define IMGU_ABI_PORT_CONFIG_TYPE_OUTPUT_COPYSINK	(1 << 5)
#define IMGU_ABI_PORT_CONFIG_TYPE_OUTPUT_TAGGERSINK	(1 << 6)
	u32 required_bds_factor;
	u32 dvs_frame_delay;
	u32 num_stages;		/* the pipe config */
	u32 running;			/* needed for pipe termination */
	imgu_addr_t sp_stage_addr[IMGU_ABI_MAX_STAGES];
	imgu_addr_t scaler_pp_lut;	/* Early bound LUT */
	u32 stage;			/* stage ptr is only used on sp */
	s32 num_execs;			/* number of times to run if this is
					 * an acceleration pipe.
					 */
	union {
		struct {
			u32 bytes_available;
		} bin;
		struct {
			u32 height;
			u32 width;
			u32 padded_width;
			u32 max_input_width;
			u32 raw_bit_depth;
		} raw;
	} copy;

	/* Parameters passed to Shading Correction kernel. */
	struct {
		/* Origin X (bqs) of internal frame on shading table */
		u32 internal_frame_origin_x_bqs_on_sctbl;
		/* Origin Y (bqs) of internal frame on shading table */
		u32 internal_frame_origin_y_bqs_on_sctbl;
	} shading;
};

struct imgu_abi_sp_debug_command {
	/*
	 * The DMA software-mask,
	 *      Bit 31...24: unused.
	 *      Bit 23...16: unused.
	 *      Bit 15...08: reading-request enabling bits for DMA channel 7..0
	 *      Bit 07...00: writing-request enabling bits for DMA channel 7..0
	 *
	 * For example, "0...0 0...0 11111011 11111101" indicates that the
	 * writing request through DMA Channel 1 and the reading request
	 * through DMA channel 2 are both disabled. The others are enabled.
	 */
	u32 dma_sw_reg;
};

#define IMGU_ABI_MAX_SP_THREADS	4

/*
 * Group all host initialized SP variables into this struct.
 * This is initialized every stage through dma.
 * The stage part itself is transferred through imgu_abi_sp_stage.
 */
struct imgu_abi_sp_group {
	struct imgu_abi_sp_config config;
	struct imgu_abi_sp_pipeline pipe[IMGU_ABI_MAX_SP_THREADS];
	struct imgu_abi_sp_debug_command debug;
};

/***** parameter and state class binary configurations *****/

#define IMGU_ABI_FRAMES_REF		3
#define IMGU_ABI_FRAMES_TNR		4
#define IMGU_ABI_BUF_SETS_TNR		1

struct imgu_abi_isp_iterator_config {
	struct imgu_abi_frame_sp_info input_info;
	struct imgu_abi_frame_sp_info internal_info;
	struct imgu_abi_frame_sp_info output_info;
	struct imgu_abi_frame_sp_info vf_info;
	struct imgu_abi_sp_resolution dvs_envelope;
};

struct imgu_abi_dma_port_config {
	u8 crop, elems;
	u16 width;
	u32 stride;
};

struct imgu_abi_isp_ref_config {
	u32 width_a_over_b;
	struct imgu_abi_dma_port_config port_b;
	u32 ref_frame_addr_y[IMGU_ABI_FRAMES_REF];
	u32 ref_frame_addr_c[IMGU_ABI_FRAMES_REF];
	u32 dvs_frame_delay;
};

struct imgu_abi_isp_ref_dmem_state {
	u32 ref_in_buf_idx;
	u32 ref_out_buf_idx;
};

struct imgu_abi_isp_dvs_config {
	u32 num_horizontal_blocks;
	u32 num_vertical_blocks;
};

struct imgu_abi_isp_tnr3_config {
	u32 width_a_over_b;
	u32 frame_height;
	struct imgu_abi_dma_port_config port_b;
	u32 delay_frame;
	u32 frame_addr[IMGU_ABI_FRAMES_TNR];
};

struct imgu_abi_isp_tnr3_dmem_state {
	u32 in_bufidx;
	u32 out_bufidx;
	u32 total_frame_counter;
	u32 buffer_frame_counter[IMGU_ABI_BUF_SETS_TNR];
	u32 bypass_filter;
};

/***** DVS statistics metadata *****/

#define IMGU_ABI_DVS_METADATA_L0_REGS		(84 * 10)
#define IMGU_ABI_DVS_METADATA_L1_REGS		(66 * 10)
#define IMGU_ABI_DVS_METADATA_L2_REGS		(45 * 10)

struct imgu_abi_dvs_meta_data {
	u32 dvs_prev_frame_fe_l0[IMGU_ABI_DVS_METADATA_L0_REGS]
		__aligned(IMGU_ABI_ISP_DDR_WORD_BYTES);
	u32 dvs_prev_frame_fe_l1[IMGU_ABI_DVS_METADATA_L1_REGS]
		__aligned(IMGU_ABI_ISP_DDR_WORD_BYTES);
	u32 dvs_prev_frame_fe_l2[IMGU_ABI_DVS_METADATA_L2_REGS]
		__aligned(IMGU_ABI_ISP_DDR_WORD_BYTES);
};

/***** Queues *****/

#define IMGU_ABI_EVENT_BUFFER_ENQUEUED(thread, queue)	\
				(0 << 24 | (thread) << 16 | (queue) << 8)
#define IMGU_ABI_EVENT_BUFFER_DEQUEUED(queue)	(1 << 24 | (queue) << 8)
#define IMGU_ABI_EVENT_EVENT_DEQUEUED		(2 << 24)
#define IMGU_ABI_EVENT_START_STREAM		(3 << 24)
#define IMGU_ABI_EVENT_STOP_STREAM		(4 << 24)
#define IMGU_ABI_EVENT_MIPI_BUFFERS_READY	(5 << 24)
#define IMGU_ABI_EVENT_UNLOCK_RAW_BUFFER	(6 << 24)
#define IMGU_ABI_EVENT_STAGE_ENABLE_DISABLE	(7 << 24)

#define IMGU_ABI_HOST2SP_BUFQ_SIZE	3
#define IMGU_ABI_SP2HOST_BUFQ_SIZE	(2 * IMGU_ABI_MAX_SP_THREADS)
#define IMGU_ABI_HOST2SP_EVTQ_SIZE	(IMGU_ABI_QUEUE_NUM * \
		IMGU_ABI_MAX_SP_THREADS * 2 + IMGU_ABI_MAX_SP_THREADS * 4)
#define IMGU_ABI_SP2HOST_EVTQ_SIZE	(6 * IMGU_ABI_MAX_SP_THREADS)

#define IMGU_ABI_EVTTYPE_EVENT_SHIFT	0
#define IMGU_ABI_EVTTYPE_EVENT_MASK	(0xff << IMGU_ABI_EVTTYPE_EVENT_SHIFT)
#define IMGU_ABI_EVTTYPE_PIPE_SHIFT	8
#define IMGU_ABI_EVTTYPE_PIPE_MASK	(0xff << IMGU_ABI_EVTTYPE_PIPE_SHIFT)
#define IMGU_ABI_EVTTYPE_PIPEID_SHIFT	16
#define IMGU_ABI_EVTTYPE_PIPEID_MASK	(0xff << IMGU_ABI_EVTTYPE_PIPEID_SHIFT)
#define IMGU_ABI_EVTTYPE_MODULEID_SHIFT	8
#define IMGU_ABI_EVTTYPE_MODULEID_MASK (0xff << IMGU_ABI_EVTTYPE_MODULEID_SHIFT)
#define IMGU_ABI_EVTTYPE_LINENO_SHIFT	16
#define IMGU_ABI_EVTTYPE_LINENO_MASK   (0xffff << IMGU_ABI_EVTTYPE_LINENO_SHIFT)

/* Output frame ready */
#define IMGU_ABI_EVTTYPE_OUT_FRAME_DONE			0
/* Second output frame ready */
#define IMGU_ABI_EVTTYPE_2ND_OUT_FRAME_DONE		1
/* Viewfinder Output frame ready */
#define IMGU_ABI_EVTTYPE_VF_OUT_FRAME_DONE		2
/* Second viewfinder Output frame ready */
#define IMGU_ABI_EVTTYPE_2ND_VF_OUT_FRAME_DONE		3
/* Indication that 3A statistics are available */
#define IMGU_ABI_EVTTYPE_3A_STATS_DONE			4
/* Indication that DIS statistics are available */
#define IMGU_ABI_EVTTYPE_DIS_STATS_DONE			5
/* Pipeline Done event, sent after last pipeline stage */
#define IMGU_ABI_EVTTYPE_PIPELINE_DONE			6
/* Frame tagged */
#define IMGU_ABI_EVTTYPE_FRAME_TAGGED			7
/* Input frame ready */
#define IMGU_ABI_EVTTYPE_INPUT_FRAME_DONE		8
/* Metadata ready */
#define IMGU_ABI_EVTTYPE_METADATA_DONE			9
/* Indication that LACE statistics are available */
#define IMGU_ABI_EVTTYPE_LACE_STATS_DONE		10
/* Extension stage executed */
#define IMGU_ABI_EVTTYPE_ACC_STAGE_COMPLETE		11
/* Timing measurement data */
#define IMGU_ABI_EVTTYPE_TIMER				12
/* End Of Frame event, sent when in buffered sensor mode */
#define IMGU_ABI_EVTTYPE_PORT_EOF			13
/* Performance warning encountered by FW */
#define IMGU_ABI_EVTTYPE_FW_WARNING			14
/* Assertion hit by FW */
#define IMGU_ABI_EVTTYPE_FW_ASSERT			15

struct imgu_abi_queue_info {
	u8 size;		/* the maximum number of elements*/
	u8 step;		/* number of bytes per element */
	u8 start;		/* index of the oldest element */
	u8 end;			/* index at which to write the new element */
};

struct imgu_abi_queues {
	/*
	 * Queues for the dynamic frame information,
	 * i.e. the "in_frame" buffer, the "out_frame"
	 * buffer and the "vf_out_frame" buffer.
	 */
	struct imgu_abi_queue_info host2sp_bufq_info
			[IMGU_ABI_MAX_SP_THREADS][IMGU_ABI_QUEUE_NUM];
	u32 host2sp_bufq[IMGU_ABI_MAX_SP_THREADS][IMGU_ABI_QUEUE_NUM]
			[IMGU_ABI_HOST2SP_BUFQ_SIZE];
	struct imgu_abi_queue_info sp2host_bufq_info[IMGU_ABI_QUEUE_NUM];
	u32 sp2host_bufq[IMGU_ABI_QUEUE_NUM][IMGU_ABI_SP2HOST_BUFQ_SIZE];

	/*
	 * The queues for the events.
	 */
	struct imgu_abi_queue_info host2sp_evtq_info;
	u32 host2sp_evtq[IMGU_ABI_HOST2SP_EVTQ_SIZE];
	struct imgu_abi_queue_info sp2host_evtq_info;
	u32 sp2host_evtq[IMGU_ABI_SP2HOST_EVTQ_SIZE];
};

/***** Buffer descriptor *****/

struct imgu_abi_metadata_info {
	struct imgu_abi_resolution resolution;	/* Resolution */
	u32 stride;				/* Stride in bytes */
	u32 size;				/* Total size in bytes */
};

struct imgu_abi_isp_3a_statistics {
	union {
		struct {
			imgu_addr_t s3a_tbl;
		} dmem;
		struct {
			imgu_addr_t s3a_tbl_hi;
			imgu_addr_t s3a_tbl_lo;
		} vmem;
	} data;
	struct {
		imgu_addr_t rgby_tbl;
	} data_hmem;
	u32 exp_id;	/* exposure id, to match statistics to a frame, */
	u32 isp_config_id;		/* Tracks per-frame configs */
	imgu_addr_t data_ptr;		/* pointer to base of all data */
	u32 size;			/* total size of all data */
	u32 dmem_size;
	u32 vmem_size;			/* both lo and hi have this size */
	u32 hmem_size;
};

struct imgu_abi_isp_dvs_statistics {
	imgu_addr_t hor_proj;
	imgu_addr_t ver_proj;
	u32 hor_size;
	u32 ver_size;
	u32 exp_id;
	imgu_addr_t data_ptr;		/* base pointer containing all memory */
	u32 size;			/* size of memory in data_ptr */
};

struct imgu_abi_metadata {
	struct imgu_abi_metadata_info info;	/* Layout info */
	imgu_addr_t address;		/* CSS virtual address */
	u32 exp_id;			/* Exposure ID */
};

struct imgu_abi_time_meas {
	u32 start_timer_value;		/* measured time in ticks */
	u32 end_timer_value;		/* measured time in ticks */
};

struct imgu_abi_buffer {
	union {
		struct imgu_abi_isp_3a_statistics s3a;
		struct imgu_abi_isp_dvs_statistics dis;
		imgu_addr_t skc_dvs_statistics;
		imgu_addr_t lace_stat;
		struct imgu_abi_metadata metadata;
		struct {
			imgu_addr_t frame_data;
			u32 flashed;
			u32 exp_id;
			u32 isp_parameters_id;   /* Tracks per-frame configs */
			u32 padded_width;
		} frame;
		imgu_addr_t ddr_ptrs;
	} payload;
	/*
	 * kernel_ptr is present for host administration purposes only.
	 * type is uint64_t in order to be 64-bit host compatible.
	 * uint64_t does not exist on SP/ISP.
	 * Size of the struct is checked by sp.hive.c.
	 */
	u64 cookie_ptr __aligned(8);
	u64 kernel_ptr;
	struct imgu_abi_time_meas timing_data;
	u32 isys_eof_clock_tick;
};

#define IMGU_ABI_NUM_CONTINUOUS_FRAMES		10
#define IMGU_ABI_SP_COMM_COMMAND		0x00

/*
 * The host2sp_cmd_ready command is the only command written by the SP
 * It acknowledges that is previous command has been received.
 * (this does not mean that the command has been executed)
 * It also indicates that a new command can be send (it is a queue
 * with depth 1).
 */
#define IMGU_ABI_SP_COMM_COMMAND_READY		1
/* Command written by the Host */
#define IMGU_ABI_SP_COMM_COMMAND_DUMMY		2	/* No action */
#define IMGU_ABI_SP_COMM_COMMAND_START_FLASH	3	/* Start the flash */
#define IMGU_ABI_SP_COMM_COMMAND_TERMINATE	4	/* Terminate */

/* n = 0..IMGU_ABI_NUM_CONTINUOUS_FRAMES-1 */
#define IMGU_ABI_SP_COMM_OFFLINE_FRAME(n)	((n) * 4 + 0x04)
#define IMGU_ABI_SP_COMM_OFFLINE_METADATA(n)	((n) * 4 + 0x2c)
#define IMGU_ABI_SP_COMM_CONT_AVAIL_RAW_FRAMES	0x54
#define IMGU_ABI_SP_COMM_CONT_EXTRA_RAW_FRAMES	0x58
#define IMGU_ABI_SP_COMM_CONT_TARGET_RAW_FRAMES	0x5c
/* n = 0..IPU3_CSS_PIPE_ID_NUM-1 */
#define IMGU_ABI_SP_COMM_EVENT_IRQ_MASK(n)	((n) * 4 + 0x60)
#define IMGU_ABI_SP_COMM_EVENT_IRQ_MASK_OR_SHIFT	0
#define IMGU_ABI_SP_COMM_EVENT_IRQ_MASK_AND_SHIFT	16

struct imgu_abi_bl_dma_cmd_entry {
	u32 src_addr;			/* virtual DDR address */
	u32 size;			/* number of bytes to transferred */
	u32 dst_type;
#define IMGU_ABI_BL_DMACMD_TYPE_SP_PMEM	1	/* sp_pmem */
	u32 dst_addr;			/* hmm address of xMEM or MMIO */
};

struct imgu_abi_sp_init_dmem_cfg {
	u32 ddr_data_addr;		/* data segment address in ddr  */
	u32 dmem_data_addr;		/* data segment address in dmem */
	u32 dmem_bss_addr;		/* bss segment address in dmem  */
	u32 data_size;			/* data segment size            */
	u32 bss_size;			/* bss segment size             */
	u32 sp_id;			/* sp id */
};

/***** For parameter computation *****/

#define IMGU_SCALER_ELEMS_PER_VEC		0x10
#define IMGU_SCALER_FILTER_TAPS_Y		0x4
#define IMGU_SCALER_OUT_BPP			0x8

#define IMGU_HIVE_OF_SYS_SCALER_TO_FA_OFFSET	0xC
#define IMGU_HIVE_OF_SYS_OF_TO_FA_OFFSET	0x8

#define IMGU_SCALER_MS_TO_OUTFORMACC_SL_ADDR	0x400
#define IMGU_SCALER_TO_OF_ACK_FA_ADDR \
	(0xC00  + IMGU_HIVE_OF_SYS_SCALER_TO_FA_OFFSET)
#define IMGU_OF_TO_ACK_FA_ADDR (0xC00 + IMGU_HIVE_OF_SYS_OF_TO_FA_OFFSET)
#define IMGU_OUTFORMACC_MS_TO_SCALER_SL_ADDR 0
#define IMGU_OSYS_PHASES			0x20
#define IMGU_OSYS_FILTER_TAPS			0x4
#define IMGU_SCALER_INTR_BPP			10

#define IMGU_PS_SNR_PRESERVE_BITS		3
#define IMGU_CNTX_BPP				11
#define IMGU_SCALER_FILTER_TAPS_UV	(IMGU_SCALER_FILTER_TAPS_Y/2)

#define IMGU_VMEM2_ELEMS_PER_VEC	(IMGU_SCALER_ELEMS_PER_VEC)
#define IMGU_STRIDE_Y			(IMGU_SCALER_FILTER_TAPS_Y + 1)
#define IMGU_MAX_FRAME_WIDTH		3840
#define IMGU_VMEM3_ELEMS_PER_VEC	(IMGU_SCALER_ELEMS_PER_VEC)

#define IMGU_VER_CNTX_WORDS		DIV_ROUND_UP((IMGU_SCALER_OUT_BPP + \
	IMGU_PS_SNR_PRESERVE_BITS), IMGU_CNTX_BPP)	/* 1 */
#define IMGU_MAX_INPUT_BLOCK_HEIGHT	64
#define IMGU_HOR_CNTX_WORDS		DIV_ROUND_UP((IMGU_SCALER_INTR_BPP + \
	IMGU_PS_SNR_PRESERVE_BITS), IMGU_CNTX_BPP)	/* 2 */
#define IMGU_MAX_OUTPUT_BLOCK_WIDTH		128
#define IMGU_CNTX_STRIDE_UV		(IMGU_SCALER_FILTER_TAPS_UV + 1)

#define IMGU_OSYS_PHASE_COUNTER_PREC_REF	6
#define IMGU_VMEM1_Y_SIZE \
	(IMGU_OSYS_BLOCK_HEIGHT * IMGU_VMEM1_Y_STRIDE)
#define IMGU_VMEM1_UV_SIZE			(IMGU_VMEM1_Y_SIZE / 4)
#define IMGU_VMEM1_OUT_BUF_ADDR			(IMGU_VMEM1_INP_BUF_ADDR + \
	(IMGU_OSYS_NUM_INPUT_BUFFERS * IMGU_VMEM1_BUF_SIZE))
#define IMGU_OSYS_NUM_OUTPUT_BUFFERS		2

/* transpose of input height */
#define IMGU_VMEM2_VECS_PER_LINE \
	(DIV_ROUND_UP(IMGU_OSYS_BLOCK_HEIGHT, IMGU_VMEM2_ELEMS_PER_VEC))
/* size in words (vectors)  */
#define IMGU_VMEM2_BUF_SIZE \
	(IMGU_VMEM2_VECS_PER_LINE * IMGU_VMEM2_LINES_PER_BLOCK)
#define IMGU_VMEM3_VER_Y_SIZE	\
			((IMGU_STRIDE_Y * IMGU_MAX_FRAME_WIDTH \
			 / IMGU_VMEM3_ELEMS_PER_VEC) * IMGU_VER_CNTX_WORDS)
#define IMGU_VMEM3_HOR_Y_SIZE \
	((IMGU_STRIDE_Y * IMGU_MAX_INPUT_BLOCK_HEIGHT \
	 / IMGU_VMEM3_ELEMS_PER_VEC) * IMGU_HOR_CNTX_WORDS)
#define IMGU_VMEM3_VER_Y_EXTRA \
	((IMGU_STRIDE_Y * IMGU_MAX_OUTPUT_BLOCK_WIDTH \
	 / IMGU_VMEM3_ELEMS_PER_VEC) * IMGU_VER_CNTX_WORDS)
#define IMGU_VMEM3_VER_U_SIZE \
	(((IMGU_CNTX_STRIDE_UV * IMGU_MAX_FRAME_WIDTH \
	 / IMGU_VMEM3_ELEMS_PER_VEC) * IMGU_VER_CNTX_WORDS) / 2)
#define IMGU_VMEM3_HOR_U_SIZE \
	(((IMGU_STRIDE_Y * IMGU_MAX_INPUT_BLOCK_HEIGHT \
	 / IMGU_VMEM3_ELEMS_PER_VEC) * IMGU_HOR_CNTX_WORDS) / 2)
#define IMGU_VMEM3_VER_U_EXTRA \
	(((IMGU_CNTX_STRIDE_UV * IMGU_MAX_OUTPUT_BLOCK_WIDTH \
	 / IMGU_VMEM3_ELEMS_PER_VEC) * IMGU_VER_CNTX_WORDS) / 2)
#define IMGU_VMEM3_VER_V_SIZE \
	(((IMGU_CNTX_STRIDE_UV * IMGU_MAX_FRAME_WIDTH \
	 / IMGU_VMEM3_ELEMS_PER_VEC) * IMGU_VER_CNTX_WORDS) / 2)

#define IMGU_OSYS_DMA_CROP_W_LIMIT	64
#define IMGU_OSYS_DMA_CROP_H_LIMIT	4

#define IMGU_ISP_VEC_NELEMS		64
#define IMGU_LUMA_TO_CHROMA_RATIO	2
#define IMGU_OSYS_FIR_PHASES \
	(IMGU_OSYS_PHASES << IMGU_OSYS_PHASE_COUNTER_PREC_REF)
#define IMGU_OSYS_TAPS_UV		(IMGU_OSYS_FILTER_TAPS / 2)
#define IMGU_INPUT_BLOCK_WIDTH			(128)
#define IMGU_OSYS_TAPS_Y		(IMGU_OSYS_FILTER_TAPS)
#define IMGU_FIFO_ADDR_SCALER_TO_FMT \
	(IMGU_SCALER_MS_TO_OUTFORMACC_SL_ADDR >> 2)
#define IMGU_FIFO_ADDR_SCALER_TO_SP	(IMGU_SCALER_TO_OF_ACK_FA_ADDR >> 2)
#define IMGU_VMEM1_INP_BUF_ADDR		0
#define IMGU_VMEM1_Y_STRIDE \
	(IMGU_OSYS_BLOCK_WIDTH / IMGU_VMEM1_ELEMS_PER_VEC)
#define IMGU_VMEM1_BUF_SIZE	(IMGU_VMEM1_V_OFFSET + IMGU_VMEM1_UV_SIZE)

#define IMGU_VMEM1_U_OFFSET		(IMGU_VMEM1_Y_SIZE)
#define IMGU_VMEM1_V_OFFSET	(IMGU_VMEM1_U_OFFSET + IMGU_VMEM1_UV_SIZE)
#define IMGU_VMEM1_UV_STRIDE		(IMGU_VMEM1_Y_STRIDE / 2)
#define IMGU_OSYS_NUM_INPUT_BUFFERS	2
#define IMGU_VMEM1_INT_BUF_ADDR		(IMGU_VMEM1_OUT_BUF_ADDR + \
	(IMGU_OSYS_NUM_OUTPUT_BUFFERS * IMGU_VMEM1_BUF_SIZE))

#define IMGU_VMEM1_ELEMS_PER_VEC	(IMGU_HIVE_OF_SYS_OF_SYSTEM_NWAYS)
#define IMGU_OSYS_NUM_INTERM_BUFFERS	2
#define IMGU_VMEM2_BUF_Y_ADDR		0
#define IMGU_VMEM2_BUF_Y_STRIDE		(IMGU_VMEM2_VECS_PER_LINE)
#define IMGU_VMEM2_BUF_U_ADDR \
	(IMGU_VMEM2_BUF_Y_ADDR + IMGU_VMEM2_BUF_SIZE)
#define IMGU_VMEM2_BUF_V_ADDR \
	(IMGU_VMEM2_BUF_U_ADDR + IMGU_VMEM2_BUF_SIZE/4)
#define IMGU_VMEM2_BUF_UV_STRIDE	(IMGU_VMEM2_VECS_PER_LINE/2)
/* 1.5 x depth of intermediate buffer */
#define IMGU_VMEM2_LINES_PER_BLOCK	192
#define IMGU_VMEM3_HOR_Y_ADDR \
	(IMGU_VMEM3_VER_Y_ADDR + IMGU_VMEM3_VER_Y_SIZE)
#define IMGU_VMEM3_HOR_U_ADDR \
	(IMGU_VMEM3_VER_U_ADDR + IMGU_VMEM3_VER_U_SIZE)
#define IMGU_VMEM3_HOR_V_ADDR \
	(IMGU_VMEM3_VER_V_ADDR + IMGU_VMEM3_VER_V_SIZE)
#define IMGU_VMEM3_VER_Y_ADDR		0
#define IMGU_VMEM3_VER_U_ADDR \
	(IMGU_VMEM3_VER_Y_ADDR + IMGU_VMEM3_VER_Y_SIZE + \
	max(IMGU_VMEM3_HOR_Y_SIZE, IMGU_VMEM3_VER_Y_EXTRA))
#define IMGU_VMEM3_VER_V_ADDR \
	(IMGU_VMEM3_VER_U_ADDR + IMGU_VMEM3_VER_U_SIZE + \
	max(IMGU_VMEM3_HOR_U_SIZE, IMGU_VMEM3_VER_U_EXTRA))
#define IMGU_HIVE_OF_SYS_OF_SYSTEM_NWAYS	32
#define IMGU_FIFO_ADDR_FMT_TO_SP	(IMGU_OF_TO_ACK_FA_ADDR >> 2)
#define IMGU_FIFO_ADDR_FMT_TO_SCALER (IMGU_OUTFORMACC_MS_TO_SCALER_SL_ADDR >> 2)
#define IMGU_VMEM1_HST_BUF_ADDR		(IMGU_VMEM1_INT_BUF_ADDR + \
	(IMGU_OSYS_NUM_INTERM_BUFFERS * IMGU_VMEM1_BUF_SIZE))
#define IMGU_VMEM1_HST_BUF_STRIDE	120
#define IMGU_VMEM1_HST_BUF_NLINES	3

#endif
