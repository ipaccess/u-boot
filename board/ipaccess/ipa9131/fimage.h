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
    unsigned int reserved_0;         /* Unused */
    unsigned int reserved_1;         /* Unused */
    unsigned int swversion;          /* IBN payload version (i.e. version of the wrapped software) */
    unsigned int revocation;         /* IBN revocation count */
    unsigned int reserved_3;         /* Unused */
    unsigned int magic1;             /* IBN identification */
    unsigned int magic2;             /* IBN identification */
    unsigned int magic3;             /* IBN identification */
};
typedef struct fimage_header_s fimage_header_t;

#define FIMAGE_HEADER_SIZE (sizeof(struct fimage_header_s))

#endif //FIMAGE_H_20150408
