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
#include <asm/arch/clkrst.h>
#include <asm/arch/transcede-2200.h>

// defines from sysrc.h
#define SYSRC_SUCCESS                       0x00000000
#define SYSRC_INVALID_DEVICE_ID             0x00000014

#define _ASSERT_PTR(Ptr)
#define _ASSERT(x)
#define _ASSERT_RC(rc)

static UINT32 GetDevAddrReg(UINT32 nDevID, PUINT32 ctrl, PUINT32 ctrlDiv, PUINT32 rst);

inline UINT32 ClkRstGetRefClock (void)
{
    return 25*1000*1000;
}

UINT32 ClkGetSrcPll(UINT32 nDevID) /* in progress */
{
    UINT32 rc;
    UINT32 nPllID;
    UINT32 clkCtrlReg;
    PUINT32 pClkCtrlReg;


    rc = GetDevAddrReg(nDevID, &clkCtrlReg, NULL, NULL);
    if (rc != SYSRC_SUCCESS)
    {
        printf("GetDevAddrReg: failed\n");
        _ASSERT(0);
    }

    pClkCtrlReg = (UINT32 *) clkCtrlReg;

    nPllID = (*pClkCtrlReg >> 1) & 0x07;

    return nPllID;
}

BOOL ClkRstIsPllInReset (UINT32 nDevID)
{
    UINT32 nCtrl = 0;

    nCtrl = REG32(PLL_CNTRL(nDevID));

    if (nCtrl & (1 << 0)/* || nCtrl & (1 << 3)*/)
        return TRUE;

    return FALSE;
}


UINT32 ClkRstGetPllFreq (UINT32 nPllID)
{
    UINT64 nFrq = 0;

    UINT32 nCtrl = REG32(PLL_CNTRL(nPllID));

    UINT32 nVal, p, s;

    if (nPllID > CR_PLL_3)
        return 0;

    // if in reset or in PowerDown mode
    if (ClkRstIsPllInReset(nPllID))
        return 0;

    nFrq = ClkRstGetRefClock();   /* need to check */

    nVal = REG32(PLL_M_LSB (nPllID)) & 0xFF;
    nVal |= (REG32(PLL_M_MSB (nPllID)) & 0x1F) << 8;


    nFrq *= nVal;

    p = REG32(PLL_P (nPllID)) & 0x3F;
    s = (REG32(PLL_S (nPllID)) & 0x07) - 1;

    // nFrq /= (p * (1 << s));
    do_div(nFrq, (p * (1 << s)));

    // if bypass mode
    if (nCtrl & (1 << 4))
        return ClkRstGetRefClock();

    // in this case the PLL output freq. = REF * M / P / 2^S

    return (UINT32)nFrq;

}

