#include <common.h>
#include <malloc.h>
#include <linux/compiler.h>
#include <errno.h>
#include <i2c.h>

#include "characterisation.h"
#include "damm.h"
#include "ipat2k_fuse.h"

#if defined(CONFIG_CHARACTERISATION_IPAT2K)

/*RSM bitmap in fuse/eeprom: r:reserved p:production d:development s:specials t:test*/
/*p|d|s|r| r|r|r|r*/
#define BOARD_RSM_MASK_FUSE 0xE0
/*p|d|s|r| r|r|r|t*/
#define BOARD_RSM_MASK_EEPROM 0xE1

uint8_t serialised_characterisation_data[CONFIG_CHARACTERISATION_IPAT2K_SIZE];

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
    { "0000", "000", "0",   0, '0' }, /* Bands: N/A              - Notes: N/A                                                             */
    { "230A", "230", "A", 230, 'A' }, /* Bands: 1                - Notes: Europe and RoW                                                  */
    { "230B", "230", "B", 230, 'B' }, /* Bands: 2, 5             - Notes: United States                                                   */
    { "267I", "267", "I", 267, 'I' }, /* Bands: N/A              - Notes: No Radio                                                        */
    { "400D", "400", "D", 400, 'D' }, /* Bands: 1, 3, 8          - Notes: Europe and RoW                                                  */
    { "400E", "400", "E", 400, 'E' }, /* Bands: 2, 4, 5          - Notes: United States                                                   */
    { "245F", "245", "F", 245, 'F' }, /* Bands: 7, 13            - Notes: LTE Demonstrator - Freescale bsc9131rdb                         */
    { "245H", "245", "H", 245, 'H' }, /* Bands: 1, 12            - Notes: LTE Demonstrator - Freescale bsc9131rdb                         */
    { "248G", "248", "G", 248, 'G' }, /* Bands: 2, 5, 10, 12, 17 - Notes: Freescale bsc9131-based E-40, 1 Tile US, non secure             */
    { "248J", "248", "J", 248, 'J' }, /* Bands: 1, 3, 7, 20      - Notes: Freescale bsc9131-based E-40, 1 Tile EU, non secure             */
    { "247G", "247", "G", 247, 'G' }, /* Bands: 2, 5, 10, 12, 17 - Notes: Freescale bsc9131-based E-40, 2 Tile US (LTE part), non secure  */
    { "278J", "278", "J", 278, 'J' }, /* Bands: 1, 3, 7, 20      - Notes: Freescale bsc9131-based E-40, 1 Tile EU, secure boot            */
    { "278L", "278", "L", 278, 'L' }, /* Bands: 3, 7, 8, 20      - Notes: Freescale bsc9131-based E-40, 1 Tile EU band 8, secure boot     */
    { "248L", "248", "L", 248, 'L' }, /* Bands: 3, 7, 8, 20      - Notes: Freescale bsc9131-based E-40, 1 Tile EU band 8, non secure      */
    { "431I", "431", "I", 431, 'I' }, /* Bands: N/A              - Notes: Transcede t2200evm Evaluation Board                             */
    { "248M", "248", "M", 248, 'M' }, /* Bands: 2, 4, 13, 17     - Notes: Freescale bsc9131-based E-40, 1 Tile US vII, non secure         */
    { "278M", "278", "M", 278, 'M' }, /* Bands: 2, 4, 13, 17     - Notes: Freescale bsc9131-based E-40, 1 Tile US vII, secure boot        */
    { "400N", "400", "N", 400, 'N' }, /* Bands: 8                - Notes: 2G/3G Product Refresh Band8 Only                                */
    { "433I", "433", "I", 433, 'I' }, /* Bands: N/A              - Notes: Transcede t3300evm Evaluation Board                             */
    { "453A", "453", "A", 453, 'A' }, /* Bands: 1                - Notes: P110 integrated AP and WBH, EU                                  */
    { "453B", "453", "B", 453, 'B' }, /* Bands: 2,5              - Notes: P110 integrated AP and WBH, US                                  */
    { "435A", "435", "A", 435, 'A' }, /* Bands: 1                - Notes: S60 single band FDD HW, T2K 2130 + ADI radio                    */
    { "436A", "436", "A", 436, 'A' }, /* Bands: 1                - Notes: S60 single band FDD HW, T2K 2100 + ADI radio (Presence)         */
    { "435R", "435", "R", 435, 'R' }, /* Bands: 2                - Notes: S60 single band FDD HW, T2K 2130 + ADI radio                    */
    { "436R", "436", "R", 436, 'R' }, /* Bands: 2                - Notes: S60 single band FDD HW, T2K 2100 + ADI radio (Presence)         */
    { "435S", "435", "S", 435, 'S' }, /* Bands: 3                - Notes: S60 single band FDD HW, T2K 2130 + ADI radio                    */
    { "436S", "436", "S", 436, 'S' }, /* Bands: 3                - Notes: S60 single band FDD HW, T2K 2100 + ADI radio (Presence)         */
    { "431C", "431", "C", 431, 'C' }, /* Bands: 4                - Notes: MitraStar T2K Development Board                                 */
    { "437T", "437", "T", 437, 'T' }, /* Bands: 40               - Notes: S60 single band TDD HW, T2K 2130 + ADI radio                    */
    { "438T", "438", "T", 438, 'T' }, /* Bands: 40               - Notes: S60 single band TDD HW, T2K 2100 + ADI radio (Presence)         */
    { "437U", "437", "U", 437, 'U' }, /* Bands: 41               - Notes: S60 single band TDD HW, T2K 2130 + ADI radio                    */
    { "438U", "438", "U", 438, 'U' }, /* Bands: 41               - Notes: S60 single band TDD HW, T2K 2100 + ADI radio (Presence          */

};

