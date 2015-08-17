/*
 * Copyright (c) 2015, ip.access Ltd. All Rights Reserved.
 */
#include "raw_container.h"

#include <linux/stddef.h>
#include <errno.h>
#include <common.h>
#include <malloc.h>
#include <linux/string.h>
//#include <stdlib.h>


#define CONTAINER_VERSION_1 0x1
#define CONTAINER_RESERVED 0x0
#define CONTAINER_MCLASS 0xFE
#define CONTAINER_MTYPE 0x1

static int parse_tag(const uint8_t * data, uint32_t len, uint32_t * consumed, uint16_t * tag)
{
    if (len < 2)
    {
        return -EBADMSG;
    }

    memcpy(tag, data, 2);
    *consumed = 2;
    return 0;
}

static int parse_length(const uint8_t * data, uint32_t len, uint32_t * consumed, uint16_t * length)
{
    if (len < 2)
    {
        return -EBADMSG;
    }

    memcpy(length, data, 2);
    *consumed = 2;
    return 0;
}

static int parse_value(const uint8_t * data, uint32_t len, uint32_t * consumed, uint16_t length, const uint8_t ** value)
{
    if (len < length)
    {
        return -EBADMSG;
    }

    *value = data;

    while (length % 4)
    {
        ++length;
    }

    *consumed = length;
    return 0;
}

static int parse_tlv(const uint8_t * data, uint32_t len, uint32_t * consumed, uint16_t * tag, uint16_t * length, const uint8_t ** value)
{
    uint32_t c;
    int ret;

    if (0 > (ret = parse_tag(data, len, &c, tag)))
    {
        return ret;
    }

    len -= c;
    data += c;
    *consumed += c;

    if (0 > (ret = parse_length(data, len, &c, length)))
    {
        return ret;
    }

    len -= c;
    data += c;
    *consumed += c;
    *length -= 4; /* the length reported to the user is the actual length of the data */

    if (0 > (ret = parse_value(data, len, &c, *length, value)))
    {
        return ret;
    }

    *consumed += c;

    return 0;
}

static int parse_header(const uint8_t * data, uint32_t len, uint32_t * consumed, uint8_t * version, uint8_t * reserved, uint8_t * mclass, uint8_t * mtype, uint32_t * length)
{
    if (len < 8)
    {
        return -EBADMSG;
    }

    *version = data[0];
    *reserved = data[1];
    *mclass = data[2];
    *mtype = data[3];
    memcpy(length, data + 4, 4);
    *consumed += 8;
    return 0;
}

/*
 * Free container fields in the passed linked list
 */
void free_container_fields(struct container_field_t * head)
{
    struct container_field_t * tmp;

    while (head)
    {
        tmp = head;
        head = head->next;
        free(tmp);
    }
}

/*
 * Free container fields and their values in the passed linked list
 */
void free_container_fields_and_values(struct container_field_t * head)
{
    struct container_field_t * tmp;

    while (head)
    {
        tmp = head;
        head = head->next;

        if (tmp->value)
        {
            free((uint8_t *)tmp->value);
        }

        free(tmp);
    }
}

/*
 * Append a container field to a linked list
 *
 * After this operation the linked list is responsible for the field memory
 */
void put_container_field(struct container_field_t ** head, struct container_field_t * field)
{
    struct container_field_t * tmp;

    if (!*head)
    {
        *head = field;
        return;
    }

    tmp = *head;

    while (tmp->next)
    {
        tmp = tmp->next;
    }

    field->prev = tmp;
    tmp->next = field;
}


/*
 *  * Append a container field to a linked list
 *   *
 *    * After this operation the linked list is responsible for the field memory
 *     */
void debug_container_field(struct container_field_t * head)
{
    struct container_field_t * tmp;

    if (!head)
    {
        return;
    }

    tmp = head;

    while (tmp->next)
    {
        tmp = tmp->next;
    }

}
/*
 * Remove a container field from a linked list
 *
 * After this operation the caller is responsible for the field memory
 */
void take_container_field(struct container_field_t ** head, const struct container_field_t * field)
{
    struct container_field_t * tmp;

    if (!*head)
    {
        return;
    }

    tmp = *head;

    do {
        if (tmp == field)
        {
            if (tmp->prev)
            {
                tmp->prev->next = tmp->next;
            }

            if (tmp->next)
            {
                tmp->next->prev = tmp->prev;
            }

            if (*head == tmp)
            {
                if (tmp->next)
                {
                    *head = tmp->next;
                }
                else
                {
                    *head = NULL;
                }
            }

            tmp->prev = tmp->next = NULL;
            return;
        }

        tmp = tmp->next;
    } while (tmp);
}

/*
 * Find a container field with a given tag in a linked list
 */
struct container_field_t * find_container_field(uint16_t tag, struct container_field_t * head)
{
    while (head)
    {
        if (head->tag == tag)
        {
            return head;
        }

        head = head->next;
    }

