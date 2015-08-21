/*
 * Copyright(c) 2007-2014 Intel Corporation. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify 
 * it under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but 
 * WITHOUT ANY WARRANTY; without even the implied warranty of 
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU 
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License 
 * along with this program; if not, write to the Free Software 
 * Foundation, Inc., 51 Franklin St - Fifth Floor, Boston, MA 02110-1301 USA.
 * The full GNU General Public License is included in this distribution 
 * in the file called LICENSE.GPL.
 *
 * Contact Information:
 * Intel Corporation
 */

#ifndef _T2000_H_
#define _T2000_H_


#define NUM_MAP_DEV                     4
#define NUM_DSP_DEV                     2
#define NUM_JESD_DEV                    2

#define ROM_BASEADDR                    0xFF000000  /**< Internal ROM base address  */
#define ROM_SIZE                        0x00008000  /**< Internal ROM size in bytes */

/*
  * Ceva Group Slaves - CevaXC, INTC, Timer, Semaphore Blocks
 */

#define CEVA0_SLAVE                     0xF0000000
#define CEVA1_SLAVE                     0xF0100000
#define CEVA_SLAVE(n)                   (CEVA0_SLAVE+(n)*0x00100000)

#define CEVA_RESET                      0xF0D00000
#define CEVA_BOOT_ADDR                  0xF0D00010

#define SET_INT0_REG                    0xF0D20000
#define CLEAR_INT0_REG                  0xF0D20004
#define AUTO_CLEAR_INT0                 0xF0D20008
#define ARM_IRQ_EN                      0xF0D2000C

#define SET_INT1_REG                    0xF0D20010
#define CLEAR_INT1_REG                  0xF0D20014
#define AUTO_CLEAR_INT1                 0xF0D20018
#define MESSAGE_IRQ_EN                  0xF0D2001C

#define SET_INT2_REG                    0xF0D20020
#define CLEAR_INT2_REG                  0xF0D20024
#define AUTO_CLEAR_INT2                 0xF0D20028
#define SNOOP_TIMER_IRQ_EN              0xF0D2002C

#define SET_NMI_REG                     0xF0D20030
#define CLEAR_NMI_REG                   0xF0D20034
#define AUTO_CLEAR_NMI                  0xF0D20038
#define VIOLATION_IRQ_EN                0xF0D2003C

#define SET_BP1_REG                     0xF0D20040
#define BP1_STATUS_REG                  0xF0D20040
#define CLEAR_BP1_REG                   0xF0D20044
#define AUTO_CLEAR_BP1                  0xF0D20048
#define EXT_BP1_IRQ_EN                  0xF0D2004C

#define SET_BP2_REG                     0xF0D20050
#define BP2_STATUS_REG                  0xF0D20050
#define CLEAR_BP2_REG                   0xF0D20054
#define AUTO_CLEAR_BP2                  0xF0D20058
#define EXT_BP2_IRQ_EN                  0xF0D2005C

#define TIMER_CEVA                      0xF0D30000
#define BPROC_BASEADDR                  0xF0D80000

#define SEMACEVA_BASEADDR               0xF0F00000 /**< CEVA Hardware semaphores base address */
#define SEMACEVACTRL(semaid)            (SEMACEVA_BASEADDR+semaid*4) /**< Macro to get address based on semaphore ID */
#define SEMACEVANUM                     32

/*
 * SPU Partition 0xF3xxxxxx - CRAM, FFT_TOP, CRP, DSP ROM Blocks
 */

#define CRAM_BASEADDR                   0xF3000000  /**< Common RAM (CRAM) base address */
#define CRAM_SIZE                       0x00060000  /**< Common RAM (CRAM) total size */

#define SPU_CONFIG_BASEADDR             0xF3D00000  /**< SPU Configuration base address */

#define MAP_BASEADDR                    0xF3D40000  /**< MAP (FFT) base address*/

#define MDMACRP0_BASEADDR               0xF3D80000  /**< CRP 8 Channel MDMA 0*/
#define MDMACRP1_BASEADDR               0xF3D84000  /**< CRP 8 Channel MDMA 1*/
#define MDMACRP_BASEADDR(id)            (MDMACRP0_BASEADDR + (id)*0x4000)

/*
 * SYS Partition (0xF4xxxxxx) - IRAM, FEC, DDR Config, Semaphore Blocks
 */
#define IRAM_BASEADDR                   0xF4000000 /**< Fast internal system bus static RAM base address */
#define IRAM_SIZE                       0x00040000 /**< Size of internal system bus static RAM base address */

#define DDR0_CONTROLLER_BASE            0xF4C20000 /**< Internal DDR3 DRAM controller base address */

#define DDRPHY0_BASE                    0xF4CA0000 /**< Internal DDR3 PHY for DDR controller base address */

#define SEMAARM_BASEADDR                0xF4F00000 /**< Internal Hardware semaphores base address */
#define SEMAARMCTRL(semaid)             (SEMAARM_BASEADDR+semaid*4) /**< Macro to get address based on semaphore ID */
#define SEMAARMNUM                      32

#define MDMASYS0_BASEADDR               0xF4C80000
#define MDMASYS1_BASEADDR               0xF4C90000
#define MDMASYS_BASEADDR(id)            (MDMASYS0_BASEADDR + (id)*0x10000)

/*
 * Top Level Clock and Reset Control Registers
 */
#define DEVICE_RST_CNTRL                0xF4CF0000  /**< Device Reset Control Register */
#define SERDES_RST_CNTRL                0xF4CF0004  /**< Serdes Reset Control Register */
#define PCIe_RST_CNTRL                  0xF4CF0008  /**< PCIe Reset Control Register */
#define USB_RST_CNTRL                   0xF4CF000C  /**< USB Reset Control Register */
#define CA9_MC_PWR_STAT                 0xF4CF0028  /**< ARM9 DP Power Status Register */
#define CA9_MC_PWR_CNTRL                0xF4CF002C  /**< ARM9 DP Power Control Register */
#define PLLS_GLOBAL_CNTRL               0xF4CF0038  /**< PLLs Global Control Register */
#define AXI_CLK_CNTRL_0                 0xF4CF0040  /**< AXI Clock Control Register 0 */
#define AXI_CLK_CNTRL_1                 0xF4CF0044  /**< AXI Clock Control Register 1 */
#define AXI_CLK_CNTRL_2                 0xF4CF0048  /**<AXI Clock Control Register 2 */
#define AXI_CLK_DIV_CNTRL               0xF4CF004C  /**< AXI Clock Divider Control Register */
#define AXI_RESET_0                     0xF4CF0050  /**< AXI Reset Control Register 0 */
#define AXI_RESET_1                     0xF4CF0054  /**< AXI Reset Control Register 1 */
#define AXI_RESET_2                     0xF4CF0058  /**< AXI Reset Control Register 2 */
#define CA9_MC_MPU_CLK_CNTRL            0xF4CF0068  /**< CA9_MC MP Units Clock Control Register */
#define CA9_MC_MPU_CLK_DIV_CNTRL        0xF4CF006C  /**< CA9_MC MP Units Clock Divider Control Register */
#define CA9_MC_MPU_RESET                0xF4CF0070  /**< CA9_MC MP Units Reset Register */
#define CA9_MC_CPU_CLK_CNTRL            0xF4CF0074  /**< CA9_MC CPU Clock Control Register */
#define CA9_MC_CPU_RESET                0xF4CF0078  /**< CA9_MC CPU Reset Register */
#define CA9_MC_CLK_CNTRL                0xF4CF0080  /**< CA9_MC Clock Control Register */
#define CA9_MC_CLK_DIV_CNTRL            0xF4CF0084  /**< CA9_MC Clock Divider Control Register */
#define CA9_MC_RESET                    0xF4CF0088  /**< CA9_MC Reset Register */

