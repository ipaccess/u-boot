#include <common.h>

/* Storage of bootcount in SRAM:
 *   0x2001ffbc - bootcount reversed copy 2
 *   0x2001ffb8 - bootcount copy 2
 *   0x2001ffb4 - bootcount reversed copy 1
 *   0x2001ffb0 - bootcount copy 1
 */

#define SRAM_BOOTCOUNT_ADDRESS 0x2001ffb0


/* Store a new value of bootcount, bit-reversing as necessary
 */
void bootcount_store(ulong a)
{
    volatile ulong* save_addr = (volatile ulong*)(SRAM_BOOTCOUNT_ADDRESS);
    
    save_addr[3] = ~a;
    save_addr[2] = a;
    save_addr[1] = ~a;
    save_addr[0] = a;
}

/* Read the current value of the bootcount - if the four stored values do
 * not all match, assume this is a power-on reboot and clear the count
 */
ulong bootcount_load(void)
{
    ulong r0, r1, r2, r3;
    volatile ulong* save_addr = (volatile ulong*)(SRAM_BOOTCOUNT_ADDRESS);
    
    r3 = ~(save_addr[3]);
    r2 = save_addr[2];
    r1 = ~(save_addr[1]);
    r0 = save_addr[0];
    
    if ( (r0 != r1) || (r0 != r2) || (r0 != r3) )
    {
        printf("*** Invalid bootcount - power on? Setting to 0 ***\n", r0, r1, r2, r3);
        save_addr[3] = ~0;
        save_addr[2] = 0;
        save_addr[1] = ~0;
        save_addr[0] = 0;
        r0 = 0;
    }
    return r0;
}

static int do_bootcount_combined_exceeded(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
    ulong bc = bootcount_load();

    printf("*** Combined bootcount = %u ***\n", bc);
    
    if ( bc > (CONFIG_BOOTCOUNT_LIMIT * 2) )
    {
        printf("!!! Maximum combined bootcount exceeded - halting !!!\n");
        return CMD_RET_SUCCESS;
    }
    else
    {
        return CMD_RET_FAILURE;
    }
}


U_BOOT_CMD(
    bootcount_combined_exceeded, 1, 0, do_bootcount_combined_exceeded,
    "Determines if the boot count limit has been exceeded for both images",
    ""
);
