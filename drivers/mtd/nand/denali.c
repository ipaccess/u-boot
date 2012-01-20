/*
 * NAND Flash Controller Device Driver
 * Copyright © 2009-2010, Intel Corporation and its suppliers.
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms and conditions of the GNU General Public License,
 * version 2, as published by the Free Software Foundation.
 *
 * This program is distributed in the hope it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin St - Fifth Floor, Boston, MA 02110-1301 USA.
 *
 */

/* Includes ---------------------------------------------------------------- */
#include <common.h>
#include <errno.h>
#include <ubi_uboot.h>
#include <asm/io.h>
#include <linux/compiler.h>
#include <asm/processor.h>
#include <asm/arch/picoxcell.h>

#include "denali.h"

MODULE_LICENSE ("GPL");

/* Macros ------------------------------------------------------------------ */

/*!
 * \brief Debug macro
 */
#ifdef	DENALI_DEBUG
#define nand_dbg(fmt,args...)	printf (fmt ,##args)
#else
#define nand_dbg(fmt,args...)
#endif /* DENALI_DEBUG */

#define false 0
#define true 1

/* Define the io read and write functions used in here */
#define ioread32 readl
#define iowrite32 writel

/* We define a module parameter that allows the user to override
 * the hardware and decide what timing mode should be used.
 */
#define NAND_DEFAULT_TIMINGS	-1

static int onfi_timing_mode = NAND_DEFAULT_TIMINGS;

#define DENALI_NAND_NAME    "denali-nand"

/* We define a macro here that combines all interrupts this driver uses into
 * a single constant value, for convenience. */
#define DENALI_IRQ_ALL	(INTR_STATUS__DMA_CMD_COMP | \
			INTR_STATUS__ECC_TRANSACTION_DONE | \
			INTR_STATUS__ECC_ERR | \
			INTR_STATUS__PROGRAM_FAIL | \
			INTR_STATUS__LOAD_COMP | \
			INTR_STATUS__PROGRAM_COMP | \
			INTR_STATUS__TIME_OUT | \
			INTR_STATUS__ERASE_FAIL | \
			INTR_STATUS__RST_COMP | \
			INTR_STATUS__ERASE_COMP | \
			INTR_STATUS__ECC_UNCOR_ERR)

/* indicates whether or not the internal value for the flash bank is
 * valid or not */
#define CHIP_SELECT_INVALID	-1

/* This macro divides two integers and rounds fractional values up
 * to the nearest integer value. */
#define CEIL_DIV(X, Y) (((X)%(Y)) ? ((X)/(Y)+1) : ((X)/(Y)))

/* These constants are defined by the driver to enable common driver
 * configuration options. */
#define SPARE_ACCESS		0x41
#define MAIN_ACCESS		0x42
#define MAIN_SPARE_ACCESS	0x43

#define DENALI_READ	0
#define DENALI_WRITE	0x100

/* types of device accesses. We can issue commands and get status */
#define COMMAND_CYCLE	0
#define ADDR_CYCLE	1
#define STATUS_CYCLE	2

/* this is a helper macro that allows us to
 * format the bank into the proper bits for the controller */
#define BANK(x) ((x) << 24)

#define NAND_TIMEOUT    (1000)

/* DMA operation */
#define DMA_INT_GENERATION  ( 1 << 24 )
#define DMA_BURST_LENGTH    ( 8 << 16 )

/* Prototypes--------------------------------------------------------------- */
/* forward declarations */
static void clear_interrupts (struct denali_nand_info *denali);
static uint32_t wait_for_irq (struct denali_nand_info *denali,
			      uint32_t irq_mask);
static void denali_irq_enable (struct denali_nand_info *denali,
			       uint32_t int_mask);
static uint32_t read_interrupt_status (struct denali_nand_info *denali);

/* Functions --------------------------------------------------------------- */

/* Certain operations for the denali NAND controller use
 * an indexed mode to read/write data. The operation is
 * performed by writing the address value of the command
 * to the device memory followed by the data. This function
 * abstracts this common operation.
*/
static void index_addr (struct denali_nand_info *denali,
			uint32_t address, uint32_t data)
{
	iowrite32 (address, denali->flash_mem);
	iowrite32 (data, denali->flash_mem + 0x10);
}

/* Perform an indexed read of the device */
static void index_addr_read_data (struct denali_nand_info *denali,
				  uint32_t address, uint32_t * pdata)
{
	iowrite32 (address, denali->flash_mem);
	*pdata = ioread32 (denali->flash_mem + 0x10);
}

/* We need to buffer some data for some of the NAND core routines.
 * The operations manage buffering that data. */
static void reset_buf (struct denali_nand_info *denali)
{
	nand_dbg ("reset_buf()\n");

        denali->buf.head = denali->buf.tail = 0;
}

static void write_byte_to_buf (struct denali_nand_info *denali, uint8_t byte)
{
	BUG_ON (denali->buf.tail >= sizeof (denali->buf.buf));
	denali->buf.buf[denali->buf.tail++] = byte;
}

/* reads the status of the device */
static void read_status (struct denali_nand_info *denali)
{
	uint32_t cmd = 0x0;

	nand_dbg ("read_status()\n");

	/* initialize the data buffer to store status */
	reset_buf (denali);

	cmd = ioread32 (denali->flash_reg + WRITE_PROTECT);
	if (cmd)
		write_byte_to_buf (denali, NAND_STATUS_WP);
	else
		write_byte_to_buf (denali, 0);
}

/* resets a specific device connected to the core */
static void reset_bank (struct denali_nand_info *denali)
{
	uint32_t irq_status = 0;
	uint32_t irq_mask = INTR_STATUS__RST_COMP | INTR_STATUS__TIME_OUT;

	nand_dbg ("reset_bank()\n");

        clear_interrupts (denali);

	iowrite32 (1 << denali->flash_bank, denali->flash_reg + DEVICE_RESET);

	irq_status = wait_for_irq (denali, irq_mask);

	if (irq_status & INTR_STATUS__TIME_OUT)
		printf ("reset bank (#%d) failed.\n", denali->flash_bank);
}

/* Reset the flash controller */
static uint16_t denali_nand_reset (struct denali_nand_info *denali)
{
	uint32_t i;

	nand_dbg ("denali_nand_reset()\n");

	for (i = 0; i < denali->max_banks; i++)
		iowrite32 (INTR_STATUS__RST_COMP | INTR_STATUS__TIME_OUT,
			   denali->flash_reg + INTR_STATUS (i));

	for (i = 0; i < denali->max_banks; i++) {
		iowrite32 (1 << i, denali->flash_reg + DEVICE_RESET);
		while (!(ioread32 (denali->flash_reg +
				   INTR_STATUS (i)) &
			 (INTR_STATUS__RST_COMP | INTR_STATUS__TIME_OUT)))
			cpu_relax ();
		if (ioread32 (denali->flash_reg + INTR_STATUS (i)) &
		    INTR_STATUS__TIME_OUT)
			nand_dbg ("NAND Reset operation timed out on bank %d\n",
				  i);
	}

	for (i = 0; i < denali->max_banks; i++)
		iowrite32 (INTR_STATUS__RST_COMP | INTR_STATUS__TIME_OUT,
			   denali->flash_reg + INTR_STATUS (i));

	return PASS;
}