#define L2CC_CLK_CNTRL                  0xF4CF0090  /**< L2CC Clock Control Register */
#define L2CC_CLK_DIV_CNTRL              0xF4CF0094  /**< L2CC Clock Divider Control Register */
#define L2CC_RESET                      0xF4CF0098  /**< L2CC Clock Domain Reset Register */

#define TPI_CLK_CNTRL                   0xF4CF00A0  /**< TPI Clock Control Register */
#define TPI_CLK_DIV_CNTRL               0xF4CF00A4  /**< TPI Clock Divider Control Register */
#define TPI_RESET                       0xF4CF00A8  /**< TPI Reset Register */

#define CSYS_CLK_CNTRL                  0xF4CF00B0  /**< CORESIGHT Clock Control Register */
#define CSYS_CLK_DIV_CNTRL              0xF4CF00B4  /**< CORESIGHT Clock Divider Control Register */
#define CSYS_RESET                      0xF4CF00B8  /**< CORESIGHT Reset Register */

#define EXTPHY0_CLK_CNTRL               0xF4CF00C0  /**< External PHY0 Clock Control Register */
#define EXTPHY0_CLK_DIV_CNTRL           0xF4CF00C4  /**< External PHY0 Clock Divider Control Register */
#define EXTPHY0_RESET                   0xF4CF00C8  /**< External PHY0 Reset Register */

#define EXTPHY1_CLK_CNTRL               0xF4CF00D0  /**< External PHY1 Clock Control Register */
#define EXTPHY1_CLK_DIV_CNTRL           0xF4CF00D4  /**< External PHY1 Clock Divider Control Register */
#define EXTPHY1_RESET                   0xF4CF00D8  /**< External PHY1 Reset Register */

#define FEC_UL_CLK_CNTRL                0xF4CF00E0  /**< FEC Uplink Clock Control Register */
#define FEC_UL_CLK_DIV_CNTRL            0xF4CF00E4  /**< FEC Uplink Clock Divider Control Register */
#define FEC_UL_RESET                    0xF4CF00E8  /**< FEC Uplink Reset Register */

#define FEC_DL_CLK_CNTRL                0xF4CF00F0  /**< FEC Downlink Clock Control Register */
#define FEC_DL_CLK_DIV_CNTRL            0xF4CF00F4  /**< FEC Downlink Clock Divider Control Register */
#define FEC_DL_RESET                    0xF4CF00F8  /**< FEC Downlink Reset Register */

#define FFT_CLK_CNTRL                   0xF4CF0100  /**< FFT Top Clock Control Register */
#define FFT_CLK_DIV_CNTRL               0xF4CF0104  /**< FFT Top Clock Divider Control Register */
#define FFT_RESET                       0xF4CF0108  /**< FFT Top Reset Register */

#define IPSEC_CLK_CNTRL                 0xF4CF0110  /**< IPSEC Clock Control Register */
#define IPSEC_CLK_DIV_CNTRL             0xF4CF0114  /**< IPSEC Clock Divider Control Register */
#define IPSEC_RESET                     0xF4CF0118  /**< IPSEC Reset Register */

#define DDR3_CLK_CNTRL                  0xF4CF0120  /**< DDR3 PHY Clock Control Register */
#define DDR3_CLK_DIV_CNTRL              0xF4CF0124  /**< DDR3 PHY Clock Divider Control Register */
#define DDR3_RESET                      0xF4CF0128  /**< DDR3 PHY Reset Register */

#define GEMTX_CLK_CNTRL                 0xF4CF0130  /**< GEM TX Clock Control Register */
#define GEMTX_CLK_DIV_CNTRL             0xF4CF0134  /**< GEM TX Clock Divider Control Register */
#define GEMTX_RESET                     0xF4CF0138  /**< GEM TX Reset Register */

#define TDMNTG_REF_CLK_CNTRL            0xF4CF0140  /**< TDM NTG Clock Control Register */
#define TDMNTG_REF_CLK_DIV_CNTRL        0xF4CF0144  /**< TDM NTG Clock Divider Control Register */
#define TDMNTG_RESET                    0xF4CF0148  /**< TDM NTG Reset Register */

#define TSUNTG_REF_CLK_CNTRL            0xF4CF0150  /**< TSU NTG Clock Control Register */
#define TSUNTG_REF_CLK_DIV_CNTRL        0xF4CF0154  /**< TSU NTG Clock Divider Control Register */
#define TSUNTG_RESET                    0xF4CF0158  /**< TSU NTG Reset Register */

#define CRP_CLK_CNTRL                   0xF4CF0170  /**< CRP Clock Control Register */
#define CRP_CLK_DIV_CNTRL               0xF4CF0174  /**< CRP Clock Divider Control Register */
#define CRP_CLK_RESET                   0xF4CF0178  /**< CRP Reset Register */

#define CEVA_CLK_CNTRL                  0xF4CF0180  /**< CEVA Clock Control Register */
#define CEVA_CLK_DIV_CNTRL              0xF4CF0184  /**< CEVA Clock Divider Control Register */
#define CEVA_CLK_RESET                  0xF4CF0188  /**< CEVA Reset Register */

#define SPACC_CLK_CNTRL                 0xF4CF0190  /**< SPACC Clock Control Register */
#define SPACC_CLK_DIV_CNTRL             0xF4CF0194  /**< SPACC Clock Divider Control Register */
#define SPACC_RESET                     0xF4CF0198  /**< SPACC Reset Register */

#define SASPA_CLK_CNTRL                 0xF4CF01A0  /**< SASPA Clock Control Register */
#define SASPA_CLK_DIV_CNTRL             0xF4CF01A4  /**< SASPA Clock Divider Control Register */
#define SASPA_RESET                     0xF4CF01A8  /**< SASPA Reset Register */

#define CRP_CLK_CNTRL                   0xF4CF0170  /**< CRP Clock Control Register */
#define CRP_CLK_DIV_CNTRL               0xF4CF0174  /**< CRP Clock Divider Control Register */
#define CRP_CLK_RESET                   0xF4CF0178  /**< CRP Reset Register */

#define CEVA_BM_CLK_CNTRL               0xF4CF0300  /**< CEVA Clock Control Register */
#define CEVA_BM_CLK_DIV_CNTRL           0xF4CF0304  /**< CEVA Clock Divider Control Register */
#define CRP_BM_CLK_CNTRL                0xF4CF0310  /**< CRP Clock Control Register */
#define CRP_BM_CLK_DIV_CNTRL            0xF4CF0314  /**< CRP Clock Divider Control Register */
#define FFT_BM_CLK_CNTRL                0xF4CF0320  /**< FFT Clock Control Register */
#define FFT_BM_CLK_DIV_CNTRL            0xF4CF0324  /**< FFT Clock Divider Control Register */

#define FEC_DL_DIV2_CLK_CNTRL           0xF4CF0330  /**< FEC Clock Control Register */
#define FEC_DL_DIV2_CLK_DIV_CNTRL       0xF4CF0334  /**< FEC Clock Divider Control Register */
#define FEC_DL_DIV2_UNDOCUMENTED        0xF4CF0338  /**< Unknown Divider Control Register */


#define TMR_AXI_RESET_1                 (1<<7)
#define UART_AXI_RESET_1                (1<<6)
#define I2CSPI_AXI_RESET_1              (1<<5)
#define TDM_AXI_RESET_1                 (1<<4)
#define FEC_AXI_RESET_1                 (1<<3)
#define SPACC_AXI_RESET_1               (1<<2)
#define IPSEC_AXI_RESET_1               (1<<1)
#define DUS_AXI_RESET_1                 (1<<0)