static const int num_variants = sizeof(variant_lookup) / sizeof(variant_lookup[0]);



static void set_ipat2k_ethernet_mac_addresses(void);



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
    uint64_t serial;
    uint8_t production_mode;
    uint8_t test_mode;
    uint8_t development_mode;
    uint8_t specials_mode;
    uint8_t boot_license_disabled
};

struct characterisation_data_t cdo;


void serialise_characterisation_info_eeprom(const struct characterisation_data_t * cd, uint8_t payload[CONFIG_CHARACTERISATION_IPAT2K_SIZE])
{
    memset(payload, 0, CONFIG_CHARACTERISATION_IPAT2K_SIZE);

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

    payload[17] = cd->oui[0];
    payload[18] = cd->oui[1];
    payload[19] = cd->oui[2];
    payload[20] = (((cd->serial & 0x300000000) >> 32) & 0xFF);
    payload[21] = (((cd->serial & 0xFF000000) >> 24) & 0xFF);
    payload[22] = (((cd->serial & 0x00FF0000) >> 16) & 0xFF);
    payload[23] = (((cd->serial & 0x0000FF00) >>  8) & 0xFF);
    payload[24] = (((cd->serial & 0x000000FF) >>  0) & 0xFF);

    if (cd->production_mode)
        payload[239] = 0x80;
    else if (cd->development_mode)
        payload[239] = 0x40;
    else if (cd->test_mode)
        payload[239] = 0x01;
    else
        payload[239] = 0x20; /*Specials*/

    if (cd->boot_license_disabled)
        payload[1] |= 0x10;

    /*Check if board is already EEPROM characterised*/
    if (memcmp(cdo.eth1addr, "\0\0\0\0\0\0", 6) && memcmp(cdo.eth1addr, "\xFF\xFF\xFF\xFF\xFF\xFF", 6))
    {
        /*Case of EEPROM re-characterisation: preserve provisioning status info*/
        if (i2c_read(CONFIG_CHARACTERISATION_EEPROM_ADDR,CONFIG_CHARACTERISATION_IPAT2K_OFFSET + 226, 2,&payload[226],1))
            /*failed reading try once more*/
            if (i2c_read(CONFIG_CHARACTERISATION_EEPROM_ADDR,CONFIG_CHARACTERISATION_IPAT2K_OFFSET + 226, 2,&payload[226],1))
                /*failed reading try once more*/
                i2c_read(CONFIG_CHARACTERISATION_EEPROM_ADDR,CONFIG_CHARACTERISATION_IPAT2K_OFFSET + 226, 2,&payload[226],1);

	/*Case of EEPROM re-characterisation: preserve Master Key value*/
	if (i2c_read(CONFIG_CHARACTERISATION_EEPROM_ADDR,CONFIG_CHARACTERISATION_IPAT2K_OFFSET + 192, 2,&payload[192],32))
		/*failed reading try once more*/
		if (i2c_read(CONFIG_CHARACTERISATION_EEPROM_ADDR,CONFIG_CHARACTERISATION_IPAT2K_OFFSET + 192, 2,&payload[192],32))
			/*failed reading try once more*/
			i2c_read(CONFIG_CHARACTERISATION_EEPROM_ADDR,CONFIG_CHARACTERISATION_IPAT2K_OFFSET + 192, 2,&payload[192],32);

    }

}


