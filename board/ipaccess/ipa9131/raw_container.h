/*
 * Copyright (c) 2015, ip.access Ltd. All Rights Reserved.
 */
#ifndef RAW_CONTAINER_H_20150611
#define RAW_CONTAINER_H_20150611

#include <common.h>

#define UINT16_C(value) value##U

#define RAW_CONTAINER_TAG_PUBLIC_KEY UINT16_C(0xef02)
#define RAW_CONTAINER_TAG_ADVERT_DATA UINT16_C(0xef03)
#define RAW_CONTAINER_TAG_PRIVATE_KEY UINT16_C(0xef04)
#define RAW_CONTAINER_TAG_PRIVATE_KEY_BLOB UINT16_C(0xef05)
#define RAW_CONTAINER_TAG_CSR UINT16_C(0xef06)
#define RAW_CONTAINER_TAG_CONTAINER_CHECKSUM UINT16_C(0xef01)

struct container_field_t
{
    const uint8_t * value;
    struct container_field_t * prev;
    struct container_field_t * next;
    uint16_t tag;
    uint16_t length;
    uint8_t pad[4];
};

struct raw_container_t
{
	uint32_t partnum;
	uint32_t eraseblock_size;
	struct container_field_t * fields;
	uint8_t * data;
	uint32_t len;
	uint8_t faulty;
	uint32_t length; /* the total length of the raw container data */
};

extern void free_container_fields(struct container_field_t * head);
extern void free_container_fields_and_values(struct container_field_t * head);
extern void put_container_field(struct container_field_t ** head, struct container_field_t * field);
extern void take_container_field(struct container_field_t ** head, const struct container_field_t * field);
extern struct container_field_t * find_container_field(uint16_t tag, struct container_field_t * head);
extern int container_union(struct container_field_t * a, struct container_field_t * b, struct container_field_t ** head);
extern int field_compare(struct container_field_t * a, struct container_field_t * b);
extern int container_compare(struct container_field_t * a, struct container_field_t * b);

extern int parse_container(const uint8_t * data, uint32_t len, int (*hashfunc)(const uint8_t *, uint32_t, uint8_t *, uint32_t), struct container_field_t ** fields, uint32_t * length);
extern int serialise_container(uint8_t * data, uint32_t * len, int (*hashfunc)(const uint8_t *, uint32_t, uint8_t *, uint32_t), const struct container_field_t * fields);

#endif