#define PLL_M_LSB(n)                   (0xF4CF01C0+0x20*n+0x00)
#define PLL_M_MSB(n)                   (0xF4CF01C0+0x20*n+0x04)
#define PLL_P(n)                       (0xF4CF01C0+0x20*n+0x08)
#define PLL_S(n)                       (0xF4CF01C0+0x20*n+0x0C)
#define PLL_CNTRL(n)                   (0xF4CF01C0+0x20*n+0x10)
#define PLL_STATUS(n)                  (0xF4CF01C0+0x20*n+0x1C)

#define PLL_SOURCE(pll)                 (((pll)<<1)|1)


#define	FEC_DL_BASEADDR                 0xF4D00000	
#define	FEC_UL_BASEADDR                 0xF4D10000

/*
 * RAD Group Slaves
 */

#define JDMA0_BASEADDR                  0xF5B80000
#define JESD0_BASEADDR                  0xF5B88000
#define JDMA1_BASEADDR                  0xF5B90000
#define JESD1_BASEADDR                  0xF5B98000
#define JDMA_BASEADDR(id)               (0xF5B80000+id*0x10000)
#define JESD_BASEADDR(id)               (0xF5B88000+id*0x10000)

#define CPRI0_BASEADDR                  0xF5B00000
#define CPRI1_BASEADDR                  0xF5B10000
#define CPRI2_BASEADDR                  0xF5B20000
#define CPRI3_BASEADDR                  0xF5B30000
#define CPRI_BASEADDR(id)               (0xF5B30000+id*0x10000)

#define CPDMA0_BASEADDR                 0xF5C00000
#define CPDMA1_BASEADDR                 0xF5C10000
#define CPDMA2_BASEADDR                 0xF5C20000
#define CPDMA3_BASEADDR                 0xF5C30000
#define CPDMA_BASEADDR(id)              (0xF5C00000+id*0x10000)

#define PCIE0_BASEADDR                  0xF5D00000
#define RAD_CFG_BASEADDR                0xF5E00000
#define GEMAC0_BASEADDR                 0xF5E60000
#define GEMAC1_BASEADDR                 0xFE190000
#define SERDES0_BASEADDR                0xF5E80000
#define SERDES1_BASEADDR                0xF5E90000
#define PCIESLAVE_BASEADDR              0xF6000000

#define PCIE0_SLV_AXI_BASE              0xF6000000
#define PCIE1_BASEADDR                  0xFA000000  // PCIe 1 has shared DBI access
#define PCIE1_SLV_AXI_BASE              0xFA200000

/*
 *  RAD cluster configuration registers
 */
#define RAD_CFG_PCIE_X4_CFG0        (RAD_CFG_BASEADDR + 0x200)
#define RAD_CFG_PCIE_X4_CFG1        (RAD_CFG_BASEADDR + 0x204)
#define RAD_CFG_PCIE_X4_CFG2        (RAD_CFG_BASEADDR + 0x208)
#define RAD_CFG_PCIE_X4_CFG3        (RAD_CFG_BASEADDR + 0x20C)
#define RAD_CFG_PCIE_X4_CFG4        (RAD_CFG_BASEADDR + 0x210)
#define RAD_CFG_PCIE_X4_CFG5        (RAD_CFG_BASEADDR + 0x214)
#define RAD_CFG_PCIE_X4_CFG6        (RAD_CFG_BASEADDR + 0x218)
#define RAD_CFG_PCIE_X4_CFG7        (RAD_CFG_BASEADDR + 0x21C)

#define RAD_CFG_PCIE_X4_STAT0       (RAD_CFG_BASEADDR + 0x224)
#define RAD_CFG_PCIE_X4_STAT1       (RAD_CFG_BASEADDR + 0x228)
#define RAD_CFG_PCIE_X4_STAT2       (RAD_CFG_BASEADDR + 0x22C)
#define RAD_CFG_PCIE_X4_STAT3       (RAD_CFG_BASEADDR + 0x230)
#define RAD_CFG_PCIE_X4_STAT4       (RAD_CFG_BASEADDR + 0x234)
#define RAD_CFG_PCIE_X4_STAT5       (RAD_CFG_BASEADDR + 0x238)
#define RAD_CFG_PCIE_X4_STAT6       (RAD_CFG_BASEADDR + 0x23C)

#define RAD_CFG_PCIE_X1_CFG0        (RAD_CFG_BASEADDR + 0x078)
#define RAD_CFG_PCIE_X1_CFG1        (RAD_CFG_BASEADDR + 0x09C)

#define RAD_CFG_PCIE_X1_STAT0       (RAD_CFG_BASEADDR + 0x100)
#define RAD_CFG_PCIE_X1_STAT1       (RAD_CFG_BASEADDR + 0x104)
#define RAD_CFG_PCIE_X1_STAT2       (RAD_CFG_BASEADDR + 0x110)
#define RAD_CFG_PCIE_X1_STAT5       (RAD_CFG_BASEADDR + 0x114)
#define RAD_CFG_PCIE_X1_STAT6       (RAD_CFG_BASEADDR + 0x118)



/*
 * Misc. macros to access TDM
 */
#define TDM_BASEADDR                    0xFE000000
#define TDMA_BASEADDR                   0xFE020000
#define TDM_BUS_BASEADDR(bus)           (TDM_BASEADDR + (bus << 12))
#define TDMA_BUS_BASEADDR(bus)          (TDMA_BASEADDR + (bus << 13))

/*
 * Timer control registers
*/
#define XPTIMER_BASEADDR                0xFE050000
#define TIMER0_CNTR_REG                 (XPTIMER_BASEADDR + 0x00)
#define TIMER0_CURR_COUNT               (XPTIMER_BASEADDR + 0x04)
#define TIMER1_CNTR_REG                 (XPTIMER_BASEADDR + 0x08)
#define TIMER1_CURR_COUNT               (XPTIMER_BASEADDR + 0x0C)
#define TIMER2_LBOUND_REG               (XPTIMER_BASEADDR + 0x10)
#define TIMER2_HBOUND_REG               (XPTIMER_BASEADDR + 0x14)
#define TIMER2_CNTR_REG                 (XPTIMER_BASEADDR + 0x18)
#define TIMER2_CURR_COUNT               (XPTIMER_BASEADDR + 0x1C)
#define TIMER3_LBOUND_REG               (XPTIMER_BASEADDR + 0x20)
#define TIMER3_HBOUND_REG               (XPTIMER_BASEADDR + 0x24)
#define TIMER3_CNTR_REG                 (XPTIMER_BASEADDR + 0x28)
#define TIMER3_CURR_COUNT               (XPTIMER_BASEADDR + 0x2C)

/*
 * General Purpose Input/Output (GPIO) block control registers
 * NOTE: Also controls some misc. devices as well.  See
 * specification for details.
 */
