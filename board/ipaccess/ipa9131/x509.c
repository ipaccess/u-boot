/*
 * Copyright (c) 2014, ip.access Ltd.
 */
#include "x509.h"

#include "asn1.h"
#include "hash.h"
#include "security.h"
#include "tfm.h"

#include <linux/string.h>


#define SIG_ALGO_SHA1_WITH_RSA_ENCRYPTION 0
#define SIG_ALGO_SHA256_WITH_RSA_ENCRYPTION 1

#define RSA_OPERATION_SIZE_BITS 2048
#define RSA_OPERATION_SIZE_BYTES (RSA_OPERATION_SIZE_BITS/8)

static const unsigned char sha1Oid[] = { 0x2b, 0x0e, 0x03, 0x02, 0x1a };
static const unsigned int sha1OidSize = sizeof(sha1Oid) / sizeof(sha1Oid[0]);
static const unsigned char sha256Oid[] = { 0x60, 0x86, 0x48, 0x01, 0x65, 0x03, 0x04, 0x02, 0x01 };
static const unsigned int sha256OidSize = sizeof(sha256Oid) / sizeof(sha256Oid[0]);

static const unsigned char sha1WithRSAEncryption[] = { 0x2a, 0x86, 0x48, 0x86, 0xf7, 0x0d, 0x01, 0x01, 0x05 };
static const unsigned int sha1WithRSAEncryptionSize = sizeof(sha1WithRSAEncryption) / sizeof(sha1WithRSAEncryption[0]);
static const unsigned char sha256WithRSAEncryption[] = { 0x2a, 0x86, 0x48, 0x86, 0xf7, 0x0d, 0x01, 0x01, 0x0b };
static const unsigned int sha256WithRSAEncryptionSize = sizeof(sha256WithRSAEncryption) / sizeof(sha256WithRSAEncryption[0]);

static const unsigned char rsaEncryption[] = { 0x2a, 0x86, 0x48, 0x86, 0xf7, 0x0d, 0x01, 0x01, 0x01 };
static const unsigned int rsaEncryptionSize = sizeof(rsaEncryption) / sizeof(rsaEncryption[0]);




static int check_signature_algorithm(const unsigned char * data, unsigned int size, unsigned int * algo)
{
    asn1_tag_and_length tlv;

    asn1_reset(&tlv);

    if (!asn1_read_tlv(data, size, &tlv))
    {
        goto failure;
    }

    if (!asn1_check_tlv(&tlv, 0, 0x00, 0x06))
    {
        goto failure;
    }

    if (sha256WithRSAEncryptionSize != tlv.length ||
        0 != memcmp(sha256WithRSAEncryption, tlv.value, sha256WithRSAEncryptionSize))
    {
        if (sha1WithRSAEncryptionSize != tlv.length ||
            0 != memcmp(sha1WithRSAEncryption, tlv.value, sha1WithRSAEncryptionSize))
        {
            goto failure;
        }

        *algo = SIG_ALGO_SHA1_WITH_RSA_ENCRYPTION;
    }
    else
    {
        *algo = SIG_ALGO_SHA256_WITH_RSA_ENCRYPTION;
    }

    data += tlv.size;
    size -= tlv.size;

    if (!size)
    {
        goto failure;
    }

    asn1_reset(&tlv);

    if (!asn1_read_tlv(data, size, &tlv) || !asn1_check_tlv(&tlv, 0, 0x00, 0x05))
    {
        goto failure;
    }

    return 1;

failure:
    return 0;
}




static int check_public_key_algorithm_is_rsa_encryption(const unsigned char * data, unsigned int size)
{
    asn1_tag_and_length tlv;

    asn1_reset(&tlv);

    if (!asn1_read_tlv(data, size, &tlv) || !asn1_check_tlv(&tlv, 0, 0x00, 0x06))
    {
        goto failure;
    }

    if (rsaEncryptionSize != tlv.length || 0 != memcmp(rsaEncryption, tlv.value, rsaEncryptionSize))
    {
        goto failure;
    }

    data += tlv.size;
    size -= tlv.size;

    if (!size)
    {
        goto failure;
    }

    asn1_reset(&tlv);

    if (!asn1_read_tlv(data, size, &tlv) || !asn1_check_tlv(&tlv, 0, 0x00, 0x05))
    {
        goto failure;
    }

    return 1;

failure:
    return 0;
}




