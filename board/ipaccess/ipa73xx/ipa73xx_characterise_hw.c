#include "damm.h"
#include <common.h>
#include <errno.h>
#include "ipa73xx_fuse.h"

#undef NEEDS_ETH1_ADDRESS
#undef NEEDS_ETH2_ADDRESS
#undef NEEDS_ETH3_ADDRESS

#define ULONG_MAX (~0UL)


#define HIGHEST_DEFINED_BOARD_INDEX 5



static const char * oscillator_text[] = {
    "030", /* VCTCXO */
    "015", /* OCXO   */
    "000",
    "000"
};
static const int num_oscillators = sizeof(oscillator_text) / sizeof(oscillator_text[0]);



typedef struct variant_record_s {
    const char full[5];
    const char part[4];
    const char variant[2];
    uint16_t part_num;
    uint8_t variant_char;
} variant_record;



static const variant_record variant_lookup[] = {
    { "0000", "000", "0",   0, '0' }, /* Bands: N/A     - Notes: N/A               */
    { "230A", "230", "A", 230, 'A' }, /* Bands: 1       - Notes: Rest of the World */
    { "230B", "230", "B", 230, 'B' }, /* Bands: 2, 5    - Notes: United States     */
    { "267I", "267", "I", 267, 'I' }, /* Bands: N/A     - Notes: No Radio          */
    { "400D", "400", "D", 400, 'D' }, /* Bands: 1, 3, 8 - Notes: Rest of the World */
    { "400E", "400", "E", 400, 'E' }, /* Bands: 2, 4, 5 - Notes: United States     */
};
static const int num_variants = sizeof(variant_lookup) / sizeof(variant_lookup[0]);




static const char * lookup_oscillator(uint8_t oscillator)
{
    if (oscillator >= num_oscillators) oscillator = (num_oscillators - 1);
    return oscillator_text[oscillator];
}




static const variant_record * lookup_variant(uint16_t hw_variant)
{
    if (hw_variant >= num_variants) hw_variant = 0;
    return &variant_lookup[hw_variant];
}




static int usage(const char * progname, int ret)
{
    fprintf((ret ? stderr : stdout), "Usage: %s -s SERIAL_NUMBER -0 ETH0ADDR [-1 ETH1ADDR [-2 ETH2ADDR [-3 ETH3ADDR]]] -t TYPE -p PCB_ASSEMBLY_ISSUE\n", progname);
    fprintf((ret ? stderr : stdout), "       -s SERIAL_NUMBER     : Specify the serial number (with DAMM digit) of this board.\n");
    fprintf((ret ? stderr : stdout), "                              This is specified as a 10 digit decimal number made up of the 9\n");
    fprintf((ret ? stderr : stdout), "                              serial number digits and 1 DAMM check digit.\n");
    fprintf((ret ? stderr : stdout), "       -0 ETH0ADDR          : Specify the first Ethernet MAC address.\n");
    fprintf((ret ? stderr : stdout), "                              An Ethernet address is specified as six colon separated hexadecimal octets.\n");
    fprintf((ret ? stderr : stdout), "                              Octets are left padded to two digits with zeroes.\n");
    fprintf((ret ? stderr : stdout), "                              For example: CA:FE:BA:BE:B0:0C.\n");
#if defined(NEEDS_ETH1_ADDRESS)    
    fprintf((ret ? stderr : stdout), "       -1 ETH1ADDR          : Specify the second Ethernet MAC address.");
    fprintf((ret ? stderr : stdout), "\n");
#endif
#if defined(NEEDS_ETH2_ADDRESS)
    fprintf((ret ? stderr : stdout), "       -2 ETH1ADDR          : Specify the third Ethernet MAC address.");
    fprintf((ret ? stderr : stdout), "\n");
#endif
#if defined(NEEDS_ETH3_ADDRESS)    
    fprintf((ret ? stderr : stdout), "       -3 ETH1ADDR          : Specify the fourth Ethernet MAC address.");
    fprintf((ret ? stderr : stdout), "\n");
#endif
    fprintf((ret ? stderr : stdout), "       -t TYPE              : Specify the board type as a 16 bit decimal value (0x0000-0xFFFF).\n");
    fprintf((ret ? stderr : stdout), "                              The TYPE argument must correspond to a known board hardware type and oscillator value.\n");
    fprintf((ret ? stderr : stdout), "                              Unused bits must be set to 0.\n");
    fprintf((ret ? stderr : stdout), "       -p PCB_ASSEMBLY_ISSUE: Specify the PCB assembly issue as a 32 bit decimal value (0x00000000-0xFFFFFFFF).\n");
    fprintf((ret ? stderr : stdout), "                              Only the first 17 bits of the PCB_ASSEMBLY_ISSUE may be set.\n");
    fprintf((ret ? stderr : stdout), "       -f                   : Force acceptance of a serial number that fails the DAMM algorithm check.\n");
    fprintf((ret ? stderr : stdout), "       -Test                : Enable Test mode where actual fuses will not be blown, only mapped mem will be updated.\n");
    fprintf((ret ? stderr : stdout), "       -no-prompt           : Do not prompt user for input before blowing the fuses (must be used when fuse blowing is done via scripts).\n");
    fprintf((ret ? stderr : stdout), "\n");
    fprintf((ret ? stderr : stdout), "       This utility is only supposed to work on blank boards.\n");
    return ret;
}