#define GPIO_BASEADDR                   0xFE070000
#define GPIO_OUTPUT_REG                 (GPIO_BASEADDR + 0x00)
#define GPIO_OE_REG                     (GPIO_BASEADDR + 0x04)
#define GPIO_INT_CONF_REG               (GPIO_BASEADDR + 0x08)
#define GPIO_INPUT_REG                  (GPIO_BASEADDR + 0x10)
#define APB_ACCESS_WS_REG               (GPIO_BASEADDR + 0x14)
#define SYSCONF_STAT_REG                (GPIO_BASEADDR + 0x18)
#define GPIO_INT_STAT_REG               (GPIO_BASEADDR + 0x1C)
#define GPIO_INT_MASK_REG               (GPIO_BASEADDR + 0x20)
#define GPIO_INT_CLEAR_REG              (GPIO_BASEADDR + 0x24)
#define TDM_MUX_CTRL_REG                (GPIO_BASEADDR + 0x28)
#define GPIO_MISC_CTRL_REG              (GPIO_BASEADDR + 0x34)
#define DDRC_AXI_CTRL_REG               (GPIO_BASEADDR + 0x38)
#define DDRC_STATUS_REG                 (GPIO_BASEADDR + 0x3C)
#define BOOTSTRAP_STATUS_REG            (GPIO_BASEADDR + 0x40)
#define BOOTSTRAP_OVERRIDE_REG          (GPIO_BASEADDR + 0x44)
#define DEVICE_ID_AND_VERSION_REG       (GPIO_BASEADDR + 0x50)
#define GPIO_15_0_PIN_SELECT_REG        (GPIO_BASEADDR + 0x58)
#define GPIO_31_16_PIN_SELECT_REG       (GPIO_BASEADDR + 0x5C)
#define MISC_PIN_SELECT_REG             (GPIO_BASEADDR + 0x60)
#define A9_AUTHENT_REG                  (GPIO_BASEADDR + 0x70)
#define A9_MISC_STAT_REG                (GPIO_BASEADDR + 0x78)
#define GPIO_MISC_STAT_REG              (GPIO_BASEADDR + 0x90)
#define GPIO_MISC_RAW_INT_STAT_REG      (GPIO_BASEADDR + 0x94)
#define GPIO_MISC_INT_SET_REG           (GPIO_BASEADDR + 0x98)
#define GPIO_MISC_INT_CLEAR_REG         (GPIO_BASEADDR + 0x9C)
#define GPIO_MISC_INT_MASK_REG          (GPIO_BASEADDR + 0xA0)
#define PAD_CONF0_REG                   (GPIO_BASEADDR + 0x100)
#define PAD_CONF1_REG                   (GPIO_BASEADDR + 0x104)
#define PAD_CONF2_REG                   (GPIO_BASEADDR + 0x108)
#define PAD_CONF3_REG                   (GPIO_BASEADDR + 0x10C)
#define PAD_CONF4_REG                   (GPIO_BASEADDR + 0x110)
#define PAD_CONF5_REG                   (GPIO_BASEADDR + 0x114)
#define PAD_CONF6_REG                   (GPIO_BASEADDR + 0x118)
#define PAD_CONF7_REG                   (GPIO_BASEADDR + 0x11C)
#define PAD_CONF8_REG                   (GPIO_BASEADDR + 0x120)
#define PAD_CONF9_REG                   (GPIO_BASEADDR + 0x124)
#define PAD_CONF10_REG                  (GPIO_BASEADDR + 0x128)
#define PAD_CONF11_REG                  (GPIO_BASEADDR + 0x12C)
#define PAD_CONF12_REG                  (GPIO_BASEADDR + 0x130)
#define PAD_CONF13_REG                  (GPIO_BASEADDR + 0x134)
#define PAD_CONF14_REG                  (GPIO_BASEADDR + 0x138)
#define PAD_CONF_REG(x)                 (PAD_CONF0_REG + (4*x))

#define DEVICE_ROM_REMAP                (1 << 2)

#define GPIO_PIN_SELECT_UART1           ((1 << 22) | (1 << 23))

#define GPIO_0          (1 << 0)
#define GPIO_1          (1 << 1)
#define GPIO_2          (1 << 2)
#define GPIO_3          (1 << 3)
#define GPIO_4          (1 << 4)
#define GPIO_5          (1 << 5)
#define GPIO_6          (1 << 6)
#define GPIO_7          (1 << 7)
#define GPIO_8          (1 << 8)
#define GPIO_9          (1 << 9)
#define GPIO_10         (1 << 10)
#define GPIO_11         (1 << 11)
#define GPIO_12         (1 << 12)
#define GPIO_13         (1 << 13)
#define GPIO_14         (1 << 14)
#define GPIO_15         (1 << 15)
#define GPIO_16         (1 << 16)
#define GPIO_17         (1 << 17)
#define GPIO_18         (1 << 18)
#define GPIO_19         (1 << 19)
#define GPIO_20         (1 << 20)
#define GPIO_21         (1 << 21)
#define GPIO_22         (1 << 22)
#define GPIO_23         (1 << 23)
#define GPIO_24         (1 << 24)
#define GPIO_25         (1 << 25)
#define GPIO_26         (1 << 26)
#define GPIO_27         (1 << 27)
#define GPIO_28         (1 << 28)
#define GPIO_29         (1 << 29)
#define GPIO_30         (1 << 30)
#define GPIO_31         ((UINT32)1 << 31)

#define GPIO_RISING_EDGE    1
#define GPIO_FALLING_EDGE   2
#define GPIO_LEVEL	        3

#define GPIO_MUX_I2C_SCL	(GPIO_0 | GPIO_1)
#define GPIO_MUX_I2C_SDA	(GPIO_2 | GPIO_3)

#define UART0_BASEADDR                  0xFE800000
#define UART1_BASEADDR                  0xFE090000
#define SPI_BASEADDR                    0xFE098000


/*
 * I2C controller regsiters
 */
#define I2C_BASEADDR                    0xFE09C000
#define I2C_ADDR_REG                    (I2C_BASEADDR + 0x00*4)
#define I2C_DATA_REG                    (I2C_BASEADDR + 0x01*4)
#define I2C_CNTR_REG                    (I2C_BASEADDR + 0x02*4)
#define I2C_STAT_REG                    (I2C_BASEADDR + 0x03*4)
#define I2C_CCRFS_REG                   (I2C_BASEADDR + 0x03*4)
#define I2C_XADDR_REG                   (I2C_BASEADDR + 0x04*4)
#define I2C_CCRH_REG                    (I2C_BASEADDR + 0x05*4)
#define I2C_SRST_REG                    (I2C_BASEADDR + 0x07*4)
#define I2C_CTRL                        (I2C_BASEADDR + 0x1000) // VLSI to confirm


/* I2C Control register bits */
#define I2C_IEN                         (1 << 7)
#define I2C_ENAB                        (1 << 6)
#define I2C_STA                         (1 << 5)
#define I2C_STP                         (1 << 4)
#define I2C_IFLG                        (1 << 3)
#define I2C_AAK                         (1 << 2)



#define USIM_BASEADDR                   0xFE0E0000


/*
 * Defines for controlling settings for External Expansion bus 
 * (Chip selects, timing, etc.)
 */
