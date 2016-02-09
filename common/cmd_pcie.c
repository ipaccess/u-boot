/*
 * PCIe routines
 */

#include <common.h>
#include <command.h>
#include <asm/processor.h>
#include <asm/io.h>
#include <asm/arch/pcie.h>
#include <asm/arch/clkrst.h>

static int locate_sbl_high = 0;     // map high memory for SBL
static int pcie_initialized = 0;    // PCIe is initialized
static int pcie_rate = 2500;        // PCIe rate

int do_pcie (cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
	int i, pcie_lanes = 1;
	int root_complex = 1;
	unsigned int window_size = 16 * 1024 * 1024;	// bytes
	u32 linux_window_addr = PRIMARY_IMAGE_ADDR;
	u32 j, tmp;
	PCIE_CONF_SPACE_HEADER_TYPE0 *pCfg = (PCIE_CONF_SPACE_HEADER_TYPE0*)PCIESLAVE_BASEADDR;

	if (argc > 1) {
		for (i = 1; i < argc; i++) {
			switch (argv[i][0]) {
			case '1':	/* 1 Lane  */
			case '4':	/* 4 Lanes */
				pcie_lanes = simple_strtoul(argv[i], NULL, 16);
				break;
			case 'r':	/* Root complex */
			case 'R':
				root_complex = 1;
				break;
			case 'e':	/* End Point */
			case 'E':
				root_complex = 0;
				break;
			case 'G':	/* gen2, 5000 */
				pcie_rate = 5000;
				break;
			case 'g':	/* gen1, 2500 */
				pcie_rate = 2500;
				break;
			case 'i':		/* init */
			case 'I':
				printf("Selected PCIe rate: %d\n", pcie_rate);
				PCIeDrvX4Init(root_complex, pcie_rate, pcie_lanes);

				PCIeDrvSetOBCfg0AT(	PCIE0_BASEADDR,
							0,
							PCIESLAVE_BASEADDR,
							COMBINE_BDF(0,0,0));

				PCIeDrvSetIBMemAT(	PCIE0_BASEADDR,
							0,
							MEMRW_TLP_BASEADDR,
							MEMRW_TLP_BASEADDR + window_size - 1,
							PRIMARY_IMAGE_ADDR);

				PCIeDrvSetIBMemAT(	PCIE0_BASEADDR,
							1,
							MEMRW_TLP_SBLADDR,
							MEMRW_TLP_SBLADDR + window_size - 1,
							locate_sbl_high ? (SBL_IMAGE_ADDR_HIGH) : (SBL_IMAGE_ADDR) );
                pcie_initialized = 1;
				break;
			case 's':		/* link up, set master bit */
			case 'S':
				//TODO add timeout and result check
				if (pcie_lanes == 4) {
					wait_PCIEX4_link_up();
				} else {
					wait_PCIEX1_link_up();
				}
				SysDelayUs(600); // Slave 'hezitates' for 500 ms
				// set Master enable bit
				printf("PCIe link is up. Setting MASTER bit\n");
				pCfg->CommandReg |=  0x7;
				break;
			case 'w':
			case 'W':
				// wait for Master enable bit being cleared
				printf("Waiting for download completion...\n");
				while(pCfg->CommandReg & (1 << 2));
				printf("Download completed.\n");

				printf("Setting MASTER bit\n");
				pCfg->CommandReg |=  0x7;
				break;
			case 'c':
			case 'C':
				// add CRC at 0x0ff fffc addres of previous data starting from 0
				tmp = 0;
				for (j = 0; j < (window_size - 4); j += 4) {
					tmp += *(u32*) j;
				}
				*(u32*)j = tmp;
				break;
			case 'l':
			case 'L':
				// Window size
				window_size = simple_strtoul(argv[++i], NULL, 10);
				if (window_size == 0) {
					printf("Incorrect window size '%i'", window_size);
					window_size = 16 * 1024 * 1024;
					printf("Using default '%i'", window_size);
				}
				break;
            case 'h':
            case 'H':
                // locate SBL in the high addresses (for window > 32MB)
                locate_sbl_high = 1;
                printf("PCIE: locate_sbl_high = %d [A = 0x%X]\n", locate_sbl_high, SBL_IMAGE_ADDR_HIGH);
                if(pcie_initialized) {
                    PCIeDrvSetIBMemAT(	PCIE0_BASEADDR,
							1,
							MEMRW_TLP_SBLADDR,
							MEMRW_TLP_SBLADDR + window_size - 1,
							SBL_IMAGE_ADDR_HIGH);
                }
                break;
			default:		/* unknown */
				printf("Unknown command (%d:'%s').\n", i, argv[i]);
				goto usage;
			}
		}
	}


	return 1;
 usage:
	cmd_usage(cmdtp);
	return 1;
}

/***************************************************/


U_BOOT_CMD(
	pcie,	6,	1,	do_pcie,
	"PCIe link related functions",
	"pcie [1|4] [l bytes] [RC|EP] [h] init\n"
	"    - initialize PCIe 'Root Complex' | 'End Point', uses 1|4 Lanes.\n"
	"      if option 'l' is used, then default 16Mb window is changed to 'bytes' size.\n"
	"      if option 'h' is used, then SBL is expected at address 0x08000000 (default is 0x02000000)\n"
	"pcie [1|4] start\n"
	"    - wait for link up and set master bit; this signals SBL to start downloading image.\n"
	"pcie [1|4] wait\n"
	"    - waits for SBL download complete (polling master bit to be cleared)\n"
	"pcie [l bytes] c\n"
	"    - calculates CRC for 0x0000_0000 - 0x00FF_FFF8, and put result to 0x00FF_FFFC\n"
	"      if option 'l' is used, then default 16Mb replaced by 'bytes' and CRC is culculated over 0x0000_0000 - (bytes - 4), and put result to (bytes - 4)\n"
);
