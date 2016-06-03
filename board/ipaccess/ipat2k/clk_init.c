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

/**
 * @T2200 Clock Frequencies

    Block               Original        New             Updated (worse case)
    CEVAs              800MHz       700MHz         500MHz
    MAPs                800MHz       700MHz         500MHz
    CEVA BM          400MHz       233.3MHz       166.67MHz
    Dual ARM          1GHz          800MHz          670MHz
    System BM        250MHz      160MHz          134MHz
    FEC DL              750MHz      675MHz          425MHz
    FEC UL              550MHz      400MHz          335MHz
    Security            250MHz      233.3MHz        223.3MHz
    MDMA CRP        400MHz      233.3MHz        175MHz
    CRP                  550MHz      500MHz           350MHz
    BSP                  400MHz      233.3MHz        166.67MHz
    CRAM               400MHz      233.3MHz        166.67MHz
    IRAM                250MHz      160MHz	           134MHz
    DDR                 800MHz      800MHz	           700MHz

*/

// T2200 Worse Case Clocks

// PLL 0 = 1000
//      DA9=fclk_1
//      L2CC=fclk_1
//      SYSBM=fclk_1/4
//      TPI=fclk_1/5
//      FEC UL=fclk_1/2
// PLL 1 = 800
//      DDR=fclk_0/2
//      IPSEC/SPACC=fclk_0/2
// PLL 2 = 699
//      FEC DL=fclk_2/1
// PLL 3 = 750
//      CEVA=fclk_3/1
//      MAP=fclk_1/1
//      SPUBM=CEVA/2

// PLL defines:
#define PLL_CTRL_RESET      (1<<0)
#define PLL_CTRL_BYPASS     (1<<4)
#define PLL_CTRL_LOCK_EN    (1<<5)
#define PLL_CTRL_VSEL       (1<<6)

#define PLL_STAT_LOCK       (1<<0)

#define T2200_REV_UNKNOWN   0
#define T2200_REV_X1        1
#define T2200_REV_X2        2
#define T2200_REV_X2_1      3

#define SYS_REF_CLK         25

#define X1_PLL0_FREQ        750
#define X1_PLL1_FREQ        700
#define X1_PLL2_FREQ        450
#define X1_PLL3_FREQ        600

#define X2_PLL0_FREQ        1000
#define X2_PLL1_FREQ        700
#define X2_PLL2_FREQ        600
#define X2_PLL3_FREQ        750

//
// Experimental, drop DDR PLL to 400 MHz and don't use divisor
// for X2.1
//

/*

PLL Min Max and Units (from PLL spec):

    Symbol  MIN      MAX  Unit
    FIN	      2      300   MHz
    P         1       63
    FREF    1.5       12   MHz
    M        64     1023
    FVCO    960     2060   MHz
    VSEL      0        1
    S         1	       5
    FOUT     30	    1030   MHz

    FIN is fixed in Minsdpeed EVMs and DM boards @ 25 MHz

    FREF is recommended to be as close as possible to 5 MHz
    so for this software assuming FIN at 25 MHz,
    "P" value is fixed at 5 MHz

*/

#define PLL0_P_VALUE    (SYS_REF_CLK/5)
#define PLL1_P_VALUE    (SYS_REF_CLK/5)
#define PLL2_P_VALUE    (SYS_REF_CLK/5)
#define PLL3_P_VALUE    (SYS_REF_CLK/5)

/*
    PLL 0
    Symbol  Value
    FIN	    25
    P	    5
    P       5
    FREF    5
    M       200
    FVCO    2000
    VSEL    1
    S       1
    FOUT    1000
*/

#define X2_1_PLL0_M_VALUE        200
#define X2_1_PLL0_VSEL_VALUE     PLL_CTRL_VSEL
#define X2_1_PLL0_S_VALUE        1

/*
    PLL 1
    Symbol	Value
    FIN	    25
    P	    5
    FREF	5
    M	    160
    FVCO	1600
    VSEL	1
    S	    2
    FOUT	400
*/
#define X2_1_PLL1_M_VALUE        160
#define X2_1_PLL1_VSEL_VALUE     PLL_CTRL_VSEL
#define X2_1_PLL1_S_VALUE        2


/*
PLL	2
    Symbol	Value
    FIN	    25
    P	    5
    FREF    5
    M       120
    FVCO    1200
    VSEL    0
    S       1
    FOUT    600
*/
#define X2_1_PLL2_M_VALUE        120
#define X2_1_PLL2_VSEL_VALUE     0
#define X2_1_PLL2_S_VALUE        1


/*
PLL	3
    Symbol	Value
    FIN	    25
    P	    5
    FREF    5
    M       150
    FVCO    1500
    VSEL    0
    S       1
    FOUT    750
*/
#define X2_1_PLL3_M_VALUE        150
#define X2_1_PLL3_VSEL_VALUE     PLL_CTRL_VSEL
#define X2_1_PLL3_S_VALUE        1