#define PC73XX_KEY2_OFFSET 256
#define PC73XX_KEY2_SIZE 128
#define PC73XX_KEY2_RELEVANT_OFFSET 0
#define PC73XX_KEY2_RELEVANT_SIZE 113
#define PC73XX_KEY3_OFFSET 384
#define PC73XX_KEY3_SIZE 128
#define PC73XX_KEY3_RELEVANT_OFFSET 0
#define PC73XX_KEY3_RELEVANT_SIZE 113
#define PC73XX_KEY4_OFFSET 512
#define PC73XX_KEY4_SIZE 128
#define PC73XX_KEY4_RELEVANT_OFFSET 0
#define PC73XX_KEY4_RELEVANT_SIZE 96

#define CHECK_BIT(var,pos) ((var) & (1<<(pos)))


#if defined(_DEBUG) || 1
static void _dump_key_region_buffer_bits(const char * name, const char * buffer)
{
    /*
     * Yes, this is agricultural.  No, I don't care.
     */
    fprintf(stdout, "%s bit dump\n", name);
    fprintf(stdout, "%d%d%d%d %d%d%d%d %d%d%d%d %d%d%d%d %d%d%d%d %d%d%d%d %d%d%d%d %d%d%d%d\n"
            "%d%d%d%d %d%d%d%d %d%d%d%d %d%d%d%d %d%d%d%d %d%d%d%d %d%d%d%d %d%d%d%d\n"
            "%d%d%d%d %d%d%d%d %d%d%d%d %d%d%d%d %d%d%d%d %d%d%d%d %d%d%d%d %d%d%d%d\n"
            "%d%d%d%d %d%d%d%d %d%d%d%d %d%d%d%d %d%d%d%d %d%d%d%d %d%d%d%d %d%d%d%d\n",
            buffer[  0], buffer[  1], buffer[  2], buffer[  3], buffer[  4], buffer[  5], buffer[  6], buffer[  7],
            buffer[  8], buffer[  9], buffer[ 10], buffer[ 11], buffer[ 12], buffer[ 13], buffer[ 14], buffer[ 15],
            buffer[ 16], buffer[ 17], buffer[ 18], buffer[ 19], buffer[ 20], buffer[ 21], buffer[ 22], buffer[ 23],
            buffer[ 24], buffer[ 25], buffer[ 26], buffer[ 27], buffer[ 28], buffer[ 29], buffer[ 30], buffer[ 31],
            buffer[ 32], buffer[ 33], buffer[ 34], buffer[ 35], buffer[ 36], buffer[ 37], buffer[ 38], buffer[ 39],
            buffer[ 40], buffer[ 41], buffer[ 42], buffer[ 43], buffer[ 44], buffer[ 45], buffer[ 46], buffer[ 47],
            buffer[ 48], buffer[ 49], buffer[ 50], buffer[ 51], buffer[ 52], buffer[ 53], buffer[ 54], buffer[ 55],
            buffer[ 56], buffer[ 57], buffer[ 58], buffer[ 59], buffer[ 60], buffer[ 61], buffer[ 62], buffer[ 63],
            buffer[ 64], buffer[ 65], buffer[ 66], buffer[ 67], buffer[ 68], buffer[ 69], buffer[ 70], buffer[ 71],
            buffer[ 72], buffer[ 73], buffer[ 74], buffer[ 75], buffer[ 76], buffer[ 77], buffer[ 78], buffer[ 79],
            buffer[ 80], buffer[ 81], buffer[ 82], buffer[ 83], buffer[ 84], buffer[ 85], buffer[ 86], buffer[ 87],
            buffer[ 88], buffer[ 89], buffer[ 90], buffer[ 91], buffer[ 92], buffer[ 93], buffer[ 94], buffer[ 95],
            buffer[ 96], buffer[ 97], buffer[ 98], buffer[ 99], buffer[100], buffer[101], buffer[102], buffer[103],
            buffer[104], buffer[105], buffer[106], buffer[107], buffer[108], buffer[109], buffer[110], buffer[111],
            buffer[112], buffer[113], buffer[114], buffer[115], buffer[116], buffer[117], buffer[118], buffer[119],
            buffer[120], buffer[121], buffer[122], buffer[123], buffer[124], buffer[125], buffer[126], buffer[127]);
}
#endif


/*
 * Board characterisation writer for the ipa73xx platform
 */