/* this routine calculates the ONFI timing values for a given mode and
 * programs the clocking register accordingly. The mode is determined by
 * the get_onfi_nand_para routine.
 */
static void nand_onfi_timing_set (struct denali_nand_info *denali,
				  uint16_t mode)
{
	uint16_t Trea[6] = { 40, 30, 25, 20, 20, 16 };
	uint16_t Trp[6] = { 50, 25, 17, 15, 12, 10 };
	uint16_t Treh[6] = { 30, 15, 15, 10, 10, 7 };
	uint16_t Trc[6] = { 100, 50, 35, 30, 25, 20 };
	uint16_t Trhoh[6] = { 0, 15, 15, 15, 15, 15 };
	uint16_t Trloh[6] = { 0, 0, 0, 0, 5, 5 };
	uint16_t Tcea[6] = { 100, 45, 30, 25, 25, 25 };
	uint16_t Tadl[6] = { 200, 100, 100, 100, 70, 70 };
	uint16_t Trhw[6] = { 200, 100, 100, 100, 100, 100 };
	uint16_t Trhz[6] = { 200, 100, 100, 100, 100, 100 };
	uint16_t Twhr[6] = { 120, 80, 80, 60, 60, 60 };
	uint16_t Tcs[6] = { 70, 35, 25, 25, 20, 15 };

	uint16_t TclsRising = 1;
	uint16_t data_invalid_rhoh, data_invalid_rloh, data_invalid;
	uint16_t dv_window = 0;
	uint16_t en_lo, en_hi;
	uint16_t acc_clks;
	uint16_t addr_2_data, re_2_we, re_2_re, we_2_re, cs_cnt;

	nand_dbg ("nand_onfi_timing_set()\n");

	en_lo = CEIL_DIV (Trp[mode], CLK_X);
	en_hi = CEIL_DIV (Treh[mode], CLK_X);
#if ONFI_BLOOM_TIME
	if ((en_hi * CLK_X) < (Treh[mode] + 2))
		en_hi++;
#endif

	if ((en_lo + en_hi) * CLK_X < Trc[mode])
		en_lo +=
		    CEIL_DIV ((Trc[mode] - (en_lo + en_hi) * CLK_X), CLK_X);

	if ((en_lo + en_hi) < CLK_MULTI)
		en_lo += CLK_MULTI - en_lo - en_hi;

	while (dv_window < 8) {
		data_invalid_rhoh = en_lo * CLK_X + Trhoh[mode];

		data_invalid_rloh = (en_lo + en_hi) * CLK_X + Trloh[mode];

		data_invalid =
		    data_invalid_rhoh <
		    data_invalid_rloh ? data_invalid_rhoh : data_invalid_rloh;

		dv_window = data_invalid - Trea[mode];

		if (dv_window < 8)
			en_lo++;
	}

	acc_clks = CEIL_DIV (Trea[mode], CLK_X);

	while (((acc_clks * CLK_X) - Trea[mode]) < 3)
		acc_clks++;

	if ((data_invalid - acc_clks * CLK_X) < 2)
		nand_dbg ("%s, Line %d: Warning!\n", __FILE__, __LINE__);

	addr_2_data = CEIL_DIV (Tadl[mode], CLK_X);
	re_2_we = CEIL_DIV (Trhw[mode], CLK_X);
	re_2_re = CEIL_DIV (Trhz[mode], CLK_X);
	we_2_re = CEIL_DIV (Twhr[mode], CLK_X);
	cs_cnt = CEIL_DIV ((Tcs[mode] - Trp[mode]), CLK_X);
	if (!TclsRising)
		cs_cnt = CEIL_DIV (Tcs[mode], CLK_X);
	if (cs_cnt == 0)
		cs_cnt = 1;

	if (Tcea[mode]) {
		while (((cs_cnt * CLK_X) + Trea[mode]) < Tcea[mode])
			cs_cnt++;
	}
#if MODE5_WORKAROUND
	if (mode == 5)
		acc_clks = 5;
#endif

	/* Sighting 3462430: Temporary hack for MT29F128G08CJABAWP:B */
	if ((ioread32 (denali->flash_reg + MANUFACTURER_ID) == 0) &&
	    (ioread32 (denali->flash_reg + DEVICE_ID) == 0x88))
		acc_clks = 6;

	iowrite32 (acc_clks, denali->flash_reg + ACC_CLKS);
	iowrite32 (re_2_we, denali->flash_reg + RE_2_WE);
	iowrite32 (re_2_re, denali->flash_reg + RE_2_RE);
	iowrite32 (we_2_re, denali->flash_reg + WE_2_RE);
	iowrite32 (addr_2_data, denali->flash_reg + ADDR_2_DATA);
	iowrite32 (en_lo, denali->flash_reg + RDWR_EN_LO_CNT);
	iowrite32 (en_hi, denali->flash_reg + RDWR_EN_HI_CNT);
	iowrite32 (cs_cnt, denali->flash_reg + CS_SETUP_CNT);
}

/* queries the NAND device to see what ONFI modes it supports. */
static uint16_t get_onfi_nand_para (struct denali_nand_info *denali)
{
	int i;
	/* we needn't to do a reset here because driver has already
	 * reset all the banks before
	 * */
	if (!(ioread32 (denali->flash_reg + ONFI_TIMING_MODE) &
	      ONFI_TIMING_MODE__VALUE))
		return FAIL;

	for (i = 5; i > 0; i--) {
		if (ioread32 (denali->flash_reg + ONFI_TIMING_MODE) &
		    (0x01 << i))
			break;
	}

	nand_onfi_timing_set (denali, i);

	/* By now, all the ONFI devices we know support the page cache */
	/* rw feature. So here we enable the pipeline_rw_ahead feature */
	/* iowrite32(1, denali->flash_reg + CACHE_WRITE_ENABLE); */
	/* iowrite32(1, denali->flash_reg + CACHE_READ_ENABLE);  */

	return PASS;
}

static void get_samsung_nand_para (struct denali_nand_info *denali,
				   uint8_t device_id)
{
	if (device_id == 0xd3) {	/* Samsung K9WAG08U1A */
		/* Set timing register values according to datasheet */
		iowrite32 (5, denali->flash_reg + ACC_CLKS);
		iowrite32 (20, denali->flash_reg + RE_2_WE);
		iowrite32 (12, denali->flash_reg + WE_2_RE);
		iowrite32 (14, denali->flash_reg + ADDR_2_DATA);
		iowrite32 (3, denali->flash_reg + RDWR_EN_LO_CNT);
		iowrite32 (2, denali->flash_reg + RDWR_EN_HI_CNT);
		iowrite32 (2, denali->flash_reg + CS_SETUP_CNT);
	}
}