#define EXPBUS_BASEADDR                 0xFE100000
#define EXP_SW_RST_R                     (EXPBUS_BASEADDR + 0x00)
#define EXP_CS_EN_R                      (EXPBUS_BASEADDR + 0x04)
#define EXP_CS0_BASE_R                   (EXPBUS_BASEADDR + 0x08)
#define EXP_CS1_BASE_R                   (EXPBUS_BASEADDR + 0x0C)
#define EXP_CS2_BASE_R                   (EXPBUS_BASEADDR + 0x10)
#define EXP_CS3_BASE_R                   (EXPBUS_BASEADDR + 0x14)
#define EXP_CS4_BASE_R                   (EXPBUS_BASEADDR + 0x18)
#define EXP_CSx_BASE_R(x)                (EXP_CS0_BASE_R + (4*x))
#define EXP_CS0_SEG_R                    (EXPBUS_BASEADDR + 0x1C)
#define EXP_CS1_SEG_R                    (EXPBUS_BASEADDR + 0x20)
#define EXP_CS2_SEG_R                    (EXPBUS_BASEADDR + 0x24)
#define EXP_CS3_SEG_R                    (EXPBUS_BASEADDR + 0x28)
#define EXP_CS4_SEG_R                    (EXPBUS_BASEADDR + 0x2C)
#define EXP_CSx_SEG_R(x)                 (EXP_CS0_SEG_R + (4*x))
#define EXP_CS0_CFG_R                    (EXPBUS_BASEADDR + 0x30)
#define EXP_CS1_CFG_R                    (EXPBUS_BASEADDR + 0x34)
#define EXP_CS2_CFG_R                    (EXPBUS_BASEADDR + 0x38)
#define EXP_CS3_CFG_R                    (EXPBUS_BASEADDR + 0x3C)
#define EXP_CS4_CFG_R                    (EXPBUS_BASEADDR + 0x40)
#define EXP_CSx_CFG_R(x)                 (EXP_CS0_CFG_R + (4*x))
#define EXP_CS0_TMG1_R                   (EXPBUS_BASEADDR + 0x44)
#define EXP_CS1_TMG1_R                   (EXPBUS_BASEADDR + 0x48)
#define EXP_CS2_TMG1_R                   (EXPBUS_BASEADDR + 0x4C)
#define EXP_CS3_TMG1_R                   (EXPBUS_BASEADDR + 0x50)
#define EXP_CS4_TMG1_R                   (EXPBUS_BASEADDR + 0x54)
#define EXP_CSx_TMG1_R(x)                (EXP_CS0_TMG1_R + (4*x))
#define EXP_CS0_TMG2_R                   (EXPBUS_BASEADDR + 0x58)
#define EXP_CS1_TMG2_R                   (EXPBUS_BASEADDR + 0x5C)
#define EXP_CS2_TMG2_R                   (EXPBUS_BASEADDR + 0x60)
#define EXP_CS3_TMG2_R                   (EXPBUS_BASEADDR + 0x64)
#define EXP_CS4_TMG2_R                   (EXPBUS_BASEADDR + 0x68)
#define EXP_CSx_TMG2_R(x)                (EXP_CS0_TMG2_R + (4*x))
#define EXP_CS0_TMG3_R                   (EXPBUS_BASEADDR + 0x6C)
#define EXP_CS1_TMG3_R                   (EXPBUS_BASEADDR + 0x70)
#define EXP_CS2_TMG3_R                   (EXPBUS_BASEADDR + 0x74)
#define EXP_CS3_TMG3_R                   (EXPBUS_BASEADDR + 0x78)
#define EXP_CS4_TMG3_R                   (EXPBUS_BASEADDR + 0x7C)
#define EXP_CSx_TMG3_R(x)                (EXP_CS0_TMG3_R + (4*x))
#define EXP_CLOCK_DIV_R                  (EXPBUS_BASEADDR + 0x80)
#define EXP_MFSM_R                       (EXPBUS_BASEADDR + 0x100)
#define EXP_CSFSM_R                      (EXPBUS_BASEADDR + 0x104)
#define EXP_WRSM_R                       (EXPBUS_BASEADDR + 0x108)
#define EXP_RDSM_R                       (EXPBUS_BASEADDR + 0x10C)

/* EXP_SWRST_R register*/
#define EXP_SW_RST                       0x00000001

#define EXP_CLK_EN                       0x00000001
#define EXP_CSBOOT_EN                    0x00000002

/* EXP_CS_EN_R register*/
#define EXP_CS0_EN                       0x00000002
#define EXP_CS1_EN                       0x00000004
#define EXP_CS2_EN                       0x00000008
#define EXP_CS3_EN                       0x00000010
#define EXP_CS4_EN                       0x00000020
#define EXP_CS_LEVEL                     0x00000008
#define EXP_CS_HIGH                      0x00000008
#define EXP_WE_HIGH                      0x00000010
#define EXP_RE_HIGH                      0x00000020
#define EXP_ALE_MODE                     0x00000040
#define EXP_STRB_MODE                    0x00000080
#define EXP_DM_MODE                      0x00000100
#define EXP_NAND_MODE                    0x00000200
#define EXP_RDY_EN                       0x00000400
#define EXP_RDY_EDG                      0x00000800
#define EXP_RE_CMD_LVL                   0x00000020
#define EXP_WE_CMD_LVL                   0x00000010
#define EXP_MEM_BUS_SIZE                 0x00000006
#define EXP_MEM_BUS_SIZE_32              0x00000004
#define EXP_MEM_BUS_SIZE_1               0x00000002
#define EXP_MEM_BUS_SIZE_8               0x00000000


/*
 * ARM Cortex-A9 Interrupt Distributor (A9IC) internal registers
 */
#define A9IC_SCU_BASEADDR               0xFFF00000
#define A9IC_CPUINT_BASEADDR            0xFFF00100
#define A9IC_WD_BASEADDR            	0xFFF00600
#define A9IC_DISTR_BASEADDR             0xFFF01000
#define A9IC_SCU_FILTER_START           0xFFF00040
#define A9IC_SCU_FILTER_END             0xFFF00044

#define A9IC_NUM_INTIDS                 160

// Local to core IRQs

#define HAL_IC_IRQ(id)                  (0 + (id))   // [0 - 15]
#define HAL_IRQ_LOCAL_TIMER             29

#define HAL_IRQ_SP804_TIMER_01          33
#define HAL_IRQ_SP804_TIMER_23          34

/** 
 * This macro converts interrupt line to 
 * SPI (Shared Peripheral Interrupt) ID 
 */
#define SPI_ID(x)                       ((x) + 32)

/*
 * Peripheral Interrupt Assignments
 */
#define HAL_IRQ_INTER_ARM_0             SPI_ID(0)
#define HAL_IRQ_INTER_ARM_1             SPI_ID(1)
#define HAL_IRQ_INTER_ARM_2             SPI_ID(2)
#define HAL_IRQ_INTER_ARM_3             SPI_ID(3)
#define HAL_IRQ_INTER_ARM_4             SPI_ID(4)
#define HAL_IRQ_INTER_ARM_5             SPI_ID(5)
#define HAL_IRQ_INTER_ARM_6             SPI_ID(6)
#define HAL_IRQ_INTER_ARM_7             SPI_ID(7)
#define SOFT_ICPU_FROM_UARM             (HAL_IRQ_INTER_ARM_6) /**< I-CPU from U-ARM message */
#define SOFT_ICPU_TO_UARM               (HAL_IRQ_INTER_ARM_7) /**< I-CPU to   U-ARM message */

#define HAL_IRQ_INTER_ARM(id)           (HAL_IRQ_INTER_ARM_0 + (id))
#define HAL_IRQ_ARM2_L2_CACHE           SPI_ID(8)
#define HAL_IRQ_ARM4_L2_CACHE           SPI_ID(9)
#define HAL_IRQ_ARM2_MONITOR            SPI_ID(10)
#define HAL_IRQ_ARM4_MONITOR            SPI_ID(11)
#define HAL_IRQ_FEC_DL_DMA              SPI_ID(12)
#define HAL_IRQ_FEC_UL_DMA              SPI_ID(14)
#define HAL_IRQ_SPAcc                   SPI_ID(16)
#define HAL_IRQ_ESPAH                   SPI_ID(18)
#define HAL_IRQ_DDR_CTRL                SPI_ID(20)
#define HAL_IRQ_MDMASYS0                SPI_ID(22)
#define HAL_IRQ_MDMASYS1                SPI_ID(24)
#define HAL_IRQ_CEVA_0                  SPI_ID(30)
#define HAL_IRQ_CEVA_1                  SPI_ID(31)
#define HAL_IRQ_CEVA(id)                (HAL_IRQ_CEVA_0 + (id))

#define HAL_IRQ_FPDMA0_OUT              SPI_ID(44)
#define HAL_IRQ_FPDMA0_IN               SPI_ID(45)
#define HAL_IRQ_FPDMA1_OUT              SPI_ID(46)
#define HAL_IRQ_FPDMA1_IN               SPI_ID(47)
#define HAL_IRQ_FP_TO_ARM               SPI_ID(48)
#define HAL_IRQ_MDMA_SPU                SPI_ID(49)