static int characterise_fuses(uint32_t serial_number, uint8_t * eth0_address, uint8_t * eth1_address, uint16_t board, uint8_t oscillator, uint32_t pcb_assembly_issue)
{
    unsigned int i;
    unsigned int j;
    unsigned int k;
    int ret;
    int fd;
    char key2_buffer[PC73XX_KEY2_SIZE];
    char key3_buffer[PC73XX_KEY3_SIZE];
    char key4_buffer[PC73XX_KEY4_SIZE];
    unsigned int key2_buffer_count;
    unsigned int key3_buffer_count;
    unsigned int key4_buffer_count;

    ret = 1;

    memset(key2_buffer, 0, sizeof(key2_buffer) / sizeof(key2_buffer[0]));
    memset(key3_buffer, 0, sizeof(key3_buffer) / sizeof(key3_buffer[0]));
    memset(key4_buffer, 0, sizeof(key4_buffer) / sizeof(key4_buffer[0]));
    key2_buffer_count = PC73XX_KEY2_SIZE;
    key3_buffer_count = PC73XX_KEY3_SIZE;
    key4_buffer_count = PC73XX_KEY4_SIZE;


    if (0 != read_fuse_in_range( PC73XX_KEY2_OFFSET, key2_buffer, &key2_buffer_count))
    {
        printf("read_fuse_in_range");
        goto cleanup;
    }

    if (PC73XX_KEY2_SIZE != key2_buffer_count)
    {
        fprintf(stderr, "%s: %s\n", __func__, "Failed to read all fuses from the KEY2 region.");
        goto cleanup;
    }

    if (0 != read_fuse_in_range( PC73XX_KEY3_OFFSET, key3_buffer, &key3_buffer_count))
    {
        printf("read_fuse_in_range");
        goto cleanup;
    }

    if (PC73XX_KEY3_SIZE != key3_buffer_count)
    {
        fprintf(stderr, "%s: %s\n", __func__, "Failed to read all fuses from the KEY3 region.");
        goto cleanup;
    }

    if (0 != read_fuse_in_range( PC73XX_KEY4_OFFSET, key4_buffer, &key4_buffer_count))
    {
        printf("read_fuse_in_range");
        goto cleanup;
    }

    if (PC73XX_KEY4_SIZE != key4_buffer_count)
    {
        fprintf(stderr, "%s: %s\n", __func__, "Failed to read all fuses from the KEY4 region.");
        goto cleanup;
    }

    for (i = PC73XX_KEY2_RELEVANT_OFFSET; i < PC73XX_KEY2_RELEVANT_SIZE; ++i)
    {
        if (key2_buffer[i])
        {
            fprintf(stderr, "%s: Bit %u of the KEY2 region is already set.\n", __func__, i);
            goto cleanup;
        }
    }

    for (i = PC73XX_KEY3_RELEVANT_OFFSET; i < PC73XX_KEY3_RELEVANT_SIZE; ++i)
    {
        if (key3_buffer[i])
        {
            fprintf(stderr, "%s: Bit %u of the KEY3 region is already set.\n", __func__, i);
            goto cleanup;
        }
    }

    for (i = PC73XX_KEY4_RELEVANT_OFFSET; i < PC73XX_KEY4_RELEVANT_SIZE; ++i)
    {
        if (key4_buffer[i])
        {
            fprintf(stderr, "%s: Bit %u of the KEY4 region is already set.\n", __func__, i);
            goto cleanup;
        }
    }

    memset(key2_buffer, 0, sizeof(key2_buffer) / sizeof(key2_buffer[0]));
    memset(key3_buffer, 0, sizeof(key3_buffer) / sizeof(key3_buffer[0]));
    memset(key4_buffer, 0, sizeof(key4_buffer) / sizeof(key4_buffer[0]));
    key2_buffer_count = PC73XX_KEY2_SIZE;
    key3_buffer_count = PC73XX_KEY3_SIZE;
    key4_buffer_count = PC73XX_KEY4_SIZE;

    j = PC73XX_KEY2_RELEVANT_OFFSET;

    /*
     * Copy in the serial number bits
     */
    for (i = 0; i < 32; ++i)
    {
        key2_buffer[j] = CHECK_BIT(serial_number, i) ? 1 : 0;
        key3_buffer[j] = key2_buffer[j];
        ++j;
    }

    /*
     * Copy in the Ethernet MAC address bits
     */
    for (k = 0; k < 6; ++k)
    {
        for (i = 0; i < 8; ++i)
        {
            key2_buffer[j] = CHECK_BIT(eth0_address[k], i) ? 1 : 0;
            key3_buffer[j] = key2_buffer[j];
            ++j;
        }
    }

    /*
     * Copy in the oscillator bits
     */
    for (i = 0; i < 2; ++i)
    {
        key2_buffer[j] = CHECK_BIT(oscillator, i) ? 1 : 0;
        key3_buffer[j] = key2_buffer[j];
        ++j;
    }

    /*
     * Copy in the board bits
     */
    for (i = 0; i < 10; ++i)
    {
        key2_buffer[j] = CHECK_BIT(board, i) ? 1 : 0;
        key3_buffer[j] = key2_buffer[j];
        ++j;
    }

    /* four reserved bits in hw_characterisation*/
    for (i = 0; i < 4; ++i)
    {
        key2_buffer[j] = 0;
        key3_buffer[j] = key2_buffer[j];
        ++j;
    }

    /*
     * Copy in the PCB assembly issue bits
     */
    for (i = 0; i < 16; ++i)
    {
        key2_buffer[j] = CHECK_BIT(pcb_assembly_issue, i) ? 1 : 0;
        key3_buffer[j] = key2_buffer[j];
        ++j;
    }

    j = PC73XX_KEY4_RELEVANT_OFFSET;

    /*
     * Copy in the secondary MAC address
     */
    for (k = 0; k < 6; ++k)
    {
        for (i = 0; i < 8; ++i)
        {
            key4_buffer[j] = CHECK_BIT(eth1_address[k], i) ? 1 : 0;
            ++j;
        }
    }

    /*
     * Copy in the redundant secondary MAC address
     */
    for (k = 0; k < 6; ++k)
    {
        for (i = 0; i < 8; ++i)
        {
            key4_buffer[j] = CHECK_BIT(eth1_address[k], i) ? 1 : 0;
            ++j;
        }
    }

#if defined(_DEBUG) || 1
    _dump_key_region_buffer_bits("KEY2", key2_buffer);
    _dump_key_region_buffer_bits("KEY3", key3_buffer);
    _dump_key_region_buffer_bits("KEY4", key4_buffer);
#endif


    /*
     * Blow the relevant KEY2 bits
     */
    key2_buffer_count = PC73XX_KEY2_SIZE;

    if (0 != blow_fuse_in_range( PC73XX_KEY2_OFFSET, key2_buffer, &key2_buffer_count))
    {
        printf("Error: blow_fuse_in_range for KEY2");
        goto cleanup;
    }


    /*
     * Blow the relevant KEY3 bits
     */
    key3_buffer_count = PC73XX_KEY3_SIZE;

    if (0 != blow_fuse_in_range( PC73XX_KEY3_OFFSET, key3_buffer, &key3_buffer_count))
    {
        printf("Error: blow_fuse_in_range for KEY3");
        goto cleanup;
    }

#if 0
    /*Yet not decide so commented out*/
    /*
     * Blow the relevant KEY4 bits.
     *
     * KEY4 is only blown if contains any bits to blow, which it may not do for
     * boards that have only one Ethernet MAC address.
     */
    k = 0;

    for (i = PC73XX_KEY4_RELEVANT_OFFSET; i < PC73XX_KEY4_RELEVANT_SIZE; ++i)
    {
        if (key4_buffer[i])
        {
            k = 1;
            break;
        }
    }

    if (k)
    {
        key4_buffer_count = PC73XX_KEY4_SIZE;

        if (0 != blow_fuse_in_range( PC73XX_KEY4_OFFSET, key4_buffer, &key4_buffer_count))
        {
            printf("blow_fuse_in_range");
            goto cleanup;
        }
    }
#endif    

    fprintf(stdout, "%s\n", "This board has been characterised.");
    ret = 0;

cleanup:

    return ret;
}