#define X2_1_PLL0_FREQ      1000
#define X2_1_PLL1_FREQ      400
#define X2_1_PLL2_FREQ      600
#define X2_1_PLL3_FREQ      750

#define CA9_MC_PLL          0
#define L2CC_PLL            0
#define TPI_PLL             0
#define CSYS_PLL            0
#define CRP_PLL             0
#define FEC_UL_PLL          0
#define GEM_TX_PLL          0
#define IPSEC_PLL           1
#define SPACC_PLL           1
#define DDR_PLL             1
#define FEC_DL_PLL          2
#define CEVA_GROUP_PLL      3

#define DDR_PLL_DIV_CNTRL   0x82


int get_t2200_rev(void)
{
    if(REG32(0xFF000020) == 0x8b32)
        return  T2200_REV_X2_1;
    if (REG32(0xFF0000C0) == 0xFF003EFC)
        return T2200_REV_X1;
    if (REG32(0xFF0000C0) == 0xFF000158)
       return T2200_REV_X2;
    return T2200_REV_UNKNOWN;
}

unsigned int get_bus_clock(void)
{
    unsigned int clock_val = SYS_REF_CLK * ((REG32(PLL_M_MSB(0)) << 8) | REG32(PLL_M_LSB(0))) / 5 / REG32(AXI_CLK_DIV_CNTRL);

    return clock_val;
}

unsigned int get_arm_clock(void)
{
    unsigned int clock_val = SYS_REF_CLK * ((REG32(PLL_M_MSB(0)) << 8) | REG32(PLL_M_LSB(0))) / 5;

    return clock_val;
}

unsigned int get_arm_periph_clock(void)
{
    unsigned int clock_val = SYS_REF_CLK * ((REG32(PLL_M_MSB(0)) << 8) | REG32(PLL_M_LSB(0))) / 5 / (REG32(CA9_MC_MPU_CLK_DIV_CNTRL) & 0xf);

    return clock_val;
}