static int parse_rsapub(x509_certificate * cert, const unsigned char * data_ptr, unsigned int data_size)
{
    int ret;
    asn1_tag_and_length tlv;

    /*
     * The RSA Public key is a sequence which looks like this:
     *
     * SEQUENCE (
     *   INTEGER n (modulus)
     *   INTEGER e (public exponent)
     * )
     */

    ret = -1;
    asn1_reset(&tlv);

    /*
     * Wrapping the RSA public key is a sequence
     */
    if (!asn1_read_tlv(data_ptr, data_size, &tlv) || !asn1_check_tlv(&tlv, 1, 0x00, 0x10))
    {
        goto cleanup;
    }

    /* descend into the sequence */
    data_ptr = tlv.value;
    data_size = tlv.length;


    /*
     * Read the modulus
     */
    if (!asn1_read_tlv(data_ptr, data_size, &tlv) || !asn1_check_tlv(&tlv, 0, 0x00, 0x02))
    {
        goto cleanup;
    }

    cert->n = tlv.value;
    cert->n_size = tlv.length;

    /*
     * This looks really strange, but OpenSSL
     * is happy to stuff 257+ bytes into a 2048
     * bit INTEGER if the leading octets are 0x00,
     * so I just compensate here...
     */
    while (cert->n_size && !*cert->n)
    {
        cert->n++;
        cert->n_size--;
    }

    data_ptr += tlv.size;
    data_size -= tlv.size;


    /*
     * Read the public exponent
     */
    if (!asn1_read_tlv(data_ptr, data_size, &tlv) || !asn1_check_tlv(&tlv, 0, 0x00, 0x02))
    {
        goto cleanup;
    }

    cert->e = tlv.value;
    cert->e_size = tlv.length;

    /*
     * As above, clean up OpenSSL brain damage
     */
    while (cert->e_size && !*cert->e)
    {
        cert->e++;
        cert->e_size--;
    }


    data_size -= tlv.size;

    if (data_size)
    {
        goto cleanup;
    }

    ret = 0;

cleanup:
    return ret;
}