static void get_toshiba_nand_para (struct denali_nand_info *denali)
{
	uint32_t tmp;

	/* Workaround to fix a controller bug which reports a wrong */
	/* spare area size for some kind of Toshiba NAND device */
	if ((ioread32 (denali->flash_reg + DEVICE_MAIN_AREA_SIZE) == 4096) &&
	    (ioread32 (denali->flash_reg + DEVICE_SPARE_AREA_SIZE) == 64)) {
		iowrite32 (216, denali->flash_reg + DEVICE_SPARE_AREA_SIZE);
		tmp = ioread32 (denali->flash_reg + DEVICES_CONNECTED) *
		    ioread32 (denali->flash_reg + DEVICE_SPARE_AREA_SIZE);
		iowrite32 (tmp, denali->flash_reg + LOGICAL_PAGE_SPARE_SIZE);
		iowrite32 (denali->nr_ecc_bits,
			   denali->flash_reg + ECC_CORRECTION);
	}
}

static void get_hynix_nand_para (struct denali_nand_info *denali,
				 uint8_t device_id)
{
	uint32_t main_size, spare_size;

	switch (device_id) {
	case 0xD5:		/* Hynix H27UAG8T2A, H27UBG8U5A or H27UCG8VFA */
	case 0xD7:		/* Hynix H27UDG8VEM, H27UCG8UDM or H27UCG8V5A */
		iowrite32 (128, denali->flash_reg + PAGES_PER_BLOCK);
		iowrite32 (4096, denali->flash_reg + DEVICE_MAIN_AREA_SIZE);
		iowrite32 (224, denali->flash_reg + DEVICE_SPARE_AREA_SIZE);
		main_size = 4096 *
		    ioread32 (denali->flash_reg + DEVICES_CONNECTED);
		spare_size = 224 *
		    ioread32 (denali->flash_reg + DEVICES_CONNECTED);
		iowrite32 (main_size,
			   denali->flash_reg + LOGICAL_PAGE_DATA_SIZE);
		iowrite32 (spare_size,
			   denali->flash_reg + LOGICAL_PAGE_SPARE_SIZE);
		iowrite32 (0, denali->flash_reg + DEVICE_WIDTH);
		iowrite32 (denali->nr_ecc_bits,
			   denali->flash_reg + ECC_CORRECTION);
		break;
	default:
		nand_dbg ("unknown Hynix NAND (Device ID: 0x%x). Will use "
			  "default parameter values instead.\n", device_id);
	}
}

/* determines how many NAND chips are connected to the controller. Note for
 * Intel CE4100 devices we don't support more than one device.
 */
static void find_valid_banks (struct denali_nand_info *denali)
{
	uint32_t id[denali->max_banks];
	int i;

	denali->total_used_banks = 1;
	for (i = 0; i < denali->max_banks; i++) {
		index_addr (denali, (uint32_t) (MODE_11 | (i << 24) | 0), 0x90);
		index_addr (denali, (uint32_t) (MODE_11 | (i << 24) | 1), 0);
		index_addr_read_data (denali,
				      (uint32_t) (MODE_11 | (i << 24) | 2),
				      &id[i]);

		nand_dbg ("Return 1st ID for bank[%d]: %x\n", i, id[i]);

		if (i == 0) {
			if (!(id[i] & 0x0ff))
				break;	/* WTF? */
		} else {
			if ((id[i] & 0x0ff) == (id[0] & 0x0ff))
				denali->total_used_banks++;
			else
				break;
		}
	}

	if (denali->platform == INTEL_CE4100) {
		/* Platform limitations of the CE4100 device limit
		 * users to a single chip solution for NAND.
		 * Multichip support is not enabled.
		 */
		if (denali->total_used_banks != 1) {
			printf ("Sorry, Intel CE4100 only supports a "
				"single NAND device.\n");
			BUG ();
		}
	}
	nand_dbg ("denali->total_used_banks: %d\n", denali->total_used_banks);
}

/*
 * Use the configuration feature register to determine the maximum number of
 * banks that the hardware supports.
 */
static void detect_max_banks (struct denali_nand_info *denali)
{
	uint32_t features = ioread32 (denali->flash_reg + FEATURES);

	denali->max_banks = 2 << (features & FEATURES__N_BANKS);
}

static void detect_partition_feature (struct denali_nand_info *denali)
{
	/* For MRST platform, denali->fwblks represent the
	 * number of blocks firmware is taken,
	 * FW is in protect partition and MTD driver has no
	 * permission to access it. So let driver know how many
	 * blocks it can't touch.
	 * */
	if (ioread32 (denali->flash_reg + FEATURES) & FEATURES__PARTITION) {
		if ((ioread32 (denali->flash_reg + PERM_SRC_ID (1)) &
		     PERM_SRC_ID__SRCID) == SPECTRA_PARTITION_ID) {
			denali->fwblks =
			    ((ioread32 (denali->flash_reg + MIN_MAX_BANK (1)) &
			      MIN_MAX_BANK__MIN_VALUE) * denali->blksperchip)
			    +
			    (ioread32 (denali->flash_reg + MIN_BLK_ADDR (1)) &
			     MIN_BLK_ADDR__VALUE);
		} else
			denali->fwblks = SPECTRA_START_BLOCK;
	} else
		denali->fwblks = SPECTRA_START_BLOCK;
}

static uint16_t denali_nand_timing_set (struct denali_nand_info *denali)
{
	uint16_t status = PASS;
	uint32_t id_bytes[5], addr;
	uint8_t i, maf_id, device_id;

	nand_dbg ("denali_nand_timing_set()\n");

        /* Use read id method to get device ID and other
	 * params. For some NAND chips, controller can't
	 * report the correct device ID by reading from
	 * DEVICE_ID register
	 * */
	addr = (uint32_t) MODE_11 | BANK (denali->flash_bank);
	index_addr (denali, (uint32_t) addr | 0, 0x90);
	index_addr (denali, (uint32_t) addr | 1, 0);
	for (i = 0; i < 5; i++)
		index_addr_read_data (denali, addr | 2, &id_bytes[i]);
	maf_id = id_bytes[0];
	device_id = id_bytes[1];

	if (ioread32 (denali->flash_reg + ONFI_DEVICE_NO_OF_LUNS) & ONFI_DEVICE_NO_OF_LUNS__ONFI_DEVICE) {	/* ONFI 1.0 NAND */
		if (FAIL == get_onfi_nand_para (denali))
			return FAIL;
	} else if (maf_id == 0xEC) {	/* Samsung NAND */
		get_samsung_nand_para (denali, device_id);
	} else if (maf_id == 0x98) {	/* Toshiba NAND */
		get_toshiba_nand_para (denali);
	} else if (maf_id == 0xAD) {	/* Hynix NAND */
		get_hynix_nand_para (denali, device_id);
	}

	nand_dbg ("Dump timing register values: acc_clks: %d, re_2_we: "
		  "%d, re_2_re: %d\n"
		  "we_2_re: %d, addr_2_data: %d, rdwr_en_lo_cnt: %d\n"
		  "rdwr_en_hi_cnt: %d, cs_setup_cnt: %d\n",
		  ioread32 (denali->flash_reg + ACC_CLKS),
		  ioread32 (denali->flash_reg + RE_2_WE),
		  ioread32 (denali->flash_reg + RE_2_RE),
		  ioread32 (denali->flash_reg + WE_2_RE),
		  ioread32 (denali->flash_reg + ADDR_2_DATA),
		  ioread32 (denali->flash_reg + RDWR_EN_LO_CNT),
		  ioread32 (denali->flash_reg + RDWR_EN_HI_CNT),
		  ioread32 (denali->flash_reg + CS_SETUP_CNT));

	find_valid_banks (denali);

	detect_partition_feature (denali);

	/* If the user specified to override the defaulFt timings
	 * with a specific ONFI mode, we apply those changes here.
	 */
	if (onfi_timing_mode != NAND_DEFAULT_TIMINGS)
		nand_onfi_timing_set (denali, onfi_timing_mode);

	return status;
}