/*
 * Parse a serial number string to a uint32_t and verify that it passes the DAMM check
 */
static int parse_serial_number(const char * serial_text, uint32_t * serial_number, int ignore_damm)
{
    char * ep;
    unsigned long ulval;

    if (!serial_text || !serial_number)
    {
        fprintf(stderr, "%s\n", "Invalid serial number: NULL argument to conversion function.");
        return -1;
    }

    if (strlen(serial_text) != 10)
    {
        fprintf(stderr, "%s\n", "Invalid serial number: not 10 digits in length.");
        return -1;
    }

    if (!dammValidCheckDigit(serial_text))
    {
        if (ignore_damm)
        {
            fprintf(stderr, "WARNING: DAMM check digit verification failed.\n");
            fprintf(stderr, "         DAMM verification failures as errors suppressed.\n");
        }
        else
        {
            fprintf(stderr, "%s\n", "Invalid serial number: DAMM check failed.");
            return -1;
        }
    }

    errno = 0;
    ulval = simple_strtoul(serial_text, &ep, 10);

    if (serial_text[0] == '\0' || *ep != '\0')
    {
        fprintf(stderr, "%s\n", "Invalid serial number: not a decimal number.");
        return -1;
    }

    if (errno == ERANGE && ulval == ULONG_MAX)
    {
        fprintf(stderr, "%s\n", "Invalid serial number: out of range.");
        return -1;
    }

    *serial_number = (uint32_t)ulval;
    return 0;
}




