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

#include <common.h>
#include <asm/arch/hardware.h>
#include <asm/arch/transcede-2200.h>
#include <asm/arch/pcie.h>
#include <asm/arch/serdes.h>

#ifdef CONFIG_PCIE
#ifndef CONFIG_RTSM_ONLY
#define PCIE_USE_DMA
#endif

#define TIMER0_CNTL_REG 0xFE050000
#define TIMER0_CURR_CNT 0xFE050004


/******************************************************************************
    @brief Set outbound address translarion for MEM requests (RC/EP).

    @param  PCIeBaseAddr    Base address of PCIe controller
    @param  RegionID        iATU OB region ID (0..PCIE_CX_ATU_NUM_OUTBOUND_REGIONS)
                            to configure
    @param  StartAddr       Start address of the region to be translated
    @param  LimitAddr       Last address of the region to be translated
    @param  TargetAddr      Target (translated) base address
******************************************************************************/
void PCIeDrvSetOBMemAT(u32 PCIeBaseAddr, u32 RegionID, u32 StartAddr, u32 LimitAddr, u32 TargetAddr)
{
    PIATU_REGISTERS iATU = (PIATU_REGISTERS)PCIE_PL_iATU_BASE(PCIeBaseAddr);

    // assert(RegionID < PCIE_CX_ATU_NUM_OUTBOUND_REGIONS);

    iATU->Viewport      = (RegionID & 0x0f);
    iATU->Ctrl2         = 0;               // disable AT for this region
    iATU->HiBaseAddr    = 0;
    iATU->LowBaseAddr   = StartAddr;
    iATU->LimitAddr     = LimitAddr;
    iATU->HiTargetAddr  = 0;
    iATU->LowTargetAddr = TargetAddr;
    iATU->Ctrl1 =   (PCIE_TLP_TYPE_MEM << 0) |
                    (0 << 5) |              // traffic class
                    (1 << 8) |              // add didgest to TLP
                    (0 << 9);               // attributes: no snoop, no relaxing order
    iATU->Ctrl2 = 0x80000000;               // enable AT for this region
}

/******************************************************************************
    @brief Set outbound address translarion for MEM->CFG0 requests (RC). Always
           mapped minimal (PCIE_IATU_MIN_REGION_SIZE) size

    @param  PCIeBaseAddr    Base address of PCIe controller
    @param  RegionID        iATU OB region ID (0..PCIE_CX_ATU_NUM_OUTBOUND_REGIONS)
                            to configure
    @param  StartAddr       Start address of the region to be translated
    @param  BDF             Bus number(8 bits), device number(5 bits), function
                            number (3 bits) - request ID
******************************************************************************/

void PCIeDrvSetOBCfg0AT(u32 PCIeBaseAddr,  u32 RegionID, u32 StartAddr, u16 BDF)
{
    PIATU_REGISTERS iATU = (PIATU_REGISTERS)PCIE_PL_iATU_BASE(PCIeBaseAddr);

    // assert( RegionID < PCIE_CX_ATU_NUM_OUTBOUND_REGIONS);

    iATU->Viewport      = (RegionID & 0x0f);
    iATU->Ctrl2 = 0;                        // disable AT for this region
    iATU->HiBaseAddr    = 0;
    iATU->LowBaseAddr   = StartAddr;
    iATU->LimitAddr     = StartAddr + PCIE_IATU_MIN_REGION_SIZE - 1;
    iATU->HiTargetAddr  = 0;
    iATU->LowTargetAddr = ((u32)BDF) << 16;
    iATU->Ctrl1 =   (PCIE_TLP_TYPE_CFG0 << 0) |
                    (0 << 5) |              // traffic class
                    (1 << 8) |              // add didgest to TLP
                    (0 << 9);               // attributes: no snoop, no relaxing order
    iATU->Ctrl2 = 0x80000000;               // enable AT for this region
}