    return NULL;
}

static struct container_field_t * container_field_clone(struct container_field_t * a)
{
    struct container_field_t * current;

    if (!(current = calloc(sizeof(struct container_field_t), 1)))
    {
        return NULL;
    }

    memcpy(current, a, sizeof(*current));
    current->prev = current->next = NULL;
    return current;
}

static struct container_field_t * container_clone(struct container_field_t * a)
{
    struct container_field_t * head;
    struct container_field_t * tmp;

    head = NULL;

    while (a)
    {
        tmp = container_field_clone(a);

        if (!tmp)
        {
            free_container_fields(head);
            return NULL;
        }

        put_container_field(&head, tmp);
        a = a->next;
    }

    return head;
}

/*
 * Perform a union of two linked lists, returning the results in a third
 *
 * The values in the 'b' linked list take precedence over those in the 'a' linked list
 */
int container_union(struct container_field_t * a, struct container_field_t * b, struct container_field_t ** head)
{
    struct container_field_t * current;
    struct container_field_t * tmp;

    *head = container_clone(b);

    if (!*head && b)
    {
        return -ENOMEM;
    }

    current = a;

    while (current)
    {
        if (!find_container_field(current->tag, *head))
        {
            if (!(tmp = container_field_clone(current)))
            {
                free_container_fields(*head);
                *head = NULL;
                return -ENOMEM;
            }

            put_container_field(head, tmp);
        }

        current = current->next;
    }

    return 0;
}

int field_compare(struct container_field_t * a, struct container_field_t * b)
{
    if (!a && !b)
    {
        return 0;
    }

    if ((a && !b) || (!a && b))
    {
        return 1;
    }

    if (a->tag != b->tag || a->length != b->length)
    {
        return 1;
    }

    if (a->value == b->value)
    {
        return 0;
    }

    if (0 != memcmp(a->value, b->value, a->length))
    {
        return 1;
    }

    return 0;
}

static int simple_compare(struct container_field_t * a, struct container_field_t * b)
{
    struct container_field_t * tmp;

    while (a)
    {
        if (!(tmp = find_container_field(a->tag, b)))
        {
            return 1;
        }

        a = a->next;
    }

    return 0;
}

int container_compare(struct container_field_t * a, struct container_field_t * b)
{
    struct container_field_t * tmp;

    if ((a && !b) || (!a && b))
    {
        return 1;
    }

    if (!a && !b)
    {
        return 0;
    }

    tmp = a;

    while (tmp)
    {
        if (0 != field_compare(tmp, find_container_field(tmp->tag, b)))
        {
            return 1;
        }

        tmp = tmp->next;
    }

    tmp = b;

    while (tmp)
    {
        if (0 != field_compare(tmp, find_container_field(tmp->tag, a)))
        {
            return 1;
        }

        tmp = tmp->next;
    }

    return 0;
}

/*
 * Parse a raw container data blob, verifying the container checksum and returning a linked list of fields
 */
int parse_container(const uint8_t * data, uint32_t len, int (*hashfunc)(const uint8_t *, uint32_t, uint8_t *, uint32_t), struct container_field_t ** fields, uint32_t * total_length)
{
    int ret;
    uint8_t version;
    uint8_t reserved;
    uint8_t mclass;
    uint8_t mtype;
    uint32_t length;
    uint32_t cksum_len;
    uint32_t consumed;
    struct container_field_t * current;
    uint8_t hash[32];
    const uint8_t * ptr;

    current = NULL;
    *fields = NULL;
    consumed = 0;
    ptr = data;

    if (0 > (ret = parse_header(ptr, len, &consumed, &version, &reserved, &mclass, &mtype, &length)))
    {
        return ret;
    }

    if (version != CONTAINER_VERSION_1 ||
        reserved != CONTAINER_RESERVED ||
        mclass != CONTAINER_MCLASS ||
        mtype != CONTAINER_MTYPE)
    {
        return -EBADMSG;
    }

    if (length > len)
    {
        return -EBADMSG;
    }

    if (length < 44) /* 8 byte header + 36 byte checksum */
    {
        return -EBADMSG;
    }

    *total_length = length;

    cksum_len = length - 32; /* where 32 is the number of octets used for the checksum (sha256) */
    length -= consumed;
    ptr += consumed;

    while (length)
    {
        consumed = 0;

        if (!(current = calloc(sizeof(struct container_field_t), 1)))
        {
            ret = -errno;
            goto cleanup;
        }

        put_container_field(fields, current);

        if (0 > (ret = parse_tlv(ptr, length, &consumed, &current->tag, &current->length, &current->value)))
        {
            goto cleanup;
        }

        length -= consumed;
        ptr += consumed;
    }

    if (!(current = find_container_field(RAW_CONTAINER_TAG_CONTAINER_CHECKSUM, *fields)))
    {
        ret = -EBADMSG;
        goto cleanup;
    }

    if (current->length != (uint32_t)sizeof(hash))
    {
        ret = -EBADMSG;
        goto cleanup;
    }

    if (0 > (ret = hashfunc(data, cksum_len, hash, current->length)))
    {
        goto cleanup;
    }

    if (0 != memcmp(current->value, hash, current->length))
    {
        ret = -EBADMSG;
        goto cleanup;
    }

    take_container_field(fields, current);
    free_container_fields(current);

    ret = 0; /* we're all good */

cleanup:
    if (ret)
    {
        if (*fields)
        {
            free_container_fields(*fields);
            *fields = NULL;
        }
    }

    return ret;
}

