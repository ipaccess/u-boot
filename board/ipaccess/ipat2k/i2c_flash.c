#include <common.h>
#include <malloc.h>
#include <linux/compiler.h>
#include <errno.h>
#include <i2c.h>

int do_i2c_flash_eeprom(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
    char * ep;
    unsigned long offset=0,mem_addr=0,length=0;
    int ret;
    unsigned int chip_addr = 0x50;
    unsigned int addr = 0;
    offset = simple_strtoul(argv[1], &ep, 16);

    if (ep == argv[1] || *ep != '\0')
    {
        fprintf(stderr, "%s\n", "Invalid address offset: not a hexadecimal number.");
        return CMD_RET_USAGE;
    }

    if (offset > (255 * 1024))
    {
        fprintf(stderr, "%s\n", "Invalid address offset: can't write above 255K ");
        return CMD_RET_USAGE;
    }

    if ( offset % 256 != 0)
    {
	fprintf(stderr, "%s\n", "Invalid address offset should be at page boundary i.e 256 bytes ");

    }

    mem_addr = simple_strtoul(argv[2], &ep, 16);
    if (ep == argv[2] || *ep != '\0')
    {
        fprintf(stderr, "%s\n", "Invalid memmory address: not a hexadecimal number.");
        return CMD_RET_USAGE;
    }

    length = simple_strtoul(argv[3], &ep, 16);
    if (ep == argv[3] || *ep != '\0')
    {
	    fprintf(stderr, "%s\n", "Invalid length: not a hexadecimal number.");
	    return CMD_RET_USAGE;
    }

    int tmp=0;
    while (length > 256 )
    {
	    chip_addr |= ((offset >> 16) & 3); 
	    
	    while(i2c_read(chip_addr, 0, 2, &tmp, 1) != 0);

	    if (0 != (ret = i2c_write(chip_addr,offset & 0xFFFF, 2, (uint8_t *)mem_addr, 256)))
	    {
		    printf("i2c_write returned %d length %d offset %X: failed\n", ret,length,offset);
		    return CMD_RET_FAILURE;
	    }
	    
	    length -= 256;
	    mem_addr +=256;
	    offset+=256;
    }

    if (length)
    {
	    chip_addr |= ((offset >> 16) & 3);
	    while(i2c_read(chip_addr, 0, 2, &tmp, 1) != 0);
	    if (0 != (ret = i2c_write(chip_addr,offset & 0xFFFF, 2, (uint8_t *)mem_addr, length)))
	    {
		    printf("i2c_write returned at remaining bytes length %d ret %d: failed\n",length, ret);
		    return CMD_RET_FAILURE;
	    }

    }
    printf("\nDone\n");
    return CMD_RET_SUCCESS;

}


U_BOOT_CMD(
        i2c_flash_eeprom, 4, 0, do_i2c_flash_eeprom,
        "i2c_flash_eeprom to write data from memory to eeprom in page mode",
        "<i2c_flash_eeprom <eeprom address offset(hex)> <memory address(hex)> <data length(hex)> >"
        );

