/*****************************************************************************
 * $picoChipHeaderSubst$
 *****************************************************************************/

/*!
* \file axi2cfg.h
* \brief Definitions for the picoxcell axi2cfg block.
*
* Copyright (c) 2006-2012 Picochip Ltd
*
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License version 2 as
* published by the Free Software Foundation.
*
* All enquiries to support@picochip.com
*/

#ifndef __PICOXCELL_AXI2CFG_H__
#define __PICOXCELL_AXI2CFG_H__

/* Constants --------------------------------------------------------------- */

/*****************************************************************************/
/* Register Offset Addresses                                                 */
/*****************************************************************************/

/* Functional Registers */

#define AXI2CFG_SYS_CONFIG_REG_OFFSET               (0x0000)
#define AXI2CFG_JTAG_ISC_REG_OFFSET                 (0x0004)
#define AXI2CFG_IRQ_REG_OFFSET                      (0x0008)
#define AXI2CFG_PURGE_CFG_PORT_REG_OFFSET           (0x000C)
#define AXI2CFG_DMA_CFG_REG_OFFSET                  (0x0010)
#define AXI2CFG_DEVICE_ID_REG_OFFSET                (0x0014)
#define AXI2CFG_REVISION_ID_REG_OFFSET              (0x0018)
#define AXI2CFG_SHD_GPIO_MUXING_REG_OFFSET          (0x0034)
#define AXI2CFG_USE_PAI_GPIO_REG_OFFSET             (0x0038)
#define AXI2CFG_USE_EBI_GPIO_REG_OFFSET             (0x003C)
#define AXI2CFG_USE_DECODE_GPIO_REG_OFFSET          (0x0040)
#define AXI2CFG_USE_MISC_INT_GPIO_REG_OFFSET        (0x0044)
#define AXI2CFG_CLK_GATING_REG_OFFSET               (0x0048)
#define AXI2CFG_ARM_PLL_CLK_REG_OFFSET              (0x0050)
#define AXI2CFG_FREQ_SENSE_REG_OFFSET               (0x0054)
#define AXI2CFG_DECODE_MUX_REG_OFFSET               (0x0064)
#define AXI2CFG_ID_REG_OFFSET                       (0x0068)
#define AXI2CFG_CONFIG_WRITE_REG_OFFSET             (0x0100)
#define AXI2CFG_CONFIG_READ_REG_OFFSET              (0x0200)
#define AXI2CFG_DMAC1_CONFIG_REG_OFFSET             (0x0300)

/*****************************************************************************/
/* Register Bit Fields		                                             */
/*****************************************************************************/

#define AXI2CFG_SYS_CONFIG_PA_RST_IDX               (30)
#define AXI2CFG_SYS_CONFIG_SD_ARM_GPIO_SEL_SZ       (8)
#define AXI2CFG_SYS_CONFIG_SD_ARM_GPIO_SEL_HI       (23)
#define AXI2CFG_SYS_CONFIG_SD_ARM_GPIO_SEL_LO       (16)
#define AXI2CFG_SYS_CONFIG_RW_EBI_CLK_DISABLE_IDX   (15)
#define AXI2CFG_SYS_CONFIG_RW_EXCVEC_EN_IDX         (14)
#define AXI2CFG_SYS_CONFIG_RW_RMII_EN_IDX           (13)
#define AXI2CFG_SYS_CONFIG_RW_REVMII_EN_IDX         (12)
#define AXI2CFG_SYS_CONFIG_SSI_EBI_SEL_SZ           (4)
#define AXI2CFG_SYS_CONFIG_SSI_EBI_SEL_HI           (11)
#define AXI2CFG_SYS_CONFIG_SSI_EBI_SEL_LO           (8)
#define AXI2CFG_SYS_CONFIG_FREQ_SYNTH_MUX_IDX       (7)
#define AXI2CFG_SYS_CONFIG_MASK_AXI_ERR_IDX         (6)
#define AXI2CFG_SYS_CONFIG_RW_REMAP_IDX             (5)
#define AXI2CFG_SYS_CONFIG_WDG_PAUSE_IDX            (4)
#define AXI2CFG_SYS_CONFIG_DMAC1_CH6_IDX            (3)
#define AXI2CFG_SYS_CONFIG_DMAC1_CH7_IDX            (2)
#define AXI2CFG_SYS_CONFIG_BOOT_MODE_SZ             (2)
#define AXI2CFG_SYS_CONFIG_BOOT_MODE_HI             (1)
#define AXI2CFG_SYS_CONFIG_BOOT_MODE_LO             (0)