static int parse_tbs(x509_certificate * cert, const unsigned char * data_ptr, unsigned int data_size, unsigned int * sig_algo)
{
    int ret;
    asn1_tag_and_length tlv;

    /*
     * We need the cross-reference the signatureAlgorithm in the to-be-signed data
     * with that in the top-level certificate, verify that the publicKeyAlgorithm
     * is rsaEncryption and extract the public exponent and modulus from the
     * subject public key.
     *
     * All other data is discarded, and we stop reading once we have the required data.
     */

    ret = -1;
    asn1_reset(&tlv);


    /*
     * Version is an explicit tag
     */
    if (!asn1_read_tlv(data_ptr, data_size, &tlv))
    {
        goto cleanup;
    }

    if (asn1_check_tlv(&tlv, 1, 0x80, 0x00))
    {
        /*
         * Unwrap the version and ensure that this is x509v3
         */
        asn1_tag_and_length version_tlv;
        asn1_reset(&version_tlv);

        if (!asn1_read_tlv(tlv.value, tlv.length, &version_tlv) || !asn1_check_tlv(&version_tlv, 0, 0x00, 0x02))
        {
            goto cleanup;
        }

        /* x509v3 has version 0x02 */
        if (version_tlv.length != 1 || version_tlv.value[0] != 0x02)
        {
            goto cleanup;
        }
    }
    else
    {
        goto cleanup;
    }

    data_ptr += tlv.size;
    data_size -= tlv.size;


    /*
     * serialNumber is an integer
     */
    asn1_reset(&tlv);

    if (!asn1_read_tlv(data_ptr, data_size, &tlv) || !asn1_check_tlv(&tlv, 0, 0x00, 0x02))
    {
        goto cleanup;
    }

    data_ptr += tlv.size;
    data_size -= tlv.size;


    /*
     * signatureAlgorithm is a sequence
     */
    asn1_reset(&tlv);

    if (!asn1_read_tlv(data_ptr, data_size, &tlv) || !asn1_check_tlv(&tlv, 1, 0x00, 0x10))
    {
        goto cleanup;
    }

    if (!check_signature_algorithm(tlv.value, tlv.length, sig_algo))
    {
        goto cleanup;
    }

    data_ptr += tlv.size;
    data_size -= tlv.size;


    /*
     * Issuer is a sequence
     */
    asn1_reset(&tlv);

    if (!asn1_read_tlv(data_ptr, data_size, &tlv) || !asn1_check_tlv(&tlv, 1, 0x00, 0x10))
    {
        goto cleanup;
    }

    data_ptr += tlv.size;
    data_size -= tlv.size;


    /*
     * Validity is a sequence
     */
    asn1_reset(&tlv);

    if (!asn1_read_tlv(data_ptr, data_size, &tlv) || !asn1_check_tlv(&tlv, 1, 0x00, 0x10))
    {
        goto cleanup;
    }

    data_ptr += tlv.size;
    data_size -= tlv.size;


    /*
     * Subject is a sequence
     */
    asn1_reset(&tlv);

    if (!asn1_read_tlv(data_ptr, data_size, &tlv) || !asn1_check_tlv(&tlv, 1, 0x00, 0x10))
    {
        goto cleanup;
    }

    data_ptr += tlv.size;
    data_size -= tlv.size;


    /*
     * subjectPublicKeyInfo is a sequence that looks like this:
     * SEQUENCE (
     *   SEQUENCE (
     *     OID
     *     NULL
     *   )
     *   BIT STRING
     * )
     *
     * Within the BIT STRING is more DER encoded data, which is the
     * RSA public key itself, and looks like this:
     * SEQUENCE (
     *   INTEGER n (public modulus)
     *   INTEGER e (exponent)
     * )
     */
    asn1_reset(&tlv);

    if (!asn1_read_tlv(data_ptr, data_size, &tlv) || !asn1_check_tlv(&tlv, 1, 0x00, 0x10))
    {
        goto cleanup;
    }

    /*
     * We're not going to ascend again from within the subjectPublicKeyInfo SEQUENCE, so just
     * use the data_ptr for parsing.
     */
    data_ptr = tlv.value;
    data_size = tlv.length;

    /*
     * Read and validate the subjectPublicKeyInfo algorithm information
     */
    asn1_reset(&tlv);

    if (!asn1_read_tlv(data_ptr, data_size, &tlv) || !asn1_check_tlv(&tlv, 1, 0x00, 0x10))
    {
        goto cleanup;
    }

    if (!check_public_key_algorithm_is_rsa_encryption(tlv.value, tlv.length))
    {
        goto cleanup;
    }

    data_ptr += tlv.size;
    data_size -= tlv.size;


    /*
     * Now read the BIT STRING containing the RSA public key data
     */
    asn1_reset(&tlv);

    if (!asn1_read_tlv(data_ptr, data_size, &tlv) || !asn1_check_tlv(&tlv, 0, 0x00, 0x03))
    {
        goto cleanup;
    }

    if (0 != parse_rsapub(cert, tlv.value + 1, tlv.length - 1))
    {
        goto cleanup;
    }

    cert->publicKey = tlv.value + 1;
    cert->publicKey_size = tlv.length - 1;

    ret = 0;

cleanup:
    return ret;
}




