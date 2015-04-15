#include "fimage.h"

#include <common.h>
#include <command.h>
#include <nand.h>

static uint32_t block_size = 0;
static uint32_t block_count = 0;


// 2 second stage u-boots in flash, one at 1MB and one at 2MB.
typedef void (*entry_func_ptr)(void);

static void fimage_init(void)
{
    nand_info_t* nand = &nand_info[0];

    block_size = nand->erasesize;
    block_count = nand->size / nand->erasesize;
}

// Scand a block for a fimage header.
// ret:
//  -2 : read error.
//  -1 : bad block.
//  0  : no fimage found.
//  1  : fimage found.
static int scan_block(uint32_t block_index, fimage_header_t* header)
{
    nand_info_t* nand = &nand_info[0];
    loff_t flash_offset = block_index * block_size;
    size_t tmp_header_size = FIMAGE_HEADER_SIZE;

    if(nand_block_isbad(nand, flash_offset))
        return -1;

    memset(header, 0, FIMAGE_HEADER_SIZE);
    if(0 > nand_read(nand, flash_offset, &tmp_header_size, (u_char*)header))
        return -2;

    if( header->magic1              == FIMAGE_HEADER_MAGIC1 &&
        header->magic2              == FIMAGE_HEADER_MAGIC2 &&
        header->magic3              == FIMAGE_HEADER_MAGIC3 &&
        header->ub_cookie_magic     == UNIFIED_BOOT_COOKIE_MAGIC_NUMBER &&
        header->ub_cookie_version   == UNIFIED_BOOT_COOKIE_VERSION &&
        header->version             == FLASH_PARTITION_VERSION &&
        header->type                == FIMAGE_HEADER_TYPE_APPS9131 )
    {
        return 1;
    }
    else
    {
        return 0;
    }
}

static void boot_image(fimage_header_t* header)
{
    entry_func_ptr func = (entry_func_ptr)(header->destination);

    flush_cache(header->destination, header->image_size);
    // We should never return from this.
    func();
    printf("BROKEN BOOT\n");
    while(1){}
}

// return 0 - on success
static int verify_image(fimage_header_t* header)
{
    #warning "TODO verify_image"
    return 0;
}

// return 0 - on success
static int load_image(uint32_t block_index, fimage_header_t* header)
{
    nand_info_t* nand = &nand_info[0];
    loff_t flash_offset = block_index * block_size + FIMAGE_HEADER_SIZE;
    uint32_t remaining_read = header->image_size;
    uint32_t read;
    uint32_t dest = header->destination;

    printf("Loading image at block %d to 0x%x size %d\n", block_index, header->destination, header->image_size);

    // The first block must be non-bad as the scan has already found the header in
    // this block. Read the rest of the first block into memory.
    read = MIN(block_size, remaining_read);
    read -= FIMAGE_HEADER_SIZE;

    //printf("%s reading %d, remaining %d dest 0x%x offset 0x%llx\n", __func__, read, remaining_read, dest, flash_offset);
    if(0 > nand_read(nand, flash_offset, &read, (uchar*)dest))
    {
        printf("Error: %s failed nand_read\n", __func__);
        return -1;
    }

    flash_offset += read;
    dest += read;
    remaining_read -= read;

    // Now reading on a block boundary
    while(remaining_read)
    {
        while(nand_block_isbad(nand, flash_offset))
        {
            printf("%s skipping bad block\n", __func__);
            // Move to the next block
            flash_offset += block_size;
        }

        read = MIN(block_size, remaining_read);

        //printf("%s reading %d, remaining %d dest 0x%x offset 0x%llx\n", __func__, read, remaining_read, dest, flash_offset);
        if(0 > nand_read(nand, flash_offset, &read, (uchar*)dest))
        {
            printf("Error: %s failed nand_read\n", __func__);
            return -1;
        }

        flash_offset += read;
        dest += read;
        remaining_read -= read;
    }
    return 0;
}

static void fimage_scan(void)
{
    uint32_t b;
    int bad_count = 0;
    int read_errors = 0;
    int ret;
    fimage_header_t header;

    printf("Scanning %d blocks for fimage headers.\n", block_count);

    for(b = 0; b < block_count; b++)
    {
        ret = scan_block(b, &header);
        if(-1 == ret)
        {
            bad_count++;
        }
        else if(-2 == ret)
        {
            read_errors++;
        }
        else if(1 == ret)
        {
            printf("Found fimage at block %d\n", b);
            printf("Size %d\n", header.image_size);
            printf("Src 0x%x\n", header.source);
            printf("Dst 0x%x\n", header.destination);
        }
    }
    printf("Bad blocks: %d\n", bad_count);
    printf("Read errors: %d\n", read_errors);
}

static void fimage_boot(void)
{
    uint32_t b;
    int ret;
    fimage_header_t header;

    for(b = 0; b < block_count; b++)
    {
        ret = scan_block(b, &header);
        if(1 == ret)
        {
            if(0 != load_image(b, &header))
            {
                printf("%s failed to load image at %d\n", __func__, b);
                continue;
            }

            if(0 != verify_image(&header))
            {
                printf("%s failed to verify image at %d\n", __func__, b);
                continue;
            }

            boot_image(&header);
        }
    }
}

static int do_fimage(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
    char* cmd;

    if (argc < 1)
        goto usage;

    cmd = argv[1];

    fimage_init();

    if(0 == strcmp(cmd, "scan"))
    {
        fimage_scan();
        return 0;
    }

    if(0 == strcmp(cmd, "boot"))
    {
        fimage_boot();
        return 0;
    }

usage:
    return CMD_RET_USAGE;
}

static char fimage_help_text[] =
    "scan - scans flash for fimage headers.\n"
    "fimage boot - boots the first image found that passes verification.\n";

U_BOOT_CMD(
        fimage, CONFIG_SYS_MAXARGS, 1, do_fimage,
        "Perform fimage related commands.",
        fimage_help_text
);
