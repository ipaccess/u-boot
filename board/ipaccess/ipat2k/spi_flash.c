#include <common.h>
#include <malloc.h>
#include <linux/compiler.h>
#include <errno.h>
#include "asm/arch/spieprom.h"

int do_spi_flash_eeprom(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
    char * ep;
    unsigned long mem_addr=0,length=0;

    mem_addr = simple_strtoul(argv[1], &ep, 16);
    if (ep == argv[2] || *ep != '\0')
    {
        fprintf(stderr, "%s\n", "Invalid memmory address: not a hexadecimal number.");
        return CMD_RET_USAGE;
    }

    length = simple_strtoul(argv[2], &ep, 16);
    if (ep == argv[3] || *ep != '\0')
    {
	    fprintf(stderr, "%s\n", "Invalid length: not a hexadecimal number.");
	    return CMD_RET_USAGE;
    }

    program_spi_loader(mem_addr,length);
    printf("\nDone\n");
    return CMD_RET_SUCCESS;

}


U_BOOT_CMD(
        spi_flash_eeprom, 4, 0, do_spi_flash_eeprom,
        "spi_flash_eeprom to write data from memory to spi chip",
        "<spi_flash_eeprom <data length(hex)> >"
        );