#define HAL_IRQ_CPRI_0                  SPI_ID(50)
#define HAL_IRQ_CPRI_1                  SPI_ID(51)
#define HAL_IRQ_CPRI_2                  SPI_ID(52)
#define HAL_IRQ_CPRI_3                  SPI_ID(53)
#define HAL_IRQ_CPRI_4                  SPI_ID(54)
#define HAL_IRQ_CPRI_5                  SPI_ID(55)

#define HAL_IRQ_CPDMA_TX_0              SPI_ID(56)
#define HAL_IRQ_CPDMA_TX_1              SPI_ID(57)
#define HAL_IRQ_CPDMA_TX_2              SPI_ID(58)
#define HAL_IRQ_CPDMA_TX_3              SPI_ID(59)
#define HAL_IRQ_CPDMA_TX_4              SPI_ID(60)
#define HAL_IRQ_CPDMA_TX_5              SPI_ID(61)

#define HAL_IRQ_CPDMA_RX_0              SPI_ID(62)
#define HAL_IRQ_CPDMA_RX_1              SPI_ID(63)
#define HAL_IRQ_CPDMA_RX_2              SPI_ID(64)
#define HAL_IRQ_CPDMA_RX_3              SPI_ID(65)
#define HAL_IRQ_CPDMA_RX_4              SPI_ID(66)
#define HAL_IRQ_CPDMA_RX_5              SPI_ID(67)

#define HAL_IRQ_SRIO_0                  SPI_ID(80)
#define HAL_IRQ_SRIO_1                  SPI_ID(81)

#define HAL_IRQ_PCIE_X1                 SPI_ID(72)
#define HAL_IRQ_PCIE_X4                 SPI_ID(73)
#define HAL_IRQ_PCIE_DMA                SPI_ID(74)

#define HAL_IRQ_MDMA_RAD                SPI_ID(75)
#define HAL_IRQ_FSYNC                   SPI_ID(76)
#define HAL_IRQ_FSYNC_CHIP              SPI_ID(77)
#define HAL_IRQ_FSYNC_SLOT              SPI_ID(78)
#define HAL_IRQ_FSYNC_FRAME             SPI_ID(79)

#define HAL_IRQ_GEM_0                   SPI_ID(100)
#define HAL_IRQ_ADM_0                   SPI_ID(101)
#define HAL_IRQ_GEM_1                   SPI_ID(102)
#define HAL_IRQ_ADM_1                   SPI_ID(103)

#define HAL_IRQ_SGMII_SERDES            SPI_ID(104)
#define HAL_IRQ_GEM_NTG                 SPI_ID(105)

#define HAL_IRQ_GPIO_0                  SPI_ID(106)
#define HAL_IRQ_GPIO_1                  SPI_ID(107)
#define HAL_IRQ_GPIO_2                  SPI_ID(108)
#define HAL_IRQ_GPIO_3                  SPI_ID(109)
#define HAL_IRQ_GPIO(id)                (HAL_IRQ_GPIO_0 + (id))
#define HAL_IRQ_GPIO_COMB               SPI_ID(110)

#define HAL_IRQ_TIMER                   SPI_ID(112)
#define HAL_IRQ_TDM_TIMER               SPI_ID(113)

#define HAL_IRQ_UART_0                  SPI_ID(114)
#define HAL_IRQ_UART_1                  SPI_ID(115)
#define HAL_IRQ_UART_2                  SPI_ID(116)
#define HAL_IRQ_UART(id)                (HAL_IRQ_UART_0 + (id))

#define HAL_IRQ_SPI                     SPI_ID(117)
#define HAL_IRQ_I2C                     SPI_ID(118)
#define HAL_IRQ_HBI                     SPI_ID(119)    /**< HostBus mailbox irq */

#define HAL_IRQ_TDM_NTG                 SPI_ID(121)
#define HAL_IRQ_TDM_SIG                 SPI_ID(122)
#define HAL_IRQ_TDM_ERR_RX              SPI_ID(123)
#define HAL_IRQ_TDM_ERR_TX              SPI_ID(124)
#define HAL_IRQ_TDMA                    SPI_ID(125)
#define HAL_IRQ_TDM_TX                  SPI_ID(126)
#define HAL_IRQ_TDM_RX                  SPI_ID(127)

#define HAL_IRQPRI(pri)                 (pri<<3)

/*
 * ARM Level 2 Cache Controller (L2CC) defines
 */
#define L2CC_BASE                       0xFFF10000
#define L2CC_ID                         (L2CC_BASE + 0x000)
#define L2CC_TYPE                       (L2CC_BASE + 0x004)
#define L2CC_CTRL                       (L2CC_BASE + 0x100)
#define L2CC_AUX_CTRL                   (L2CC_BASE + 0x104)
#define L2CC_TAG_LATENCY_CTRL           (L2CC_BASE + 0x108)
#define L2CC_DATA_LATENCY_CTRL          (L2CC_BASE + 0x10C)
#define L2CC_EVENT_CNT_CTRL             (L2CC_BASE + 0x200)
#define L2CC_EVENT_CNT1_CFG             (L2CC_BASE + 0x204)
#define L2CC_EVENT_CNT0_CFG             (L2CC_BASE + 0x208)
#define L2CC_EVENT_CNT1_VAL             (L2CC_BASE + 0x20C)
#define L2CC_EVENT_CNT0_VAL             (L2CC_BASE + 0x210)
#define L2CC_INTR_MASK                  (L2CC_BASE + 0x214)
#define L2CC_MASKED_INTR_STAT           (L2CC_BASE + 0x218)
#define L2CC_RAW_INTR_STAT              (L2CC_BASE + 0x21C)
#define L2CC_INTR_CLEAR                 (L2CC_BASE + 0x220)
#define L2CC_CACHE_SYNC                 (L2CC_BASE + 0x730)
#define L2CC_INVL_LINE_PA               (L2CC_BASE + 0x770)
#define L2CC_INVL_WAY                   (L2CC_BASE + 0x77C)
#define L2CC_CLEAN_LINE_PA              (L2CC_BASE + 0x7B0)
#define L2CC_CLEAN_INDX_WAY             (L2CC_BASE + 0x7B8)
#define L2CC_CLEAN_WAY                  (L2CC_BASE + 0x7BC)
#define L2CC_CLEAN_INVL_LINE_PA         (L2CC_BASE + 0x7F0)
#define L2CC_CLEAN_INVL_LINE_WAY_INDX   (L2CC_BASE + 0x7F8)
#define L2CC_CLEAN_INV_WAY              (L2CC_BASE + 0x7FC)
#define L2CC_FILTER_START               (L2CC_BASE + 0xC00)
#define L2CC_FILTER_END                 (L2CC_BASE + 0xC04)
#define L2CC_LINE_DATA                  (L2CC_BASE + 0xF10)
#define L2CC_LINE_TAG                   (L2CC_BASE + 0xF30)
#define L2CC_DEBUG_CTRL                 (L2CC_BASE + 0xF40)

// L2CC Interrupt bits
#define L2CC_INTR_ECNTR                 0x01

// L2CC Aux Control bits
#define L2CC_AUX_DOUBLE_LINE_FILL       (1<<30)
#define L2CC_AUX_CODE_PREF              (1<<29)
#define L2CC_AUX_DATA_PREF              (1<<28)
#define L2CC_AUX_WRITE_ALLOC            (2<<23)
#define L2CC_AUX_CTRL_EMBUS             (1<<20)


// L2CC Event Counter Control bits
#define L2CC_EVENT_CONTROL_ENABLE       0x1
#define L2CC_EVENT_CONTROL_RESET_ALL    0x6

