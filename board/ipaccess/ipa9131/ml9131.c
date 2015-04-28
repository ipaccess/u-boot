#include <common.h>
#include <command.h>

#include "image.h"

typedef void (*entry_func_ptr)(void);

static int do_ml9131(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
    entry_func_ptr ep;

    if (0 != load_image(APPS9131_SEARCH_START_BLOCK, APPS9131_SEARCH_NUM_BLOCKS, APPS9131_SEARCH_IMAGE_TYPE))
    {
        puts("load_image failed\n");
        return 1;
    }

    ep = (entry_func_ptr)APPS9131_IMAGE_ENTRY_POINT;
    ep();
    printf("BROKEN BOOT\n");
    while(1){}
    return -1;
}

U_BOOT_CMD(ml9131, 1, 0, do_ml9131, "", "");
