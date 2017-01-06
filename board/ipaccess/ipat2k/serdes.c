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

#if !defined (CONFIG_RTSM_ONLY)

#include <asm/arch/transcede-2200.h>
#include <asm/arch/serdes.h>

#define SRDS_WAIT(us)   udelay(us)

/**
	@brief  Initialize SERDES registers for PCIe mode, the same settings are used for rates 2500 and 5000
	@param  SrdsId - serdes ID to be initialized (0 - SERDES0 (PCIEx4), 1 - SERDES1 (PCIEx1)
	@return SYSRC_SUCCESS if succeeded
*/
u32 SerdesRegsInit_PCIe(u32 SrdsId)
{
	u32 baseaddr, laneaddr, lanes, lane, rc;

	switch(SrdsId)
	{
		case 0:
			baseaddr = SERDES0_BASEADDR;
			laneaddr = RAD_CFG_BASEADDR + 0x44;
			lanes = 4;
			break;

		case 1:
			baseaddr = SERDES1_BASEADDR;
			laneaddr = RAD_CFG_BASEADDR + 0x40;
			lanes = 1;
			break;

		default:
			return -1;
	}

	for(lane = 0; lane < lanes; lane++)
	{
		REG32(laneaddr) = lane;	 // select lane 0 - 3
		REG32(baseaddr + (101<<2))= 0xB7;
		REG32(baseaddr + (102<<2))= 0xC;
		REG32(baseaddr + (103<<2))= 0xC;
		REG32(baseaddr + (104<<2))= 0x1A;
		REG32(baseaddr + (105<<2))= 0x1A;
		REG32(baseaddr + (106<<2))= 0x2;
		REG32(baseaddr + (107<<2))= 0x11;
		REG32(baseaddr + (108<<2))= 0xCF;


		// This update is needed to set appropriate TX signal level
		// Real values in registers TXDRV_HLEV and TXDRV_LEVN differ from expected default values
		REG32(baseaddr + (0x14<<2)) &= ~(1 << 3);		 // clear TXDRV_LOCWREN (enable override)
		REG32(baseaddr + (0x12<<2)) = (0x13 << 3) | 0x5;  // set TXDRV_LEVN, TXDRV_HLEV

		//SET_SERDES_REG(baseaddr, TXDRV_LOCWREN, 0);
		//SET_SERDES_REG(baseaddr, TXDRV_HLEV, 0x5);
		//SET_SERDES_REG(baseaddr, TXDRV_LEVN, 0x13);

	}

	REG32(laneaddr) = 4;			// select common lane
	REG32(baseaddr + (111<<2)) = 0x99;
	REG32(baseaddr + (112<<2)) = 0x0;
	REG32(baseaddr + (113<<2)) = 0x76; // 52
	REG32(baseaddr + (114<<2)) = 0xB7;
	REG32(baseaddr + (115<<2)) = 0xC;
	REG32(baseaddr + (116<<2)) = 0xC;
	REG32(baseaddr + (117<<2)) = 0x1A;
	REG32(baseaddr + (118<<2)) = 0x1A;
	REG32(baseaddr + (119<<2)) = 0x6;
	REG32(baseaddr + (120<<2)) = 0x16;
	REG32(baseaddr + (121<<2)) = 0x8;
	REG32(baseaddr + (122<<2)) = 0x0;
	REG32(baseaddr + (123<<2)) = 0x8;
	REG32(baseaddr + (124<<2)) = 0x0;
	REG32(baseaddr + (125<<2)) = 0xFF;
	REG32(baseaddr + (126<<2)) = 0xB3;
	REG32(baseaddr + (127<<2)) = 0xf6;
	REG32(baseaddr + (128<<2)) = 0xD0;
	REG32(baseaddr + (129<<2)) = 0xEf;
	REG32(baseaddr + (130<<2)) =  0xFB;
	REG32(baseaddr + (131<<2)) =  0xFF;
	REG32(baseaddr + (132<<2)) =  0xFF;
	REG32(baseaddr + (133<<2)) =  0xFF;
	REG32(baseaddr + (134<<2)) =  0xFF;
	REG32(baseaddr + (135<<2)) =  0xFF;
	REG32(baseaddr + (136<<2)) =  0x17; //??
	REG32(baseaddr + (137<<2)) =  0xd5; //??
	REG32(baseaddr + (138<<2)) =  0xE2;
	REG32(baseaddr + (139<<2)) =  0xEF;
	REG32(baseaddr + (140<<2)) =  0xFB;
	REG32(baseaddr + (141<<2)) =  0xFB;
	REG32(baseaddr + (142<<2)) =  0xFF;
	REG32(baseaddr + (143<<2)) =  0xEF;
	REG32(baseaddr + (144<<2)) =  0xFF;
	REG32(baseaddr + (145<<2)) =  0xFF;
	REG32(baseaddr + (146<<2)) =  0x17;
	REG32(baseaddr + (147<<2)) =  0xD5;
	REG32(baseaddr + (148<<2)) =  0xE2;
	REG32(baseaddr + (149<<2)) =  0xEF;
	REG32(baseaddr + (150<<2)) =  0xFB;
	REG32(baseaddr + (151<<2)) =  0xFB;
	REG32(baseaddr + (152<<2)) =  0xFF;
	REG32(baseaddr + (153<<2)) =  0xEF;
	REG32(baseaddr + (154<<2)) =  0xFF;
	REG32(baseaddr + (155<<2)) =  0xFF;
	REG32(baseaddr + (156<<2)) =  0xFB;
	REG32(baseaddr + (157<<2)) =  0xFF;
	REG32(baseaddr + (158<<2)) =  0x3F;
	REG32(baseaddr + (159<<2)) =  0x0;
	REG32(baseaddr + (160<<2)) =  0x32;
	REG32(baseaddr + (161<<2)) =  0x0;
	REG32(baseaddr + (162<<2)) =  0x4;
	REG32(baseaddr + (163<<2)) =  0x5;
	REG32(baseaddr + (164<<2)) =  0x4;
	REG32(baseaddr + (165<<2)) =  0x0;
	REG32(baseaddr + (166<<2)) =  0x0;
	REG32(baseaddr + (167<<2)) =  0x8;
	REG32(baseaddr + (168<<2)) =  0x4;
	REG32(baseaddr + (169<<2)) =  0x0;
	REG32(baseaddr + (170<<2)) =  0x0;
	REG32(baseaddr + (171<<2)) =  0x4;
	REG32(baseaddr + (172<<2)) =  0x4;

	return 0;
}


