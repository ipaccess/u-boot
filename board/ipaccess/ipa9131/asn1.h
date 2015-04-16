/*
 * Copyright (c) 2014, ip.access Ltd.
 */
#ifndef ASN1_H_20140925
#define ASN1_H_20140925

typedef struct asn1_tag_and_length_t
{
    unsigned int length;            /* payload length */
    unsigned int size;              /* total length of the TLV */
    const unsigned char * value;    /* and the payload location please */
    const unsigned char * location; /* location of the TLV */
    unsigned char is_constructed;   /* a boolen location indicating that this is a constructed value */
    unsigned char class;            /* TLV class */
    unsigned char type;             /* TLV type */
    unsigned char padding;
} asn1_tag_and_length;

extern int asn1_read_tlv(const unsigned char * data, unsigned int size, asn1_tag_and_length * tlv);
extern int asn1_check_tlv(const asn1_tag_and_length * tlv, unsigned char is_constructed, unsigned char class, unsigned char type);
extern void asn1_reset(asn1_tag_and_length * tlv);

#endif
