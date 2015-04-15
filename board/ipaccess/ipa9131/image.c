/*
 * Copyright (c) 2015, ip.access Ltd.
 */
#include <common.h>

#include "image.h"
#include "flash.h"
#include "fimage.h"
#include "security.h"
#include "verify_image.h"


#define FIMAGE_MAX_REPLICA_COUNT 4

#define IMAGE_LOAD_ERROR 42
#define IMAGE_VERIFY_ERROR 43
#define IMAGE_REVOKED_ERROR 44

struct fimage_table_s
{
    fimage_header_t images[FIMAGE_MAX_REPLICA_COUNT];
    unsigned int entries;
};
typedef struct fimage_table_s fimage_table_t;

static fimage_table_t fimage_table;




static int get_image_limits(unsigned int image_type, unsigned int * max_size, unsigned int * loadaddress)
{
    if (image_type == FIMAGE_HEADER_TYPE_APPS9131)
    {
        *max_size = APPS9131_MAX_IMAGE_SIZE;
        *loadaddress = APPS9131_IMAGE_ENTRY_POINT;
        return 0;
    }

    return -1;
}




/**
 * Determine if an image should be preferred over another image
 *
 * The rules for image preference determination:
 * <ul>
 *  <li>If the image to consider is NULL, then it shall not be chosen.</li>
 *  <li>If the comparison base image is NULL then the image to consider shall be chosen.</li>
 *  <li>If the image to consider has a revocation count higher the the comparison base image then the image to consider shall be chosen.</li>
 *  <li>If the image to consider has a version number higher than the comparison base image then the image to consider shall be chosen.</li>
 * </ul>
 *
 * @param a [IN] The image to consider using
 * @param b [IN] The image to compare against
 * @return A truth value if \c a is more suitable for use than \c b
 */
static int is_more_suitable(fimage_header_t * a, fimage_header_t * b)
{
    if (!a)
    {
        return 0;
    }

    if (!b)
    {
        return 1;
    }

    if (a->revocation > b->revocation)
    {
        return 1;
    }

    if (a->revocation == b->revocation)
    {
        return a->swversion > b->swversion;
    }

    return 0;
}



int load_image(unsigned int start_block, unsigned int num_blocks, unsigned int image_type)
{
    unsigned int max_image_size;
    unsigned int image_load_address;
    unsigned int revocation_count;
    unsigned int i;
    fimage_header_t * most_suitable;
    fimage_header_t * fallback;

    read_board_revocation_count(&revocation_count);

    if (0 != get_image_limits(image_type, &max_image_size, &image_load_address))
    {
        return -1;
    }

    if (0 != flash_init())
    {
        return -1;
    }

    if (0 != fimage_init(start_block, num_blocks, image_type, max_image_size, image_load_address, fimage_table.images, &fimage_table.entries, FIMAGE_MAX_REPLICA_COUNT))
    {
        return -1;
    }

    /*
     * We only get back images that match the requested image type and do not exceed
     * the maximum size specified.
     */

    for (i = 0; i < fimage_table.entries; ++i)
    {
        fimage_table.images[i].reserved_3 = 0;

        if (0 != flash_read_bytes((unsigned char *)image_load_address,
                                  fimage_table.images[i].source - sizeof(fimage_header_t),
                                  fimage_table.images[i].image_size + sizeof(fimage_header_t)))
        {
            fimage_table.images[i].reserved_3 = IMAGE_LOAD_ERROR;
            continue;
        }

        if (0 != verify_image(&fimage_table.images[i]))
        {
            fimage_table.images[i].reserved_3 = IMAGE_VERIFY_ERROR;
            puts("Image failed verification\n");
            continue;
        }

        if (fimage_table.images[i].revocation > revocation_count)
        {
            fimage_table.images[i].reserved_3 = IMAGE_REVOKED_ERROR;
            puts("Image is revoked\n");
            continue;
        }
    }

    /*
     * Pick the most suitable software
     *
     * The image with the higher revocation count wins
     * If revocation counts match, then the image with the higher software version wins
     * If more than one copy with the same version number exists, then the first of those images found wins
     *
     * While we're at it we select a fallback choice (in case of NAND read errors) using the same
     * rules as above.
     */

    most_suitable = NULL;
    fallback = NULL;

    for (i = 0; i < fimage_table.entries; ++i)
    {
        if (0 != fimage_table.images[i].reserved_3)
        {
            continue;
        }

        if (is_more_suitable(&fimage_table.images[i], most_suitable))
        {
            if (is_more_suitable(most_suitable, fallback))
            {
                fallback = most_suitable;
            }

            most_suitable = &fimage_table.images[i];
        }
        else
        {
            if (is_more_suitable(&fimage_table.images[i], fallback))
            {
                fallback = &fimage_table.images[i];
            }
        }
    }

    /*
     * Load up the most suitable software image, reverting to the fallback image in event
     * of NAND read failure.
     */

    if (!most_suitable)
    {
        return -1;
    }

    if (0 != flash_read_bytes((unsigned char *)image_load_address, most_suitable->source, most_suitable->image_size))
    {
        if (fallback)
        {
            if (0 != flash_read_bytes((unsigned char *)image_load_address, fallback->source, fallback->image_size))
            {
                return -1;
            }
        }
        else
        {
            return -1;
        }
    }

    return 0;
}
