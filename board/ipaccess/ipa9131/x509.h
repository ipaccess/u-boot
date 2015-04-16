/*
 * Copyright (c) 2014, ip.access Ltd.
 */
#ifndef X509_H_20140925
#define X509_H_20140925

typedef struct x509_certificate_t
{
    unsigned int certificate_size;       /* Size of the raw certificate memory - used when walking the raw DER certificate chain */
    const unsigned char * signature;     /* Signature data (third field of the Certificate structure) - used in establishing a trust chain */
    unsigned int signature_size;         /* Size of signature data - used in establishing a trust chain */
    const unsigned char * tbsCertificate;/* Pointer to the to-be-signed certificate memory - used in establishing a trust chain */
    unsigned int tbsCertificate_size;    /* Size of the to-be-signed certificate memory - used in establishing a trust chain */
    const unsigned char * publicKey;     /* Pointer to the public key memory - used to identify trust roots */
    unsigned int publicKey_size;         /* Size of the public key memory - used to identify trust roots */
    const unsigned char * n;             /* Pointer to the RSA modulus memory - used for signature verification (both payload and trust chain) */
    unsigned int n_size;                 /* Size of the RSA modulus memory - used for signature verification (both payload and trust chain) */
    const unsigned char * e;             /* Pointer to the RSA public exponent memory - used for signature verification (both payload and trust chain) */
    unsigned int e_size;                 /* Size of the RSA public exponent memory - used for signature verification (both payload and trust chain) */
    unsigned int signature_algorithm;    /* Signature algorithm in use by this certificate - used for signature verification when establishing a trust chain */
} x509_certificate;

extern int x509_parse(const unsigned char * data_ptr, unsigned int data_size, x509_certificate * cert);

extern int x509_is_trust_anchor(x509_certificate * cert);
extern int x509_verify(x509_certificate * cert, const unsigned char * signature, unsigned int signature_len, const unsigned char * data, unsigned int data_len);
extern int x509_verify_cert(x509_certificate * parent, x509_certificate * child);

#endif