void read_characterisation_from_fuses(struct characterisation_data_t * cd)
{

    uint8_t buff[16];
    memset(buff,0,16);

    efuse_read_instance(AP_CHARACTERISATION_EFUSE_INSTANCE,buff);

    cd->production_mode = cd->test_mode = cd->development_mode = cd->specials_mode = 0;

    if ( (buff[1] & BOARD_RSM_MASK_FUSE ) == 0x80 )
    {
        cd->production_mode = 1;
    }
    else if ( (buff[1] & BOARD_RSM_MASK_FUSE ) == 0x20 )
    {
        cd->specials_mode = 1;

    }
    else if ( (buff[1] & BOARD_RSM_MASK_FUSE ) == 0x40)
    {

        if (is_test_mode())
            cd->test_mode = 1;
        else
            cd->development_mode = 1;

    }
    else /*No mode set or multiple bit set*/
    {
        cd->production_mode = 1;
    }



    cd->osc = ((buff[2] & 0xC0) >> 6) & 0x3;
    cd->variant = buff[2] & 0x3F;
    cd->variant <<= 4;
    cd->variant |= (((buff[3] & 0xF0) >> 4) & 0xF);

    cd->pcbai = buff[4];
    cd->pcbai <<= 8;
    cd->pcbai |= buff[5];

    cd->oui[0] = buff[8];
    cd->oui[1] = buff[9];
    cd->oui[2] = buff[10];




    /*34 bits of serial*/
    cd->serial = ((((uint64_t)(buff[11])) << 32) & 0x300000000) |
	    ((((uint64_t)(buff[12])) << 24) & 0xFF000000) |
	    ((((uint64_t)(buff[13])) << 16) & 0x00FF0000) |
	    ((((uint64_t)(buff[14])) <<  8) & 0x0000FF00) |
            ((((uint64_t)(buff[15])) <<  0) & 0x000000FF);

    efuse_read_instance(MAC_ADDR_EFUSE_INSTANCE,buff);
    cd->eth0addr[0]= buff[0];
    cd->eth0addr[1]= buff[1];
    cd->eth0addr[2]= buff[2];
    cd->eth0addr[3]= buff[3];
    cd->eth0addr[4]= buff[4];
    cd->eth0addr[5]= buff[5];
    cd->eth1addr[0]= buff[6];
    cd->eth1addr[1]= buff[7];
    cd->eth1addr[2]= buff[8];
    cd->eth1addr[3]= buff[9];
    cd->eth1addr[4]= buff[10];
    cd->eth1addr[5]= buff[11];



}

