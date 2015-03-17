#include <common.h>
#include <malloc.h>
#include <linux/compiler.h>
#include <i2c.h>

#include "characterisation.h"
#include "damm.h"

#if defined(CONFIG_CHARACTERISATION_IPA9131)

uint8_t serialised_characterisation_data[CONFIG_CHARACTERISATION_IPA9131_SIZE];

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
    { "0000", "000", "0",   0, '0' }, /* Bands: N/A          - Notes: N/A                                   */
    { "230A", "230", "A", 230, 'A' }, /* Bands: 1            - Notes: Rest of the World                     */
    { "230B", "230", "B", 230, 'B' }, /* Bands: 2, 5         - Notes: United States                         */
    { "267I", "267", "I", 267, 'I' }, /* Bands: N/A          - Notes: No Radio                              */
    { "400D", "400", "D", 400, 'D' }, /* Bands: 1, 3, 8      - Notes: Rest of the World                     */
    { "400E", "400", "E", 400, 'E' }, /* Bands: 2, 4, 5      - Notes: United States                         */
    { "245F", "245", "F", 245, 'F' }, /* Bands: 7, 3         - Notes: LTE Demonstrator                      */
    { "245H", "245", "H", 245, 'H' }, /* Bands: 1, 12        - Notes: LTE Demonstrator                      */
    { "248G", "248", "G", 248, 'G' }, /* Bands: 2, 5, 10, 12 - Notes: Freescale bsc9131-based E-40, 1 Tile  */
    { "248J", "248", "J", 248, 'J' }, /* Bands: 1, 3, 7, 20  - Notes: Freescale bsc9131-based E-40, 1 Tile  */
};
static const int num_variants = sizeof(variant_lookup) / sizeof(variant_lookup[0]);



static void set_ipa9131_ethernet_mac_addresses(void);



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




struct characterisation_data_t
{
    uint8_t version;
    uint8_t eth0addr[6];
    uint8_t eth1addr[6];
    uint8_t osc;
    uint16_t variant;
    uint16_t pcbai;
    uint8_t oui[3];
    uint32_t serial;
};

struct characterisation_data_t cdo;


void serialise_characterisation_info(const struct characterisation_data_t * cd, uint8_t payload[CONFIG_CHARACTERISATION_IPA9131_SIZE])
{
    memset(payload, 0, CONFIG_CHARACTERISATION_IPA9131_SIZE);

    payload[ 0] = cd->version;

    payload[ 1] = cd->eth0addr[0];
    payload[ 2] = cd->eth0addr[1];
    payload[ 3] = cd->eth0addr[2];
    payload[ 4] = cd->eth0addr[3];
    payload[ 5] = cd->eth0addr[4];
    payload[ 6] = cd->eth0addr[5];

    payload[ 7] = cd->eth1addr[0];
    payload[ 8] = cd->eth1addr[1];
    payload[ 9] = cd->eth1addr[2];
    payload[10] = cd->eth1addr[3];
    payload[11] = cd->eth1addr[4];
    payload[12] = cd->eth1addr[5];

    payload[13] = (((cd->osc & 0x3) << 6) & 0xC0) | (((cd->variant & 0x3FF) >> 4) & 0x3F);
    payload[14] = (((cd->variant & 0xF) << 4) & 0xF0); /* remaining four bits are reserved */

    payload[15] = (((cd->pcbai & 0xFF00) >> 8) & 0xFF);
    payload[16] = (cd->pcbai & 0xFF);

    if (cd->version == 0x00)
    {
        payload[17] = cd->oui[0];
        payload[18] = cd->oui[1];
        payload[19] = cd->oui[2];
        payload[20] = (((cd->serial & 0xFF000000) >> 24) & 0xFF);
        payload[21] = (((cd->serial & 0x00FF0000) >> 16) & 0xFF);
        payload[22] = (((cd->serial & 0x0000FF00) >>  8) & 0xFF);
        payload[23] = (((cd->serial & 0x000000FF) >>  0) & 0xFF);
    }
}