static void denali_set_intr_modes (struct denali_nand_info *denali,
				   uint16_t INT_ENABLE)
{
	nand_dbg ("denali_set_intr_modes()\n");

	if (INT_ENABLE)
		iowrite32 (1, denali->flash_reg + GLOBAL_INT_ENABLE);
	else
		iowrite32 (0, denali->flash_reg + GLOBAL_INT_ENABLE);
}

static void denali_irq_init (struct denali_nand_info *denali)
{
	uint32_t int_mask = 0;
	int i;

	/* Disable global interrupts */
	denali_set_intr_modes (denali, false);

	int_mask = DENALI_IRQ_ALL;

	/* Clear all status bits */
	for (i = 0; i < denali->max_banks; ++i)
		iowrite32 (0xFFFF, denali->flash_reg + INTR_STATUS (i));

	denali_irq_enable (denali, int_mask);
}

static void denali_irq_enable (struct denali_nand_info *denali,
			       uint32_t int_mask)
{
	int i;

	for (i = 0; i < denali->max_banks; ++i)
		iowrite32 (int_mask, denali->flash_reg + INTR_EN (i));
}

/* This function only returns when an interrupt that this driver cares about
 * occurs. This is to reduce the overhead of servicing interrupts
 */
static inline uint32_t denali_irq_detected (struct denali_nand_info *denali)
{
	return read_interrupt_status (denali) & DENALI_IRQ_ALL;
}

/* Interrupts are cleared by writing a 1 to the appropriate status bit */
static inline void clear_interrupt (struct denali_nand_info *denali,
				    uint32_t irq_mask)
{
	uint32_t intr_status_reg = 0;

	intr_status_reg = INTR_STATUS (denali->flash_bank);

	iowrite32 (irq_mask, denali->flash_reg + intr_status_reg);
}

static void clear_interrupts (struct denali_nand_info *denali)
{
	uint32_t status = 0x0;

	status = read_interrupt_status (denali);
	clear_interrupt (denali, status);

	denali->irq_status = 0x0;
}

static uint32_t read_interrupt_status (struct denali_nand_info *denali)
{
	uint32_t intr_status_reg = 0;

	intr_status_reg = INTR_STATUS (denali->flash_bank);

	return ioread32 (denali->flash_reg + intr_status_reg);
}

static uint32_t wait_for_irq (struct denali_nand_info *denali,
			      uint32_t irq_mask)
{
	uint32_t intr_status = 0;
	unsigned int timebase = 0;

	timebase = get_timer (0);
	do {
		intr_status = denali_irq_detected (denali);

		if (intr_status & irq_mask) {
			denali->irq_status &= ~irq_mask;
			/* our interrupt was detected */
			break;
		}
	} while (get_timer (timebase) < NAND_TIMEOUT);

	return intr_status;
}

/* This helper function setups the registers for ECC and whether or not
 * the spare area will be transferred. */
static void setup_ecc_for_xfer (struct denali_nand_info *denali, bool ecc_en,
				bool transfer_spare)
{
	int ecc_en_flag = 0, transfer_spare_flag = 0;

	/* set ECC, transfer spare bits if needed */
	ecc_en_flag = ecc_en ? ECC_ENABLE__FLAG : 0;
	transfer_spare_flag = transfer_spare ? TRANSFER_SPARE_REG__FLAG : 0;

	/* Enable spare area/ECC per user's request. */
	iowrite32 (ecc_en_flag, denali->flash_reg + ECC_ENABLE);
	iowrite32 (transfer_spare_flag, denali->flash_reg + TRANSFER_SPARE_REG);
}

/* sends a pipeline command operation to the controller. See the Denali NAND
 * controller's user guide for more information (section 4.2.3.6).
 */
static int denali_send_pipeline_cmd (struct denali_nand_info *denali,
				     bool ecc_en,
				     bool transfer_spare,
				     int access_type, int op)
{
	int status = PASS;
	uint32_t addr = 0x0, cmd = 0x0, page_count = 1;
        uint32_t irq_status = 0, irq_mask = 0;

	if (op == DENALI_READ)
		irq_mask = INTR_STATUS__LOAD_COMP;
	else if (op == DENALI_WRITE)
		irq_mask = 0;
	else
		BUG ();

	setup_ecc_for_xfer (denali, ecc_en, transfer_spare);

	/* clear interrupts */
	clear_interrupts (denali);

	addr = BANK (denali->flash_bank) | denali->page;

	if (op == DENALI_WRITE && access_type != SPARE_ACCESS) {
		cmd = MODE_01 | addr;
		iowrite32 (cmd, denali->flash_mem);
	} else if (op == DENALI_WRITE && access_type == SPARE_ACCESS) {
		/* read spare area */
		cmd = MODE_10 | addr;
		index_addr (denali, (uint32_t) cmd, access_type);

		cmd = MODE_01 | addr;
		iowrite32 (cmd, denali->flash_mem);
	} else if (op == DENALI_READ) {
		/* setup page read request for access type */
		cmd = MODE_10 | addr;
		index_addr (denali, (uint32_t) cmd, access_type);

		/* page 33 of the NAND controller spec indicates we should not
		   use the pipeline commands in Spare area only mode. So we
		   don't.
		 */
		if (access_type == SPARE_ACCESS) {
			cmd = MODE_01 | addr;
			iowrite32 (cmd, denali->flash_mem);
		} else {
			index_addr (denali, (uint32_t) cmd,
				    0x2000 | op | page_count);

			/* wait for command to be accepted
			 * can always use status0 bit as the
			 * mask is identical for each
			 * bank. */
			irq_status = wait_for_irq (denali, irq_mask);

			if (irq_status == 0) {
				printf ("cmd, page, addr on timeout "
					"(0x%x, 0x%x, 0x%x)\n",
					cmd, denali->page, addr);
				status = FAIL;
			} else {
				cmd = MODE_01 | addr;
				iowrite32 (cmd, denali->flash_mem);
			}
		}
	}
	return status;
}

