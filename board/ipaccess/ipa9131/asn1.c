/*
 * Copyright (c) 2014, ip.access Ltd.
 */
#include "asn1.h"

#include <linux/string.h>



int asn1_read_tlv(const unsigned char * data, unsigned int size, asn1_tag_and_length * tlv)
{
    unsigned int offset;

    if (!size)
    {
        goto failure;
    }

    tlv->class = data[0] & 0xC0;
    tlv->is_constructed = (data[0] & 0x20) ? 1 : 0;

    if ((data[0] & 0x1F) < 31)
    {
        tlv->type = data[0] & 0x1F;
        offset = 1; /* skip ahead to the length data */
    }
    else
    {
        tlv->type = 0;

        for (offset = 1; ; ++offset)
        {
            if (offset > 4)
            {
                goto failure; /* invalid tag, must have a size less than 4 octets */
            }

            if (!(size - offset))
            {
                goto failure; /* not enough bytes available to decode the tag */
            }

            tlv->length = (tlv->length << 7) | (data[offset] & 0x7F);

            if (data[offset] & 0x80)
            {
                break; /* if bit 8 is set we're done */
            }
        }

        ++offset; /* skip ahead to the length data */
    }

    if (!(size - offset))
    {
        goto failure; /* no data for the length portion */
    }

    if (data[offset] < 128)
    {
        tlv->length = data[offset];
        ++offset;
    }
    else if (data[offset] > 128 && data[offset] < 255)
    {
        unsigned int num_octets = data[offset] & 0x7F;

        if (num_octets > 4)
        {
            goto failure;
        }

        if ((size - offset) < num_octets)
        {
            goto failure; /* not enough octets remain */
        }

        tlv->length = 0;

        for (offset++; num_octets > 0; num_octets--)
        {
            tlv->length = (tlv->length << 8) | data[offset++];
        }
    }
    else
    {
        goto failure; /* this is DER, so we don't accept indefinite length */
    }

    if ((size - offset) < tlv->length)
    {
        goto failure; /* not enough octets remain for the stated length */
    }

    tlv->value = data + offset;
    tlv->size = offset + tlv->length;
    tlv->location = data;
    return 1;

failure:
    return 0;
}




int asn1_check_tlv(const asn1_tag_and_length * tlv, unsigned char is_constructed, unsigned char class, unsigned char type)
{
    return tlv->is_constructed == is_constructed &&
        tlv->class == class &&
        tlv->type == type;
}




void asn1_reset(asn1_tag_and_length * tlv)
{
    if (tlv)
    {
        memset(tlv, 0, sizeof(*tlv));
    }
}