UINT32 ClkRstGetFreq (UINT32 nDevID)
{
    UINT32 nDiv = 1;
    UINT32 rc;
    UINT32 clkCtrlDivReg;
    PUINT32 pClkCtrlDivReg;

    if (nDevID <= CR_PLL_3)
        return ClkRstGetPllFreq(nDevID-CR_PLL_0);

    rc = GetDevAddrReg(nDevID, NULL, &clkCtrlDivReg, NULL);
    if (rc != SYSRC_SUCCESS)
    {
        _ASSERT(0);
        return 0;
    }

    pClkCtrlDivReg = (UINT32 *) clkCtrlDivReg;

    switch(nDevID)
    {
        case CR_CA9_MC_MPU_ACP:
            nDiv = (*pClkCtrlDivReg & (1 << 7))? 1 :( (*pClkCtrlDivReg >> 4) & 0x07);
            break;

        case CR_CA9_MC_MPU_PERIPH:
            nDiv = (*pClkCtrlDivReg & (1 << 3))? 1 :( *pClkCtrlDivReg & 0x07);
            break;


        case CR_CEVA_BM_AXI:
            nDiv = (*pClkCtrlDivReg & (1 << 3))? 1 :( *pClkCtrlDivReg & 0x07);
            break;

        case CR_CEVA_BM_AHB:
            nDiv = (*pClkCtrlDivReg >> 4) & 0x07;
            break;

        case CR_CRP_BM:
            nDiv = *pClkCtrlDivReg  & 0x07;
            break;

        case CR_FFT_BM:
            nDiv = (*pClkCtrlDivReg & (1 << 3))? 1 :( *pClkCtrlDivReg & 0x07);
            break;

        case CR_CA9_MC_CPU0:
        case CR_CA9_MC_CPU1:
            nDiv = 1;
            break;

        case CR_FEC_DL:
            // due to HW, the divider is 1 , even if it's read like 2 because bit 8 - bypass read as 0 always
            nDiv = 1;//(*pClkCtrlDivReg & (1 << 7)) ? 1 :( *pClkCtrlDivReg & 0x1F);
            break;
#ifdef PLLS_SETUP_FOR_CHARTERIZATION
        case CR_FEC_UL:
        case CR_IPSEC:
        case CR_SPACC:
            // due to HW, the divider is 1 , even if it's read like 2 because bit 8 - bypass read as 0 always
            nDiv = 1;
            break;
#endif
        case CR_CA9_MC:
            // due to HW, the divider is 1 always, even if it's read like 2
            nDiv = 1;//(*pClkCtrlDivReg & (1 << 7)) ? 1 :( *pClkCtrlDivReg & 0x1F);
            break;

        case CR_CEVA:
            // due to HW, the divider is 1 always, even if it's read like 2
            nDiv = 1;//(*pClkCtrlDivReg & (1 << 7)) ? 1 :( *pClkCtrlDivReg & 0x1F);
            break;

        case CR_FFT:
            // due to HW, the divider is 1 always, even if it's read like 2
            nDiv = 1;
            break;

        default:
            nDiv = (*pClkCtrlDivReg & (1 << 7)) ? 1 :(*pClkCtrlDivReg & 0x1F);
            break;
    }


    return ClkRstGetPllFreq(ClkGetSrcPll(nDevID))/nDiv;
}