/* helper function that simply writes a buffer to the flash */
static int write_data_to_flash_mem (struct denali_nand_info *denali,
				    const uint8_t * buf, int len)
{
	uint32_t i = 0, *buf32;

	/* verify that the len is a multiple of 4. see comment in
	 * read_data_from_flash_mem() */
	BUG_ON ((len % 4) != 0);

	/* write the data to the flash memory */
	buf32 = (uint32_t *) buf;
	for (i = 0; i < len / 4; i++)
		iowrite32 (*buf32++, denali->flash_mem + 0x10);
	return i * 4;		/* intent is to return the number of bytes read */
}

/* helper function that simply reads a buffer from the flash */
static int read_data_from_flash_mem (struct denali_nand_info *denali,
				     uint8_t * buf, int len)
{
	uint32_t i = 0, *buf32;

	/* we assume that len will be a multiple of 4, if not
	 * it would be nice to know about it ASAP rather than
	 * have random failures...
	 * This assumption is based on the fact that this
	 * function is designed to be used to read flash pages,
	 * which are typically multiples of 4...
	 */

	BUG_ON ((len % 4) != 0);

	/* transfer the data from the flash */
	buf32 = (uint32_t *) buf;
	for (i = 0; i < len / 4; i++)
		*buf32++ = ioread32 (denali->flash_mem + 0x10);
	return i * 4;		/* intent is to return the number of bytes read */
}

/* writes OOB data to the device */
static int write_oob_data (struct mtd_info *mtd, uint8_t * buf, int page)
{
	struct nand_chip *nand = mtd->priv;
	struct denali_nand_info *denali = nand->priv;

	uint32_t irq_status = 0;
	uint32_t irq_mask = INTR_STATUS__PROGRAM_COMP |
	    INTR_STATUS__PROGRAM_FAIL;
	int status = 0;

	nand_dbg ("write_oob_data()\n");

        denali->page = page;

	if (denali_send_pipeline_cmd (denali, false, false, SPARE_ACCESS,
				      DENALI_WRITE) == PASS) {
		write_data_to_flash_mem (denali, buf, mtd->oobsize);

		/* wait for operation to complete */
		irq_status = wait_for_irq (denali, irq_mask);

		if (irq_status == 0) {
			printf ("OOB write failed.\n");
			status = -EIO;
		}
	} else {
		printf ("unable to send pipeline command.\n");
		status = -EIO;
	}
	return status;
}

/* reads OOB data from the device */
static void read_oob_data (struct mtd_info *mtd, uint8_t * buf, int page)
{
	struct nand_chip *nand = mtd->priv;
	struct denali_nand_info *denali = nand->priv;

	uint32_t irq_mask = INTR_STATUS__LOAD_COMP,
	    irq_status = 0, addr = 0x0, cmd = 0x0;

	nand_dbg ("read_oob_data()\n");

	denali->page = page;

	if (denali_send_pipeline_cmd (denali, false, true, SPARE_ACCESS,
				      DENALI_READ) == PASS) {
		read_data_from_flash_mem (denali, buf, mtd->oobsize);

		/* wait for command to be accepted
		 * can always use status0 bit as the mask is identical for each
		 * bank. */
		irq_status = wait_for_irq (denali, irq_mask);

		if (irq_status == 0)
			printf ("page on OOB timeout %d\n", denali->page);

		/* We set the device back to MAIN_ACCESS here as I observed
		 * instability with the controller if you do a block erase
		 * and the last transaction was a SPARE_ACCESS. Block erase
		 * is reliable (according to the MTD test infrastructure)
		 * if you are in MAIN_ACCESS.
		 */
		addr = BANK (denali->flash_bank) | denali->page;
		cmd = MODE_10 | addr;
		index_addr (denali, (uint32_t) cmd, MAIN_ACCESS);
	}
}

/* this function examines buffers to see if they contain data that
 * indicate that the buffer is part of an erased region of flash.
 */
bool is_erased (uint8_t * buf, int len)
{
	int i = 0;
	for (i = 0; i < len; i++)
		if (buf[i] != 0xFF)
			return false;
	return true;
}

#define ECC_SECTOR_SIZE 512

#define ECC_SECTOR(x)	(((x) & ECC_ERROR_ADDRESS__SECTOR_NR) >> 12)
#define ECC_BYTE(x)	(((x) & ECC_ERROR_ADDRESS__OFFSET))
#define ECC_CORRECTION_VALUE(x) ((x) & ERR_CORRECTION_INFO__BYTEMASK)
#define ECC_ERROR_CORRECTABLE(x) (!((x) & ERR_CORRECTION_INFO__ERROR_TYPE))
#define ECC_ERR_DEVICE(x)	(((x) & ERR_CORRECTION_INFO__DEVICE_NR) >> 8)
#define ECC_LAST_ERR(x)		((x) & ERR_CORRECTION_INFO__LAST_ERR_INFO)

static bool handle_ecc (struct denali_nand_info *denali, uint8_t * buf,
			uint32_t irq_status)
{
	bool check_erased_page = false;

	if (denali->have_hw_ecc_fixup &&
	    (irq_status & INTR_STATUS__ECC_UNCOR_ERR)) {
		clear_interrupts (denali);
		denali_set_intr_modes (denali, true);

		return true;
	}

	if (irq_status & INTR_STATUS__ECC_ERR) {
		/* read the ECC errors. we'll ignore them for now */
		uint32_t err_address = 0, err_correction_info = 0;
		uint32_t err_byte = 0, err_sector = 0, err_device = 0;
		uint32_t err_correction_value = 0;
		denali_set_intr_modes (denali, false);

		do {
			err_address = ioread32 (denali->flash_reg +
						ECC_ERROR_ADDRESS);
			err_sector = ECC_SECTOR (err_address);
			err_byte = ECC_BYTE (err_address);

			err_correction_info = ioread32 (denali->flash_reg +
							ERR_CORRECTION_INFO);
			err_correction_value =
			    ECC_CORRECTION_VALUE (err_correction_info);
			err_device = ECC_ERR_DEVICE (err_correction_info);

			if (ECC_ERROR_CORRECTABLE (err_correction_info)) {
				/* If err_byte is larger than ECC_SECTOR_SIZE,
				 * means error happened in OOB, so we ignore
				 * it. It's no need for us to correct it
				 * err_device is represented the NAND error
				 * bits are happened in if there are more
				 * than one NAND connected.
				 * */
				if (err_byte < ECC_SECTOR_SIZE) {
					int offset;
					offset = (err_sector *
						  ECC_SECTOR_SIZE +
						  err_byte) *
					    denali->devnum + err_device;
					/* correct the ECC error */
					buf[offset] ^= err_correction_value;
					denali->mtd.ecc_stats.corrected++;
				}
			} else {
				/* if the error is not correctable, need to
				 * look at the page to see if it is an erased
				 * page. if so, then it's not a real ECC error
				 * */
				check_erased_page = true;
			}
		} while (!ECC_LAST_ERR (err_correction_info));
		/* Once handle all ecc errors, controller will triger
		 * a ECC_TRANSACTION_DONE interrupt, so here just wait
		 * for a while for this interrupt
		 * */
		while (!(read_interrupt_status (denali) &
			 INTR_STATUS__ECC_TRANSACTION_DONE))
			cpu_relax ();
		clear_interrupts (denali);
		denali_set_intr_modes (denali, true);
	}
	return check_erased_page;
}