/******************************************************************************
    @brief Set address inbound translarion, BAR matching mode (EP). Address
           matching configured BAR will be translated into address based on
           TargetAddr.

    @param  PCIeBaseAddr    Base address of PCIe controller
    @param  RegionID        iATU IB region ID (0..PCIE_CX_ATU_NUM_INBOUND_REGIONS)
                            to configure
    @param  BarNum          BAR number to be used for this region (0..5, 6 = ROM)
    @param  TargetAddr      Target (translated) base address
******************************************************************************/
void PCIeDrvSetIBBarAT(u32 PCIeBaseAddr, u32 RegionID, u32 BarNum, u32 TargetAddr)
{
    PIATU_REGISTERS iATU = (PIATU_REGISTERS)PCIE_PL_iATU_BASE(PCIeBaseAddr);

    // assert(RegionID < PCIE_CX_ATU_NUM_INBOUND_REGIONS);

    iATU->Viewport = (1 << 31) | (RegionID & 0x0f);
    iATU->Ctrl2 = 0;               // disable AT for this region
    iATU->HiTargetAddr = 0;
    iATU->LowTargetAddr = TargetAddr;
    iATU->Ctrl1 =   (PCIE_TLP_TYPE_MEM << 0);
    iATU->Ctrl2 =   ((BarNum & 0x7) << 8) | // BAR number
                    (1 << 30) |             // BAR match mode
                    (1 << 31);              // enable AT for this region
}

/******************************************************************************
    @brief Set address inbound translarion, address matching mode (RC).

    @param  PCIeBaseAddr    Base address of PCIe controller
    @param  RegionID        iATU IB region ID (0..PCIE_CX_ATU_NUM_INBOUND_REGIONS)
                            to configure
    @param  BaseAddr        Start address of the region to be translated (TLP address)
    @param  LimitAddr       Last address of the region to be translated (TLP address)
    @param  TargetAddr      Target (translated) base address (local)
******************************************************************************/
void PCIeDrvSetIBMemAT(u32 PCIeBaseAddr, u32 RegionID, u32 BaseAddr, u32 LimitAddr, u32 TargetAddr)
{
    PIATU_REGISTERS iATU = (PIATU_REGISTERS)PCIE_PL_iATU_BASE(PCIeBaseAddr);

    // assert(RegionID < PCIE_CX_ATU_NUM_INBOUND_REGIONS);

    iATU->Viewport = (1 << 31) | (RegionID & 0x0f);
    iATU->Ctrl2 = 0;                        // disable AT for this region
    iATU->HiBaseAddr    = 0;
    iATU->LowBaseAddr   = BaseAddr;
    iATU->LimitAddr     = LimitAddr;
    iATU->HiTargetAddr  = 0;                // local bus is 32 bit
    iATU->LowTargetAddr = TargetAddr;
    iATU->Ctrl1 =  (PCIE_TLP_TYPE_MEM << 0);
    iATU->Ctrl2 =  (1 << 31);               // enable AT for this region
}

