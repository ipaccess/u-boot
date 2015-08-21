
#ifndef _TRANSCEDE_M82XXX_CLKRST_H_
#define _TRANSCEDE_M82XXX_CLKRST_H_

#include <asm/arch/systypes.h>
#include <div64.h>


typedef enum
{
    CR_PLL_0 = 0,
    CR_PLL_1 = 1,
    CR_PLL_2 = 2,
    CR_PLL_3 = 3,

    CR_PLL_REF_CLOCK = 4,

    CR_CA9_MC_MPU_ACP,
    CR_CA9_MC_MPU_PERIPH,

    CR_CA9_MC_CPU0,
    CR_CA9_MC_CPU1,

    // devices have own clk&rst block
    CR_CA9_MC,
    CR_L2CC,
    CR_TPI,
    CR_CSYS,
    CR_EXTPHY0,
    CR_EXTPHY1,
    CR_FEC_UL,
    CR_FEC_DL,
    CR_FFT,
    CR_IPSEC,
    CR_DDR3,
    CR_GEMTX,
    CR_TDMNTG,
    CR_TSUNTG,
    CR_CRP,
    CR_CEVA,
    CR_SPACC,
    CR_SASPA,

    // AXI_CNTRL Group start
    // axi clk&rst 0
    CR_MDMA_SYS1,
    CR_MDMA_SYS0,

    // axi clk&rst 1
    CR_SYS_AXI,
    CR_TIMERS,
    CR_UART,
    CR_I2C_SPI,
    CR_PCI,
    CR_TDM,
    CR_FEC,
    CR_DUS,

    // axi clk&rst 2
    CR_TBD,
    CR_GEM0,
    CR_GEM1,
    CR_USB_MISC,
    CR_USB,
    CR_JESD207,
    CR_CPRI,
    // AXI_CNTRL Group end

    CR_CEVA_BM_AXI,
    CR_CEVA_BM_AHB,
    CR_CRP_BM,
    CR_FFT_BM,
}ClkRstDev;

#define FREQ_MHZ(nDevID) (ClkRstGetFreq (nDevID)/1000000)

UINT32 ClkRstGetFreq (UINT32 nDevID);
void SysDelayUs(u32 us);

#endif
