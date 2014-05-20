
#include "ipafsm92xx_nand.h"
#include "flash.h"

/* global structure */
flash_info_t flash_info[CONFIG_SYS_MAX_FLASH_BANKS];


static struct mtd_info* _mtd = &nand_info[0];

int msm_nand_probe(struct mtd_info *mtd);          //TODO use header

/* global funcations define in flash.h */
unsigned long flash_init (void)
{
    int retval;

    //printf("%s mtd->writesize:%d\n",__func__,_mtd->writesize);

    retval = msm_nand_probe(_mtd);

    //printf("%s after msm_nand_probe %d mtd->writesize:%d\n",__func__,retval,_mtd->writesize);
 
    return _mtd->size;
}

void flash_print_info (flash_info_t* info)
{
    printf("%s\n",__func__);
     int i;

    puts("FSM29xx flash\n  Size: ");
    print_size(info->size, " in ");
    printf("%d Sectors\n", info->sector_count);

    printf("  Sector Start Addresses:");
    for (i=0; i<info->sector_count; ++i) {
        if ((i % 5) == 0)
            printf("\n   ");
        printf(" %08lX%s",
            info->start[i],
            info->protect[i] ? " (RO)" : "     "
        );
    }
    printf ("\n"); 
}

int flash_erase(flash_info_t* info, int first_sector, int last_sector)
{
    printf("%s %d -> %d\n",__func__,first_sector,last_sector);
    return 0;
}

int write_buff (flash_info_t *info, uchar *src, ulong addr, ulong cnt)
{
    printf("%s\n",__func__);
    return 0;
}

//**********************************
/* public funcations */


int ipafsm92xx_nand_init(void)
{
    int retval = 0;

    retval = nand_register(0); //TODO register each device
    if (retval)
        return -1; 
    return 0;
}
