#include <common.h>
#include <bootcount.h>
#include <part.h>
#include <malloc.h>
#include <linux/compiler.h>

/*
 * Finds the eMMC partition that contains the bootcount.
 *
 * Returns a pointer to the partition information for the characterisation
 * partition, which contains the bootcount. This function will check the
 * partition's name, and only return a pointer if it is 'chr'.
 *
 * Returns a NULL if the partition cannot be found.
 *
 */
disk_partition_t * find_bc_part()
{
    char *partition=NULL;
    disk_partition_t * info_p;
    block_dev_desc_t * dev_desc;
    int part;
    char devAndPart[10];

    if (NULL == (partition=getenv("chr_pnum")))
        partition = "0x1D"; //use default

    info_p = malloc(sizeof(disk_partition_t));

    if (!info_p)
    {
        puts("Bootcount: Could not allocate memory for partition info.\n");
        return NULL;
    }

    // Combine partition number with device number
    snprintf(devAndPart, 10, "0:%s", partition);

    part = get_device_and_partition("mmc", devAndPart, &dev_desc, info_p, 1);
    if (part < 0)
    {
        printf("Bootcount: Could not find partition %s.\n", devAndPart);
        free(info_p);
        return NULL;
    }

    // Check the partition name looks correct
    if ((strlen(info_p->name)            == 3) &&
        (strncmp(info_p->name, "chr", 3) == 0)    )
    {
        return info_p;
    }
    else
    {
        printf("Bootcount: Partition %i name %s incorrect, should be chr\n", part, info_p->name);
        free(info_p);
        return NULL;
    }
}

void bootcount_store(ulong a)
{
    int                ret;
    char               command_buffer[100];
    disk_partition_t * info_p;

    lbaint_t   bcBlockNum;
    uint8_t  * bootcountBlock_p;
    uint8_t    bootcount;

    info_p = find_bc_part();
    if (!info_p)
    {
        puts("Bootcount: Error finding bootcount eMMC partition.\n");
        return;
    }

    bcBlockNum = info_p->start + 1; // Move to second block in characterisation partition

    bootcountBlock_p = malloc(info_p->blksz);
    if (!bootcountBlock_p)
    {
        puts("Bootcount: Could not allocate memory for block to write to eMMC.\n");
        free(info_p);
        return;
    }

    // Set the bootcount magic and bootcount value
    memset(bootcountBlock_p, 0x00, info_p->blksz);
    bootcountBlock_p[0] = 'B';
    bootcountBlock_p[1] = 't';
    bootcountBlock_p[2] = 'C';
    bootcountBlock_p[3] = 't';
    bootcountBlock_p[4] = (a & 0xff);

    snprintf(command_buffer,sizeof(command_buffer),"mmc write 0x%08x 0x%08x 1", bootcountBlock_p, bcBlockNum);

    // Write the block to eMMC
    ret = run_command(command_buffer,0);
    if (ret != 0)
    {
        puts("Bootcount: Error writing eMMC\n");
    }

    // Clean up
    free(info_p);
    free(bootcountBlock_p);
}

ulong bootcount_load(void)
{
    int                ret;
    char               command_buffer[100];
    disk_partition_t * info_p;

    lbaint_t   bcBlockNum;
    uint8_t  * bootcountBlock_p;
    uint8_t    bootcount;
    
    info_p = find_bc_part();
    if (!info_p)
    {
        puts("Bootcount: Error finding bootcount eMMC partition.\n");
        return 0;
    }

    bcBlockNum = info_p->start + 1; // Move to second block in characterisation partition

    bootcountBlock_p = malloc(info_p->blksz);
    if (!bootcountBlock_p)
    {
        puts("Bootcount: Could not allocate memory to read block from eMMC.\n");
        free(info_p);
        return 0;
    }
    snprintf(command_buffer,sizeof(command_buffer),"mmc read 0x%08x 0x%08x 1", bootcountBlock_p, bcBlockNum);

    // Read the block from eMMC
    ret = run_command(command_buffer,0);
    if (ret != 0)
    {
        puts("Bootcount: Error reading eMMC\n");
        free(info_p);
        free(bootcountBlock_p);
        return 0;
    }

    // Check for the bootcount magic
    if ( (bootcountBlock_p[0] == 'B') &&
         (bootcountBlock_p[1] == 't') &&
         (bootcountBlock_p[2] == 'C') &&
         (bootcountBlock_p[3] == 't')
        )
    {
        bootcount = bootcountBlock_p[4];
    }
    else
    {
        bootcount = 0x00;
    }

    // Clean up
    free(info_p);
    free(bootcountBlock_p);

#if defined(CONFIG_BOOTCOUNT_FSM90XX_MODULUS)
    return bootcount % CONFIG_BOOTCOUNT_FSM90XX_MODULUS;
#else
    return bootcount;
#endif
}

static int do_bootcount_combined_exceeded(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
    ulong bc = bootcount_load();

    if ( bc > (CONFIG_BOOTCOUNT_LIMIT_COUNT * 2) )
        return CMD_RET_SUCCESS;
    else
        return CMD_RET_FAILURE;
}


U_BOOT_CMD(
    bootcount_combined_exceeded, 1, 0, do_bootcount_combined_exceeded,
    "Determines if the boot count limit has been exceeded for both images",
    ""
);