#define AXI2CFG_SYS_CONFIG_PA_RST_MASK              (1 << AXI2CFG_SYS_CONFIG_PA_RST_IDX)
#define AXI2CFG_SYS_CONFIG_SD_ARM_GPIO_MASK         (((1 << AXI2CFG_SYS_CONFIG_SD_ARM_GPIO_SEL_SZ)-1) << AXI2CFG_SYS_CONFIG_SD_ARM_GPIO_SEL_LO)
#define AXI2CFG_SYS_CONFIG_RW_EXCVEC_EN_MASK        (1 << AXI2CFG_SYS_CONFIG_RW_EXCVEC_EN_IDX)
#define AXI2CFG_SYS_CONFIG_RW_RMII_EN_MASK          (1 << AXI2CFG_SYS_CONFIG_RW_RMII_EN_IDX)
#define AXI2CFG_SYS_CONFIG_RW_REVMII_EN_MASK        (1 << AXI2CFG_SYS_CONFIG_RW_REVMII_EN_IDX)
#define AXI2CFG_SYS_CONFIG_SSI_EBI_SEL_MASK         (((1 << AXI2CFG_SYS_CONFIG_SSI_EBI_SEL_SZ)-1) << AXI2CFG_SYS_CONFIG_SSI_EBI_SEL_LO)
#define AXI2CFG_SYS_CONFIG_FREQ_SYNTH_MUX_MASK      (1<<AXI2CFG_SYS_CONFIG_FREQ_SYNTH_MUX_IDX)
#define AXI2CFG_SYS_CONFIG_MASK_AXI_ERR_MASK        (1 << AXI2CFG_SYS_CONFIG_MASK_AXI_ERR_IDX)
#define AXI2CFG_SYS_CONFIG_RW_REMAP_MASK            (1 << AXI2CFG_SYS_CONFIG_RW_REMAP_IDX)
#define AXI2CFG_SYS_CONFIG_WDG_PAUSE_MASK           (1 << AXI2CFG_SYS_CONFIG_WDG_PAUSE_IDX)
#define AXI2CFG_SYS_CONFIG_DMAC1_CH6_MASK           (1 << AXI2CFG_SYS_CONFIG_DMAC1_CH6_IDX)
#define AXI2CFG_SYS_CONFIG_DMAC1_CH7_MASK           (1 << AXI2CFG_SYS_CONFIG_DMAC1_CH7_IDX)
#define AXI2CFG_SYS_CONFIG_BOOT_MODE_MASK           (((1 << AXI2CFG_SYS_CONFIG_BOOT_MODE_SZ)-1) << AXI2CFG_SYS_CONFIG_BOOT_MODE_LO)

#define AXI2CFG_DMAC1_CONFIG_WR_SZ                  (7)
#define AXI2CFG_DMAC1_CONFIG_WR_HI                  (16)
#define AXI2CFG_DMAC1_CONFIG_WR_LO                  (10)
#define AXI2CFG_DMAC1_CONFIG_WATERMARK_SZ           (7)
#define AXI2CFG_DMAC1_CONFIG_WATERMARK_HI           (9)
#define AXI2CFG_DMAC1_CONFIG_WATERMARK_LO           (3)
#define AXI2CFG_DMAC1_CONFIG_SNGL_IDX               (2)
#define AXI2CFG_DMAC1_CONFIG_STATE_IDX              (1)
#define AXI2CFG_DMAC1_CONFIG_ENABLE_IDX             (0)

#define AXI2CFG_JTAG_ISC_REGISTER_IDX               (0)
#define AXI2CFG_JTAG_ISC_IN_CTRL_IDX                (1)
#define AXI2CFG_JTAG_ISC_DISABLED_IDX               (2)
/* [31:2] - Reserved */

#define AXI2CFG_PURGE_CFG_RD_PORT_IDX               (0)
#define AXI2CFG_PURGE_CFG_WR_PORT_IDX               (1)
#define AXI2CFG_PURGE_CFG_WR_PRGSS_PORT_IDX         (2)
/* [31:3]   Reserved */

#define AXI2CFG_DEVICE_ID_NML_302_REG_VALUE         (0x03020004)

/*****************************************************************************/
/* Register Bit Field Manipulation                                           */
/*****************************************************************************/

#define AXI2CFG_PA_SOFT_RESET_IDX                   (30)
#define AXI2CFG_SHD_GPIO_7_IDX                      (23)
#define AXI2CFG_SHD_GPIO_6_IDX                      (22)
#define AXI2CFG_SHD_GPIO_5_IDX                      (21)
#define AXI2CFG_SHD_GPIO_4_IDX                      (20)
#define AXI2CFG_SHD_GPIO_3_IDX                      (19)
#define AXI2CFG_SHD_GPIO_2_IDX                      (18)
#define AXI2CFG_SHD_GPIO_1_IDX                      (17)
#define AXI2CFG_SHD_GPIO_0_IDX                      (16)
#define AXI2CFG_EBI_CLK_DISABLE_IDX                 (15)
#define AXI2CFG_EXCEPTION_VECT_EN_IDX               (14)
#define AXI2CFG_RMII_EN_IDX                         (13)
#define AXI2CFG_REV_MII_EN_IDX                      (12)
#define AXI2CFG_DECODE_MUX_3_IDX                    (11)
#define AXI2CFG_DECODE_MUX_2_IDX                    (10)
#define AXI2CFG_DECODE_MUX_1_IDX                    (9)
#define AXI2CFG_DECODE_MUX_0_IDX                    (8)
#define AXI2CFG_MASK_AXI_ERROR_IDX                  (6)
#define AXI2CFG_REMAP_IDX                           (5)
#define AXI2CFG_WDG_PAUSE_IDX                       (4)
#define AXI2CFG_DMA_CHAN_6_MUX_IDX                  (3)
#define AXI2CFG_DMA_CHAN_7_MUX_IDX                  (2)
#define AXI2CFG_BOOT_MODE_IDX                       (0)