void deserialise_characterisation_info(const uint8_t payload[CONFIG_CHARACTERISATION_IPA9131_SIZE], struct characterisation_data_t * cd)
{
    cd->version = payload[0];

    cd->eth0addr[0] = payload[1];
    cd->eth0addr[1] = payload[2];
    cd->eth0addr[2] = payload[3];
    cd->eth0addr[3] = payload[4];
    cd->eth0addr[4] = payload[5];
    cd->eth0addr[5] = payload[6];

    cd->eth1addr[0] = payload[7];
    cd->eth1addr[1] = payload[8];
    cd->eth1addr[2] = payload[9];
    cd->eth1addr[3] = payload[10];
    cd->eth1addr[4] = payload[11];
    cd->eth1addr[5] = payload[12];

    cd->osc = ((payload[13] & 0xC0) >> 6) & 0x3;
    cd->variant = payload[13] & 0x3F;
    cd->variant <<= 4;
    cd->variant |= (((payload[14] & 0xF0) >> 4) & 0xF);

    cd->pcbai = payload[15];
    cd->pcbai <<= 8;
    cd->pcbai |= payload[16];

    if (cd->version == 0x00)
    {
        cd->oui[0] = payload[17];
        cd->oui[1] = payload[18];
        cd->oui[2] = payload[19];
        cd->serial = ((((uint32_t)(payload[20])) << 24) & 0xFF000000) |
                     ((((uint32_t)(payload[21])) << 16) & 0x00FF0000) |
                     ((((uint32_t)(payload[22])) <<  8) & 0x0000FF00) |
                     ((((uint32_t)(payload[23])) <<  0) & 0x000000FF);
    }
}



static int do_hwchar_i2c_read(void)
{
    return i2c_read(CONFIG_CHARACTERISATION_EEPROM_ADDR,
                    CONFIG_CHARACTERISATION_IPA9131_OFFSET, 1,
                    serialised_characterisation_data,
                    CONFIG_CHARACTERISATION_IPA9131_SIZE);
}

int characterisation_init(void)
{
    int ret;
    const variant_record * hw_variant;
    char evar[20];

    if (0 != (ret = do_hwchar_i2c_read())) {
        udelay(250);
        if (0 != (ret = do_hwchar_i2c_read())) {
            udelay(250);
            if (0 != (ret = do_hwchar_i2c_read())) {
                udelay(250);
                if (0 != (ret = do_hwchar_i2c_read())) {
                    udelay(250);
                    do_hwchar_i2c_read();
                }
            }
        }
    }

    if (ret != 0) {
        puts("Error loading characterisation information\n");
        return 0;
    }

    deserialise_characterisation_info(serialised_characterisation_data, &cdo);

    hw_variant = lookup_variant(cdo.variant);

    if (!hw_variant) {
        puts("Error locating hardware variant\n");
        return 0;
    }

    if (memcmp(cdo.eth0addr, "\0\0\0\0\0\0", 6) && memcmp(cdo.eth0addr, "\xFF\xFF\xFF\xFF\xFF\xFF", 6))
    {
        setenv("board_variant_full", hw_variant->full);
        setenv("board_variant_part", hw_variant->part);
        setenv("board_variant_variant", hw_variant->variant);
        setenv("board_variant_oscillator", lookup_oscillator(cdo.osc));
        sprintf(evar, "%u", cdo.pcbai);
        setenv("board_pcb_assembly_issue", evar);
        set_ipa9131_ethernet_mac_addresses();
    }

    return 1;
}



void print_characterisation(void)
{
    if (memcmp(cdo.eth0addr, "\0\0\0\0\0\0", 6) && memcmp(cdo.eth0addr, "\xFF\xFF\xFF\xFF\xFF\xFF", 6))
    {
        printf("Board: %s%s-%c%c\n", lookup_variant(cdo.variant)->full, lookup_oscillator(cdo.osc), (cdo.pcbai & 0xFF00) >> 8, cdo.pcbai & 0xFF);
        printf("EID:   %02X%02X%02X-%010u\n", cdo.oui[0], cdo.oui[1], cdo.oui[2], cdo.serial);
        printf("eMAC0: %02X:%02X:%02X:%02X:%02X:%02X\n", cdo.eth0addr[0], cdo.eth0addr[1], cdo.eth0addr[2], cdo.eth0addr[3], cdo.eth0addr[4], cdo.eth0addr[5]);
        printf("eMAC1: %02X:%02X:%02X:%02X:%02X:%02X\n", cdo.eth1addr[0], cdo.eth1addr[1], cdo.eth1addr[2], cdo.eth1addr[3], cdo.eth1addr[4], cdo.eth1addr[5]);
    }
    else
    {
        printf("Board: %s\n", "Not Characterised");
    }
}


