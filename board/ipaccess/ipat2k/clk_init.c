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

#ifndef DEFAULT_SYS_REF_CLK_HZ
#define DEFAULT_SYS_REF_CLK_HZ  25000000
//#define DEFAULT_SYS_REF_CLK_HZ  19200000
#endif

#ifndef DEFAULT_SYS_REF_CLK_MHZ
#define DEFAULT_SYS_REF_CLK_MHZ (DEFAULT_SYS_REF_CLK_HZ / 1000000)
#endif

#if 0
#ifndef AUTO_DETECT_SYSCLK_FREQ
// New feature to allow for auto-detecting strapped
#define AUTO_DETECT_SYSCLK_FREQ
#endif
#endif

// T2200  Clock ratios

// PLL 0 = 1000
//      DA9=fclk_1
//      L2CC=fclk_1
//      SYSBM=fclk_1/4
//      TPI=fclk_1/5
//      FEC UL=fclk_1/2
// PLL 1 = 400
//      DDR=fclk_1
//      IPSEC/SPACC=fclk_1
// PLL 2 = 600
//      FEC DL=fclk_2/1
// PLL 3 = 750
//      CEVA=fclk_3/1
//      MAP=fclk_1/1
//      SPUBM=CEVA/2

// PLL control bit defines:
#define PLL_CTRL_RESET      (1<<0)
#define PLL_CTRL_BYPASS     (1<<4)
#define PLL_CTRL_LOCK_EN    (1<<5)
#define PLL_CTRL_VSEL       (1<<6)

// PLL status bit defines
#define PLL_STAT_LOCK       (1<<0)

// T2200/T3300 revisions
#define T2200_REV_UNKNOWN     0
#define T2200_REV_X1          1
#define T2200_REV_X2          2
#define T2200_REV_X2_1        3
#define T2200_REV_X2_1_HS_HP  4

// Tested/supported frequencies for Transcede ASIC revision X1
#define X1_PLL0_FREQ        750
#define X1_PLL1_FREQ        700
#define X1_PLL2_FREQ        450
#define X1_PLL3_FREQ        600

// Tested/supported frequencies for Transcede ASIC revsion X2
#define X2_PLL0_FREQ        1000
#define X2_PLL1_FREQ        700
#define X2_PLL2_FREQ        600
#define X2_PLL3_FREQ        750

//
// Frequencies for X2_1:
// Defines ported from diagnostics for PLL settings
// Now in specified values instead of calculated
// to allow for selecting optimal of PLL programming
// values using spreadsheet
//
// 1 <= p <= 63, 64 <= m <= 1023, 1 <= s <= 5, 1.5MHz <= FIN / p <= 12MHz
//

// PLL0 1 GHz
#define FREQ_HZ_19_2_MHZ_PLL0           1000000000
#define P_VALUE_19_2_MHZ_PLL0           12
#define M_VALUE_19_2_MHZ_PLL0           625
#define S_VALUE_19_2_MHZ_PLL0           1
#define VSEL_VALUE_19_2_MHZ_PLL0        PLL_CTRL_VSEL

// PLL1 400 MHz
#define FREQ_HZ_19_2_MHZ_PLL1           400000000
#define P_VALUE_19_2_MHZ_PLL1           6
#define M_VALUE_19_2_MHZ_PLL1           250
#define S_VALUE_19_2_MHZ_PLL1           2
#define VSEL_VALUE_19_2_MHZ_PLL1        PLL_CTRL_VSEL

// PLL2 600 MHz
#define FREQ_HZ_19_2_MHZ_PLL2           600000000
#define P_VALUE_19_2_MHZ_PLL2           4
#define M_VALUE_19_2_MHZ_PLL2           125
#define S_VALUE_19_2_MHZ_PLL2           1
#define VSEL_VALUE_19_2_MHZ_PLL2        0

// PLL3 748.8 MHz
#define FREQ_HZ_19_2_MHZ_PLL3           748800000
#define P_VALUE_19_2_MHZ_PLL3           4
#define M_VALUE_19_2_MHZ_PLL3           156
#define S_VALUE_19_2_MHZ_PLL3           1
#define VSEL_VALUE_19_2_MHZ_PLL3        PLL_CTRL_VSEL

// PLL0 1.17504 GHz - TURBO
#define FREQ_HZ_19_2_MHZ_TURBO_PLL0     1175040000
#define P_VALUE_19_2_MHZ_TURBO_PLL0     10
#define M_VALUE_19_2_MHZ_TURBO_PLL0     612
#define S_VALUE_19_2_MHZ_TURBO_PLL0     1
#define VSEL_VALUE_19_2_MHZ_TURBO_PLL0  PLL_CTRL_VSEL

// PLL3 874.97143 MHz - TURBO
#define FREQ_HZ_19_2_MHZ_TURBO_PLL3     874971430
#define P_VALUE_19_2_MHZ_TURBO_PLL3     7
#define M_VALUE_19_2_MHZ_TURBO_PLL3     319
#define S_VALUE_19_2_MHZ_TURBO_PLL3     1
#define VSEL_VALUE_19_2_MHZ_TURBO_PLL3  PLL_CTRL_VSEL