int x509_parse(const unsigned char * data_ptr, unsigned int data_size, x509_certificate * cert)
{
    int status;
    asn1_tag_and_length tlv;
    unsigned int tbs_sig_algo;

    tbs_sig_algo = 42;
    status = -1;

    asn1_reset(&tlv);

    /*
     * Certificate is a SEQUENCE
     */
    if (!asn1_read_tlv(data_ptr, data_size, &tlv) || !asn1_check_tlv(&tlv, 1, 0x00, 0x10))
    {
        goto cleanup;
    }

    cert->certificate_size = tlv.size;


    data_ptr = tlv.value;
    data_size = tlv.length;

    /*
     * tbsCertificate is a SEQUENCE
     */
    asn1_reset(&tlv);

    if (!asn1_read_tlv(data_ptr, data_size, &tlv) || !asn1_check_tlv(&tlv, 1, 0x00, 0x10))
    {
        goto cleanup;
    }

    cert->tbsCertificate = tlv.location;
    cert->tbsCertificate_size = tlv.size;

    data_ptr += tlv.size;
    data_size -= tlv.size;


    /*
     * Parse the to-be-signed certificate data
     */
    if (0 != parse_tbs(cert, tlv.value, tlv.length, &tbs_sig_algo))
    {
        goto cleanup;
    }


    /*
     * The top-level signature algorithm is a sequence
     */
    asn1_reset(&tlv);

    if (!asn1_read_tlv(data_ptr, data_size, &tlv) || !asn1_check_tlv(&tlv, 1, 0x00, 0x10))
    {
        goto cleanup;
    }

    if (!check_signature_algorithm(tlv.value, tlv.length, &cert->signature_algorithm))
    {
        goto cleanup;
    }

    if (tbs_sig_algo != cert->signature_algorithm)
    {
        goto cleanup;
    }

    data_ptr += tlv.size;
    data_size -= tlv.size;

    /*
     * The signature is a BIT STRING
     */
    asn1_reset(&tlv);

    if (!asn1_read_tlv(data_ptr, data_size, &tlv) || !asn1_check_tlv(&tlv, 0, 0x00, 0x03))
    {
        goto cleanup;
    }

    cert->signature = tlv.value + 1; /* skip the pad bits octet */
    cert->signature_size = tlv.length - 1; /* allow the pad bits octet */

    if (RSA_OPERATION_SIZE_BYTES < cert->signature_size)
    {
        goto cleanup;
    }

    data_size -= tlv.size;

    if (data_size)
    {
        goto cleanup;
    }

    status = 0;


cleanup:
    return status;
}


static unsigned char xita_pubkey_hash[SHA256_HASH_BUFFER_SIZE];

int x509_is_trust_anchor(x509_certificate * cert)
{
    int ret;
    unsigned int pubkey_hash_len;

    ret = 0;
    pubkey_hash_len = SHA256_HASH_BUFFER_SIZE;

    if (0 != hash(cert->publicKey, cert->publicKey_size, HASH_ALGO_SHA256, xita_pubkey_hash, &pubkey_hash_len))
    {
        goto cleanup;
    }

    ret = sec_is_trust_anchor(xita_pubkey_hash);

cleanup:
    return ret;
}




static int strip_pkcs1_1_5_padding(unsigned char * signature, unsigned int * signature_len)
{
    int ret;
    unsigned int len;
    unsigned char * pos;
    unsigned char * targ;

    ret = -1;
    len = *signature_len;

    /*
     * PKCS #1 v1.5, 8.1:
     *
     * EB = 00 || BT || PS || 00 || D
     * BT = 00 or 01
     * PS = k-3-||D|| times (00 if BT=00) or (FF if BT=01)
     * k = length of modulus in octets
     *
     * So, with a modulus length of 256 (2048 bits) and a message of 20 octets you will see:
     * 0x00 0x01 [<(256-3-20=233) octets of> 0xFF] 0x00 [<20 octets of your data>]
     */

    /* 16 is min hash len */
    if (len < 3 + 8 + 16 || signature[0] != 0x00 || (signature[1] != 0x00 && signature[1] != 0x01))
    {
        goto cleanup;
    }

    pos = signature + 3;

    if (signature[1] == 0x00) /* BT = 00 */
    {
        if (signature[2] != 0x00)
        {
            goto cleanup;
        }

        while (pos + 1 < signature + len && *pos == 0x00 && pos[1] == 0x00)
        {
            pos++;
        }
    }
    else /* BT = 01 */
    {
        if (signature[2] != 0xff)
        {
            goto cleanup;
        }

        while (pos < signature + len && *pos == 0xff)
        {
            pos++;
        }
    }

    /* PKCS #1 v1.5, 8.1: At least eight octets long PS */
    if (pos - signature - 2 < 8)
    {
        goto cleanup;
    }

    /* 16 is min hash len */
    if (pos + 16 >= signature + len || *pos != 0x00)
    {
        goto cleanup;
    }

    pos++;
    len -= pos - signature;

    /* Strip PKCS #1 header */
    targ = pos + len;

    while (pos != targ)
    {
        *signature++ = *pos++;
    }

    *signature_len = len;

    ret = 0;

cleanup:
    return ret;
}