u32 Serdes0PcieInit(void)
{
	u32 ln = 0;

	//
	//  Reset and take out of reset
	//
	REG32(RAD_CFG_BASEADDR + 0xC0) = 0x100;   // enable ICTL_PMA_TXENABLE_A, PMA selects reference clock
											  // based on internal register values
#if(1)
	// if need to re-initialize SERDES do reset before
	REG32(RAD_CFG_BASEADDR + 0x08) = 0x0;	 // set IRST_PMA_POR_B_A active
	REG32(RAD_CFG_BASEADDR + 0x220) = 0x0;	// set IRST_PIPE_RST_Lx_B_A active
	SRDS_WAIT(100);  // SRDS_WAIT for ~100 ns
#endif

	REG32(RAD_CFG_BASEADDR + 0x08) = 0x1; 	  // Remove IRST_PMA_POR_B_A
	SRDS_WAIT(250);

	SerdesRegsInit_PCIe(0);

	// set IRST_PIPE_RST_Lx_B_A active for all 4 lanes
	REG32(RAD_CFG_BASEADDR + 0x220) = 0xF;
	SRDS_WAIT(500);

	return 0;
}

void  Serdes1SgmiiInit(u32 SrdsLoopback)
{
	u32 status;
	// bit 0 Serdes X1 TX data (IDAT_MULTI_TXWORD_L0_ port) muxing
	// 0-TX data is driven by CPRI
	// 1-TX data is driven by SGMII
	// bit 1: Serdes X1 mode (ICTL_PCS_EN_NT port)
	// 0 - Multi mode (CPRI/SGMII)
	// 1 - PCIe PIPE mode
	// bits 6:4 Serdes X1 TX rate (Used in multi mode only) (ICTL_MULTI_TXRATE_L0_[2:0]  port)
	// 000 - Div by 8 mode, for rates lower than 1.25 Gbps
	// 001 - Div by 4 mode, for rates between 1.25 Gbps and 2.5 Gbps
	// 010 - Div by 2 mode, for rates between 2.5 Gbps and 5 Gbps
	// 011 - Div by 1 mode, for rates above 5 Gbps
	// bits 10:8	R/W 0x2 Serdes X1 RX rate (Used in multi mode only) (ICTL_MULTI_RXRATE_L0_ [2:0] port)
	// 000 - Div by 8 mode, for rates lower than 1.25 Gbps
	// 001 - Div by 4 mode, for rates between 1.25 Gbps and 2.5 Gbps
	// 010 - Div by 2 mode, for rates between 2.5 Gbps and 5 Gbps
	// 011 - Div by 1 mode, for rates above 5 Gbps
	// bits 14:12   R/W 0x3 Serdes X1 TX data width (Used in multi mode only) (ICTL_MULTI_TXDATAWIDTH_L0_[2:0]])
	// 000 - 8 bit
	// 001 - 10 bit
	// 010 - 16 bit
	// 011 - 20 bit
	// 100 - 32 bit
	// 101 - 40 bit
	// bits 18:16   R/W 0x3 Serdes X1 RX data width (Used in multi mode only) (ICTL_MULTI_TXDATAWIDTH_L0_[2:0]])
	// 000 - 8 bit
	// 001 - 10 bit
	// 010 - 16 bit
	// 011 - 20 bit
	// 100 - 32 bit
	// 101 - 40 bit

	REG32(RAD_CFG_BASEADDR+0x98) = 0x11111; // serdes_cfg_6 Serdes Tx and Rx Rate
	REG32(RAD_CFG_BASEADDR+0x04) = 0x1;  // SRDS Reset

	udelay(1000);

	// Data rate settings
	REG32(RAD_CFG_BASEADDR+0x40) = 0x0; // select Lane 0
	udelay(10);

	REG32(SERDES1_BASEADDR+(101<<2)) = 0xB7; // 0x65
	REG32(SERDES1_BASEADDR+(102<<2)) = 0xC; // 0x66
	REG32(SERDES1_BASEADDR+(103<<2)) = 0xC; // 0x67
	REG32(SERDES1_BASEADDR+(104<<2)) = 0x1A; // 0x68
	REG32(SERDES1_BASEADDR+(105<<2)) = 0x1A; // 0x69
	REG32(SERDES1_BASEADDR+(106<<2)) = 0x2; // 0x6A
	REG32(SERDES1_BASEADDR+(107<<2)) = 0x11; // 0x6B
	REG32(SERDES1_BASEADDR+(108<<2)) = 0xCF; // 0x6C

	REG32(RAD_CFG_BASEADDR+0x40) = 0x4; // select Common Lane
	udelay(10);

	REG32(SERDES1_BASEADDR+(111<<2)) = 0x99; // 0x6F
	REG32(SERDES1_BASEADDR+(112<<2)) = 0x0; // 0x70
	REG32(SERDES1_BASEADDR+(113<<2)) = 0x76; // 0x71
	REG32(SERDES1_BASEADDR+(114<<2)) = 0xB7; // 0x72
	REG32(SERDES1_BASEADDR+(115<<2)) =  0xC; // 0x73
	REG32(SERDES1_BASEADDR+(116<<2)) =  0xC; // 0x74
	REG32(SERDES1_BASEADDR+(117<<2)) =  0x1A; // 0x75
	REG32(SERDES1_BASEADDR+(118<<2)) =  0x1A; // 0x76
	REG32(SERDES1_BASEADDR+(119<<2)) =  0x6; //4  // 0x77
	REG32(SERDES1_BASEADDR+(120<<2)) =  0x16; // 0x78
	REG32(SERDES1_BASEADDR+(121<<2)) =  0x8; // 0x79
	REG32(SERDES1_BASEADDR+(122<<2)) =  0x0; // 0x7A
	REG32(SERDES1_BASEADDR+(123<<2)) =  0x8; // 0x7B
	REG32(SERDES1_BASEADDR+(124<<2)) =  0x0; // 0x7C
	REG32(SERDES1_BASEADDR+(125<<2)) =  0xFF; // 0x7D
	REG32(SERDES1_BASEADDR+(126<<2)) =  0xB3; // 0x7E
	REG32(SERDES1_BASEADDR+(127<<2)) =  0xF6; // 0x7F
	REG32(SERDES1_BASEADDR+(128<<2)) =  0xD0; // 0x80
	REG32(SERDES1_BASEADDR+(129<<2)) =  0xEF; // 0x81
	REG32(SERDES1_BASEADDR+(130<<2)) =  0xFB; // 0x82
	REG32(SERDES1_BASEADDR+(131<<2)) =  0xFF; //0x83
	REG32(SERDES1_BASEADDR+(132<<2)) =  0xFF; //0x84
	REG32(SERDES1_BASEADDR+(133<<2)) =  0xFF; //0x85
	REG32(SERDES1_BASEADDR+(134<<2)) =  0xFF; //0x86
	REG32(SERDES1_BASEADDR+(135<<2)) =  0xFF; //0x87
	REG32(SERDES1_BASEADDR+(136<<2)) =  0x17; // 0x88
	REG32(SERDES1_BASEADDR+(137<<2)) =  0xD5; // 0x89
	REG32(SERDES1_BASEADDR+(138<<2)) =  0xE2; // 0x8A
	REG32(SERDES1_BASEADDR+(139<<2)) =  0xEF; // 0x8b
	REG32(SERDES1_BASEADDR+(140<<2)) =  0xFB; // 0x8c
	REG32(SERDES1_BASEADDR+(141<<2)) =  0xFB; // 0x8d
	REG32(SERDES1_BASEADDR+(142<<2)) =  0xFF; // 0x8e
	REG32(SERDES1_BASEADDR+(143<<2)) =  0xEF; // 0x8f
	REG32(SERDES1_BASEADDR+(144<<2)) =  0xFF; // 0x90
	REG32(SERDES1_BASEADDR+(145<<2)) =  0xFF; // 0x91
	REG32(SERDES1_BASEADDR+(146<<2)) =  0x17; // 0x92
	REG32(SERDES1_BASEADDR+(147<<2)) =  0xD5; // 0x93
	REG32(SERDES1_BASEADDR+(148<<2)) =  0xE2; // 0x94
	REG32(SERDES1_BASEADDR+(149<<2)) =  0xEF; // 0x95
	REG32(SERDES1_BASEADDR+(150<<2)) =  0xFB; // 0x96
	REG32(SERDES1_BASEADDR+(151<<2)) =  0xFB; // 0x97
	REG32(SERDES1_BASEADDR+(152<<2)) =  0xFF; // 0x98
	REG32(SERDES1_BASEADDR+(153<<2)) =  0xEF; // 0x99
	REG32(SERDES1_BASEADDR+(154<<2)) =  0xFF; // 0x9a
	REG32(SERDES1_BASEADDR+(155<<2)) =  0xFF; // 0x9b
	REG32(SERDES1_BASEADDR+(156<<2)) =  0xFB; // 0x9c
	REG32(SERDES1_BASEADDR+(157<<2)) =  0xFF; // 0x9d
	REG32(SERDES1_BASEADDR+(158<<2)) =  0x3F; // 0x9e
	REG32(SERDES1_BASEADDR+(159<<2)) =  0x0; // 0x9f
	REG32(SERDES1_BASEADDR+(160<<2)) =  0x32; // 0xa0
	REG32(SERDES1_BASEADDR+(161<<2)) =  0x0; // 0xa1
	REG32(SERDES1_BASEADDR+(162<<2)) =  0x5; // 0xa2
	REG32(SERDES1_BASEADDR+(163<<2)) =  0x5; // 0xa3
	REG32(SERDES1_BASEADDR+(164<<2)) =  0x4; // 0xa4
	REG32(SERDES1_BASEADDR+(165<<2)) =  0x0; // 0xa5
	REG32(SERDES1_BASEADDR+(166<<2)) =  0x0; // 0xa6
	REG32(SERDES1_BASEADDR+(167<<2)) =  0x8; // 0xa7
	REG32(SERDES1_BASEADDR+(168<<2)) =  0x4; // 0xa8
	REG32(SERDES1_BASEADDR+(169<<2)) =  0x0; // 0xa9
	REG32(SERDES1_BASEADDR+(170<<2)) =  0x0; // 0xaa
	REG32(SERDES1_BASEADDR+(171<<2)) =  0x4; // 0xab
	REG32(SERDES1_BASEADDR+(172<<2)) =  0x4; // 0xac

	if (SrdsLoopback)
	{
		REG32(RAD_CFG_BASEADDR+0x40) = 0x0; // select Lane 0
		REG32((SERDES1_BASEADDR + (0x0002 << 2))) = 0x0; // override enable
		REG32((SERDES1_BASEADDR + (0x00005 << 2))) = 0x20;
	}

	REG32(RAD_CFG_BASEADDR + 0x04) = 0xF;	 // remove serdes pcs reset


	// Check if Serdes1 is ready
	// 0 - Connected to OCTL_MULTI_SYNTHREADY_A port of X1 Serdes.
	// 1 - Connected to OCTL_MULTI_TXREADY_L0_A port of X1 Serdes.
	// 2 - Connected to OCTL_MULTI_RXREADY_L0_A port of X1 Serdes.
	// 3 - Connected to OCTL_MULTI_SYNTHSTATUS_A port of X1 Serdes.
	// 4 - Connected to OCTL_MULTI_TXSTATUS_L0_A port of X1 Serdes.
	// 5   Connected to OCTL_MULTI_RXREADY_L0_A port of X1 Serdes.

#if(0)
    // reading status may cause trap on boards with no SERDES1 clock
	do
		status = REG32(RAD_CFG_BASEADDR+ 0x11C);
	while (status != 0x3F);
#else
    udelay(5000);
#endif

}

#else // #if !defined (CONFIG_RTSM_ONLY)

u32 SerdesRegsInit_PCIe(u32 SrdsId)
{
	return 0;
}

u32 Serdes0PcieInit(void)
{
	return 0;
}

void  Serdes1SgmiiInit(u32 SrdsLoopback)
{
	return;
}

#endif // #if !defined (CONFIG_RTSM_ONLY) #else