static UINT32 GetDevAddrReg(UINT32 nDevID, PUINT32 ctrl, PUINT32 ctrlDiv, PUINT32 rst)
{
    VUINT32 *pClkCtrlReg = NULL, *pClkCtrlDivReg = NULL, *pResetReg = NULL;

#ifdef CLKRST_DEBUG
    uart_printf("GetDevAddrReg: nDevID = %d ctrl = 0x%x ctrlDiv = 0x%x rst = 0x%x\n",  nDevID, ctrl, ctrlDiv, rst);
#endif

    switch(nDevID)
    {
        case CR_CA9_MC_MPU_ACP:
        case CR_CA9_MC_MPU_PERIPH:
            pClkCtrlReg =(UINT32 *) CA9_MC_CLK_CNTRL;
            pClkCtrlDivReg =(UINT32 *) CA9_MC_MPU_CLK_DIV_CNTRL;
            pResetReg = (UINT32 *) CA9_MC_MPU_RESET;
            break;

        case CR_CA9_MC_CPU0:
        case CR_CA9_MC_CPU1:
            pClkCtrlReg =(UINT32 *) CA9_MC_CPU_CLK_CNTRL;
            pResetReg = (UINT32 *) CA9_MC_CPU_RESET;
            break;

        case CR_CA9_MC:
            pClkCtrlReg = (UINT32 *) CA9_MC_CLK_CNTRL;
            pClkCtrlDivReg = (UINT32 *) CA9_MC_CLK_DIV_CNTRL;
            pResetReg = (UINT32 *) CA9_MC_RESET;
            break;

        case CR_L2CC:
            pClkCtrlReg = (UINT32 *) L2CC_CLK_CNTRL;
            pClkCtrlDivReg = (UINT32 *) L2CC_CLK_DIV_CNTRL;
            pResetReg = (UINT32 *) L2CC_RESET;
            break;

        case CR_TPI:
            pClkCtrlReg = (UINT32 *) TPI_CLK_CNTRL;
            pClkCtrlDivReg = (UINT32 *) TPI_CLK_DIV_CNTRL;
            pResetReg = (UINT32 *) TPI_RESET;
            break;

        case CR_CSYS:
            pClkCtrlReg = (UINT32 *) CSYS_CLK_CNTRL;
            pClkCtrlDivReg = (UINT32 *) CSYS_CLK_DIV_CNTRL;
            pResetReg = (UINT32 *) CSYS_RESET;
            break;

        case CR_EXTPHY0:
            pClkCtrlReg = (UINT32 *) EXTPHY0_CLK_CNTRL;
            pClkCtrlDivReg = (UINT32 *) EXTPHY0_CLK_DIV_CNTRL;
            pResetReg = (UINT32 *) EXTPHY0_RESET;
            break;

        case CR_EXTPHY1:
            pClkCtrlReg = (UINT32 *) EXTPHY1_CLK_CNTRL;
            pClkCtrlDivReg = (UINT32 *) EXTPHY1_CLK_DIV_CNTRL;
            pResetReg = (UINT32 *) EXTPHY1_RESET;
            break;

        case CR_FEC_UL:
            pClkCtrlReg = (UINT32 *) FEC_UL_CLK_CNTRL;
            pClkCtrlDivReg = (UINT32 *) FEC_UL_CLK_DIV_CNTRL;
            pResetReg = (UINT32 *) FEC_UL_RESET;
            break;

        case CR_FEC_DL:
            pClkCtrlReg = (UINT32 *) FEC_DL_CLK_CNTRL;
            pClkCtrlDivReg = (UINT32 *) FEC_DL_CLK_DIV_CNTRL;
            pResetReg = (UINT32 *) FEC_DL_RESET;
            break;

        case CR_FFT:
            pClkCtrlReg = (UINT32 *) FFT_CLK_CNTRL;
            pClkCtrlDivReg = (UINT32 *) FFT_CLK_DIV_CNTRL;
            pResetReg = (UINT32 *) FFT_RESET;
            break;

        case CR_IPSEC:
            pClkCtrlReg = (UINT32 *) IPSEC_CLK_CNTRL;
            pClkCtrlDivReg = (UINT32 *) IPSEC_CLK_DIV_CNTRL;
            pResetReg = (UINT32 *) IPSEC_RESET;
            break;

        case CR_DDR3:
            pClkCtrlReg = (UINT32 *) DDR3_CLK_CNTRL;
            pClkCtrlDivReg = (UINT32 *) DDR3_CLK_DIV_CNTRL;
            pResetReg = (UINT32 *) DDR3_RESET;
            break;

        case CR_GEMTX:
            pClkCtrlReg = (UINT32 *) GEMTX_CLK_CNTRL;
            pClkCtrlDivReg = (UINT32 *) GEMTX_CLK_DIV_CNTRL;
            pResetReg = (UINT32 *) GEMTX_RESET;
            break;

        case CR_TDMNTG:
            pClkCtrlReg = (UINT32 *) TDMNTG_REF_CLK_CNTRL;
            pClkCtrlDivReg = (UINT32 *) TDMNTG_REF_CLK_DIV_CNTRL;
            pResetReg = (UINT32 *) TDMNTG_RESET;
            break;

        case CR_TSUNTG:
            pClkCtrlReg = (UINT32 *) TSUNTG_REF_CLK_CNTRL;
            pClkCtrlDivReg = (UINT32 *) TSUNTG_REF_CLK_DIV_CNTRL;
            pResetReg = (UINT32 *) TSUNTG_RESET;
            break;

        case CR_CRP:
           pClkCtrlReg = (UINT32 *) CRP_CLK_CNTRL;
            pClkCtrlDivReg = (UINT32 *) CRP_CLK_DIV_CNTRL;
            pResetReg = (UINT32 *) CRP_CLK_RESET;
            break;

        case CR_CEVA:
            pClkCtrlReg = (UINT32 *) CEVA_CLK_CNTRL;
            pClkCtrlDivReg = (UINT32 *) CEVA_CLK_DIV_CNTRL;
            pResetReg = (UINT32 *) CEVA_CLK_RESET;
            break;

        case CR_SPACC:
            pClkCtrlReg = (UINT32 *) SPACC_CLK_CNTRL;
            pClkCtrlDivReg = (UINT32 *) SPACC_CLK_DIV_CNTRL;
            pResetReg = (UINT32 *) SPACC_RESET;
            break;

        case CR_SASPA:
            pClkCtrlReg = (UINT32 *) SASPA_CLK_CNTRL;
            pClkCtrlDivReg = (UINT32 *) SASPA_CLK_DIV_CNTRL;
            pResetReg = (UINT32 *) SASPA_RESET;
            break;

        case CR_SYS_AXI:
            pClkCtrlReg = (UINT32 *) AXI_CLK_CNTRL_0;
            pClkCtrlDivReg = (UINT32 *) AXI_CLK_DIV_CNTRL;
            break;

        case CR_MDMA_SYS1:
        case CR_MDMA_SYS0:
            pResetReg = (UINT32 *) AXI_RESET_0;
            pClkCtrlReg = (UINT32 *) AXI_CLK_CNTRL_0;
            pClkCtrlDivReg = (UINT32 *) AXI_CLK_DIV_CNTRL;
            break;

        case CR_TIMERS:
        case CR_UART:
        case CR_I2C_SPI:
        case CR_DUS:
            pResetReg = (UINT32 *) AXI_RESET_1;
            pClkCtrlReg = (UINT32 *) AXI_CLK_CNTRL_1;
            pClkCtrlDivReg = (UINT32 *) AXI_CLK_DIV_CNTRL;
            break;

        case CR_TBD:
        case CR_GEM0:
        case CR_GEM1:
        case CR_USB_MISC:
        case CR_USB:
        case CR_JESD207:
        case CR_CPRI:
        case CR_PCI:
            pResetReg = (UINT32 *) AXI_RESET_2;
            pClkCtrlReg = (UINT32 *) AXI_CLK_CNTRL_1;
            pClkCtrlDivReg = (UINT32 *) AXI_CLK_DIV_CNTRL;
            break;

        case CR_CEVA_BM_AHB:
        case CR_CEVA_BM_AXI:

            pClkCtrlReg = (UINT32 *) CEVA_CLK_CNTRL;
            pClkCtrlDivReg =(UINT32 *) CEVA_BM_CLK_DIV_CNTRL;
            break;

        case CR_CRP_BM:
            pClkCtrlReg =(UINT32 *) CRP_BM_CLK_CNTRL;
            pClkCtrlDivReg =(UINT32 *) CRP_BM_CLK_DIV_CNTRL;
            break;

        case CR_FFT_BM:
            pClkCtrlReg =(UINT32 *) FFT_BM_CLK_CNTRL;
            pClkCtrlDivReg =(UINT32 *) FFT_BM_CLK_DIV_CNTRL;
            break;

        default:
             return  SYSRC_INVALID_DEVICE_ID;

    }

    if (ctrl != NULL)
    {
        _ASSERT_PTR(pClkCtrlReg);
        *ctrl = (UINT32) pClkCtrlReg;
    }

    if (ctrlDiv != NULL)
    {
        _ASSERT_PTR(pClkCtrlDivReg);
        *ctrlDiv = (UINT32) pClkCtrlDivReg;
    }

    if (rst != NULL)
    {
        _ASSERT_PTR(pResetReg);
        *rst = (UINT32) pResetReg;
    }

    return SYSRC_SUCCESS;
}