// L2CC Event Counter Config bits
#define L2CC_EVENT_CONFIG_DISABLED      0x0
#define L2CC_EVENT_CONFIG_CO            (1<<2)
#define L2CC_EVENT_CONFIG_DRHIT         (2<<2)
#define L2CC_EVENT_CONFIG_DRREQ         (3<<2)
#define L2CC_EVENT_CONFIG_DWHIT         (4<<2)
#define L2CC_EVENT_CONFIG_DWREQ         (5<<2)
#define L2CC_EVENT_CONFIG_DWTREQ        (6<<2)
#define L2CC_EVENT_CONFIG_IRHIT         (7<<2)
#define L2CC_EVENT_CONFIG_IRREQ         (8<<2)
#define L2CC_EVENT_INTERRUPT_ON_INC     0x1
#define L2CC_EVENT_INTERRUPT_ON_OVF     0x2
#define L2CC_EVENT_INTERRUPT_DISABLED   0x3

#define CACHE_LINE_SIZE                 32

/**
 *
 * ARM Cortex-A9 Internal Interrupt Distributor address
 */
#define A9TMWD_BASEADDR                 0xfff00600

/*
 *  DMA Control Mask and IRQ Status registers 
 */
#define DMAIRQ_FREADY                   (1 << 0)
#define DMAIRQ_FLAST                    (1 << 1)
#define DMAIRQ_FDONE                    (1 << 2)
#define DMAIRQ_FCTRL                    (1 << 6)

#define FDESC_INTENA                    1


/*
 * Macros for making sure memory mapped registers are accessed
 * using c "volatile" keyword to force non-cached reads and writes
 */
#define REG32(regaddr)	                (*(volatile unsigned int*   )(regaddr))
#define REG16(regaddr)	                (*(volatile unsigned short* )(regaddr))
#define REG8(regaddr)	                (*(volatile unsigned char*  )(regaddr))

#define REG32CLR(addr, clr_mask) ( REG32(addr) = REG32(addr) & (~(clr_mask)) )
#define REG32SET(addr, set_mask) ( REG32(addr) = REG32(addr) | (set_mask) )
#define REG32UPD(addr, clr_mask, set_mask) ( REG32(addr) = (REG32(addr) & (~(clr_mask))) | (set_mask) )


/*
 * Security Protocol Accelerator (SPACC)
 */
#define SPACC_BASEADDR           0xFDE00000 /**< Internal Security Protocol Accelerator (SPACC) device base address */

#define SPACC_IRQ_EN            (SPACC_BASEADDR + 0x0000)
#define SPACC_IRQ_STAT          (SPACC_BASEADDR + 0x0004)
#define SPACC_IRQ_CTRL          (SPACC_BASEADDR + 0x0008)
#define SPACC_FIFO_STAT         (SPACC_BASEADDR + 0x000C)
#define SPACC_SDMA_BRST_SZ      (SPACC_BASEADDR + 0x0010)
#define SPACC_SRC_PTR           (SPACC_BASEADDR + 0x0020)
#define SPACC_DST_PTR           (SPACC_BASEADDR + 0x0024)
#define SPACC_OFFSET            (SPACC_BASEADDR + 0x0028)
#define SPACC_PRE_AAD_LEN       (SPACC_BASEADDR + 0x002C)
#define SPACC_POST_AAD_LEN      (SPACC_BASEADDR + 0x0030)
#define SPACC_PROC_LEN          (SPACC_BASEADDR + 0x0034)
#define SPACC_ICV_LEN           (SPACC_BASEADDR + 0x0038)
#define SPACC_ICV_OFFSET        (SPACC_BASEADDR + 0x003C)
#define SPACC_IV_OFFSET         (SPACC_BASEADDR + 0x0040)
#define SPACC_SW_CTRL           (SPACC_BASEADDR + 0x0044)
#define SPACC_AUX_INFO          (SPACC_BASEADDR + 0x0048)
#define SPACC_CTRL              (SPACC_BASEADDR + 0x004C)
#define SPACC_STAT_POP          (SPACC_BASEADDR + 0x0050)
#define SPACC_STATUS            (SPACC_BASEADDR + 0x0054)
#define SPACC_KEY_SZ            (SPACC_BASEADDR + 0x0100)
#define SPACC_CIPH_KEY_STORE    (SPACC_BASEADDR + 0x4000)
#define SPACC_CIPH_KEY_STORE_ADDR (SPACC_BASEADDR + 0x4000)
#define SPACC_HASH_KEY_STORE    (SPACC_BASEADDR + 0x8000)


//IPSEC registers
#define IPSEC_BASEADDR         0xFDE80000

#define IPSEC_INT_EN            (IPSEC_BASEADDR + 0x0000)
#define IPSEC_INT_STAT          (IPSEC_BASEADDR + 0x0004)
#define IPSEC_SEC_REQ           (IPSEC_BASEADDR + 0x0008)
#define IPSEC_SEC_GNT           (IPSEC_BASEADDR + 0x000c)
#define IPSEC_SRM_GO            (IPSEC_BASEADDR + 0x0010)
#define IPSEC_SRM_RDY           (IPSEC_BASEADDR + 0x0014)
#define IPSEC_ENDIAN_CTRL       (IPSEC_BASEADDR + 0x0018)

#define IPSEC_OUT_SRC_PTR       (IPSEC_BASEADDR + 0x0020)
#define IPSEC_OUT_DST_PTR       (IPSEC_BASEADDR + 0x0024)
#define IPSEC_OUT_OFFSET        (IPSEC_BASEADDR + 0x0028)
#define IPSEC_OUT_ID            (IPSEC_BASEADDR + 0x002c)
#define IPSEC_OUT_SAI           (IPSEC_BASEADDR + 0x0030)
#define IPSEC_OUT_POP           (IPSEC_BASEADDR + 0x0038)
#define IPSEC_OUT_STAT          (IPSEC_BASEADDR + 0x003C)

#define IPSEC_IN_SRC_PTR        (IPSEC_BASEADDR + 0x0040)
#define IPSEC_IN_DST_PTR        (IPSEC_BASEADDR + 0x0044)
#define IPSEC_IN_OFFSET         (IPSEC_BASEADDR + 0x0048)
#define IPSEC_IN_ID             (IPSEC_BASEADDR + 0x004C)
#define IPSEC_IN_SAI            (IPSEC_BASEADDR + 0x0050)
#define IPSEC_IN_POP            (IPSEC_BASEADDR + 0x0058)
#define IPSEC_IN_STAT           (IPSEC_BASEADDR + 0x005C)

#define IPSEC_SRM_CMD_0         (IPSEC_BASEADDR + 0x0060)
#define IPSEC_SRM_CMD_1         (IPSEC_BASEADDR + 0x0064)
#define IPSEC_SRM_CMD_2         (IPSEC_BASEADDR + 0x0068)
#define IPSEC_SRM_RET_0         (IPSEC_BASEADDR + 0x0070)
#define IPSEC_SRM_RET_1         (IPSEC_BASEADDR + 0x0074)

#define IPSEC_IRQ_CTRL          (IPSEC_BASEADDR + 0x0080)
#define IPSEC_OUT_FIFO_STAT     (IPSEC_BASEADDR + 0x0088)
#define IPSEC_IN_FIFO_STAT      (IPSEC_BASEADDR + 0x008C)


#define SYS_CFG_BASEADDR        0xF4C00000
#define SYS_SEC_CFG_BASEADDR    0xF4C08000