int parse_mac_octet(const char * octet_text, uint8_t * octet)
{
    char * ep;
    unsigned long ulval;

    if (!octet_text || !octet)
    {
        fprintf(stderr, "%s\n", "Invalid MAC octet: NULL argument to conversion function.");
        return -1;
    }

    if (strlen(octet_text) != 2)
    {
        fprintf(stderr, "%s\n", "Invalid MAC octet: not 2 hexadecimal digits in length.");
        return -1;
    }

    errno = 0;
    ulval = simple_strtoul(octet_text, &ep, 16);

    if (octet_text[0] == '\0' || *ep != '\0')
    {
        fprintf(stderr, "%s\n", "Invalid MAC octet: not a hexadecimal number.");
        return -1;
    }

    if (errno == ERANGE && ulval == ULONG_MAX)
    {
        fprintf(stderr, "%s\n", "Invalid MAC octet: out of range.");
        return -1;
    }

    if (ulval > 0xFF)
    {
        fprintf(stderr, "%s\n", "Invalid MAC octet: out of range.");
        return -1;
    }

    *octet = (uint8_t)(ulval & 0x000000FF);
    return 0;
}




int parse_ethernet_mac_address(const char * address_text, uint8_t * address)
{
    unsigned int i;

    if (!address_text || !address)
    {
        fprintf(stderr, "%s\n", "Invalid Ethernet MAC address: NULL argument to conversion function.");
        return -1;
    }

    if (strlen(address_text) != 17)
    {
        fprintf(stderr, "%s\n", "Invalid Ethernet MAC address: not 17 characters in length.");
        return -1;
    }

    for (i = 0; i < 6; ++i)
    {
        char input[3];
        input[0] = address_text[i * 3];
        input[1] = address_text[(i * 3) + 1];
        input[2] = '\0';

        if (0 != parse_mac_octet(input, &address[i]))
        {
            fprintf(stderr, "Invalid Ethernet MAC address: failed to parse octet %u.\n", i);
            return -1;
        }
    }

    return 0;
}




static int parse_hex_uint32(const char * input, uint32_t * output)
{
    char * ep;
    unsigned long ulval;

    if (!input || !output)
    {
        fprintf(stderr, "%s\n", "Invalid number: NULL argument to conversion function.");
        return -1;
    }

    errno = 0;
    ulval = simple_strtoul(input, &ep, 16);

    if (input[0] == '\0' || *ep != '\0')
    {
        fprintf(stderr, "%s\n", "Invalid number: not a hexadecimal number.");
        return -1;
    }

    if (errno == ERANGE && ulval == ULONG_MAX)
    {
        fprintf(stderr, "%s\n", "Invalid number: out of range.");
        return -1;
    }

    *output = (uint32_t)ulval;
    return 0;
}




static int parse_hex_uint16(const char * input, uint16_t * output)
{
    uint32_t v = 0;

    if (0 != parse_hex_uint32(input, &v))
    {
        return -1;
    }

    if (v > 0xFFFF)
    {
        fprintf(stderr, "%s\n", "Invalid number: out of range.");
        return -1;
    }

    *output = (uint16_t)(v & 0xFFFF);
    return 0;
}




static int parse_board_type(const char * board_text, uint16_t * board, uint8_t * oscillator)
{
    uint16_t v;

    v = 0;

    if (!board_text || !board || !oscillator)
    {
        fprintf(stderr, "%s\n", "Invalid board type: NULL argument to conversion function.");
        return -1;
    }

    if (strlen(board_text) < 3)
    {
        fprintf(stderr, "%s\n", "Invalid board type: Input too short.");
        return -1;
    }

    if (board_text[0] != '0' || !(board_text[1] == 'x' || board_text[1] == 'X'))
    {
        fprintf(stderr, "%s\n", "Invalid board type: Invalid prefix (must be 0x or 0X).");
        return -1;
    }

    if (0 != parse_hex_uint16(board_text + 2, &v))
    {
        fprintf(stderr, "%s\n", "Invalid board type: failed to parse input.");
        return -1;
    }

    *board = (uint16_t)((((v & 0x00000FFC) >> 2) & 0x000003FF) & 0x0000FFFF);
    *oscillator = (uint8_t)((v & 0x00000003) & 0x000000FF);

    if (*board == 0 || *board > HIGHEST_DEFINED_BOARD_INDEX)
    {
        fprintf(stderr, "%s\n", "Invalid board type: board index out of range");
        return -1;
    }

    if (*oscillator != 0 && *oscillator != 1)
    {
        fprintf(stderr, "%s\n", "Invalid board type: bad oscillator (must be 0 or 1)");
        return -1;
    }

    return 0;
}