// PLL0 1 GHz
#define FREQ_HZ_25_MHZ_PLL0             1000000000
#define P_VALUE_25_MHZ_PLL0             5
#define M_VALUE_25_MHZ_PLL0             200
#define S_VALUE_25_MHZ_PLL0             1
#define VSEL_VALUE_25_MHZ_PLL0          PLL_CTRL_VSEL

// PLL1 400 MHz
#define FREQ_HZ_25_MHZ_PLL1             400000000
#define P_VALUE_25_MHZ_PLL1             5
#define M_VALUE_25_MHZ_PLL1             160
#define S_VALUE_25_MHZ_PLL1             2
#define VSEL_VALUE_25_MHZ_PLL1          PLL_CTRL_VSEL

// PLL2 600 MHz
#define FREQ_HZ_25_MHZ_PLL2             600000000
#define P_VALUE_25_MHZ_PLL2             5
#define M_VALUE_25_MHZ_PLL2             120
#define S_VALUE_25_MHZ_PLL2             1
#define VSEL_VALUE_25_MHZ_PLL2          0

// PLL3 750 MHz
#define FREQ_HZ_25_MHZ_PLL3             750000000
#define P_VALUE_25_MHZ_PLL3             5
#define M_VALUE_25_MHZ_PLL3             150
#define S_VALUE_25_MHZ_PLL3             1
#define VSEL_VALUE_25_MHZ_PLL3          PLL_CTRL_VSEL

// PLL0 1.175 GHz - TURBO
#define FREQ_HZ_25_MHZ_TURBO_PLL0       1175000000
#define P_VALUE_25_MHZ_TURBO_PLL0       5
#define M_VALUE_25_MHZ_TURBO_PLL0       235
#define S_VALUE_25_MHZ_TURBO_PLL0       1
#define VSEL_VALUE_25_MHZ_TURBO_PLL0    PLL_CTRL_VSEL

// PLL3 875 MHz - TURBO
#define FREQ_HZ_25_MHZ_TURBO_PLL3       875000000
#define P_VALUE_25_MHZ_TURBO_PLL3       5
#define M_VALUE_25_MHZ_TURBO_PLL3       175
#define S_VALUE_25_MHZ_TURBO_PLL3       1
#define VSEL_VALUE_25_MHZ_TURBO_PLL3    PLL_CTRL_VSEL

// PLL0 1 GHz
#define FREQ_HZ_26_MHZ_PLL0             1000000000
#define P_VALUE_26_MHZ_PLL0             13
#define M_VALUE_26_MHZ_PLL0             500
#define S_VALUE_26_MHZ_PLL0             1
#define VSEL_VALUE_26_MHZ_PLL0          PLL_CTRL_VSEL

// PLL1 400 MHz
#define FREQ_HZ_26_MHZ_PLL1             400000000
#define P_VALUE_26_MHZ_PLL1             13
#define M_VALUE_26_MHZ_PLL1             400
#define S_VALUE_26_MHZ_PLL1             2
#define VSEL_VALUE_26_MHZ_PLL1          PLL_CTRL_VSEL

// PLL2 600 MHz
#define FREQ_HZ_26_MHZ_PLL2             600000000
#define P_VALUE_26_MHZ_PLL2             13
#define M_VALUE_26_MHZ_PLL2             300
#define S_VALUE_26_MHZ_PLL2             1
#define VSEL_VALUE_26_MHZ_PLL2          0

// PLL3 750 MHz
#define FREQ_HZ_26_MHZ_PLL3             750000000
#define P_VALUE_26_MHZ_PLL3             13
#define M_VALUE_26_MHZ_PLL3             375
#define S_VALUE_26_MHZ_PLL3             1
#define VSEL_VALUE_26_MHZ_PLL3          PLL_CTRL_VSEL

// PLL0 1 GHz
#define FREQ_HZ_40_MHZ_PLL0             1000000000
#define P_VALUE_40_MHZ_PLL0             8
#define M_VALUE_40_MHZ_PLL0             200
#define S_VALUE_40_MHZ_PLL0             1
#define VSEL_VALUE_40_MHZ_PLL0          PLL_CTRL_VSEL

// PLL1 400 MHz
#define FREQ_HZ_40_MHZ_PLL1             400000000
#define P_VALUE_40_MHZ_PLL1             8
#define M_VALUE_40_MHZ_PLL1             160
#define S_VALUE_40_MHZ_PLL1             2
#define VSEL_VALUE_40_MHZ_PLL1          PLL_CTRL_VSEL

// PLL2 600 MHz
#define FREQ_HZ_40_MHZ_PLL2             600000000
#define P_VALUE_40_MHZ_PLL2             8
#define M_VALUE_40_MHZ_PLL2             120
#define S_VALUE_40_MHZ_PLL2             1
#define VSEL_VALUE_40_MHZ_PLL2          0

