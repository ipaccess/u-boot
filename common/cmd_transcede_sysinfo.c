/*
 * SysInfo routines for T2200 board
 */

#include <common.h>
#include <command.h>
#include <asm/arch/clkrst.h>

int do_sysinfo (cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
	const char* BootModeNames[] = {"SPI", "I2C", "PCIe", "UART", "<?>", "<?>", "NOR8", "NOR16"};

	printf("\n");
	printf("PLL0    %4d, PLL1    %4d, PLL2   %4d, PLL3    %4d\n",
		FREQ_MHZ(CR_PLL_0), FREQ_MHZ(CR_PLL_1), FREQ_MHZ(CR_PLL_2), FREQ_MHZ(CR_PLL_3));
	printf("ARM     %4d, ACP     %4d, PERIP  %4d, L2CC    %4d\n",
		FREQ_MHZ(CR_CA9_MC), FREQ_MHZ(CR_CA9_MC_MPU_ACP), FREQ_MHZ(CR_CA9_MC_MPU_PERIPH), FREQ_MHZ(CR_L2CC));
	printf("CEVA    %4d, FFT     %4d, CRP    %4d\n",
		FREQ_MHZ(CR_CEVA), FREQ_MHZ(CR_FFT), FREQ_MHZ(CR_CRP));
	printf("IPSEC   %4d, SPACC   %4d\n",
		FREQ_MHZ(CR_IPSEC), FREQ_MHZ(CR_SPACC));
	printf("FEC DL  %4d, FEC UL  %4d\n",
		FREQ_MHZ(CR_FEC_DL), FREQ_MHZ(CR_FEC_UL));
	printf("SYS AXI %4d, CEVA BM %4d\n",
		FREQ_MHZ(CR_SYS_AXI), FREQ_MHZ(CR_CEVA_BM_AXI));
	printf("DDR     %4d\n",
		FREQ_MHZ(CR_DDR3) * 2);
	printf("L2CC to AXI clk: %sSYNC\n", (REG32(SYSCONF_STAT_REG) & (1 << 3)) ? "" : "A");
	printf("Boot mode:       %s\n", BootModeNames[REG32(SYSCONF_STAT_REG) & 7]);
}

/***************************************************/


U_BOOT_CMD(
	sysinfo,	1,	1,	do_sysinfo,
	"sysinfo reads PLL and clock registers and tries to calculate blocks frequencies.\n",
	"sysinfo reads PLL and clock registers and tries to calculate blocks frequencies.\n"
);
