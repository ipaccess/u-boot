/*
 * Copyright (c) 2015, ip.access Ltd.
 */
#ifndef FIMAGE_H_20150408
#define FIMAGE_H_20150408

#define FIMAGE_HEADER_MAGIC1 0x259518EC
#define FIMAGE_HEADER_MAGIC2 0x4C75363C
#define FIMAGE_HEADER_MAGIC3 0xF5FA51EF
#define FLASH_PARTITION_VERSION 0x00000003
#define UNIFIED_BOOT_COOKIE_MAGIC_NUMBER 0x440aa7f1
#define UNIFIED_BOOT_COOKIE_VERSION 0x0000002a

#define FIMAGE_HEADER_TYPE_APPS9131 0xe8a45c93




struct fimage_header_s
{
    unsigned int type;               /* Image type */
    unsigned int version;            /* Header version */
    unsigned int source;             /* Image source in FLASH */
    unsigned int destination;        /* Image destination in RAM */
    unsigned int image_size;         /* Total image size */
    unsigned int code_size;          /* Code region size */
    unsigned int signature_ptr;      /* Pointer to signature */
    unsigned int signature_size;     /* Size of signature */
    unsigned int cert_chain_ptr;     /* Pointer to certificate chain */
    unsigned int cert_chain_size;    /* Size of certificate chain */
    unsigned int ub_cookie_magic;    /* IBN cookie (identification ) */
    unsigned int ub_cookie_version;  /* IBN cookie version (identification ) */
    unsigned int code_crc;           /* A crc32 over the code area of the IBN (for booting unsigned images) */
    unsigned int unused;             /* Unused (for now) */
    unsigned int swversion;          /* IBN payload version (i.e. version of the wrapped software) */
    unsigned int revocation;         /* IBN revocation count */
    unsigned int reserved;           /* Reserved for runtime status tracking */
    unsigned int magic1;             /* IBN identification */
    unsigned int magic2;             /* IBN identification */
    unsigned int magic3;             /* IBN identification */
};
typedef struct fimage_header_s fimage_header_t;

extern int fimage_init(unsigned int start_block, unsigned int num_blocks, unsigned int image_type, unsigned int max_size, unsigned int entrypoint, fimage_header_t * table, unsigned int * num_entries, unsigned int max_entries);

#endif