/* programs the controller to either enable/disable DMA transfers */
static void denali_enable_dma (struct denali_nand_info *denali, bool en)
{
	uint32_t reg_val = 0x0;

	if (en)
		reg_val = DMA_ENABLE__FLAG;

	iowrite32 (reg_val, denali->flash_reg + DMA_ENABLE);
	ioread32 (denali->flash_reg + DMA_ENABLE);
}

/* setups the HW to perform the data DMA */
static void denali_setup_dma (struct denali_nand_info *denali, int op)
{
	uint32_t mode = 0x0;
	const int page_count = 1;
	dma_addr_t addr = denali->buf.dma_buf;

	mode = MODE_10 | BANK (denali->flash_bank);

        if (denali->platform == PICOCHIP_PC3008) {

                /* DMA is a three step process on Picochip Hoyle3 devices */

	        /* 1. setup transfer type (op) and # of pages */
	        index_addr (denali, (uint32_t)(mode | denali->page),
                            (uint32_t)(DMA_INT_GENERATION |
                                       DMA_BURST_LENGTH |
                                       0x2000 | op | page_count));

	        /* 2. set memory address bits 31:0 */
	        index_addr (denali, (uint32_t)(mode | denali->page),
                             (uint32_t)(addr));

	        /* 3. set memory address bits 64:32 */
	        index_addr (denali, (uint32_t)(mode | denali->page),
                             (uint32_t)(0));
        }
        else {
                /* DMA is a four step process */

	        /* 1. setup transfer type and # of pages */
	        index_addr (denali, mode | denali->page,
                            0x2000 | op | page_count);

	        /* 2. set memory high address bits 23:8 */
	        index_addr (denali, mode | ((uint16_t) (addr >> 16) << 8),
                            0x2200);

	        /* 3. set memory low address bits 23:8 */
	        index_addr (denali, mode | ((uint16_t) addr << 8), 0x2300);

	        /* 4.  interrupt when complete, burst len = 64 bytes */
	        index_addr (denali, mode | 0x14000, 0x2400);
        }
}

/* writes a page. user specifies type, and this function handles the
 * configuration details. */
static void write_page (struct mtd_info *mtd, struct nand_chip *chip,
			const uint8_t * buf, bool raw_xfer)
{
	struct nand_chip *nand = mtd->priv;
	struct denali_nand_info *denali = nand->priv;

	uint32_t irq_status = 0;
	uint32_t irq_mask = INTR_STATUS__DMA_CMD_COMP |
	    INTR_STATUS__PROGRAM_FAIL;

	/* if it is a raw xfer, we want to disable ecc, and send
	 * the spare area.
	 * !raw_xfer - enable ecc
	 * raw_xfer - transfer spare
	 */
	setup_ecc_for_xfer (denali, !raw_xfer, raw_xfer);

	/* copy buffer into DMA buffer */
	memcpy (denali->buf.buf, buf, mtd->writesize);

	if (raw_xfer) {
		/* transfer the data to the spare area */
		memcpy (denali->buf.buf + mtd->writesize,
			chip->oob_poi, mtd->oobsize);
	}

	clear_interrupts (denali);
	denali_enable_dma (denali, true);

	denali_setup_dma (denali, DENALI_WRITE);

	/* wait for operation to complete */
	irq_status = wait_for_irq (denali, irq_mask);

	if (irq_status == 0) {
		printf ("timeout on write_page (type = %d)\n", raw_xfer);
		denali->status =
		    (irq_status & INTR_STATUS__PROGRAM_FAIL) ?
		    NAND_STATUS_FAIL : PASS;
	}

	denali_enable_dma (denali, false);
}

/* NAND core entry points */

/* this is the callback that the NAND core calls to write a page. Since
 * writing a page with ECC or without is similar, all the work is done
 * by write_page above.
 * */
static void denali_write_page (struct mtd_info *mtd, struct nand_chip *chip,
			       const uint8_t * buf)
{
	/* for regular page writes, we let HW handle all the ECC
	 * data written to the device. */
	write_page (mtd, chip, buf, false);
}

/* This is the callback that the NAND core calls to write a page without ECC.
 * raw access is similar to ECC page writes, so all the work is done in the
 * write_page() function above.
 */
static void denali_write_page_raw (struct mtd_info *mtd, struct nand_chip *chip,
				   const uint8_t * buf)
{
	/* for raw page writes, we want to disable ECC and simply write
	   whatever data is in the buffer. */
	write_page (mtd, chip, buf, true);
}

static int denali_write_oob (struct mtd_info *mtd, struct nand_chip *chip,
			     int page)
{
	return write_oob_data (mtd, chip->oob_poi, page);
}

static int denali_read_oob (struct mtd_info *mtd, struct nand_chip *chip,
			    int page, int sndcmd)
{
	struct denali_nand_info *denali = chip->priv;

        reset_buf (denali);
        read_oob_data (mtd, denali->buf.buf, page);

	return 0;
}

static int denali_read_page (struct mtd_info *mtd, struct nand_chip *chip,
			     uint8_t * buf, int page)
{
	struct nand_chip *nand = mtd->priv;
	struct denali_nand_info *denali = nand->priv;

	uint32_t irq_status = 0;
	uint32_t irq_mask = denali->have_hw_ecc_fixup ?
	    (INTR_STATUS__ECC_TRANSACTION_DONE | INTR_STATUS__ECC_ERR) :
	    (INTR_STATUS__DMA_CMD_COMP | INTR_STATUS__ECC_UNCOR_ERR);
	bool check_erased_page = false;

	if (page != denali->page) {
		printf ("IN %s: page %d is not equal to denali->page %d, "
			"investigate!!\n", __func__, page, denali->page);
		BUG ();
	}

	setup_ecc_for_xfer (denali, true, false);

	denali_enable_dma (denali, true);

	clear_interrupts (denali);
	denali_setup_dma (denali, DENALI_READ);

	/* wait for operation to complete */
	irq_status = wait_for_irq (denali, irq_mask);

	memcpy (buf, denali->buf.buf, mtd->writesize);

	check_erased_page = handle_ecc (denali, buf, irq_status);
	denali_enable_dma (denali, false);

	if (check_erased_page) {
		read_oob_data (&denali->mtd, chip->oob_poi, denali->page);

		/* check ECC failures that may have occurred on erased pages */
		if (check_erased_page) {
			if (!is_erased (buf, denali->mtd.writesize))
				denali->mtd.ecc_stats.failed++;
			if (!is_erased (buf, denali->mtd.oobsize))
				denali->mtd.ecc_stats.failed++;
		}
	}
        return 0;
}

