/*
 * Copyright (c) 2014, ip.access Ltd.
 */
#include "verify_image.h"

#include "x509.h"

#include <linux/string.h>




int verify_image(fimage_header_t * header)
{
    int ret;
    x509_certificate attestation_cert;
    x509_certificate attestation_ca_cert;
    x509_certificate root_ca_cert;
    unsigned int cert_offset;
    unsigned int cert_size;
    unsigned int trust_established;

    ret = -1;
    memset(&attestation_cert, 0, sizeof(attestation_cert));
    memset(&attestation_ca_cert, 0, sizeof(attestation_ca_cert));
    memset(&root_ca_cert, 0, sizeof(root_ca_cert));

    cert_offset = header->cert_chain_ptr + sizeof(*header);
    cert_size = header->cert_chain_size;

    if (0 == cert_size)
    {
        goto cleanup; /* unsigned images are not allowed */
    }

    if (0 != x509_parse((const unsigned char *)cert_offset, cert_size, &attestation_cert))
    {
        goto cleanup;
    }

    trust_established = x509_is_trust_anchor(&attestation_cert);
    cert_offset += attestation_cert.certificate_size;
    cert_size -= attestation_cert.certificate_size;

    if (cert_size && !trust_established)
    {
        if (0 != x509_parse((const unsigned char *)cert_offset, cert_size, &attestation_ca_cert))
        {
            goto cleanup;
        }

        if (0 != x509_verify_cert(&attestation_ca_cert, &attestation_cert))
        {
            goto cleanup;
        }

        trust_established = x509_is_trust_anchor(&attestation_ca_cert);
        cert_offset += attestation_ca_cert.certificate_size;
        cert_size -= attestation_ca_cert.certificate_size;

        if (cert_size && !trust_established)
        {
            if (0 != x509_parse((const unsigned char *)cert_offset, cert_size, &root_ca_cert))
            {
                goto cleanup;
            }

            if (0 != x509_verify_cert(&root_ca_cert, &attestation_ca_cert))
            {
                goto cleanup;
            }

            trust_established = x509_is_trust_anchor(&root_ca_cert);
        }
    }

    if (!trust_established)
    {
        goto cleanup;
    }

    /*
     * We have established trust.  We can use the attestation certificate to verify the IBN signature.
     */
    if (0 != x509_verify(&attestation_cert, (const unsigned char *)(header->signature_ptr + sizeof(*header)), header->signature_size, (const unsigned char *)(header->destination), header->code_size + sizeof(*header)))
    {
        goto cleanup;
    }

    /*
     * This image is authentic
     */

    ret = 0;

cleanup:
    return ret;
}
