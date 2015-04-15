/*
 * Copyright (c) 2015, ip.access Ltd.
 */
#include "fimage.h"
#include "flash.h"

#include <linux/string.h>


/*
 * Scan the NAND flash for ip.access FIMAGE headers
 *
 * @param start_block [IN] The block at which to start the scan
 * @param num_blocks [IN] The number of blocks to scan
 * @param image_type [IN] The image type to search for
 * @param max_size [IN] Maximum size of an image
 * @param entrypoint [IN] Required entry point for an image
 * @param table [OUT] The table to store the results in, must contain at least \c num_blocks of entries
 * @param num_entries [OUT] The number of entries stored to the \c table variable
 * @param max_entries [IN] The maximum number of entries that can be stored in \c table
 * @return 0 on success
 */
int fimage_init(unsigned int start_block, unsigned int num_blocks, unsigned int image_type, unsigned int max_size, unsigned int entrypoint, fimage_header_t * table, unsigned int * num_entries, unsigned int max_entries)
{
    unsigned int i;

    for (i = start_block; i < (start_block + num_blocks) && *num_entries < max_entries; ++i)
    {
        if (flash_is_block_bad(i))
        {
            continue;
        }

        if (0 != flash_read_bytes((unsigned char *)&table[*num_entries], (flash_block_size() * flash_page_size()) * i, sizeof(fimage_header_t)))
        {
            memset(&table[*num_entries], 0, sizeof(fimage_header_t));
            continue;
        }

        if (table[*num_entries].magic1 == FIMAGE_HEADER_MAGIC1 &&
            table[*num_entries].magic2 == FIMAGE_HEADER_MAGIC2 &&
            table[*num_entries].magic3 == FIMAGE_HEADER_MAGIC3 &&
            table[*num_entries].ub_cookie_magic == UNIFIED_BOOT_COOKIE_MAGIC_NUMBER &&
            table[*num_entries].ub_cookie_version == UNIFIED_BOOT_COOKIE_VERSION &&
            table[*num_entries].version == FLASH_PARTITION_VERSION &&
            table[*num_entries].type == image_type &&
            table[*num_entries].image_size + sizeof(fimage_header_t) <= max_size &&
            table[*num_entries].destination == entrypoint)
        {
            table[*num_entries].source = ((flash_block_size() * flash_page_size()) * i) + sizeof(fimage_header_t);
            ++*num_entries;
            continue;
        }

        memset(&table[*num_entries], 0, sizeof(fimage_header_t));
    }

    return 0;
}