static int denali_read_page_raw (struct mtd_info *mtd, struct nand_chip *chip,
				 uint8_t * buf, int page)
{
	struct nand_chip *nand = mtd->priv;
	struct denali_nand_info *denali = nand->priv;

	uint32_t irq_status = 0;
	uint32_t irq_mask = INTR_STATUS__DMA_CMD_COMP;

	if (page != denali->page) {
		printf ("IN %s: page %d is not equal to denali->page %d, "
			"investigate!!\n", __func__, page, denali->page);
		BUG ();
	}

	setup_ecc_for_xfer (denali, false, true);
	denali_enable_dma (denali, true);

	clear_interrupts (denali);
	denali_setup_dma (denali, DENALI_READ);

	/* wait for operation to complete */
	irq_status = wait_for_irq (denali, irq_mask);

	denali_enable_dma (denali, false);

	memcpy (buf, denali->buf.buf, mtd->writesize);
	memcpy (chip->oob_poi, denali->buf.buf + mtd->writesize, mtd->oobsize);

	return 0;
}

static uint8_t denali_read_byte (struct mtd_info *mtd)
{
	struct nand_chip *nand = mtd->priv;
	struct denali_nand_info *denali = nand->priv;

	uint8_t result = 0xff;

	if (denali->buf.head < denali->buf.tail)
		result = denali->buf.buf[denali->buf.head++];

	return result;
}

static void denali_select_chip (struct mtd_info *mtd, int chip)
{
	struct nand_chip *nand = mtd->priv;
	struct denali_nand_info *denali = nand->priv;

	denali->flash_bank = chip;
}

static int denali_waitfunc (struct mtd_info *mtd, struct nand_chip *chip)
{
	struct nand_chip *nand = mtd->priv;
	struct denali_nand_info *denali = nand->priv;

	int status = denali->status;
	denali->status = 0;

	return status;
}

static void denali_erase (struct mtd_info *mtd, int page)
{
	struct nand_chip *nand = mtd->priv;
	struct denali_nand_info *denali = nand->priv;

	uint32_t cmd = 0x0, irq_status = 0;

	nand_dbg ("denali_erase()\n");

        /* clear interrupts */
	clear_interrupts (denali);

	/* setup page read request for access type */
	cmd = MODE_10 | BANK (denali->flash_bank) | page;
	index_addr (denali, (uint32_t) cmd, 0x1);

	/* wait for erase to complete or failure to occur */
	irq_status = wait_for_irq (denali, INTR_STATUS__ERASE_COMP |
				   INTR_STATUS__ERASE_FAIL);

	denali->status = (irq_status & INTR_STATUS__ERASE_FAIL) ?
	    NAND_STATUS_FAIL : PASS;
}

static void denali_cmdfunc (struct mtd_info *mtd, unsigned int cmd, int col,
			    int page)
{
	struct nand_chip *nand = mtd->priv;
	struct denali_nand_info *denali = nand->priv;

	uint32_t addr, id;
	int i;

	nand_dbg ("denali_cmdfunc()\n");

	switch (cmd) {
	case NAND_CMD_PAGEPROG:
		break;
	case NAND_CMD_STATUS:
		read_status (denali);
		break;
	case NAND_CMD_READID:
	case NAND_CMD_PARAM:
		reset_buf (denali);
		/* sometimes ManufactureId read from register is not right
		 * e.g. some of Micron MT29F32G08QAA MLC NAND chips
		 * So here we send READID cmd to NAND insteand
		 * */
		addr = (uint32_t) MODE_11 | BANK (denali->flash_bank);
		index_addr (denali, (uint32_t) addr | 0, 0x90);
		index_addr (denali, (uint32_t) addr | 1, 0);
		for (i = 0; i < 5; i++) {
			index_addr_read_data (denali, (uint32_t) addr | 2, &id);
			write_byte_to_buf (denali, id);
		}
		break;
	case NAND_CMD_READ0:
	case NAND_CMD_SEQIN:
		denali->page = page;
		break;
	case NAND_CMD_RESET:
		reset_bank (denali);
		break;
	case NAND_CMD_READOOB:
                denali_read_oob (mtd, nand, page, 0);
		break;
	case NAND_CMD_ERASE1:
		denali_erase (mtd, page);
		break;
	case NAND_CMD_ERASE2:
		/* We action the erase on the NAND_CMD_ERASE1
		 * being received, so no need to do
		 * anything here
		 */
		break;
	default:
		printf ("unsupported command received 0x%x\n", cmd);
		break;
	}
}

/* stubs for ECC functions not used by the NAND core */
static int denali_ecc_calculate (struct mtd_info *mtd, const uint8_t * data,
				 uint8_t * ecc_code)
{
	printf ("denali_ecc_calculate called unexpectedly.\n");
	BUG ();
	return -EIO;
}

static int denali_ecc_correct (struct mtd_info *mtd, uint8_t * data,
			       uint8_t * read_ecc, uint8_t * calc_ecc)
{
	printf ("denali_ecc_correct called unexpectedly.\n");
	BUG ();
	return -EIO;
}

static void denali_ecc_hwctl (struct mtd_info *mtd, int mode)
{
	printf ("denali_ecc_hwctl called unexpectedly.\n");
	BUG ();
}

/* end NAND core entry points */

/* Initialization code to bring the device up to a known good state */
static void denali_hw_init (struct denali_nand_info *denali)
{
	nand_dbg ("denali_hw_init()\n");

        denali_enable_dma (denali, false);
        detect_max_banks (denali);

	/* tell driver how many bit controller will skip before
	 * writing ECC code in OOB, this register may be already
	 * set by firmware. So we read this value out.
	 * if this value is 0, just let it be.
	 * */
	denali->bbtskipbytes = ioread32 (denali->flash_reg +
					 SPARE_AREA_SKIP_BYTES);

	denali_nand_reset (denali);
	iowrite32 (((1 << denali->max_banks) - 1),
		   denali->flash_reg + RB_PIN_ENABLED);
	iowrite32 (CHIP_EN_DONT_CARE__FLAG,
		   denali->flash_reg + CHIP_ENABLE_DONT_CARE);

	iowrite32 (0xffff, denali->flash_reg + SPARE_AREA_MARKER);
        iowrite32 (2, denali->flash_reg + SPARE_AREA_SKIP_BYTES);
        denali->bbtskipbytes = 2;

	/* Should set value for these registers when init */
	iowrite32 (0, denali->flash_reg + TWO_ROW_ADDR_CYCLES);
	iowrite32 (1, denali->flash_reg + ECC_ENABLE);

	denali_nand_timing_set (denali);
	denali_irq_init (denali);
}

/* Althogh controller spec said SLC ECC is forced to be 4bit,
 * but denali controller in MRST only support 15bit and 8bit ECC
 * correction
 * */
#define ECC_8BITS	14
static struct nand_ecclayout nand_8bit_oob = {
	.eccbytes = 14,
};

