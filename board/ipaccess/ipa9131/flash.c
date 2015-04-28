#include "flash.h"




int flash_read_page(unsigned int page, unsigned char * buffer)
{
    size_t length = nand_info[0].writesize;
    int ret = nand_read(&nand_info[0], nand_info[0].writesize * page, &length, buffer);

    if (ret < 0 && ret != -EUCLEAN)
        return ret;

    if (length != nand_info[0].writesize)
        return -1;

    flush_cache((unsigned long)buffer, nand_info[0].writesize);
    return 0;
}




/*
 * Read bytes from NAND
 *
 * The first eraseblock requested may not be bad, if it is: error
 * The first eraseblock is read using nand_read, as the data requested may not be eraseblock aligned
 * Subsequent eraseblocks are read with nand_read_skip_bad, as we are now eraseblock aligned
 * The greater of (n*2) and 2 megabytes is set as the read limit - blocks other than the first may be bad and are skipped up to this limit
 *
 * This function returns 0 if all requested data could be read
 */
int flash_read_bytes(unsigned char * dst, unsigned int src, unsigned int n)
{
    int ret;
    size_t this_read;
    loff_t flash_offset;
    loff_t lim;
    size_t length;
    size_t actual;
    unsigned char * ptr;

    ptr = dst;
    flash_offset = src;
    this_read = MIN(nand_info[0].erasesize, n);
    this_read -= (src % nand_info[0].erasesize); /* we want to be on an eraseblock boundary after this read */

    length = this_read;

    if (nand_block_isbad(&nand_info[0], flash_offset & ~(loff_t)(nand_info[0].erasesize - 1)))
    {
        return -1;
    }

    ret = nand_read(&nand_info[0], flash_offset, &this_read, ptr);

    if (ret < 0 && ret != -EUCLEAN)
    {
        return ret;
    }

    if (this_read != length)
    {
        return -1;
    }

    length = n - this_read;

    if (length == 0)
    {
        flush_cache((unsigned long)dst, n);
        return 0;
    }

    ptr += this_read;
    flash_offset += this_read;
    this_read = length;

    lim = MAX((loff_t)0x00200000, (loff_t)(n * 2)) - (loff_t)nand_info[0].erasesize;

    ret = nand_read_skip_bad(&nand_info[0], flash_offset, &this_read, &actual, lim, ptr);

    if (ret < 0 && ret != -EUCLEAN)
        return ret;

    if (this_read != length)
        return -1;

    flush_cache((unsigned long)dst, n);
    return 0;
}