static int parse_pcb_assembly_issue(const char * pcb_assembly_issue_text, uint32_t * pcb_assembly_issue)
{
    if (!pcb_assembly_issue_text || !pcb_assembly_issue)
    {
        fprintf(stderr, "%s\n", "Invalid PCB assembly issue: NULL argument to conversion function.");
        return -1;
    }

    if (strlen(pcb_assembly_issue_text) < 3)
    {
        fprintf(stderr, "%s\n", "Invalid PCB assembly issue: Input too short.");
        return -1;
    }

    if (pcb_assembly_issue_text[0] != '0' || !(pcb_assembly_issue_text[1] == 'x' || pcb_assembly_issue_text[1] == 'X'))
    {
        fprintf(stderr, "%s\n", "Invalid PCB assembly issue: Invalid prefix (must be 0x or 0X).");
        return -1;
    }

    if (0 != parse_hex_uint32(pcb_assembly_issue_text + 2, pcb_assembly_issue))
    {
        fprintf(stderr, "%s\n", "Invalid PCB assembly issue: Invalid hexadecimal value.");
        return -1;
    }

    if (*pcb_assembly_issue != (*pcb_assembly_issue & 0x0001FFFF))
    {
        fprintf(stderr, "%s\n", "Invalid PCB assembly issue: out of range.");
        return -1;
    }

    return 0;
}


int read_and_dump_all_fuse_regions()
{
    char key2_buffer[PC73XX_KEY2_SIZE];
    char key3_buffer[PC73XX_KEY3_SIZE];
    char key4_buffer[PC73XX_KEY4_SIZE];
    unsigned int key2_buffer_count;
    unsigned int key3_buffer_count;
    unsigned int key4_buffer_count;

    memset(key2_buffer, 0, sizeof(key2_buffer) / sizeof(key2_buffer[0]));
    memset(key3_buffer, 0, sizeof(key3_buffer) / sizeof(key3_buffer[0]));
    memset(key4_buffer, 0, sizeof(key4_buffer) / sizeof(key4_buffer[0]));
    key2_buffer_count = PC73XX_KEY2_SIZE;
    key3_buffer_count = PC73XX_KEY3_SIZE;
    key4_buffer_count = PC73XX_KEY4_SIZE;

    if (0 != read_fuse_in_range( PC73XX_KEY2_OFFSET, key2_buffer, &key2_buffer_count))
    {
        printf("read_fuse_in_range");
        goto cleanup;
    }

    if (PC73XX_KEY2_SIZE != key2_buffer_count)
    {
        fprintf(stderr, "%s: %s\n", __func__, "Failed to read all fuses from the KEY2 region.");
        goto cleanup;
    }

    if (0 != read_fuse_in_range( PC73XX_KEY3_OFFSET, key3_buffer, &key3_buffer_count))
    {
        printf("read_fuse_in_range");
        goto cleanup;
    }

    if (PC73XX_KEY3_SIZE != key3_buffer_count)
    {
        fprintf(stderr, "%s: %s\n", __func__, "Failed to read all fuses from the KEY3 region.");
        goto cleanup;
    }

    if (0 != read_fuse_in_range( PC73XX_KEY4_OFFSET, key4_buffer, &key4_buffer_count))
    {
        printf("read_fuse_in_range");
        goto cleanup;
    }

    if (PC73XX_KEY4_SIZE != key4_buffer_count)
    {
        fprintf(stderr, "%s: %s\n", __func__, "Failed to read all fuses from the KEY4 region.");
        goto cleanup;
    }

#if defined(_DEBUG) || 1
    printf("\n-----DUMP AFTER WRITING FUSE MAP------\n");
    _dump_key_region_buffer_bits("KEY2", key2_buffer);
    _dump_key_region_buffer_bits("KEY3", key3_buffer);
    _dump_key_region_buffer_bits("KEY4", key4_buffer);
#endif
cleanup:

    return 0;

}