void deserialise_characterisation_info_eeprom(const uint8_t payload[CONFIG_CHARACTERISATION_IPAT2K_SIZE], struct characterisation_data_t * cd)
{
    cd->version = payload[0];

    cd->production_mode = 0;
    cd->test_mode = 0;
    cd->development_mode = 0;
    cd->specials_mode = 0;

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

    cd->oui[0] = payload[17];
    cd->oui[1] = payload[18];
    cd->oui[2] = payload[19];

    cd->serial = ((((uint64_t)(payload[20])) << 32) & 0x300000000) | 
        ((((uint64_t)(payload[21])) << 24) & 0xFF000000) |
        ((((uint64_t)(payload[22])) << 16) & 0x00FF0000) |
        ((((uint64_t)(payload[23])) <<  8) & 0x0000FF00) |
        ((((uint64_t)(payload[24])) <<  0) & 0x000000FF);


    if ( (payload[239] & BOARD_RSM_MASK_EEPROM) == 0x80)
        cd->production_mode = 1;
    else if ((payload[239] & BOARD_RSM_MASK_EEPROM) == 0x40)
        cd->development_mode = 1;
    else if ( (payload[239] & BOARD_RSM_MASK_EEPROM) == 0x20)
        cd->specials_mode = 1;
    else if ( (payload[239] & BOARD_RSM_MASK_EEPROM) == 0x01)
        cd->test_mode = 1;
    else 
        cd->specials_mode = 1;

}

int is_test_mode()
{
    uint8_t val = 0;
    if (0 == i2c_read(CONFIG_CHARACTERISATION_EEPROM_ADDR,CONFIG_CHARACTERISATION_IPAT2K_OFFSET + 239, 2,&val,1))
    {
        if ( (val & BOARD_RSM_MASK_EEPROM) == 0x01 )
            return 1;
            
    }
    return 0;
}

static void set_test_mode()
{
    uint8_t val = 0x01;
    if (0 != i2c_write(CONFIG_CHARACTERISATION_EEPROM_ADDR, CONFIG_CHARACTERISATION_IPAT2K_OFFSET + 239, 2, &val, 1))
        printf("WARNING: Setting test mode bit in eeprom failed\n");

}

static void clear_test_mode()
{
    /*Clearing test mode bit means setting the board to Development mode in eeprom*/
    uint8_t val = 0x40;
    if (0 != i2c_write(CONFIG_CHARACTERISATION_EEPROM_ADDR, CONFIG_CHARACTERISATION_IPAT2K_OFFSET + 239, 2, &val, 1))
        printf("WARNING: Clearing test mode bit in eeprom failed\n");

}

int characterisation_is_production_mode(void)
{
    return cdo.production_mode != 0;
}


int characterisation_is_test_mode(void)
{
    return cdo.test_mode != 0;
}


int characterisation_is_development_mode(void)
{
    return cdo.development_mode != 0;
}


int characterisation_is_specials_mode(void)
{
    return cdo.specials_mode != 0;
}


u8 characterisation_loader_revocation(void)
{
    return ipat2k_fuse_read_loader_revocation();
}


u8 characterisation_application_revocation(void)
{
    return ipat2k_fuse_read_application_revocation();
}


static int do_hwchar_i2c_read(void)
{
    memset(serialised_characterisation_data, 0, CONFIG_CHARACTERISATION_IPAT2K_SIZE);
    return i2c_read(CONFIG_CHARACTERISATION_EEPROM_ADDR,
                    CONFIG_CHARACTERISATION_IPAT2K_OFFSET, 2,
                    serialised_characterisation_data,
                    CONFIG_CHARACTERISATION_IPAT2K_SIZE);
}

static void update_rsm_from_tz(struct characterisation_data_t * cdo)
{
    switch (smc_get_active_rsm())
    {
        case RSM_A_PROD:
            {
                cdo->production_mode = cdo->test_mode = cdo->development_mode = cdo->specials_mode = 0;
                cdo->production_mode = 1;
                break;
            }
        case RSM_A_DEV:
            {
                cdo->production_mode = cdo->test_mode = cdo->development_mode = cdo->specials_mode = 0;
                if (is_test_mode())
                    cdo->test_mode = 1;
                else
                    cdo->development_mode = 1;
                break;
            }
        case RSM_A_TEST:
            {
                cdo->production_mode = cdo->test_mode = cdo->development_mode = cdo->specials_mode = 0;
                cdo->test_mode = 1;
                break;
            }
        case RSM_A_SPECIALS:
            {
                cdo->production_mode = cdo->test_mode = cdo->development_mode = cdo->specials_mode = 0;
                cdo->specials_mode = 1;
                break;
            }
        default:
            /*Do nothing: keep the mode as read from fuse/eeprom*/
            break;
    }

}