// PLL3 750 MHz
#define FREQ_HZ_40_MHZ_PLL3            750000000
#define P_VALUE_40_MHZ_PLL3            8
#define M_VALUE_40_MHZ_PLL3            150
#define S_VALUE_40_MHZ_PLL3            1
#define VSEL_VALUE_40_MHZ_PLL3         PLL_CTRL_VSEL

// For X2.1, DDR PLL clocked at 400 MHz, use bypass instead of divide by 2
// for DDR PLL division control for proper DDR3 speed
#define X2_1_DDR_PLL_DIV_CNTRL       0x82

#define EFUSE_REG_BASE               0xFE0F0000
#define EFUSE_REG_INST_CNT           (EFUSE_REG_BASE+0x0030)
#define EFUSE_REG_DOUT1              (EFUSE_REG_BASE+0x0024)
#define EFUSE_REG_BOND_WRD0          (EFUSE_REG_BASE+0x0025)


#define CA9_MC_PLL          0
#define DDR_PLL             1
#if 0
#define L2CC_PLL            0
#define TPI_PLL             0
#define CSYS_PLL            0
#define CRP_PLL             0
#define FEC_UL_PLL          0
#define GEM_TX_PLL          0
#define IPSEC_PLL           1
#define SPACC_PLL           1
#define FEC_DL_PLL          2
#define CEVA_GROUP_PLL      3
#endif


unsigned int get_efuse_inst0_wrd0(void)
{
    /* Write instance register with instance-0 */
    REG32(EFUSE_REG_INST_CNT) = 0;
    /* Read, Shift, Mask and return bonding word0 byte */
    return ((REG32(EFUSE_REG_DOUT1) >> 8) & 0xFF);
}


int get_t2200_rev(void)
{
    if (REG32(0xFF000020) == 0x8b32) {
        if (get_efuse_inst0_wrd0() == 0x1E)
            return T2200_REV_X2_1_HS_HP;
        else
            return  T2200_REV_X2_1;
    }
    if (REG32(0xFF0000C0) == 0xFF003EFC)
        return T2200_REV_X1;
    if (REG32(0xFF0000C0) == 0xFF000158)
       return T2200_REV_X2;
    return T2200_REV_UNKNOWN;
}

unsigned int get_sys_ref_clk_hz(void)
{
#define AUTO_DETECT_SYSCLK_FREQ
#ifdef AUTO_DETECT_SYSCLK_FREQ
    switch((REG32(SYSCONF_STAT_REG) >> 20) & 3)
    {
    case 0:
        return 19200000;
    case 1:
    default:
        return 25000000;
    case 2:
        return 26000000;
    case 3:
        return 40000000;
    }
#else
    return DEFAULT_SYS_REF_CLK_HZ;
#endif
}

unsigned int get_pll_clock_mhz(int pll_num)
{
    unsigned int clock_val;

    // Do all multiplies first (should not overrun)
    clock_val  = get_sys_ref_clk_hz() / 1000;
    clock_val *= ((REG32(PLL_M_MSB(pll_num)) << 8) | REG32(PLL_M_LSB(pll_num)));
    clock_val *= 2;
    // Now do all divides
    clock_val /= REG32(PLL_P(pll_num));
    clock_val /= (1 << (REG32(PLL_S(pll_num))));
    clock_val /= 1000;
    return clock_val;
}

unsigned int get_bus_clock(void)
{
    unsigned int clock_val = get_pll_clock_mhz(CA9_MC_PLL);

    clock_val /= REG32(AXI_CLK_DIV_CNTRL);

    return clock_val;
}

unsigned int get_arm_clock(void)
{
    unsigned int clock_val = get_pll_clock_mhz(CA9_MC_PLL);

    return clock_val;
}

unsigned int get_arm_periph_clock(void)
{
    unsigned int clock_val = get_pll_clock_mhz(CA9_MC_PLL);

    clock_val /= (REG32(CA9_MC_MPU_CLK_DIV_CNTRL) & 0xf);

    return clock_val;
}