static int parse_pkcs1_1_5_signature_data(const unsigned char * data_ptr, unsigned int data_size, unsigned char ** hash_ptr, unsigned int * hash_size, unsigned int * hash_type)
{
    /*
     * The 'hash' that's been signed is actually an ASN.1 structure that
     * contains a hash algorithm identifier and the hash itself.
     *
     * The ASN.1 is structured as follows:
     *
     * SEQUENCE (
     *   SEQUENCE (
     *     OID
     *     NULL
     *   )
     *   HASH
     * )
     */
    int ret;
    asn1_tag_and_length tlv;
    unsigned char * hash_data_start;
    unsigned int hash_data_len;

    ret = -1;
    asn1_reset(&tlv);

    /*
     * Read the hash data container SEQUENCE
     */
    if (!asn1_read_tlv(data_ptr, data_size, &tlv) || !asn1_check_tlv(&tlv, 1, 0x00, 0x10))
    {
        goto cleanup;
    }

    data_ptr = tlv.value;
    data_size = tlv.length;
    hash_data_start = (unsigned char *)data_ptr;
    hash_data_len = data_size;


    /*
     * Read the algorithm identifier SEQUENCE
     */
    asn1_reset(&tlv);

    if (!asn1_read_tlv(data_ptr, data_size, &tlv) || !asn1_check_tlv(&tlv, 1, 0x00, 0x10))
    {
        goto cleanup;
    }

    data_ptr = tlv.value;
    data_size = tlv.length;
    hash_data_start += tlv.size;
    hash_data_len -= tlv.size;


    /*
     * Read the algorithm OID and copy out the symbolic value to the caller
     */
    asn1_reset(&tlv);

    if (!asn1_read_tlv(data_ptr, data_size, &tlv) || !asn1_check_tlv(&tlv, 0, 0x00, 0x06))
    {
        goto cleanup;
    }

    if (sha256OidSize == tlv.length && 0 == memcmp(sha256Oid, tlv.value, tlv.length))
    {
        *hash_type = HASH_ALGO_SHA256;
    }
    else if (sha1OidSize == tlv.length && 0 == memcmp(sha1Oid, tlv.value, tlv.length))
    {
        *hash_type = HASH_ALGO_SHA1;
    }
    else
    {
        goto cleanup;
    }

    data_ptr += tlv.size;
    data_size -= tlv.size;


    /*
     * Ensure that the NULL is where we expect it to be
     */
    asn1_reset(&tlv);

    if (!asn1_read_tlv(data_ptr, data_size, &tlv) || !asn1_check_tlv(&tlv, 0, 0x00, 0x05))
    {
        goto cleanup;
    }


    /*
     * Restore the parse pointers to get to the signature data
     */
    data_ptr = hash_data_start;
    data_size = hash_data_len;


    /*
     * Parse the signature data (this is the actual hash)
     */
    asn1_reset(&tlv);

    if (!asn1_read_tlv(data_ptr, data_size, &tlv) || !asn1_check_tlv(&tlv, 0, 0x00, 0x04))
    {
        goto cleanup;
    }

    if (*hash_type == HASH_ALGO_SHA1)
    {
        if (tlv.length != SHA1_HASH_BUFFER_SIZE)
        {
            goto cleanup;
        }
    }
    else /* SHA256 */
    {
        if (tlv.length != SHA256_HASH_BUFFER_SIZE)
        {
            goto cleanup;
        }
    }

    *hash_ptr = (unsigned char *)tlv.value;
    *hash_size = tlv.length;
    ret = 0;

cleanup:
    return ret;
}