int do_characterise(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
    int ret;
    int ch;
    const char * serial_text;
    const char * eth0addr_text;
    const char * eth1addr_text;
    const char * eth2addr_text;
    const char * eth3addr_text;
    const char * type_text;
    const char * pcb_assembly_issue_text;
    int ignore_damm;

    uint32_t serial_number;
    uint8_t eth0_address[6];
    uint8_t eth1_address[6];
    uint8_t eth2_address[6];
    uint8_t eth3_address[6];
    uint16_t board;
    uint8_t oscillator;
    uint32_t pcb_assembly_issue;
    int test_mode = 0, prompt_to_user = 1;
    ret = 1;
    serial_text = eth0addr_text = eth1addr_text = eth2addr_text = eth3addr_text = type_text = pcb_assembly_issue_text = NULL;
    serial_number = pcb_assembly_issue = 0;
    board = 0;
    oscillator = 0;
    eth0_address[0] = eth0_address[1] = eth0_address[2] = eth0_address[3] = eth0_address[4] = eth0_address[5] = 0;
    eth1_address[0] = eth1_address[1] = eth1_address[2] = eth1_address[3] = eth1_address[4] = eth1_address[5] = 0;
    eth2_address[0] = eth2_address[1] = eth2_address[2] = eth2_address[3] = eth2_address[4] = eth2_address[5] = 0;
    eth3_address[0] = eth3_address[1] = eth3_address[2] = eth3_address[3] = eth3_address[4] = eth3_address[5] = 0;
    ignore_damm = 0;
    int i = 1;

    while(i < argc)
    {
        if (0 == strcmp(argv[i],"-s"))
        {
            serial_text = argv[i+1];
        }
        else if (0 == strcmp(argv[i],"-0"))
        {
            eth0addr_text = argv[i+1];
        }
        else if (0 == strcmp(argv[i],"-t"))
        {
            type_text = argv[i+1];
        }
        else if (0 == strcmp(argv[i],"-p"))
        {
            pcb_assembly_issue_text = argv[i+1];
        }
        else if (0 == strcmp(argv[i],"-f"))
        {
            ignore_damm = 1; 
        }
        else if (0 == strcmp(argv[i],"-Test"))
        {
            test_mode = 1;
        }
        else if (0 == strcmp(argv[i],"-no-prompt"))
        {
            prompt_to_user = 0;
        }

        ++i;

    }

    if (!serial_text || !eth0addr_text || !type_text || !pcb_assembly_issue_text)
    {
        ret = usage(argv[0], 1);
        goto cleanup;
    }

#if defined(NEEDS_ETH1_ADDRESS)
    if (!eth1addr_text)
    {
        ret = usage(argv[0], 1);
        goto cleanup;
    }
#endif

#if defined(NEEDS_ETH2_ADDRESS)
    if (!eth2addr_text)
    {
        ret = usage(argv[0], 1);
        goto cleanup;
    }
#endif

#if defined(NEEDS_ETH3_ADDRESS)
    if (!eth3addr_text)
    {
        ret = usage(argv[0], 1);
        goto cleanup;
    }
#endif

    if (0 != parse_serial_number(serial_text, &serial_number, ignore_damm))
    {
        ret = usage(argv[0], 1);
        goto cleanup;
    }

    if (0 != parse_ethernet_mac_address(eth0addr_text, eth0_address))
    {
        ret = usage(argv[0], 1);
        goto cleanup;
    }

#if defined(NEEDS_ETH1_ADDRESS)
    if (0 != parse_ethernet_mac_address(eth1addr_text, eth1_address))
    {
        ret = usage(argv[0], 1);
        goto cleanup;
    }
#endif

#if defined(NEEDS_ETH2_ADDRESS)
    if (0 != parse_ethernet_mac_address(eth2addr_text, eth1_address))
    {
        ret = usage(argv[0], 1);
        goto cleanup;
    }
#endif

#if defined(NEEDS_ETH3_ADDRESS)
    if (0 != parse_ethernet_mac_address(eth3addr_text, eth1_address))
    {
        ret = usage(argv[0], 1);
        goto cleanup;
    }
#endif

    if (0 != parse_board_type(type_text, &board, &oscillator))
    {
        ret = usage(argv[0], 1);
        goto cleanup;
    }

    if (0 != parse_pcb_assembly_issue(pcb_assembly_issue_text, &pcb_assembly_issue))
    {
        ret = usage(argv[0], 1);
        goto cleanup;
    }

    fprintf(stdout, "%s\n", "Hardware Characterisation");
    fprintf(stdout, "%s\n", "=========================");
    fprintf(stdout, "  Equipment Identifier: %02X%02X%02X-%010u\n", eth0_address[0], eth0_address[1], eth0_address[2], serial_number);
    fprintf(stdout, "Ethernet MAC Address 0: %02X:%02X:%02X:%02X:%02X:%02X\n", eth0_address[0], eth0_address[1], eth0_address[2], eth0_address[3], eth0_address[4], eth0_address[5]);
#if defined(NEEDS_ETH1_ADDRESS)
    fprintf(stdout, "Ethernet MAC Address 1: %02X:%02X:%02X:%02X:%02X:%02X\n", eth1_address[0], eth1_address[1], eth1_address[2], eth1_address[3], eth1_address[4], eth1_address[5]);
#endif
#if defined(NEEDS_ETH2_ADDRESS)
    fprintf(stdout, "Ethernet MAC Address 2: %02X:%02X:%02X:%02X:%02X:%02X\n", eth2_address[0], eth2_address[1], eth2_address[2], eth2_address[3], eth2_address[4], eth2_address[5]);
#endif
#if defined(NEEDS_ETH3_ADDRESS)
    fprintf(stdout, "Ethernet MAC Address 3: %02X:%02X:%02X:%02X:%02X:%02X\n", eth3_address[0], eth3_address[1], eth3_address[2], eth3_address[3], eth3_address[4], eth3_address[5]);
#endif
    fprintf(stdout, "      Hardware Variant: %s\n", lookup_variant(board)->full);
    fprintf(stdout, "            Oscillator: %s\n", lookup_oscillator(oscillator));
    fprintf(stdout, "    PCB Assembly Issue: %05u\n", pcb_assembly_issue);
    fprintf(stdout, "%s\n", "");

    if (1 == test_mode)
    {
        fprintf(stdout,"In test_mode: it's safe to proceed\n");
        fprintf(stdout, "%s", "Type 'Y' to proceed: ");

        if ('Y' != getc())
        {
            fprintf(stdout, "\n%s\n", "User cancelled operation");
            ret = 0;
            goto cleanup;
        }

        set_fuse_test_mode();
    }
    else
    {
        if (prompt_to_user)
        {
            fprintf(stdout, "%s\n", "Do you wish to write the above values?  This will PERMANENTLY blow the fuses!");
            fprintf(stdout, "%s", "Type 'Y' to proceed: ");

            if ('Y' != getc())
            {
                fprintf(stdout, "\n%s\n", "User cancelled operation");
                ret = 0;
                goto cleanup;
            }

            fprintf(stdout, "\n%s", "You're sure? ");
            fprintf(stdout, "%s", "Type 'Y' to proceed: ");

            if ('Y' != getc())
            {
                fprintf(stdout, "\n%s\n", "User cancelled operation");
                ret = 0;
                goto cleanup;
            }

            fprintf(stdout, "\n%s", "Quite sure? ");
            fprintf(stdout, "%s", "Type 'Y' to proceed: ");

            if ('Y' != getc())
            {
                fprintf(stdout, "\n%s\n", "User cancelled operation");
                ret = 0;
                goto cleanup;
            }

            fprintf(stdout, "\n%s", "Positive? ");
            fprintf(stdout, "%s", "Type 'Y' to proceed: ");

            if ('Y' != getc())
            {
                fprintf(stdout, "\n%s\n", "User cancelled operation");
                ret = 0;
                goto cleanup;
            }
            fprintf(stdout, "%s\n", "I'm quite done annoying you, so I'm going to write the fuses now...");
        }
    }


    ret = characterise_fuses(serial_number, eth0_address,eth1_address,board, oscillator, pcb_assembly_issue);


cleanup:

    return ret;
}


