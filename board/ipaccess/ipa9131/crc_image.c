#include "crc_image.h"
#include <common.h>




static u32 ibncrc32(u32 crc, uint8_t *p, int len)
{
    int i;
    crc = ~crc;

    while (--len >= 0)
    {
        crc = crc ^ *p++;

        for (i = 8; --i >= 0;)
        {
            crc = (crc >> 1) ^ (0xedb88320 & -(crc & 1));
        }
    }

    return ~crc;
}




int crc_image(fimage_header_t * header)
{
    u32 crc = ibncrc32(0, ((uint8_t *)(header)) + sizeof(*header), header->code_size);
    return crc == header->code_crc ? 0 : -1;
}