#define SYS_SEC_CFG_BCR         (SYS_SEC_CFG_BASEADDR + 0x00)   /**< Strap Pin And Boot Control Register */
#define SYS_SEC_CFG_GPR         (SYS_SEC_CFG_BASEADDR + 0x04)   /**< General Purpose Register */
#define SYS_SEC_CFG_SBSCR       (SYS_SEC_CFG_BASEADDR + 0x10)   /**< Slave Block Secure Mode Enable Register */
#define SYS_SEC_CFG_MBSCR       (SYS_SEC_CFG_BASEADDR + 0x14)   /**< Master Block Secure Mode Enable Register */
#define SYS_SEC_CFG_IRAM_SMWR   (SYS_SEC_CFG_BASEADDR + 0x20)   /**< IRAM Secure Mode Window Register */
#define SYS_SEC_CFG_EXP_SMWR    (SYS_SEC_CFG_BASEADDR + 0x24)   /**< Expansion Bus Secure Mode Window Register */
#define SYS_SEC_CFG_TM0_CTL_REG (SYS_SEC_CFG_BASEADDR + 0x40)   /**< Secure Timer 0 Control Register */
#define SYS_SEC_CFG_TM0_CNT_REG (SYS_SEC_CFG_BASEADDR + 0x44)   /**< Secure Timer 0 Count Register */

#define SYS_CFG_CA9_CLK_RST_REG (SYS_CFG_BASEADDR + 0x00)       /**< Dual ARM Clock Reset Register */
#define SYS_CFG_DDR_WND         (SYS_CFG_BASEADDR + 0x10)       /**< DDR Address Space Window */
#define SYS_CFG_CEVAGRP_ACP_WND (SYS_CFG_BASEADDR + 0x30)       /**< ARM ACP Snoop Window For Ceva GP */
#define SYS_CFG_GEMBM_ACP_WND   (SYS_CFG_BASEADDR + 0x40)       /**< ARM ACP Snoop Window For GEM BM */
#define SYS_CFG_PCIEBM_ACP_WND  (SYS_CFG_BASEADDR + 0x44)       /**< ARM ACP Snoop Window For PCIe BM */
#define SYS_CFG_BM_ARB_PRI_REG1 (SYS_CFG_BASEADDR + 0x80)       /**< SYS BM Arbitration Priority 1 */
#define SYS_CFG_BM_ARB_PRI_REG2 (SYS_CFG_BASEADDR + 0x84)       /**< SYS BM Arbitration Priority 2 */
#define SYS_CFG_BM_ARB_PRI_REG3 (SYS_CFG_BASEADDR + 0x88)       /**< SYS BM Arbitration Priority 3 */
#define SYS_CFG_BM_BUS_SEL      (SYS_CFG_BASEADDR + 0x98)       /**< SYS BM Bus Select */
#define SYS_CFG_TRIG_SRC_ENA    (SYS_CFG_BASEADDR + 0x100)      /**< Cross Trigger Source Enable Register */
#define SYS_CFG_TRIG_RCV_ENA    (SYS_CFG_BASEADDR + 0x104)      /**< Cross Trigger Source Enable Register */
#define SYS_CFG_PCIE_C2C_INT    (SYS_CFG_BASEADDR + 0x110)      /**< PCIe Chip to Chip Interrupt Register */
#define SYS_CFG_ARM_REQ_SET_STAT (SYS_CFG_BASEADDR + 0x120      /**< ARM Request Set/Status Register */
#define SYS_CFG_ARM_REQ_CLEAR   (SYS_CFG_BASEADDR + 0x124)      /**< ARM Request Clear Register */
#define SYS_CFG_ARM_IRQ_ACK_SET (SYS_CFG_BASEADDR + 0x128)      /**< IRQ_ARM_ACK Set Register */
#define SYS_CFG_RAM_MARGIN_REG1 (SYS_CFG_BASEADDR + 0x130)      /**< RAM Memory Margin Register 1 */
#define SYS_CFG_RAM_MARGIN_REG2 (SYS_CFG_BASEADDR + 0x134)      /**< RAM Memory Margin Register 2 */
#define SYS_CFG_RAM_MARGIN_REG3 (SYS_CFG_BASEADDR + 0x138)      /**< RAM Memory Margin Register 3 */
#define SYS_CFG_RAM_MARGIN_REG4 (SYS_CFG_BASEADDR + 0x140)      /**< RAM Memory Margin Register 4 */
#define SYS_CFG_GP_REG0         (SYS_CFG_BASEADDR + 0x144)      /**< General Purpose Register 0 */
#define SYS_CFG_GP_REG1         (SYS_CFG_BASEADDR + 0x148)      /**< General Purpose Register 1 */
#define SYS_CFG_GP_REG2         (SYS_CFG_BASEADDR + 0x14C)      /**< General Purpose Register 2 */





// The following defines still need unification

#define IRAM_BASE                   IRAM_BASEADDR
#define I2C_BASE                    I2C_BASEADDR
#define RAD_BASIC_CFG_BASEADDR      RAD_CFG_BASEADDR
#define PCIE0_CFG_BASEADDR          PCIE0_BASEADDR
#define RAD_BASIC_CFG_BASEADDR      RAD_CFG_BASEADDR

#define UART0_BASE                  UART0_BASEADDR
#define MHZ                         1000000UL
// #define AHBCLK_HZ                100*MHZ // 200 - 250

#define TIMER_LOAD                  (A9IC_WD_BASEADDR+0x0)
#define TIMER_COUNT                 (A9IC_WD_BASEADDR+0x4)
#define TIMER_CONTROL               (A9IC_WD_BASEADDR+0x8)
#define TIMER_STATUS                (A9IC_WD_BASEADDR+0xC)

#define TIMER_CTRL_ENABLE           (1 << 0)
#define TIMER_CTRL_AUTO             (1 << 1)
#define TIMER_CTRL_INT_ENA          (1 << 2)

/** 140 MHz expansion bus / 1000 for timer 1 millisecond timeout feeding into timer 3 as
 * continuous millisecond timer
 */
#define TIMER0_TICKS_PER_SECOND         (1000000*get_bus_clock())
#define TIMER0_TICKS_PER_MILLISECOND    (1000*get_bus_clock())
#define TIMER0_TICKS_PER_MICROSECOND    (get_bus_clock())

#define TIMER1_CNTR_VALUE               (1000*get_bus_clock())   /**< Clock divided by 1000 to get 1000 ticks per second */


#define PIN_EXP_NAND_RDY                GPIO_29
#define AXICLK_HZ                       (get_bus_clock()*MHZ) // A9 200 - 250, CEVA 375-400
#define GPIO_EXP_MUX                    MISC_PIN_SELECT_REG // TDB
#define GPIO_PIN_SELECT_REG1            GPIO_31_16_PIN_SELECT_REG // TBD

/* GPIO Pin Select Pins */

#define SPI_S1_RXD	(1 << 31)
#define SPI_S1_TXD	(1 << 30)
#define EXP_NAND_RDY	(1 << 29)
#define EXP_NAND_CS	(1 << 28)
#define EXP_ALE		(1 << 27)
#define EXP_RDY		(1 << 26)
#define EXP_CS3_N	(1 << 25)
#define EXP_CS2_N	(1 << 24)
#define UART1_TX	(1 << 23)
#define UART1_RX	(1 << 22)
#define UART0_CTS_N	(1 << 21)
#define UART0_RTS_N	(1 << 20)
#define SPI_SS1_N	(1 << 19)
#define SPI_SS0_N	(1 << 18)
#define I2C_SDA		(1 << 17)
#define I2C_SCL		(1 << 16)

#define TDM_FSYNC	(1 << 15)
#define TDM_CLK		(1 << 14)
#define TDM1_FS		(1 << 13)
#define TDM1_CK		(1 << 12)
#define TDM0_FS		(1 << 11)
#define TDM0_CK		(1 << 10)

#define SPI_RXD     SPI_S1_RXD
#define SPI_TXD     SPI_S1_TXD

#define EXP_NAND_CLE    00000000000000000000000000000000000000000000000  
#define EXP_RDY_BSY     EXP_RDY

//#define mdelay(n)   udelay((n)*1000)

#endif /* _T2000_H_ */