int characterisation_init(void)
{
    int ret;
    const variant_record * hw_variant;
    char evar[20];

    if (ipat2k_is_board_fused())
    {
        read_characterisation_from_fuses(&cdo);

    }
    else
    {
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

        deserialise_characterisation_info_eeprom(serialised_characterisation_data, &cdo);
    }

    /*Boot license: update RSM via TZ call*/
    update_rsm_from_tz(&cdo);

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
        set_ipat2k_ethernet_mac_addresses();
    }

    return 1;
}



void print_characterisation(void)
{
    unsigned char pcbai0;
    unsigned char pcbai1;

    if (memcmp(cdo.eth0addr, "\0\0\0\0\0\0", 6) && memcmp(cdo.eth0addr, "\xFF\xFF\xFF\xFF\xFF\xFF", 6))
    {
        pcbai0 = (cdo.pcbai & 0xFF00) >> 8;
        pcbai1 = cdo.pcbai & 0xFF;

        printf("Board: %s%s", lookup_variant(cdo.variant)->full, lookup_oscillator(cdo.osc));

        if (pcbai0 >= 0x41 && pcbai0 <= 0x7a)
        {
            printf("-%c", pcbai0);
        }

        if (pcbai1 >= 0x41 && pcbai1 <= 0x7a)
        {
            if (pcbai0 >= 0x41 && pcbai0 <= 0x7a)
            {
                printf("%c", pcbai1);
            }
            else
            {
                printf("-%c", pcbai1);
            }
        }

        printf("%s", "\n");
        printf("EID:   %02X%02X%02X-%010llu\n", cdo.oui[0], cdo.oui[1], cdo.oui[2], cdo.serial);
        printf("eMAC0: %02X:%02X:%02X:%02X:%02X:%02X\n", cdo.eth0addr[0], cdo.eth0addr[1], cdo.eth0addr[2], cdo.eth0addr[3], cdo.eth0addr[4], cdo.eth0addr[5]);
        printf("eMAC1: %02X:%02X:%02X:%02X:%02X:%02X\n", cdo.eth1addr[0], cdo.eth1addr[1], cdo.eth1addr[2], cdo.eth1addr[3], cdo.eth1addr[4], cdo.eth1addr[5]);
        printf("Mode:  ");

        if (cdo.production_mode)
            printf("Production\n");
        else if (cdo.test_mode)
            printf("Test\n");
        else if (cdo.development_mode)
            printf("Development\n");
        else
            printf("Specials\n");

        printf("Fused: %s\n", (ipat2k_is_board_fused() ? "Yes" : "No"));
    }
    else
    {
        printf("Board: %s\n", "Not Characterised");
    }
}


static void set_ipat2k_ethernet_mac_addresses(void)
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