static void set_ipa9131_ethernet_mac_addresses(void)
{
    char ethaddr[18];
    char enetvar[9];

    if (memcmp(cdo.eth0addr, "\0\0\0\0\0\0", 6))
    {
        sprintf(ethaddr, "%02X:%02X:%02X:%02X:%02X:%02X",
            cdo.eth0addr[0],
            cdo.eth0addr[1],
            cdo.eth0addr[2],
            cdo.eth0addr[3],
            cdo.eth0addr[4],
            cdo.eth0addr[5]);
        sprintf(enetvar, "ethaddr");

        if (!getenv(enetvar))
            setenv(enetvar, ethaddr);
    }

    if (memcmp(cdo.eth1addr, "\0\0\0\0\0\0", 6))
    {
        sprintf(ethaddr, "%02X:%02X:%02X:%02X:%02X:%02X",
            cdo.eth1addr[0],
            cdo.eth1addr[1],
            cdo.eth1addr[2],
            cdo.eth1addr[3],
            cdo.eth1addr[4],
            cdo.eth1addr[5]);
        sprintf(enetvar, "eth1addr");

        if (!getenv(enetvar))
            setenv(enetvar, ethaddr);
    }
}



#ifdef CONFIG_CMD_CHARACTERISE_HW

int parse_octet(const char * octet_text, uint8_t * octet)
{
    char * ep;
    unsigned long ulval;

    if (!octet_text || !octet)
    {
        fprintf(stderr, "%s\n", "Invalid octet: NULL argument to conversion function.");
        return -1;
    }

    if (strlen(octet_text) != 2)
    {
        fprintf(stderr, "%s\n", "Invalid octet: not 2 hexadecimal digits in length.");
        return -1;
    }

    errno = 0;
    ulval = simple_strtoul(octet_text, &ep, 16);

    if (ep == octet_text || *ep != '\0')
    {
        fprintf(stderr, "%s\n", "Invalid octet: not a hexadecimal number.");
        return -1;
    }

    if (ulval > 0xFF)
    {
        fprintf(stderr, "%s\n", "Invalid octet: out of range.");
        return -1;
    }

    *octet = (uint8_t)(ulval & 0x000000FF);
    return 0;
}