#define ECC_15BITS	26
static struct nand_ecclayout nand_15bit_oob = {
	.eccbytes = 26,
};

static uint8_t bbt_pattern[] = { 'B', 'b', 't', '0' };
static uint8_t mirror_pattern[] = { '1', 't', 'b', 'B' };

static struct nand_bbt_descr bbt_main_descr = {
	.options = NAND_BBT_LASTBLOCK | NAND_BBT_CREATE | NAND_BBT_WRITE
	    | NAND_BBT_2BIT | NAND_BBT_VERSION | NAND_BBT_PERCHIP,
	.offs = 8,
	.len = 4,
	.veroffs = 12,
	.maxblocks = 4,
	.pattern = bbt_pattern,
};

static struct nand_bbt_descr bbt_mirror_descr = {
	.options = NAND_BBT_LASTBLOCK | NAND_BBT_CREATE | NAND_BBT_WRITE
	    | NAND_BBT_2BIT | NAND_BBT_VERSION | NAND_BBT_PERCHIP,
	.offs = 8,
	.len = 4,
	.veroffs = 12,
	.maxblocks = 4,
	.pattern = mirror_pattern,
};

int board_nand_init (struct nand_chip *nand)
{
	struct mtd_info *mtd = NULL;
	struct denali_nand_info *denali = NULL;

	nand_dbg ("board_nand_init()\n");

	denali = (struct denali_nand_info *)malloc
                    (sizeof (struct denali_nand_info));
	if (denali == NULL) {
		/* Oops, no memory available */
		return -ENOMEM;
	}

	/* Initialise our private structure */
        memset (denali, 0, sizeof (struct denali_nand_info));

	/* Link all our structures */
	mtd = &denali->mtd;
	mtd->priv = nand;
	denali->nand = nand;
	nand->priv = denali;

        nand->numchips = CONFIG_SYS_NAND_MAX_CHIPS;

	denali->flash_reg = (unsigned char *)PICOXCELL_NAND_BASE;
	denali->flash_mem = (unsigned char *)PICOXCELL_NAND_DATA_BASE;

	/* Set up for 8 bit error correction */
	denali->nr_ecc_bits = 8;

	denali->buf.dma_buf = (dma_addr_t) denali->buf.buf;

	denali_hw_init (denali);

	/* register the driver with the NAND core subsystem */
	nand->select_chip = denali_select_chip;
	nand->cmdfunc = denali_cmdfunc;
	nand->read_byte = denali_read_byte;
	nand->waitfunc = denali_waitfunc;

	/* support for multi nand
	 * MTD knows nothing about multi nand,
	 * so we should tell it the real pagesize
	 * and anything necessery
	 */
	denali->devnum = ioread32 (denali->flash_reg + DEVICES_CONNECTED);
	nand->chipsize <<= (denali->devnum - 1);
	nand->page_shift += (denali->devnum - 1);
	nand->pagemask = (nand->chipsize >> nand->page_shift) - 1;
	nand->bbt_erase_shift += (denali->devnum - 1);
	nand->phys_erase_shift = nand->bbt_erase_shift;
	nand->chip_shift += (denali->devnum - 1);
	denali->mtd.writesize <<= (denali->devnum - 1);
	denali->mtd.oobsize <<= (denali->devnum - 1);
	denali->mtd.erasesize <<= (denali->devnum - 1);
	denali->mtd.size = nand->numchips * nand->chipsize;
	denali->bbtskipbytes *= denali->devnum;

	/* second stage of the NAND scan
	 * this stage requires information regarding ECC and
	 * bad block management. */

	/* Bad block management */
	nand->bbt_td = &bbt_main_descr;
	nand->bbt_md = &bbt_mirror_descr;

	/* skip the scan for now until we have OOB read and write support */
	nand->options |= NAND_USE_FLASH_BBT | NAND_SKIP_BBTSCAN;
	nand->ecc.mode = NAND_ECC_HW_SYNDROME;

	/* Denali Controller only support 15bit and 8bit ECC in MRST,
	 * so just let controller do 15bit ECC for MLC and 8bit ECC for
	 * SLC if possible.
	 * */
	denali->mtd.oobsize = 64;
        denali->mtd.writesize = 2048;

        if (nand->cellinfo & 0xc &&
	    (denali->mtd.oobsize > (denali->bbtskipbytes +
				    ECC_15BITS * (denali->mtd.writesize /
						  ECC_SECTOR_SIZE)))) {
		/* if MLC OOB size is large enough, use 15bit ECC */
		nand->ecc.layout = &nand_15bit_oob;
		nand->ecc.bytes = ECC_15BITS;
		iowrite32 (15, denali->flash_reg + ECC_CORRECTION);
	} else if (denali->mtd.oobsize < (denali->bbtskipbytes +
					  ECC_8BITS * (denali->mtd.writesize /
						       ECC_SECTOR_SIZE))) {
		printf ("Your NAND chip OOB is not large enough to contain "
			"8bit ECC correction codes.\n");
		return -EPERM;
	} else {
		nand->ecc.layout = &nand_8bit_oob;
		nand->ecc.bytes = ECC_8BITS;
		iowrite32 (8, denali->flash_reg + ECC_CORRECTION);
	}

	nand->ecc.bytes *= denali->devnum;
	nand->ecc.layout->eccbytes *= denali->mtd.writesize / ECC_SECTOR_SIZE;
	nand->ecc.layout->oobfree[0].offset =
	    denali->bbtskipbytes + nand->ecc.layout->eccbytes;
	nand->ecc.layout->oobfree[0].length =
	    denali->mtd.oobsize - nand->ecc.layout->eccbytes -
	    denali->bbtskipbytes;

	/* Let driver know the total blocks number and
	 * how many blocks contained by each nand chip.
	 * blksperchip will help driver to know how many
	 * blocks is taken by FW.
	 * */
	denali->totalblks = denali->mtd.size >> nand->phys_erase_shift;
        denali->blksperchip = denali->totalblks / nand->numchips;

	/* These functions are required by the NAND core framework, otherwise,
	 * the NAND core will assert. However, we don't need them, so we'll stub
	 * them out. */
	nand->ecc.calculate = denali_ecc_calculate;
	nand->ecc.correct = denali_ecc_correct;
	nand->ecc.hwctl = denali_ecc_hwctl;

	/* override the default read operations */
	nand->ecc.size = ECC_SECTOR_SIZE * denali->devnum;
	nand->ecc.read_page = denali_read_page;
	nand->ecc.read_page_raw = denali_read_page_raw;
	nand->ecc.write_page = denali_write_page;
	nand->ecc.write_page_raw = denali_write_page_raw;
	nand->ecc.read_oob = denali_read_oob;
	nand->ecc.write_oob = denali_write_oob;
	nand->erase_cmd = denali_erase;

#ifdef CONFIG_PICOCHIP_PC3008
	denali->platform = PICOCHIP_PC3008;
#endif
	return 0;
}