static int parse_serial_number(const char * serial_text, uint64_t * serial_number, int ignore_damm)
{
    char * ep;
    unsigned long long ulval;

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
    ulval = simple_strtoull(serial_text, &ep, 10);

    if (ep == serial_text || *ep != '\0')
    {
        fprintf(stderr, "%s\n", "Invalid serial number: not a decimal number.");
        return -1;
    }



    *serial_number = (uint64_t)(ulval & 0x3FFFFFFFF);
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

static int user_input( int prompt_to_user, const char *type)
{
    int ret = 0;

    if ( prompt_to_user )
    {
        fprintf(stdout, "%s %s\n", "Do you wish to write the above values?  This will PERMANENTLY write",type);
        fprintf(stdout, "%s", "Type 'Y' to proceed: ");

        if ('Y' != getc())
        {
            fprintf(stdout, "\n%s\n", "User cancelled operation");
            ret = 1;
            goto end;
        }

        fprintf(stdout, "\n%s", "You're sure? ");
        fprintf(stdout, "%s", "Type 'Y' to proceed: ");

        if ('Y' != getc())
        {
            fprintf(stdout, "\n%s\n", "User cancelled operation");
            ret = 1;
            goto end;
        }

        fprintf(stdout, "\n%s", "Quite sure? ");
        fprintf(stdout, "%s", "Type 'Y' to proceed: ");

        if ('Y' != getc())
        {
            fprintf(stdout, "\n%s\n", "User cancelled operation");
            ret = 1;
            goto end;
        }

        fprintf(stdout, "\n%s", "Positive? ");
        fprintf(stdout, "%s", "Type 'Y' to proceed: ");

        if ('Y' != getc())
        {
            fprintf(stdout, "\n%s\n", "User cancelled operation");
            ret = 1;
            goto end;
        }

        fprintf(stdout, "%s %s\n", "I'm quite done annoying you, so I'm going to write the",type);
    }

end:
    return ret;
}

void characterise_fuses (const struct characterisation_data_t * cd)
{
    uint8_t buff[16];
    memset(buff,0,16);

    if (!ipat2k_is_board_fused())
    {
        if (cd->production_mode)
        {
            buff[1] |= 0x80;
            /*set test mode bit, when fused rsm is going to be production
            so that once the dev boot license is installed the board board becomes test first*/
            set_test_mode();
        }
        else if (cd->specials_mode)
        {
            buff[1] |= 0x20;
        }
        else if (cd->development_mode)
        {
            buff[1] |= 0x40;
        }
        else if (cd->test_mode)
        {
            buff[1] |= 0x40;
            /*set test mode bit*/
            set_test_mode();

        }


        if (cd->boot_license_disabled)
            buff[1] |= 0x10;

        buff[2] = (((cd->osc & 0x3) << 6) & 0xC0) | (((cd->variant & 0x3FF) >> 4) & 0x3F);
        buff[3] = (((cd->variant & 0xF) << 4) & 0xF0); /* remaining four bits are reserved */

        buff[4] = (((cd->pcbai & 0xFF00) >> 8) & 0xFF);
        buff[5] = (cd->pcbai & 0xFF);

        buff[8] = cd->oui[0];
        buff[9] = cd->oui[1];
        buff[10] = cd->oui[2];
        buff[11] = (cd->serial & 0x300000000) >> 32;
        buff[12] = (cd->serial & 0x0FF000000) >> 24;
        buff[13] = (cd->serial & 0x000FF0000) >> 16;
        buff[14] = (cd->serial & 0x00000FF00) >> 8;
        buff[15] = (cd->serial & 0x0000000FF);


        efuse_write_instance(AP_CHARACTERISATION_EFUSE_INSTANCE,buff);

        memset(buff,0,16);
        buff[0] = cd->eth0addr[0];
        buff[1] = cd->eth0addr[1];
        buff[2] = cd->eth0addr[2];
        buff[3] = cd->eth0addr[3];
        buff[4] = cd->eth0addr[4];
        buff[5] = cd->eth0addr[5];
        buff[6] = cd->eth1addr[0];
        buff[7] = cd->eth1addr[1];
        buff[8] = cd->eth1addr[2];
        buff[9] = cd->eth1addr[3];
        buff[10] = cd->eth1addr[4];
        buff[11] = cd->eth1addr[5];


        efuse_write_instance(MAC_ADDR_EFUSE_INSTANCE,buff);


        memset(buff,0,16);
        /*set mac address instance to last time program*/
        buff[byte_index_from_instance_num(MAC_ADDR_EFUSE_INSTANCE)] |= 1 << bit_shift_from_instance_num(MAC_ADDR_EFUSE_INSTANCE);


        efuse_write_instance(LAST_TIME_PROG_EFUSE_INSTANCE,buff);
	printf("\nSuccess: fuse characterisation done\n");
    }
    else
    {
        printf("\nInvalid operation: board already fuse characterised\n");
        
    }

}

static int usage(const char * progname, int ret)
{
    fprintf((ret ? stderr : stdout), "Usage: %s EEPROM -o OUI -s SERIAL_NUMBER -0 ETH0ADDR -1 ETH1ADDR -t TYPE -p PCB_ASSEMBLY_ISSUE\n", progname);
    fprintf((ret ? stderr : stdout), "Usage: %s FUSE -o OUI -s SERIAL_NUMBER -0 ETH0ADDR -1 ETH1ADDR -t TYPE -p PCB_ASSEMBLY_ISSUE\n", progname);
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
    fprintf((ret ? stderr : stdout), "       -m MODE              : Specify the board mode (p, t, d, s) = (production, test, development, specials).\n");
    fprintf((ret ? stderr : stdout), "       -f                   : Force acceptance of a serial number that fails the DAMM algorithm check.\n");
    fprintf((ret ? stderr : stdout), "       -no-prompt           : Do not prompt user for input before blowing the fuses (must be used when fuse blowing is done via scripts).\n");
    fprintf((ret ? stderr : stdout), "       -dis-boot-license    : Disables boot license feature for this board\n");
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
    int prod_mode;
    int test_mode;
    int dev_mode;
    int specials_mode;
    int ignore_damm;
    int prompt_to_user;
    struct characterisation_data_t cd;
    uint8_t serialised[CONFIG_CHARACTERISATION_IPAT2K_SIZE];

    ret = 1;
    i = 0;
    oui_text = serial_text = eth0addr_text = eth1addr_text = type_text = pcb_assembly_issue_text = NULL;
    prod_mode = test_mode = dev_mode = specials_mode = 0;
    ignore_damm = 0;
    prompt_to_user = 1;
    memset(&cd, 0, sizeof(cd));
    memset(serialised, 0, CONFIG_CHARACTERISATION_IPAT2K_SIZE);

    while(i < argc)
    {
        if (strcmp(argv[i],"-f") && strcmp(argv[i],"-no-prompt") && strcmp(argv[i],"-dis-boot-license"))
        {
            if ( argc < (i+1) )
            {
                /*check for Array overrun in all other options*/
                ret = usage(argv[0], 1);
                goto cleanup;
            }

        }

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
        else if (0 == strcmp(argv[i],"-m"))
        {
            if (prod_mode || test_mode || dev_mode || specials_mode)
            {
                ret = usage(argv[0], 1);
                goto cleanup;
            }

            switch (argv[i+1][0])
            {
                case 'p':
                {
                    prod_mode = 1;
                    break;
                }
                case 't':
                {
                    test_mode = 1;
                    break;
                }
                case 'd':
                {
                    dev_mode = 1;
                    break;
                }
                case 's':
                {
                    specials_mode = 1;
                    break;
                }
                default:
                {
                    ret = usage(argv[0], 1);
                    goto cleanup;
                }
            }
        }
        else if (0 == strcmp(argv[i],"-f"))
        {
            ignore_damm = 1;
        }
        else if (0 == strcmp(argv[i],"-no-prompt"))
        {
            prompt_to_user = 0;
        }
        else if (0 == strcmp(argv[i],"-dis-boot-license"))
        {
            cd.boot_license_disabled = 1;
        }

        ++i;
    }

    if (!prod_mode && !test_mode && !dev_mode && !specials_mode)
    {
        ret = usage(argv[0], 1);
        goto cleanup;
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

    cd.production_mode = prod_mode ? 1 : 0;
    cd.test_mode = test_mode ? 1 : 0;
    cd.development_mode = dev_mode ? 1 : 0;
    cd.specials_mode = specials_mode ? 1 : 0;

    fprintf(stdout, "%s\n", "Hardware Characterisation");
    fprintf(stdout, "%s\n", "=========================");
    fprintf(stdout, "  Equipment Identifier: %02X%02X%02X-%010llu\n", cd.oui[0], cd.oui[1], cd.oui[2], cd.serial);
    fprintf(stdout, "Ethernet MAC Address 0: %02X:%02X:%02X:%02X:%02X:%02X\n", cd.eth0addr[0], cd.eth0addr[1], cd.eth0addr[2], cd.eth0addr[3], cd.eth0addr[4], cd.eth0addr[5]);
    fprintf(stdout, "Ethernet MAC Address 1: %02X:%02X:%02X:%02X:%02X:%02X\n", cd.eth1addr[0], cd.eth1addr[1], cd.eth1addr[2], cd.eth1addr[3], cd.eth1addr[4], cd.eth1addr[5]);
    fprintf(stdout, "      Hardware Variant: %s\n", lookup_variant(cd.variant)->full);
    fprintf(stdout, "            Oscillator: %s\n", lookup_oscillator(cd.osc));
    fprintf(stdout, "    PCB Assembly Issue: %05u\n", cd.pcbai);
    fprintf(stdout, "        Operating Mode: ");

    if (cd.production_mode)
    {
        fprintf(stdout, "Production");
    }
    else if (cd.test_mode)
    {
        fprintf(stdout, "Test");
    }
    else if (cd.development_mode)
    {
        fprintf(stdout, "Development");
    }
    else
    {
        fprintf(stdout, "Specials");
    }

    fprintf(stdout, "%s\n", "\n");


    if (0 == strcmp(argv[1],"EEPROM"))
    {

        if ( 0 == (ret = user_input(prompt_to_user, "EEPROM")) )
        {
            cd.version = CONFIG_CHARACTERISATION_IPAT2K_VERSION;
            serialise_characterisation_info_eeprom(&cd, serialised);

            /* write the EEPROM in page mode (256 bytes at a time) */
            for (i = 0; i < CONFIG_CHARACTERISATION_IPAT2K_SIZE; i += 256)
            {
                if (0 != (ret = i2c_write(CONFIG_CHARACTERISATION_EEPROM_ADDR, CONFIG_CHARACTERISATION_IPAT2K_OFFSET + i, 2, serialised + i, 256)))
                {
                    printf("i2c_write returned %d for 8 bytes at offset %d\n", ret, CONFIG_CHARACTERISATION_IPAT2K_OFFSET + i);
                    goto cleanup;
                }

                udelay(5000);
            }
        }
    }
    else if (0 == strcmp(argv[1],"FUSE"))
    {
        if ( 0 == (ret = user_input(prompt_to_user, "FUSE")) )
            characterise_fuses(&cd);
    }
    else
    {
        ret = usage(argv[0], 1);
    }

cleanup:
    return ret;
}



U_BOOT_CMD(
        characterise_hw, 18, 0, do_characterise,
        "Hw characterisation command to characterise board in eeprom/fuses",
        "<characterise_hw> <args>"
        );
#endif

int do_set_provisioning_req_eeprom(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{

    int ret;
    uint8_t val = 0x05;
    if (0 != (ret = i2c_write(CONFIG_CHARACTERISATION_EEPROM_ADDR, CONFIG_CHARACTERISATION_IPAT2K_OFFSET + 226, 2, &val, 1)))
    {
        printf("i2c_write returned %d: set_provisioning_required failed\n", ret);
        return CMD_RET_FAILURE;
    }
    return CMD_RET_SUCCESS;

}


U_BOOT_CMD(
        set_provisioning_required_eeprom, 1, 0, do_set_provisioning_req_eeprom,
        "set provisioning required in provisioning status byte in eeprom",
        "<set_provisioning_required>"
        );

int do_test_mode_flag_eeprom(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{

    if (argc < 2)
        return CMD_RET_USAGE;

    if ( 0 == strcmp(argv[1],"set") )
        set_test_mode();
    else if (0 == strcmp(argv[1],"clear") )
        clear_test_mode();
    else
        return CMD_RET_USAGE;

    return CMD_RET_SUCCESS;

}


U_BOOT_CMD(
        test_mode_flag, 2, 0, do_test_mode_flag_eeprom,
        "test_mode_flag to switch the board between Dev/test mode",
        "<test_mode_flag <set|clear> >"
        );
#endif