/*
 *  Reads an ethernet address from fuses:
 *   eth_addr_string - buffer for string version of ethernet address - length >= 18
 *   index 0-1, for different addresses, as enabled
 *   Returns: 0 if all fuses are 0 (unset address) otherwise 1
 */
int read_ethaddr_from_fuses(char* eth_addr_str, int index)
{
    unsigned int offset1;
    unsigned int offset2;
    unsigned char ethaddr[6];
    int i;
    unsigned char check = 0;
    
    switch (index)
    {
        case 0:
            offset1 = PC73XX_KEY2_OFFSET + 32;
            offset2 = PC73XX_KEY3_OFFSET + 32;
            break;
            
        case 1:
            offset1 = PC73XX_KEY4_OFFSET + 0;
            offset2 = PC73XX_KEY4_OFFSET + 48;
            break;
            
        default:
            sprintf(eth_addr_str, "Invalid index");
            return 0;
    }
    
    for (i = 0; i < 6; ++i, offset1 += 8, offset2 += 8)
    {
        unsigned char val =   ((unsigned char)(read_and_reverse_fuses(offset1, 8) & 0xff))
        | ((unsigned char)(read_and_reverse_fuses(offset2, 8) & 0xff));
        check |= val;
        ethaddr[i] = val;
    }
    
    if (!check) /* All zeros means not set */
    {
        sprintf(eth_addr_str, "Not set");
        return 0;
    }
    
    sprintf(eth_addr_str, "%02X:%02X:%02X:%02X:%02X:%02X",
            ethaddr[0], ethaddr[1], ethaddr[2], ethaddr[3], ethaddr[4], ethaddr[5]);
    return 1;
}


int do_read_ethaddr_from_fuses(void)
{
    char ethaddr0[18];
    char ethaddr1[18];
    read_ethaddr_from_fuses(ethaddr0, 0);
    read_ethaddr_from_fuses(ethaddr1, 1);
    printf("eth0: %s\neth1: %s\n", ethaddr0, ethaddr1);
    return 0;
}


#ifdef CONFIG_CMD_CHARACTERISE_HW
U_BOOT_CMD(
        characterise_hw, 12, 1, do_characterise,
        "Hw characterisation command to blow specific fuses",
        "<characterise_hw> <args>"
        );
#endif

#if defined (_DEBUG) || 1 
U_BOOT_CMD(
        read_hwinfo_from_fuse, 1, 1, read_and_dump_all_fuse_regions,
        "command to see the bit dump of fuse regions KEY2 KEY3 KEY4",
        "<read_hwinfo_from_fuse>"
        );

U_BOOT_CMD(
    read_ethaddr_from_fuse, 1, 1, do_read_ethaddr_from_fuses,
    "Read ethernet address(es) from fuses",
    "<read_ethaddr_from_fuses>"
        );
#endif