/*
 * Round a value up to the nearest multiple of four
 */
static uint16_t round_up(uint16_t v)
{
    return ((v - UINT16_C(1)) | UINT16_C(3)) + UINT16_C(1);
}

/*
 * Serialise a container header into a raw data block
 */
static int serialise_header(uint8_t * data, uint32_t len, uint32_t * consumed, uint8_t version, uint8_t reserved, uint8_t mclass, uint8_t mtype, uint32_t length)
{
    if (len < 8)
    {
        return -EOVERFLOW;
    }

    data[0] = version;
    data[1] = reserved;
    data[2] = mclass;
    data[3] = mtype;
    memcpy(data + 4, &length, 4);
    *consumed = 8;
    return 0;
}

/*
 * Serialise a tag-length-value structure into a raw data block, padding the serialised data to a multiple of four octets
 */
static int serialise_tlv(uint8_t * data, uint32_t len, uint32_t * consumed, uint16_t tag, uint16_t length, const uint8_t * value)
{
    uint16_t flen;
    uint16_t rlen;
    static const uint8_t pad[3] = {0x0, 0x0, 0x0};

    flen = length + 4;
    rlen = round_up(flen);

    if (len < rlen)
    {
        return -EOVERFLOW;
    }

    memcpy(data, &tag, 2);
    *consumed = 2;
    memcpy(data + *consumed, &flen, 2);
    *consumed += 2;
    memcpy(data + *consumed, value, length);
    *consumed += length;

    if (rlen - flen)
    {
        if (rlen - flen > 3)
        {
            return -EFAULT;
        }

        memcpy(data + *consumed, pad, (rlen - flen));
        *consumed += (rlen - flen);
    }

    return 0;
}

/*
 * Serialise a checksum field header into a raw data block
 *
 * This function writes a length field suitable for 32 bytes of checksum data (SHA-256)
 */
int serialise_checksum_header(uint8_t * data, uint32_t len, uint32_t * consumed)
{
    const uint16_t checksum_tag = RAW_CONTAINER_TAG_CONTAINER_CHECKSUM;
    static const uint16_t checksum_len = 36;

    if (len < 4)
    {
        return -EOVERFLOW;
    }

    memcpy(data, &checksum_tag, 2);
    *consumed = 2;
    memcpy(data + *consumed, &checksum_len, 2);
    *consumed += 2;

    return 0;
}

/*
 * Serialises a container linked list into a raw bock of data including the checksum field
 */
int serialise_container(uint8_t * data, uint32_t * len, int (*hashfunc)(const uint8_t *, uint32_t, uint8_t *, uint32_t), const struct container_field_t * fields)
{
    int ret;
    const struct container_field_t * current;
    uint32_t total_length;
    uint32_t c;
    uint32_t consumed;

    c = 0;
    consumed = 0;

    total_length = 8; /* container header */
    total_length += 36; /* checksum field */

    current = fields;

    while (current)
    {
        if (current->tag != RAW_CONTAINER_TAG_CONTAINER_CHECKSUM)
        {
            total_length += (uint32_t)(round_up(current->length) + 4);
        }

        current = current->next;
    }

    if (total_length > *len)
    {
        return -EOVERFLOW;
    }

    if (0 > (ret = serialise_header(data + consumed, *len - consumed, &c, CONTAINER_VERSION_1, CONTAINER_RESERVED, CONTAINER_MCLASS, CONTAINER_MTYPE, total_length)))
    {
        return ret;
    }

    consumed += c;

    current = fields;

    while (current)
    {
        if (current->tag != RAW_CONTAINER_TAG_CONTAINER_CHECKSUM)
        {
            c = 0;

            if (0 > (ret = serialise_tlv(data + consumed, *len - consumed, &c, current->tag, current->length, current->value)))
            {
                return ret;
            }

            consumed += c;
        }

        current = current->next;
    }

    c = 0;

    if (0 > (ret = serialise_checksum_header(data + consumed, *len - consumed, &c)))
    {
        return ret;
    }

    consumed += c;

    if (*len - consumed < 32)
    {
        return -EOVERFLOW;
    }

    if (0 > (ret = hashfunc(data, consumed, data + consumed, 32)))
    {
        return ret;
    }

    consumed += 32;

    *len = consumed;
    return 0;
}