int parse_oui(const char * oui_text, uint8_t * oui)
{
    unsigned int i;

    if (!oui_text || !oui)
    {
        fprintf(stderr, "%s\n", "Invalid OUI: NULL argument to conversion function.");
        return -1;
    }

    if (strlen(oui_text) != 6)
    {
        fprintf(stderr, "%s\n", "Invalid OUI: not 6 characters in length.");
        return -1;
    }

    for (i = 0; i < 3; ++i)
    {
        char input[3];
        input[0] = oui_text[i * 2];
        input[1] = oui_text[(i * 2) + 1];
        input[2] = '\0';

        if (0 != parse_octet(input, &oui[i]))
        {
            fprintf(stderr, "Invalid OUI: failed to parse octet %u.\n", i);
            return -1;
        }
    }

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

        if (0 != parse_octet(input, &address[i]))
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

    if (ep == input || *ep != '\0')
    {
        fprintf(stderr, "%s\n", "Invalid number: not a hexadecimal number.");
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

    if (ep == serial_text || *ep != '\0')
    {
        fprintf(stderr, "%s\n", "Invalid serial number: not a decimal number.");
        return -1;
    }

    *serial_number = (uint32_t)ulval;
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

    if (*board == 0 || *board >= num_variants)
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


static int parse_pcb_assembly_issue(const char * pcb_assembly_issue_text, uint16_t * pcb_assembly_issue)
{
    if (!pcb_assembly_issue_text || !pcb_assembly_issue)
    {
        fprintf(stderr, "%s\n", "Invalid PCB assembly issue: NULL argument to conversion function.");
        return -1;
    }

    if (strlen(pcb_assembly_issue_text) != 6)
    {
        fprintf(stderr, "Invalid PCB assembly issue: Input length must be 6 (0x%c%c%c%c).\n", '?', '?', '?', '?');
        return -1;
    }

    if (pcb_assembly_issue_text[0] != '0' || !(pcb_assembly_issue_text[1] == 'x' || pcb_assembly_issue_text[1] == 'X'))
    {
        fprintf(stderr, "%s\n", "Invalid PCB assembly issue: Invalid prefix (must be 0x or 0X).");
        return -1;
    }

    if (0 != parse_hex_uint16(pcb_assembly_issue_text + 2, pcb_assembly_issue))
    {
        fprintf(stderr, "%s\n", "Invalid PCB assembly issue: Invalid hexadecimal value.");
        return -1;
    }

    return 0;
}


static int usage(const char * progname, int ret)
{
    fprintf((ret ? stderr : stdout), "Usage: %s -o OUI -s SERIAL_NUMBER -0 ETH0ADDR -1 ETH1ADDR -t TYPE -p PCB_ASSEMBLY_ISSUE\n", progname);
    fprintf((ret ? stderr : stdout), "       -o OUI               : Specify the OUI (part of the EID) of this board.\n");
    fprintf((ret ? stderr : stdout), "                              This is specified as three hexadecimal octets.\n");
    fprintf((ret ? stderr : stdout), "       -s SERIAL_NUMBER     : Specify the serial number (part of the EID, with DAMM digit) of this board.\n");
    fprintf((ret ? stderr : stdout), "                              This is specified as a 10 digit decimal number made up of the 9\n");
    fprintf((ret ? stderr : stdout), "                              serial number digits and 1 DAMM check digit.\n");
    fprintf((ret ? stderr : stdout), "       -0 ETH0ADDR          : Specify the first Ethernet MAC address.\n");
    fprintf((ret ? stderr : stdout), "                              An Ethernet address is specified as six colon separated hexadecimal octets.\n");
    fprintf((ret ? stderr : stdout), "                              Octets are left padded to two digits with zeroes.\n");
    fprintf((ret ? stderr : stdout), "                              For example: CA:FE:BA:BE:B0:0C.\n");
    fprintf((ret ? stderr : stdout), "       -1 ETH1ADDR          : Specify the second Ethernet MAC address.\n");
    fprintf((ret ? stderr : stdout), "       -t TYPE              : Specify the board type as a 16 bit decimal value (0x0000-0xFFFF).\n");
    fprintf((ret ? stderr : stdout), "                              The TYPE argument must correspond to a known board hardware type and oscillator value.\n");
    fprintf((ret ? stderr : stdout), "                              Unused bits must be set to 0.\n");
    fprintf((ret ? stderr : stdout), "       -p PCB_ASSEMBLY_ISSUE: Specify the PCB assembly issue as a 16 bit decimal value (0x0000-0xFFFF).\n");
    fprintf((ret ? stderr : stdout), "       -f                   : Force acceptance of a serial number that fails the DAMM algorithm check.\n");
    fprintf((ret ? stderr : stdout), "       -no-prompt           : Do not prompt user for input before blowing the fuses (must be used when fuse blowing is done via scripts).\n");
    fprintf((ret ? stderr : stdout), "\n");
    fprintf((ret ? stderr : stdout), "       This utility is only supposed to work on blank boards.\n");
    return ret;
}

int do_characterise(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
    int ret;
    int i;
    const char * oui_text;
    const char * serial_text;
    const char * eth0addr_text;
    const char * eth1addr_text;
    const char * type_text;
    const char * pcb_assembly_issue_text;
    int ignore_damm;
    int prompt_to_user;
    struct characterisation_data_t cd;
    uint8_t serialised[CONFIG_CHARACTERISATION_IPA9131_SIZE];

    ret = 1;
    i = 0;
    oui_text = serial_text = eth0addr_text = eth1addr_text = type_text = pcb_assembly_issue_text = NULL;
    ignore_damm = 0;
    prompt_to_user = 1;
    memset(&cd, 0, sizeof(cd));
    memset(serialised, 0, CONFIG_CHARACTERISATION_IPA9131_SIZE);

    while(i < argc)
    {
        if (0 == strcmp(argv[i],"-o"))
        {
            oui_text = argv[i+1];
        }
        else if (0 == strcmp(argv[i],"-s"))
        {
            serial_text = argv[i+1];
        }
        else if (0 == strcmp(argv[i],"-0"))
        {
            eth0addr_text = argv[i+1];
        }
        else if (0 == strcmp(argv[i],"-1"))
        {
            eth1addr_text = argv[i+1];
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
        else if (0 == strcmp(argv[i],"-no-prompt"))
        {
            prompt_to_user = 0;
        }

        ++i;
    }

    if (!oui_text || !serial_text || !eth0addr_text || !eth1addr_text || !type_text || !pcb_assembly_issue_text)
    {
        ret = usage(argv[0], 1);
        goto cleanup;
    }

    if (0 != parse_oui(oui_text, cd.oui) ||
        0 != parse_serial_number(serial_text, &cd.serial, ignore_damm) ||
        0 != parse_ethernet_mac_address(eth0addr_text, cd.eth0addr) ||
        0 != parse_ethernet_mac_address(eth1addr_text, cd.eth1addr) ||
        0 != parse_board_type(type_text, &cd.variant, &cd.osc) ||
        0 != parse_pcb_assembly_issue(pcb_assembly_issue_text, &cd.pcbai))
    {
        ret = usage(argv[0], 1);
        goto cleanup;
    }

    fprintf(stdout, "%s\n", "Hardware Characterisation");
    fprintf(stdout, "%s\n", "=========================");
    fprintf(stdout, "  Equipment Identifier: %02X%02X%02X-%010u\n", cd.oui[0], cd.oui[1], cd.oui[2], cd.serial);
    fprintf(stdout, "Ethernet MAC Address 0: %02X:%02X:%02X:%02X:%02X:%02X\n", cd.eth0addr[0], cd.eth0addr[1], cd.eth0addr[2], cd.eth0addr[3], cd.eth0addr[4], cd.eth0addr[5]);
    fprintf(stdout, "Ethernet MAC Address 1: %02X:%02X:%02X:%02X:%02X:%02X\n", cd.eth1addr[0], cd.eth1addr[1], cd.eth1addr[2], cd.eth1addr[3], cd.eth1addr[4], cd.eth1addr[5]);
    fprintf(stdout, "      Hardware Variant: %s\n", lookup_variant(cd.variant)->full);
    fprintf(stdout, "            Oscillator: %s\n", lookup_oscillator(cd.osc));
    fprintf(stdout, "    PCB Assembly Issue: %05u\n", cd.pcbai);
    fprintf(stdout, "%s\n", "");

    if (prompt_to_user)
    {
        fprintf(stdout, "%s\n", "Do you wish to write the above values?  This will PERMANENTLY blow fuses!");
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

    cd.version = CONFIG_CHARACTERISATION_IPA9131_VERSION;
    serialise_characterisation_info(&cd, serialised);

    /* write the EEPROM in page mode (8 bytes at a time) */

    if (0 != (ret = i2c_write(CONFIG_CHARACTERISATION_EEPROM_ADDR, CONFIG_CHARACTERISATION_IPA9131_OFFSET + 0, 1, serialised + 0, 8)))
    {
        printf("i2c_write returned %d\n", ret);
        goto cleanup;
    }

    udelay(5000);

    if (0 != (ret = i2c_write(CONFIG_CHARACTERISATION_EEPROM_ADDR, CONFIG_CHARACTERISATION_IPA9131_OFFSET + 8, 1, serialised + 8, 8)))
    {
        printf("i2c_write returned %d\n", ret);
        goto cleanup;
    }

    udelay(5000);

    if (0 != (ret = i2c_write(CONFIG_CHARACTERISATION_EEPROM_ADDR, CONFIG_CHARACTERISATION_IPA9131_OFFSET + 16, 1, serialised + 16, 8)))
    {
        printf("i2c_write returned %d\n", ret);
        goto cleanup;
    }

    udelay(5000);

    if (0 != (ret = i2c_write(CONFIG_CHARACTERISATION_EEPROM_ADDR, CONFIG_CHARACTERISATION_IPA9131_OFFSET + 24, 1, serialised + 24, 7)))
    {
        printf("i2c_write returned %d\n", ret);
        goto cleanup;
    }

    udelay(5000);

cleanup:
    return ret;
}



U_BOOT_CMD(
        characterise_hw, 15, 0, do_characterise,
        "Hw characterisation command to blow specific fuses",
        "<characterise_hw> <args>"
        );
#endif

#endif