void clk_init(void)
{
    unsigned int L2CC_PLL       = 0;
    unsigned int TPI_PLL        = 0;
    unsigned int CSYS_PLL       = 0;
    unsigned int CRP_PLL        = 0;
    unsigned int FEC_UL_PLL     = 0;
    unsigned int GEM_TX_PLL     = 0;
    unsigned int IPSEC_PLL      = 1;
    unsigned int SPACC_PLL      = 1;
    unsigned int FEC_DL_PLL     = 2;
    unsigned int CEVA_GROUP_PLL = 3;

    unsigned int vco, p, m, s, pll_ctrl_val, delay;
    unsigned int ddr_pll_val;
    unsigned int ca9_pll_val;
    unsigned int fec_pll_val;
    unsigned int spu_pll_val;

    unsigned int pll_vsel_val[4] = {0,0,0,0};
    unsigned int pll_p_val[4];
    unsigned int pll_s_val[4];
    unsigned int pll_m_val[4];
#if 0
    unsigned int pll_p_val[4]    = {P_VALUE_25_MHZ_PLL0,
                                    P_VALUE_25_MHZ_PLL1,
                                    P_VALUE_25_MHZ_PLL2,
                                    P_VALUE_25_MHZ_PLL3
    };

    unsigned int pll_s_val[4]    = {S_VALUE_25_MHZ_PLL0,
                                    S_VALUE_25_MHZ_PLL1,
                                    S_VALUE_25_MHZ_PLL2,
                                    S_VALUE_25_MHZ_PLL3
    };

    unsigned int pll_m_val[4]    = {M_VALUE_25_MHZ_PLL0,
                                    M_VALUE_25_MHZ_PLL1,
                                    M_VALUE_25_MHZ_PLL2,
                                    M_VALUE_25_MHZ_PLL3
    };
#endif

    unsigned int sys_ref_clk_hz = 25000000;

    unsigned int dev_rev;
    unsigned int trap;

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
        // Get system timing reference setting
        // and based on that, get table based
        // pointers and values to program PLLs
        // based on selected system reference timing
        sys_ref_clk_hz = get_sys_ref_clk_hz();
        switch(sys_ref_clk_hz) {
        case 19200000:
            pll_p_val[0]    = P_VALUE_19_2_MHZ_PLL0;
            pll_p_val[1]    = P_VALUE_19_2_MHZ_PLL1;
            pll_p_val[2]    = P_VALUE_19_2_MHZ_PLL2;
            pll_p_val[3]    = P_VALUE_19_2_MHZ_PLL3;
            pll_m_val[0]    = M_VALUE_19_2_MHZ_PLL0;
            pll_m_val[1]    = M_VALUE_19_2_MHZ_PLL1;
            pll_m_val[2]    = M_VALUE_19_2_MHZ_PLL2;
            pll_m_val[3]    = M_VALUE_19_2_MHZ_PLL3;
            pll_s_val[0]    = S_VALUE_19_2_MHZ_PLL0;
            pll_s_val[1]    = S_VALUE_19_2_MHZ_PLL1;
            pll_s_val[2]    = S_VALUE_19_2_MHZ_PLL2;
            pll_s_val[3]    = S_VALUE_19_2_MHZ_PLL3;
            pll_vsel_val[0] = VSEL_VALUE_19_2_MHZ_PLL0;
            pll_vsel_val[1] = VSEL_VALUE_19_2_MHZ_PLL1;
            pll_vsel_val[2] = VSEL_VALUE_19_2_MHZ_PLL2;
            pll_vsel_val[3] = VSEL_VALUE_19_2_MHZ_PLL3;
            ca9_pll_val     = FREQ_HZ_19_2_MHZ_PLL0/1000000;
            ddr_pll_val     = FREQ_HZ_19_2_MHZ_PLL1/1000000;
            fec_pll_val     = FREQ_HZ_19_2_MHZ_PLL2/1000000;
            spu_pll_val     = FREQ_HZ_19_2_MHZ_PLL3/1000000;
            break;
        case 25000000:
        default:
            pll_p_val[0]    = P_VALUE_25_MHZ_PLL0;
            pll_p_val[1]    = P_VALUE_25_MHZ_PLL1;
            pll_p_val[2]    = P_VALUE_25_MHZ_PLL2;
            pll_p_val[3]    = P_VALUE_25_MHZ_PLL3;
            pll_m_val[0]    = M_VALUE_25_MHZ_PLL0;
            pll_m_val[1]    = M_VALUE_25_MHZ_PLL1;
            pll_m_val[2]    = M_VALUE_25_MHZ_PLL2;
            pll_m_val[3]    = M_VALUE_25_MHZ_PLL3;
            pll_s_val[0]    = S_VALUE_25_MHZ_PLL0;
            pll_s_val[1]    = S_VALUE_25_MHZ_PLL1;
            pll_s_val[2]    = S_VALUE_25_MHZ_PLL2;
            pll_s_val[3]    = S_VALUE_25_MHZ_PLL3;
            pll_vsel_val[0] = VSEL_VALUE_25_MHZ_PLL0;
            pll_vsel_val[1] = VSEL_VALUE_25_MHZ_PLL1;
            pll_vsel_val[2] = VSEL_VALUE_25_MHZ_PLL2;
            pll_vsel_val[3] = VSEL_VALUE_25_MHZ_PLL3;
            ca9_pll_val     = FREQ_HZ_25_MHZ_PLL0/1000000;
            ddr_pll_val     = FREQ_HZ_25_MHZ_PLL1/1000000;
            fec_pll_val     = FREQ_HZ_25_MHZ_PLL2/1000000;
            spu_pll_val     = FREQ_HZ_25_MHZ_PLL3/1000000;
            break;
        case 26000000:
            pll_p_val[0]    = P_VALUE_26_MHZ_PLL0;
            pll_p_val[1]    = P_VALUE_26_MHZ_PLL1;
            pll_p_val[2]    = P_VALUE_26_MHZ_PLL2;
            pll_p_val[3]    = P_VALUE_26_MHZ_PLL3;
            pll_m_val[0]    = M_VALUE_26_MHZ_PLL0;
            pll_m_val[1]    = M_VALUE_26_MHZ_PLL1;
            pll_m_val[2]    = M_VALUE_26_MHZ_PLL2;
            pll_m_val[3]    = M_VALUE_26_MHZ_PLL3;
            pll_s_val[0]    = S_VALUE_26_MHZ_PLL0;
            pll_s_val[1]    = S_VALUE_26_MHZ_PLL1;
            pll_s_val[2]    = S_VALUE_26_MHZ_PLL2;
            pll_s_val[3]    = S_VALUE_26_MHZ_PLL3;
            pll_vsel_val[0] = VSEL_VALUE_26_MHZ_PLL0;
            pll_vsel_val[1] = VSEL_VALUE_26_MHZ_PLL1;
            pll_vsel_val[2] = VSEL_VALUE_26_MHZ_PLL2;
            pll_vsel_val[3] = VSEL_VALUE_26_MHZ_PLL3;
            ca9_pll_val     = FREQ_HZ_26_MHZ_PLL0/1000000;
            ddr_pll_val     = FREQ_HZ_26_MHZ_PLL1/1000000;
            fec_pll_val     = FREQ_HZ_26_MHZ_PLL2/1000000;
            spu_pll_val     = FREQ_HZ_26_MHZ_PLL3/1000000;
            break;
        case 40000000:
            pll_p_val[0]    = P_VALUE_40_MHZ_PLL0;
            pll_p_val[1]    = P_VALUE_40_MHZ_PLL1;
            pll_p_val[2]    = P_VALUE_40_MHZ_PLL2;
            pll_p_val[3]    = P_VALUE_40_MHZ_PLL3;
            pll_m_val[0]    = M_VALUE_40_MHZ_PLL0;
            pll_m_val[1]    = M_VALUE_40_MHZ_PLL1;
            pll_m_val[2]    = M_VALUE_40_MHZ_PLL2;
            pll_m_val[3]    = M_VALUE_40_MHZ_PLL3;
            pll_s_val[0]    = S_VALUE_40_MHZ_PLL0;
            pll_s_val[1]    = S_VALUE_40_MHZ_PLL1;
            pll_s_val[2]    = S_VALUE_40_MHZ_PLL2;
            pll_s_val[3]    = S_VALUE_40_MHZ_PLL3;
            pll_vsel_val[0] = VSEL_VALUE_40_MHZ_PLL0;
            pll_vsel_val[1] = VSEL_VALUE_40_MHZ_PLL1;
            pll_vsel_val[2] = VSEL_VALUE_40_MHZ_PLL2;
            pll_vsel_val[3] = VSEL_VALUE_40_MHZ_PLL3;
            ca9_pll_val     = FREQ_HZ_40_MHZ_PLL0/1000000;
            ddr_pll_val     = FREQ_HZ_40_MHZ_PLL1/1000000;
            fec_pll_val     = FREQ_HZ_40_MHZ_PLL2/1000000;
            spu_pll_val     = FREQ_HZ_40_MHZ_PLL3/1000000;
            break;
        }
    } else if (dev_rev == T2200_REV_X2_1_HS_HP) {
        sys_ref_clk_hz = get_sys_ref_clk_hz();
        if ((sys_ref_clk_hz == 25000000) || (sys_ref_clk_hz == 19200000)) {
            L2CC_PLL            = 0;
            TPI_PLL             = 0;
            CSYS_PLL            = 0;
            CRP_PLL             = 2;
            FEC_UL_PLL          = 2;
            GEM_TX_PLL          = 3;
            IPSEC_PLL           = 2;
            SPACC_PLL           = 2;
            FEC_DL_PLL          = 3;
            CEVA_GROUP_PLL      = 3;
        }
        switch(sys_ref_clk_hz) {
        case 19200000:
            pll_p_val[0]	= P_VALUE_19_2_MHZ_TURBO_PLL0;
            pll_p_val[1]	= P_VALUE_19_2_MHZ_PLL1;
            pll_p_val[2]	= P_VALUE_19_2_MHZ_PLL2;
            pll_p_val[3]	= P_VALUE_19_2_MHZ_TURBO_PLL3;
            pll_m_val[0]	= M_VALUE_19_2_MHZ_TURBO_PLL0;
            pll_m_val[1]	= M_VALUE_19_2_MHZ_PLL1;
            pll_m_val[2]	= M_VALUE_19_2_MHZ_PLL2;
            pll_m_val[3]	= M_VALUE_19_2_MHZ_TURBO_PLL3;
            pll_s_val[0]	= S_VALUE_19_2_MHZ_TURBO_PLL0;
            pll_s_val[1]	= S_VALUE_19_2_MHZ_PLL1;
            pll_s_val[2]	= S_VALUE_19_2_MHZ_PLL2;
            pll_s_val[3]	= S_VALUE_19_2_MHZ_TURBO_PLL3;
            pll_vsel_val[0] = VSEL_VALUE_19_2_MHZ_TURBO_PLL0;
            pll_vsel_val[1] = VSEL_VALUE_19_2_MHZ_PLL1;
            pll_vsel_val[2] = VSEL_VALUE_19_2_MHZ_PLL2;
            pll_vsel_val[3] = VSEL_VALUE_19_2_MHZ_TURBO_PLL3;
            ca9_pll_val 	= FREQ_HZ_19_2_MHZ_TURBO_PLL0/1000000;
            ddr_pll_val 	= FREQ_HZ_19_2_MHZ_PLL1/1000000;
            fec_pll_val 	= FREQ_HZ_19_2_MHZ_PLL2/1000000;
            spu_pll_val 	= FREQ_HZ_19_2_MHZ_TURBO_PLL3/1000000;
            break;
        case 25000000:
            pll_p_val[0]	= P_VALUE_25_MHZ_TURBO_PLL0;
            pll_p_val[1]	= P_VALUE_25_MHZ_PLL1;
            pll_p_val[2]	= P_VALUE_25_MHZ_PLL2;
            pll_p_val[3]	= P_VALUE_25_MHZ_TURBO_PLL3;
            pll_m_val[0]	= M_VALUE_25_MHZ_TURBO_PLL0;
            pll_m_val[1]	= M_VALUE_25_MHZ_PLL1;
            pll_m_val[2]	= M_VALUE_25_MHZ_PLL2;
            pll_m_val[3]	= M_VALUE_25_MHZ_TURBO_PLL3;
            pll_s_val[0]	= S_VALUE_25_MHZ_TURBO_PLL0;
            pll_s_val[1]	= S_VALUE_25_MHZ_PLL1;
            pll_s_val[2]	= S_VALUE_25_MHZ_PLL2;
            pll_s_val[3]	= S_VALUE_25_MHZ_TURBO_PLL3;
            pll_vsel_val[0] = VSEL_VALUE_25_MHZ_TURBO_PLL0;
            pll_vsel_val[1] = VSEL_VALUE_25_MHZ_PLL1;
            pll_vsel_val[2] = VSEL_VALUE_25_MHZ_PLL2;
            pll_vsel_val[3] = VSEL_VALUE_25_MHZ_TURBO_PLL3;
            ca9_pll_val 	= FREQ_HZ_25_MHZ_TURBO_PLL0/1000000;
            ddr_pll_val 	= FREQ_HZ_25_MHZ_PLL1/1000000;
            fec_pll_val 	= FREQ_HZ_25_MHZ_PLL2/1000000;
            spu_pll_val 	= FREQ_HZ_25_MHZ_TURBO_PLL3/1000000;
            break;
        case 26000000:
            pll_p_val[0]	= P_VALUE_26_MHZ_PLL0;
            pll_p_val[1]	= P_VALUE_26_MHZ_PLL1;
            pll_p_val[2]	= P_VALUE_26_MHZ_PLL2;
            pll_p_val[3]	= P_VALUE_26_MHZ_PLL3;
            pll_m_val[0]	= M_VALUE_26_MHZ_PLL0;
            pll_m_val[1]	= M_VALUE_26_MHZ_PLL1;
            pll_m_val[2]	= M_VALUE_26_MHZ_PLL2;
            pll_m_val[3]	= M_VALUE_26_MHZ_PLL3;
            pll_s_val[0]	= S_VALUE_26_MHZ_PLL0;
            pll_s_val[1]	= S_VALUE_26_MHZ_PLL1;
            pll_s_val[2]	= S_VALUE_26_MHZ_PLL2;
            pll_s_val[3]	= S_VALUE_26_MHZ_PLL3;
            pll_vsel_val[0] = VSEL_VALUE_26_MHZ_PLL0;
            pll_vsel_val[1] = VSEL_VALUE_26_MHZ_PLL1;
            pll_vsel_val[2] = VSEL_VALUE_26_MHZ_PLL2;
            pll_vsel_val[3] = VSEL_VALUE_26_MHZ_PLL3;
            ca9_pll_val 	= FREQ_HZ_26_MHZ_PLL0/1000000;
            ddr_pll_val 	= FREQ_HZ_26_MHZ_PLL1/1000000;
            fec_pll_val 	= FREQ_HZ_26_MHZ_PLL2/1000000;
            spu_pll_val 	= FREQ_HZ_26_MHZ_PLL3/1000000;
            break;
        case 40000000:
            pll_p_val[0]	= P_VALUE_40_MHZ_PLL0;
            pll_p_val[1]	= P_VALUE_40_MHZ_PLL1;
            pll_p_val[2]	= P_VALUE_40_MHZ_PLL2;
            pll_p_val[3]	= P_VALUE_40_MHZ_PLL3;
            pll_m_val[0]	= M_VALUE_40_MHZ_PLL0;
            pll_m_val[1]	= M_VALUE_40_MHZ_PLL1;
            pll_m_val[2]	= M_VALUE_40_MHZ_PLL2;
            pll_m_val[3]	= M_VALUE_40_MHZ_PLL3;
            pll_s_val[0]	= S_VALUE_40_MHZ_PLL0;
            pll_s_val[1]	= S_VALUE_40_MHZ_PLL1;
            pll_s_val[2]	= S_VALUE_40_MHZ_PLL2;
            pll_s_val[3]	= S_VALUE_40_MHZ_PLL3;
            pll_vsel_val[0] = VSEL_VALUE_40_MHZ_PLL0;
            pll_vsel_val[1] = VSEL_VALUE_40_MHZ_PLL1;
            pll_vsel_val[2] = VSEL_VALUE_40_MHZ_PLL2;
            pll_vsel_val[3] = VSEL_VALUE_40_MHZ_PLL3;
            ca9_pll_val 	= FREQ_HZ_40_MHZ_PLL0/1000000;
            ddr_pll_val 	= FREQ_HZ_40_MHZ_PLL1/1000000;
            fec_pll_val 	= FREQ_HZ_40_MHZ_PLL2/1000000;
            spu_pll_val 	= FREQ_HZ_40_MHZ_PLL3/1000000;
            break;
        }
    } else {
        trap = 1;
        while (trap);
    }

    pll_ctrl_val = PLL_CTRL_LOCK_EN | PLL_CTRL_BYPASS | PLL_CTRL_RESET;

    if (dev_rev < T2200_REV_X2_1) {
        vco = ca9_pll_val;
        m = 5 * vco / (sys_ref_clk_hz / 1000000);
        s = 1;
        p = 5;
    } else {
        m = pll_m_val[0];
        s = pll_s_val[0];
        p = pll_p_val[0];
    }
    REG32(PLL_CNTRL(0)) = pll_ctrl_val | pll_vsel_val[0]; // always use high VCO, lock enable, bypass and reset
    REG32(PLL_M_LSB(0)) = m & 0xff;
    REG32(PLL_M_MSB(0)) = m >> 8;
    REG32(PLL_P(0)) = p;
    REG32(PLL_S(0)) = s;

    if (dev_rev < T2200_REV_X2_1) {
        vco = ddr_pll_val;
        m = 5 * vco / (sys_ref_clk_hz / 1000000);
        s = 1;
        p = 5;
    } else {
        m = pll_m_val[1];
        s = pll_s_val[1];
        p = pll_p_val[1];
    }
    REG32(PLL_CNTRL(1)) = pll_ctrl_val | pll_vsel_val[1]; // always use high VCO, lock enable, bypass and reset
    REG32(PLL_M_LSB(1)) = m & 0xff;
    REG32(PLL_M_MSB(1)) = m >> 8;
    REG32(PLL_P(1)) = p;
    REG32(PLL_S(1)) = s;

    if (dev_rev < T2200_REV_X2_1) {
        vco = fec_pll_val;
        m = 5 * vco / (sys_ref_clk_hz / 1000000);
        s = 1;
        p = 5;
    } else {
        m = pll_m_val[2];
        s = pll_s_val[2];
        p = pll_p_val[2];
    }
    REG32(PLL_CNTRL(2)) = pll_ctrl_val | pll_vsel_val[2]; // always use high VCO, lock enable, bypass and reset
    REG32(PLL_M_LSB(2)) = m & 0xff;
    REG32(PLL_M_MSB(2)) = m >> 8;
    REG32(PLL_P(2)) = p;
    REG32(PLL_S(2)) = s;

    if (dev_rev < T2200_REV_X2_1) {
        vco = spu_pll_val;
        m = 5 * vco / (sys_ref_clk_hz / 1000000);
        s = 1;
        p = 5;
    } else {
        m = pll_m_val[3];
        s = pll_s_val[3];
        p = pll_p_val[3];
    }
    REG32(PLL_CNTRL(3)) = pll_ctrl_val | pll_vsel_val[3]; // always use high VCO, lock enable, bypass and reset
    REG32(PLL_M_LSB(3)) = m & 0xff;
    REG32(PLL_M_MSB(3)) = m >> 8;
    REG32(PLL_P(3)) = p;
    REG32(PLL_S(3)) = s;

    REG32(CA9_MC_MPU_CLK_DIV_CNTRL) = (dev_rev == T2200_REV_X1) ? 0x33 : 0x44; // need to keep the same ARM peripheral clock for X1 and X2
    REG32(CA9_MC_CLK_CNTRL) = PLL_SOURCE(CA9_MC_PLL);
    REG32(CA9_MC_CLK_DIV_CNTRL) = 0x82;

    REG32(L2CC_CLK_CNTRL) = PLL_SOURCE(L2CC_PLL);
    REG32(L2CC_CLK_DIV_CNTRL) = 0x82;

    REG32(TPI_CLK_CNTRL) = PLL_SOURCE(TPI_PLL);
    REG32(TPI_CLK_DIV_CNTRL) = 10;	// current clock fix for T3300 trace port

    REG32(CSYS_CLK_CNTRL) = PLL_SOURCE(CSYS_PLL);
    REG32(CSYS_CLK_DIV_CNTRL) = (dev_rev >= T2200_REV_X2_1) ? 4 : 5;

    REG32(FEC_DL_CLK_CNTRL) = PLL_SOURCE(FEC_DL_PLL);
    REG32(AXI_CLK_CNTRL_1) |= FEC_AXI_RESET_1;
    REG32(FEC_DL_CLK_DIV_CNTRL) = 0x82;
    REG32(FEC_DL_DIV2_CLK_CNTRL) = 1;
    REG32(FEC_DL_DIV2_CLK_DIV_CNTRL) = 2;
    REG32(FEC_DL_DIV2_UNDOCUMENTED) = 0;

    REG32(FEC_UL_CLK_CNTRL) = PLL_SOURCE(FEC_UL_PLL);
    REG32(FEC_UL_CLK_DIV_CNTRL) = (dev_rev == T2200_REV_X2_1_HS_HP) ? 0x82 : 0x2;

    REG32(IPSEC_CLK_CNTRL) = PLL_SOURCE(IPSEC_PLL);
    if (   (IPSEC_PLL == 1)
        && (dev_rev == T2200_REV_X2_1)
        && (X2_1_DDR_PLL_DIV_CNTRL == 0x82)
       )
        REG32(IPSEC_CLK_DIV_CNTRL) = 0x82; // Bypass
    else
        REG32(IPSEC_CLK_DIV_CNTRL) = (dev_rev == T2200_REV_X2_1_HS_HP) ? 0x82 : 0x2;

    REG32(SPACC_CLK_CNTRL) = PLL_SOURCE(SPACC_PLL);
    if (   (SPACC_PLL == 1)
        && (dev_rev == T2200_REV_X2_1)
        && (X2_1_DDR_PLL_DIV_CNTRL == 0x82)
       )
        REG32(SPACC_CLK_DIV_CNTRL) = 0x82; // Bypass
    else
        REG32(SPACC_CLK_DIV_CNTRL) = (dev_rev == T2200_REV_X2_1_HS_HP) ? 0x82 : 0x2;

    REG32(DDR3_CLK_CNTRL) = PLL_SOURCE(DDR_PLL);
    REG32(DDR3_CLK_DIV_CNTRL) = (dev_rev >= T2200_REV_X2_1) ? X2_1_DDR_PLL_DIV_CNTRL : 0x02;

    //
    // GEM TX clock (when used by GEM for TX instead
    // of external TX reference timing source)
    // must be exactly 125 MHz
    //
    // This means input PLL must be multiple of
    // 125 or 125, 250, 375, 500, 625, 750, 875 or 1000 MHz
    //
    REG32(GEMTX_CLK_DIV_CNTRL) = (dev_rev == T2200_REV_X2_1_HS_HP) ? 7 : ca9_pll_val/125;
    REG32(GEMTX_CLK_CNTRL) = PLL_SOURCE(GEM_TX_PLL);
    REG32(GEMTX_RESET) = 0;

    REG32(CRP_CLK_CNTRL) =  PLL_SOURCE(CRP_PLL);
    REG32(CRP_CLK_DIV_CNTRL) = (dev_rev == T2200_REV_X2_1_HS_HP) ? 0x82 : 0x2;
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

    REG32(PLL_CNTRL(0)) = pll_ctrl_val | pll_vsel_val[0];
    REG32(PLL_CNTRL(1)) = pll_ctrl_val | pll_vsel_val[1];
    REG32(PLL_CNTRL(2)) = pll_ctrl_val | pll_vsel_val[2];
    REG32(PLL_CNTRL(3)) = pll_ctrl_val | pll_vsel_val[3];

    while ((REG32(PLL_STATUS(0)) & PLL_STAT_LOCK) == 0); // wait PLL0 to lock
    while ((REG32(PLL_STATUS(1)) & PLL_STAT_LOCK) == 0); // wait PLL1 to lock
    while ((REG32(PLL_STATUS(2)) & PLL_STAT_LOCK) == 0); // wait PLL2 to lock
    while ((REG32(PLL_STATUS(3)) & PLL_STAT_LOCK) == 0); // wait PLL2 to lock

    pll_ctrl_val = PLL_CTRL_LOCK_EN; // activate PLLs

    REG32(PLL_CNTRL(0)) = pll_ctrl_val | pll_vsel_val[0];
    REG32(PLL_CNTRL(1)) = pll_ctrl_val | pll_vsel_val[1];
    REG32(PLL_CNTRL(2)) = pll_ctrl_val | pll_vsel_val[2];
    REG32(PLL_CNTRL(3)) = pll_ctrl_val | pll_vsel_val[3];

    REG32(AXI_CLK_CNTRL_0) = (REG32(AXI_CLK_CNTRL_0) & 0xF0) | PLL_SOURCE(0);

    REG32(AXI_CLK_DIV_CNTRL) = (dev_rev >= T2200_REV_X2_1) ? 4 : 5;

    // enable clocking from 4 PLLs
    REG32(PLLS_GLOBAL_CNTRL) &= ~(0xf<<0);

    REG32(AXI_RESET_0) = 0;
    REG32(AXI_RESET_1) = 0;
    REG32(AXI_RESET_2) = 0;

    delay = 10000;
    while (delay--)
        asm("nop"); // 10000 cycles delay
}