static fp_int dads_m;
static fp_int dads_e_m;
static fp_int dads_n;
static fp_int dads_e;
static unsigned char dads_buffer[RSA_OPERATION_SIZE_BYTES];

static int decrypt_and_decode_signature(x509_certificate * cert, const unsigned char * signature, unsigned int signature_len, unsigned char * out, unsigned int * out_len, unsigned int * algo)
{
    int ret;
    unsigned int ubsize;
    unsigned char * hash_ptr;
    unsigned int hash_size;

    memset(&dads_m, 0, sizeof(dads_m));
    memset(&dads_e_m, 0, sizeof(dads_e_m));
    memset(&dads_n, 0, sizeof(dads_n));
    memset(&dads_e, 0, sizeof(dads_e));
    memset(dads_buffer, 0, RSA_OPERATION_SIZE_BYTES);
    ret = -1;

    if (signature_len != RSA_OPERATION_SIZE_BYTES || cert->n_size > RSA_OPERATION_SIZE_BYTES)
    {
        goto cleanup;
    }

    fp_init(&dads_m);
    fp_init(&dads_n);
    fp_init(&dads_e);

    fp_read_unsigned_bin(&dads_m, (unsigned char *)signature, signature_len);
    fp_read_unsigned_bin(&dads_n, (unsigned char *)cert->n, cert->n_size);
    fp_read_unsigned_bin(&dads_e, (unsigned char *)cert->e, cert->e_size);

    if (FP_OKAY != fp_exptmod(&dads_m, &dads_e, &dads_n, &dads_e_m))
    {
        goto cleanup;
    }

    if ((ubsize = fp_unsigned_bin_size(&dads_e_m)) > RSA_OPERATION_SIZE_BYTES)
    {
        goto cleanup;
    }

    fp_to_unsigned_bin(&dads_e_m, &dads_buffer[0] + (RSA_OPERATION_SIZE_BYTES - ubsize));

    ubsize = RSA_OPERATION_SIZE_BYTES;

    if (0 != strip_pkcs1_1_5_padding(dads_buffer, &ubsize))
    {
        goto cleanup;
    }

    if (0 != parse_pkcs1_1_5_signature_data(dads_buffer, ubsize, &hash_ptr, &hash_size, algo))
    {
        goto cleanup;
    }

    if (*out_len < hash_size)
    {
        goto cleanup;
    }

    memcpy(out, hash_ptr, hash_size);
    *out_len = hash_size;
    ret = 0;

cleanup:
    return ret;
}



static unsigned char xv_buffer[HASH_BUFFER_SIZE];
static unsigned char xv_hashbuf[HASH_BUFFER_SIZE];

int x509_verify(x509_certificate * cert, const unsigned char * signature, unsigned int signature_len, const unsigned char * data, unsigned int data_len)
{
    int ret;
    unsigned int algo;
    unsigned int buffer_len = HASH_BUFFER_SIZE;
    unsigned int hashbuf_len = HASH_BUFFER_SIZE;

    ret = -1;
    algo = 0xffffffff;

    if (0 != decrypt_and_decode_signature(cert, signature, signature_len, xv_buffer, &buffer_len, &algo))
    {
        goto cleanup;
    }

    if (0 != hash(data, data_len, algo, xv_hashbuf, &hashbuf_len))
    {
        goto cleanup;
    }

    if (buffer_len != hashbuf_len || 0 != memcmp(xv_buffer, xv_hashbuf, hashbuf_len))
    {
        goto cleanup;
    }

    ret = 0;

cleanup:
    return ret;
}




int x509_verify_cert(x509_certificate * parent, x509_certificate * child)
{
    return x509_verify(parent, child->signature, child->signature_size, child->tbsCertificate, child->tbsCertificate_size);
}