/******************************************************************************
******************************************************************************/
int PCIeDrvX4Init(int RC_mode, int PCIeRate, int lanes)
{
    u32 lm;
    u32 TD = 1;                        // add ECRC
    u32 mode = RC_mode ? 0x04 : 0x00;  // mode selector

    Serdes0PcieInit();

    // configure PCIe x4
    REG32(RAD_CFG_PCIE_X4_CFG0) = (mode << 0) |     // RC/EP mode selection
                                  (0x07 << 4) |     // set signals to not active state
                                  (0x03 << 7) |     // map DECERR -> UR, SLVERR->UR
                                  (0x3f << 9) |     // map all CPL errors -> AXI SLVERR
                                  (TD << 23)  |     // add digest into read response
                                  (1 << 27);        // TODO: this bit is reserved, but set in VLSI code

    REG32(RAD_CFG_PCIE_X4_CFG1) = (TD << 7);        // AXI read transactions params
    REG32(RAD_CFG_PCIE_X4_CFG2) = (TD << 8);        // write response params
    REG32(RAD_CFG_PCIE_X4_CFG3) = (TD << 7);        // AXI write transaction params
    REG32(RAD_CFG_PCIE_X4_CFG4) = //(1 << 15) |       // force fast link mode
                                  (1 << 4);         // sys_aux_pwr_det
    ////REG32(RAD_CFG_PCIE_X4_CFG5) |= 0x4;             // app_ready_entr_l32

    REG32(RAD_CFG_PCIE_X4_CFG7) = 0x3;              // TODO: from VLSI code, no information on signals in documentation

	// change power state
	REG32(PCIE0_PM_CAP_BASE + 0x4) &= 0xfffffffc;   // do it to enable RC connect to EP started earlier

    // set lanes (see p.1283)
    REG32UPD(PCIE0_PL_G2CTRL_REG, 0x00001ff00, (lanes & 7) << 8); // Predetermined Number of Lanes
    if(lanes == 3) lm = 0x7;
    else lm = (lanes << 1) - 1;
    REG32UPD(PCIE0_PL_CTL_REG, 0x003f0000, lm << 16);             // Link Mode Enable

    // advertise only Gen1
    if(PCIeRate == 2500)
    {
        REG32UPD(PCIE_PCIE_LCAP_REG(PCIE0_PCIE_CAP_BASE), 0x0f, 0x01);
        REG32UPD(PCIE_PCIE_LCNT2_REG(PCIE0_PCIE_CAP_BASE), 0x0f, 0x01);
    }

    // disable crosslink
    REG32CLR(PCIE0_PL_CTL_REG, 1 << 22);

    // increasing the l0 & l1 entry times to 7 & 64 us
    // num. FTS = 0x16,  num.COM.FTS = 0x16, ACK_FREQ = 0x01
	// page 763 DW ref manual, please clean code.
    REG32(PCIE0_PL_AFL0L1_REG) =  0x3F161601 ;

    // Set number of FTS (Fast training sequences) symbols to 64 for Gen2
    REG32UPD(PCIE0_PL_G2CTRL_REG, 0xFF, 0x40);

    if(RC_mode)
    {
        // enable memory access from EP side
        REG32(PCIE0_BASEADDR + 0x4) |= 0x6; 	// Control register of RC
    }

    // app_ltssm_enable
    REG32(RAD_CFG_PCIE_X4_CFG5) = 2;  ////6;

    // enable link
    REG32SET(PCIE0_PL_CTL_REG, 1 << 5); // DLL link enable

    return 0;
}


void start_PCIEX1_link_up(void)
{
	PCIeX1_CONTROLLER_CONFIG_REG1 *x1c_cfg_reg1 = (PCIeX1_CONTROLLER_CONFIG_REG1 *)X1C_CFG_REG1;

	x1c_cfg_reg1->app_ltssm_enable = 0x1;
}

void start_PCIEX4_link_up(void)
{
	PCIeX4_CONTROLLER_CONFIG_REGISTERS *dwc_cfg_regs = (PCIeX4_CONTROLLER_CONFIG_REGISTERS *)DWC_CFG_BASE;

	dwc_cfg_regs->app_ltssm_enable = 0x1;
}

void wait_PCIEX1_link_up(void)
{
	PCIeX1_CONTROLLER_STATUS_REG0 *x1c_sts_reg0 = (PCIeX1_CONTROLLER_STATUS_REG0 *)X1C_STS_REG0;

	while(!(x1c_sts_reg0->rdlh_link_up))
	;
}

void wait_PCIEX4_link_up(void)
{
	PCIeX4_CONTROLLER_STATUS_REGISTERS *dwc_sts_regs = (PCIeX4_CONTROLLER_STATUS_REGISTERS *)DWC_STS_REG_BASE;

	while (!(dwc_sts_regs->rdlh_link_up))
	;
}

#if !defined (PCIE_USE_DMA)
/*
 ********************************************************************
 *   pcie_read ()
 *
 *   PCIe Driver Read API
 ******************************************************************
 */
PCIe_retcode pcie_read(u32 start_address, u8 *target_address, u32 len)
{
	memcpy(target_address, (void *) (PCIE0_SLV_AXI_BASE  + start_address), len);

	return RETCODE_OK;
}

/*
 ********************************************************************
 *   pcie_write ()
 *   ?? do we nead this, is it possiple ??
 *   PCIe Driver Write API
 ******************************************************************
 */
PCIe_retcode pcie_write(u32 src, u32 dst, u32 len)
{
	memcpy((void *) (PCIE0_SLV_AXI_BASE  + dst), (void *)src, len);

	return RETCODE_OK;
}
#else