#define AXI2CFG_PC30XX_MII_MODE_IDX                 (11)

#define AXI2CFG_PA_SOFT_RESET               (1 << AXI2CFG_PA_SOFT_RESET_IDX)
#define AXI2CFG_SHD_GPIO_7                  (1 << AXI2CFG_SHD_GPIO_7_IDX)
#define AXI2CFG_SHD_GPIO_6                  (1 << AXI2CFG_SHD_GPIO_6_IDX)
#define AXI2CFG_SHD_GPIO_5                  (1 << AXI2CFG_SHD_GPIO_5_IDX)
#define AXI2CFG_SHD_GPIO_4                  (1 << AXI2CFG_SHD_GPIO_4_IDX)
#define AXI2CFG_SHD_GPIO_3                  (1 << AXI2CFG_SHD_GPIO_3_IDX)
#define AXI2CFG_SHD_GPIO_2                  (1 << AXI2CFG_SHD_GPIO_2_IDX)
#define AXI2CFG_SHD_GPIO_1                  (1 << AXI2CFG_SHD_GPIO_1_IDX)
#define AXI2CFG_SHD_GPIO_0                  (1 << AXI2CFG_SHD_GPIO_0_IDX)
#define AXI2CFG_EBI_CLK_DISABLE             (1 << AXI2CFG_EBI_CLK_DISABLE_IDX)
#define AXI2CFG_EXCEPTION_VECT_EN           (1 << AXI2CFG_EXCEPTION_VECT_EN_IDX)
#define AXI2CFG_RMII_EN                     (1 << AXI2CFG_RMII_EN_IDX)
#define AXI2CFG_REV_MII_EN                  (1 << AXI2CFG_REV_MII_EN_IDX)
#define AXI2CFG_DECODE_MUX_3                (1 << AXI2CFG_DECODE_MUX_3_IDX)
#define AXI2CFG_DECODE_MUX_2                (1 << AXI2CFG_DECODE_MUX_2_IDX)
#define AXI2CFG_DECODE_MUX_1                (1 << AXI2CFG_DECODE_MUX_1_IDX)
#define AXI2CFG_DECODE_MUX_0                (1 << AXI2CFG_DECODE_MUX_0_IDX)
#define AXI2CFG_MASK_AXI_ERROR              (1 << AXI2CFG_MASK_AXI_ERROR_IDX)
#define AXI2CFG_REMAP                       (1 << AXI2CFG_REMAP_IDX)
#define AXI2CFG_WDG_PAUSE                   (1 << AXI2CFG_WDG_PAUSE_IDX)
#define AXI2CFG_DMA_CHAN_6_MUX              (1 << AXI2CFG_DMA_CHAN_6_MUX_IDX)
#define AXI2CFG_DMA_CHAN_7_MUX              (1 << AXI2CFG_DMA_CHAN_7_MUX_IDX)
#define AXI2CFG_BOOT_MODE                   (1 << AXI2CFG_BOOT_MODE_IDX)

#define AXI2CFG_PC30XX_MII_MODE             (0x7 << AXI2CFG_PC30XX_MII_MODE_IDX)
#define AXI2CFG_PC30XX_RGMII_EN             (0x2 << AXI2CFG_PC30XX_MII_MODE_IDX)

/* Clock Control Stuff */
#define PICOXCELL_TZ_PROT_CTRL_CLOCK        (1 << 0)
#define PICOXCELL_SSI_CLOCK                 (1 << 1)
#define PICOXCELL_DMAC_1_CLOCK              (1 << 2)
#define PICOXCELL_DMAC_2_CLOCK              (1 << 3)
#define PICOXCELL_EBI_CLOCK                 (1 << 4)
#define PICOXCELL_SPACC_1_CLOCK             (1 << 5)
#define PICOXCELL_SPACC_3_CLOCK             (1 << 6)
#define PICOXCELL_TRNG_CLOCK                (1 << 7)
#define PICOXCELL_AXI_FUSE_CLOCK            (1 << 8)
#define PICOXCELL_AXI_ROM_CLOCK             (1 << 9)
#define PICOXCELL_CASCADE_CLOCK             (1 << 10)
#define PICOXCELL_NAND_CLOCK                (1 << 11)
#define PICOXCELL_MEMIF_ARM_CLOCK           (1 << 12)
#define PICOXCELL_MEMIF_SHD_SRAM_CLOCK      (1 << 13)
#define PICOXCELL_MEMIF_SHD_SDRAM_CLOCK     (1 << 14)
#define PICOXCELL_AXI2PICO_CLOCK            (1 << 15)

/* Id Register Stuff */
#define PICOXCELL_MEMIF_ARM_NOT_PRESENT     (1 << 10)

/* PC30XX Decode Mux stuff */
#define PC30XX_SPI_DECODE_STEP_SIZE         (8)

#endif /* __PICOXCELL_AXI2CFG_H__ */