void clk_init(void)
{
	/* 1 <= p <= 63, 64 <= m <= 1023, 1 <= s <= 5, 1.5MHz <= FIN / p <= 12MHz */
    u32 vco, m, s, pll_ctrl_val, delay;
    u32 ddr_pll_val;
    u32 ca9_pll_val;
    u32 fec_pll_val;
    u32 spu_pll_val;
    u32 pll0_vsel_val = 0;
    u32 pll1_vsel_val = 0;
    u32 pll2_vsel_val = 0;
    u32 pll3_vsel_val = 0;

    u32 dev_rev;
    u32 trap;

    dev_rev = get_t2200_rev();

    if (dev_rev == T2200_REV_X1) {
        ca9_pll_val = X1_PLL0_FREQ;
        ddr_pll_val = X1_PLL1_FREQ;
        fec_pll_val = X1_PLL2_FREQ;
        spu_pll_val = X1_PLL3_FREQ;
    } else if (dev_rev == T2200_REV_X2) {
        ca9_pll_val = X2_PLL0_FREQ;
        ddr_pll_val = X2_PLL1_FREQ;
        fec_pll_val = X2_PLL2_FREQ;
        spu_pll_val = X2_PLL3_FREQ;
    } else if (dev_rev == T2200_REV_X2_1) {
        // Set PLL frequencies
        ca9_pll_val = X2_1_PLL0_FREQ;
        ddr_pll_val = X2_1_PLL1_FREQ;
        fec_pll_val = X2_1_PLL2_FREQ;
        spu_pll_val = X2_1_PLL3_FREQ;
        // For 2.1 and later, set VSEL bits
        pll0_vsel_val = X2_1_PLL0_VSEL_VALUE;
        pll1_vsel_val = X2_1_PLL1_VSEL_VALUE;
        pll2_vsel_val = X2_1_PLL2_VSEL_VALUE;
        pll3_vsel_val = X2_1_PLL3_VSEL_VALUE;
    } else {
        trap = 1;
        while (trap);
    }

    pll_ctrl_val = PLL_CTRL_LOCK_EN | PLL_CTRL_BYPASS | PLL_CTRL_RESET;

    vco = ca9_pll_val;
    if (vco != X2_1_PLL0_FREQ) {
    m = 5 * vco / SYS_REF_CLK;
        s = 1;
    } else {
        m = X2_1_PLL0_M_VALUE;
        s = X2_1_PLL0_S_VALUE;
    }
    REG32(PLL_CNTRL(0)) = pll_ctrl_val | pll0_vsel_val; // always use high VCO, lock enable, bypass and reset
    REG32(PLL_M_LSB(0)) = m & 0xff;
    REG32(PLL_M_MSB(0)) = m >> 8;
    REG32(PLL_P(0)) = 5;
    REG32(PLL_S(0)) = s;

    vco = ddr_pll_val;
    if (vco != X2_1_PLL1_FREQ) {
    m = 5 * vco / SYS_REF_CLK;
        s = 1;
    } else {
        m = X2_1_PLL1_M_VALUE;
        s = X2_1_PLL1_S_VALUE;
    }
    REG32(PLL_CNTRL(1)) = pll_ctrl_val | pll1_vsel_val; // always use high VCO, lock enable, bypass and reset
    REG32(PLL_M_LSB(1)) = m & 0xff;
    REG32(PLL_M_MSB(1)) = m >> 8;
    REG32(PLL_P(1)) = 5;
    REG32(PLL_S(1)) = s;

    vco = fec_pll_val;
    if (vco != X2_1_PLL2_FREQ) {
    m = 5 * vco / SYS_REF_CLK;
        s = 1;
    } else {
        m = X2_1_PLL2_M_VALUE;
        s = X2_1_PLL2_S_VALUE;
    }
    REG32(PLL_CNTRL(2)) = pll_ctrl_val | pll2_vsel_val; // always use high VCO, lock enable, bypass and reset
    REG32(PLL_M_LSB(2)) = m & 0xff;
    REG32(PLL_M_MSB(2)) = m >> 8;
    REG32(PLL_P(2)) = 5;
    REG32(PLL_S(2)) = s;

    vco = spu_pll_val;
    if (vco != X2_1_PLL3_FREQ) {
    m = 5 * vco / SYS_REF_CLK;
        s = 1;
    } else {
        m = X2_1_PLL3_M_VALUE;
        s = X2_1_PLL3_S_VALUE;
    }
    REG32(PLL_CNTRL(3)) = pll_ctrl_val | pll3_vsel_val; // always use high VCO, lock enable, bypass and reset
    REG32(PLL_M_LSB(3)) = m & 0xff;
    REG32(PLL_M_MSB(3)) = m >> 8;
    REG32(PLL_P(3)) = 5;
    REG32(PLL_S(3)) = s;

    REG32(CA9_MC_MPU_CLK_DIV_CNTRL) = (dev_rev == T2200_REV_X1) ? 0x33 : 0x44; // need to keep the same ARM peripheral clock for X1 and X2
    REG32(CA9_MC_CLK_CNTRL) = PLL_SOURCE(CA9_MC_PLL);
    REG32(CA9_MC_CLK_DIV_CNTRL) = 0x82;

    REG32(L2CC_CLK_CNTRL) = PLL_SOURCE(L2CC_PLL);
    REG32(L2CC_CLK_DIV_CNTRL) = 0x82;

    REG32(TPI_CLK_CNTRL) = PLL_SOURCE(TPI_PLL);
    REG32(TPI_CLK_DIV_CNTRL) = 10;	// current clock fix for T3300 trace port

    REG32(CSYS_CLK_CNTRL) = PLL_SOURCE(CSYS_PLL);
    REG32(CSYS_CLK_DIV_CNTRL) = (dev_rev == T2200_REV_X2_1) ? 4 : 5;

    REG32(FEC_DL_CLK_CNTRL) = PLL_SOURCE(FEC_DL_PLL);
    REG32(AXI_CLK_CNTRL_1) |= FEC_AXI_RESET_1;
    REG32(FEC_DL_CLK_DIV_CNTRL) = 0x82;
    REG32(FEC_DL_DIV2_CLK_CNTRL) = 1;
    REG32(FEC_DL_DIV2_CLK_DIV_CNTRL) = 2;
    REG32(FEC_DL_DIV2_UNDOCUMENTED) = 0;

    REG32(FEC_UL_CLK_CNTRL) = PLL_SOURCE(FEC_UL_PLL);
    REG32(FEC_UL_CLK_DIV_CNTRL) = 0x2;

    REG32(IPSEC_CLK_CNTRL) = PLL_SOURCE(IPSEC_PLL);
    if ((IPSEC_PLL == 1) && (ddr_pll_val == X2_1_PLL1_FREQ))
        REG32(IPSEC_CLK_DIV_CNTRL) = 0x82; // Bypass
    else
        REG32(IPSEC_CLK_DIV_CNTRL) = 0x2;  // Divide by 2

    REG32(SPACC_CLK_CNTRL) = PLL_SOURCE(SPACC_PLL);
    if ((SPACC_PLL == 1) && (ddr_pll_val == X2_1_PLL1_FREQ))
        REG32(SPACC_CLK_DIV_CNTRL) = 0x82; // Bypass
    else
        REG32(SPACC_CLK_DIV_CNTRL) = 0x2;  // Divide by 2

    REG32(DDR3_CLK_CNTRL) = PLL_SOURCE(DDR_PLL);
    REG32(DDR3_CLK_DIV_CNTRL) = DDR_PLL_DIV_CNTRL;

    //
    // GEM TX clock (when used by GEM for TX instead
    // of external TX reference timing source)
    // must be exactly 125 MHz
    //
    // This means input PLL must be multiple of
    // 125 or 125, 250, 375, 500, 625, 750, 850 or 1000 MHz
    //
    REG32(GEMTX_CLK_DIV_CNTRL) = ca9_pll_val/125;
    REG32(GEMTX_CLK_CNTRL) = PLL_SOURCE(GEM_TX_PLL); // Assumes PLL 0 is multiple of 125 MHz
    REG32(GEMTX_RESET) = 0;

    REG32(CRP_CLK_CNTRL) =  PLL_SOURCE(CRP_PLL);
    REG32(CRP_CLK_DIV_CNTRL) = 0x2;
    REG32(CRP_CLK_RESET) = 0;

    REG32(FFT_CLK_CNTRL) = PLL_SOURCE(CEVA_GROUP_PLL);
    REG32(FFT_CLK_DIV_CNTRL) = 0x82; // Bypass

    REG32(CEVA_BM_CLK_CNTRL) = 3; // 0-AXI_Clk_Enable 1-AHB_Clk_Enable
    REG32(CEVA_BM_CLK_DIV_CNTRL) = 0x42; // 2:0-AXI_Clk_Div_Ratio 3-AXI_Clk_Div_Bypass 6:4-AHB_Clk_Div_Ratio 7-AHB_Clk_Div_Bypass

    REG32(CEVA_CLK_CNTRL) = PLL_SOURCE(CEVA_GROUP_PLL);
    REG32(CEVA_CLK_DIV_CNTRL) = 0x82;

    REG32(CEVA_CLK_RESET) = 0; // enable CEVA clock

    REG32(CRP_BM_CLK_CNTRL) = 1; // 0-AXI_Clk_Enable
    REG32(CRP_BM_CLK_DIV_CNTRL) = 0x2;

    REG32(SPU_CONFIG_BASEADDR) = 0x00110011; // take CRP MDMA out of reset - set bit 20

    REG32(FFT_BM_CLK_CNTRL) = 1; //0-AXI_Clk_Enable
    REG32(FFT_BM_CLK_DIV_CNTRL) = 2; // 2:0-AXI_Clk_Div_Ratio, FFT AXI BM Clock Division Ratio from FFT Clock

    // take PLL out of reset
    pll_ctrl_val = PLL_CTRL_LOCK_EN | PLL_CTRL_BYPASS;

    REG32(PLL_CNTRL(0)) = pll_ctrl_val | pll0_vsel_val;
    REG32(PLL_CNTRL(1)) = pll_ctrl_val | pll1_vsel_val;
    REG32(PLL_CNTRL(2)) = pll_ctrl_val | pll2_vsel_val;
    REG32(PLL_CNTRL(3)) = pll_ctrl_val | pll3_vsel_val;

    while ((REG32(PLL_STATUS(0)) & PLL_STAT_LOCK) == 0); // wait PLL0 to lock
    while ((REG32(PLL_STATUS(1)) & PLL_STAT_LOCK) == 0); // wait PLL1 to lock
    while ((REG32(PLL_STATUS(2)) & PLL_STAT_LOCK) == 0); // wait PLL2 to lock
    while ((REG32(PLL_STATUS(3)) & PLL_STAT_LOCK) == 0); // wait PLL2 to lock

    pll_ctrl_val = PLL_CTRL_LOCK_EN; // activate PLLs

    REG32(PLL_CNTRL(0)) = pll_ctrl_val | pll0_vsel_val;
    REG32(PLL_CNTRL(1)) = pll_ctrl_val | pll1_vsel_val;
    REG32(PLL_CNTRL(2)) = pll_ctrl_val | pll2_vsel_val;
    REG32(PLL_CNTRL(3)) = pll_ctrl_val | pll3_vsel_val;

    REG32(AXI_CLK_CNTRL_0) = (REG32(AXI_CLK_CNTRL_0) & 0xF0) | PLL_SOURCE(0);

    REG32(AXI_CLK_DIV_CNTRL) = (dev_rev == T2200_REV_X2_1) ? 4 : 5;

    // enable clocking from 4 PLLs
    REG32(PLLS_GLOBAL_CNTRL) &= ~(0xf<<0);

    REG32(AXI_RESET_0) = 0;
    REG32(AXI_RESET_1) = 0;
    REG32(AXI_RESET_2) = 0;

    delay = 10000;
    while (delay--)
        asm("nop"); // 10000 cycles delay
}