PCIe_retcode pcie_read(u32 start_address, u8 *target_address, u32 len)
{
//	u32 timeout = 0;
	u32 val = 0;

	// enable dma to read & fill in f400_0000

	REG32(PCIE0_DMA_RD_ENG_EN_REG) = 0x0;
	REG32(PCIE0_DMA_RD_ENG_EN_REG) = 0x1;

	REG32(PCIE0_DMA_RD_INT_MSK_REG) = 0x0;
	REG32(PCIE0_DMA_CNL_CXT_IND_REG) = 0x80000000;	// read context
	REG32(PCIE0_DMA_CNL_CTRL_REG) = 0x04000008;	// | 1 << 3  : Local interrupt enable
							// | 1 << 26 : Traffic Digest TLP Header Bit (TD)

	REG32(PCIE0_DMA_TR_SIZE_REG) = len;

	REG32(PCIE0_DMA_SAR_LOW_REG) = (PCIE0_SLV_AXI_BASE  + start_address) & ~0x3;	// DWORD aligned
	REG32(PCIE0_DMA_SAR_HIGH_REG) = 0x00000000;
	REG32(PCIE0_DMA_DAR_LOW_REG) = (u32)target_address & ~0x3; // cram; DWORD aligned
	REG32(PCIE0_DMA_DAR_HIGH_REG) = 0x00000000;
	REG32(PCIE0_DMA_RD_DRBL_REG) = 0x0; // dma stop & start

	while (!(REG32(PCIE0_DMA_RD_INT_STAT_REG) & 0x00000001))
	;
//	    while( (REG32(PCIE0_DMA_CNL_CTRL_REG) & (1 << 6)) == 0);  // wait for DMA to complete

        /* Clear the Read Int */
	val = REG32(PCIE0_DMA_RD_INT_CLR_REG);
	REG32(PCIE0_DMA_RD_INT_CLR_REG) = val | 0x01;

    	return (REG32(PCIE0_DMA_CNL_CTRL_REG) & (1 << 5)) ? RETCODE_OK : RETCODE_ERROR;
}

PCIe_retcode pcie_write(u32 src, u32 dst, u32 len)
{
	u32 val = 0;

	// enable dma to write

	REG32(PCIE0_DMA_WR_ENG_EN_REG) = 0x0;
	REG32(PCIE0_DMA_WR_ENG_EN_REG) = 0x1;

	REG32(PCIE0_DMA_WR_INT_MSK_REG) = 0x0;
	REG32(PCIE0_DMA_CNL_CXT_IND_REG) = 0x0;		// write context
	REG32(PCIE0_DMA_CNL_CTRL_REG) = 0x04000008;	// | 1 << 3  : Local interrupt enable
							// | 1 << 26 : Traffic Digest TLP Header Bit (TD)
							// 	The PCIe core adds the ECRC field and sets the TD bit in the TLP header.no ecrc

	REG32(PCIE0_DMA_TR_SIZE_REG) = len;

	REG32(PCIE0_DMA_SAR_LOW_REG) = src & ~0x3; // cram; DWORD aligned
	REG32(PCIE0_DMA_SAR_HIGH_REG) = 0x00000000;
	REG32(PCIE0_DMA_DAR_LOW_REG) = (PCIE0_SLV_AXI_BASE  + dst) & ~0x3;	// DWORD aligned
	REG32(PCIE0_DMA_DAR_HIGH_REG) = 0x00000000;
	REG32(PCIE0_DMA_WR_DRBL_REG) = 0x0; // dma stop & start

	while (!(REG32(PCIE0_DMA_WR_INT_STAT_REG) & 0x00000001))
	;
//	    while( (REG32(PCIE0_DMA_CNL_CTRL_REG) & (1 << 6)) == 0);  // wait for DMA to complete

        /* Clear the Write Int */
	val = REG32(PCIE0_DMA_WR_INT_CLR_REG);
	REG32(PCIE0_DMA_WR_INT_CLR_REG) = val | 0x01;

    	return (REG32(PCIE0_DMA_CNL_CTRL_REG) & (1 << 5)) ? RETCODE_OK : RETCODE_ERROR;
}
#endif //defined (PCIE_USE_DMA)
#endif //defined CONFIG_PCIE

