/********************************************************************
---------------------------------------------------------------------
 Copyright (c) 2020 ipaccess, Inc.
 All Rights Reserved.
----------------------------------------------------------------------
*********************************************************************/
#include <common.h>
#include <malloc.h>
#include <linux/compiler.h>
#include <errno.h>

#include "ipafsm90xx_fuse.h"
#include "characterisation.h"
#include "ike_api.h"

#if defined(CONFIG_CHARACTERISATION)

/*RSM bitmap in mmc : r:reserved p:production d:development s:specials t:test*/
/*p|d|s|r| r|r|r|t*/
#define BOARD_RSM_MASK_MMC 0xE1

/*p|d|s|r| r|r|r|r*/
#define BOARD_RSM_MASK_FUSE 0x0E
#define BOARD_RSM_MASK_PROD 0x08
#define BOARD_RSM_MASK_DEV  0x04
#define BOARD_RSM_MASK_SPEC 0x02


/*mmc partition charaterisation data buffer*/
uint8_t serialised_characterisation_data[CONFIG_CHARACTERISATION_SIZE];

static const char * oscillator_text[] = {
    "030", /* OCXO  */
    "015", /* VCTXO */
    "000", /* MEMS  */
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



static int do_hwchar_mmc_read(void);
static int do_hwchar_mmc_write(uint8_t offset,uint8_t * payload, uint8_t size);
void print_characterisation(void);
static int is_test_bit_set(void);

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
    { "470Z", "470", "Z", 470, 'Z' }, /* Bands: 1-48             - Notes: S60 unbanded FDD and TDD                                        */
    { "499_", "499", "_", 499, '_' }, /* Bands: No radio present - Notes: S60 Digital FDD and TDD                                         */
    { "495X", "495", "X", 495, 'X' }, /* Band:  48               - Notes: S60 Digital lvds + band 48 FEM  aka E61                         */
    { "495T", "495", "T", 495, 'T' }, /* Band:  40               - Notes: S60 Digital lvds + band 40 FEM                                  */
    { "496Y", "496", "Y", 496, 'Y' }, /* Band:  1 & 3            - Notes: S60 Digital lvds + band 1 & 3 FEM aka Aero                      */
    { "492R", "492", "R", 492, 'R' }, /* Band:  2                - Notes: S60 Digital lvds + band 2 benetel aka R60                       */
    { "495Y", "495", "Y", 495, 'Y' }, /* Band:  1 & 3            - Notes: 495 Radio or S60 Digital lvds + band 1 & 3 fem E61              */
    { "503S", "503", "S", 503, 'S' }, /* Band:  3                - Notes: C60                                                             */
    { "503V", "503", "V", 503, 'V' }, /* Band:  7                - Notes: C60                                                             */
    { "502A", "502", "A", 502, 'A' }, /* 41: Band:  1            - Notes: S8v2 Picochip + ADI 13dBm band 1                                */
    { "502N", "502", "N", 502, 'N' }, /* 42: Band:  8            - Notes: S8v2 Picochip + ADI 13dBm band 8                                */
    { "505A", "505", "A", 505, 'A' }, /* 43: Band:  1            - Notes: S8v2 Picochip + ADI 20dBm band 1                                */
    { "505N", "505", "N", 505, 'N' }, /* 44: Band:  8            - Notes: S8v2 Picochip + ADI 20dBm band 8                                */
    { "509S", "509", "S", 509, 'S' }, /* 45: Band:  3            - Notes: C70 sercom Indiagate based                                      */

};

static const int num_variants = sizeof(variant_lookup) / sizeof(variant_lookup[0]);



static void set_ipafsm90xx_ethernet_mac_addresses(void);


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

static int get_variant_index(const char * full_variant )
{
    int ret = -1;
    int i;
    for ( i=1; i < num_variants; i++)
    {
        if (0 == strcmp(full_variant,variant_lookup[i].full))
        {
            ret = i;
            break;
        }
    }
    return ret;
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
    uint8_t boot_license_disabled;
    uint8_t app_rev_count;
    uint8_t loader_rev_count;
    uint64_t parent_serial;
    uint8_t hwsw_compat;
    uint16_t bb_variant;
};

struct radio_characterisation_data_t
{
    uint8_t version;
    uint64_t serial; 
    uint16_t variant;
    uint16_t pcbai;
};

struct characterisation_data_t cdo;

void read_characterisation_from_fuses(struct characterisation_data_t * cd)
{
    u32 payload = 0;
    unsigned int offset = 0;
    unsigned int len = 0;

    cd->production_mode = 1;
    cd->test_mode = 0;
    cd->development_mode = 0;
    cd->specials_mode = 0;

    /*PK hash row 0 LSB*/
    offset = QFPROM_CORR_FUSE_PK_HASH_ROW0_LSB;
    len = 4;
    read_fuse_in_range(offset, &payload , &len);
 	
    cd->osc = ((payload & 0x000000C0) >> 6) & 0x3;
    cd->variant = payload & 0x0000003F;
    cd->variant <<= 4;
    cd->variant |= (((payload & 0x0000F000) >> 12) & 0xF);
    
    cd->pcbai = (uint16_t)(((payload & 0x00FF0000) >> 16) & 0x00FF);
    cd->pcbai <<= 8;
    cd->pcbai |= (uint16_t)(((payload & 0xFF000000) >> 24) & 0xFFFF); 

    payload = 0;

    /*PK hash row 0 MSB*/
    offset = QFPROM_CORR_FUSE_PK_HASH_ROW0_LSB + 0x04;
    len = 4;
    read_fuse_in_range(offset, &payload , &len);
    
    cd->oui[0] =   payload & 0x000000FF ;
    cd->oui[1] = ((payload & 0x0000FF00) >> 8 ) & 0x000000FF;
    cd->oui[2] = ((payload & 0x00FF0000) >> 16) & 0x000000FF;

    payload = 0;

    /*PK hash row 1 LSB*/
    offset = QFPROM_CORR_FUSE_PK_HASH_ROW1_LSB;
    len = 4;
    read_fuse_in_range(offset, &payload, &len);
    
    cd->serial = payload; 
    
    payload = 0;

    /*PK hash row 1 MSB*/
    offset = QFPROM_CORR_FUSE_PK_HASH_ROW1_LSB + 0x04;
    len = 4;
    read_fuse_in_range(offset, &payload, &len);

    cd->serial |= (((uint64_t)payload) << 32) & 0x0300000000; 

    payload = 0;

    /*PK hash row 2 LSB*/
    offset = QFPROM_CORR_FUSE_PK_HASH_ROW2_LSB;
    len = 4;
    read_fuse_in_range(offset, &payload , &len);
    cd->eth0addr[0]= payload & 0x000000FF;
    cd->eth0addr[1]= ((payload & 0x0000FF00 ) >> 8 ) & 0xFF;
    cd->eth0addr[2]= ((payload & 0x00FF0000 ) >> 16) & 0xFF;
    cd->eth0addr[3]= ((payload & 0xFF000000 ) >> 24) & 0xFF;
    payload = 0;

    /*PK hash row 2 MSB*/
    offset = QFPROM_CORR_FUSE_PK_HASH_ROW2_LSB + 0x04;
    len = 4;
    read_fuse_in_range(offset, &payload , &len);
    cd->eth0addr[4]= payload & 0x000000FF;
    cd->eth0addr[5]= ((payload & 0x0000FF00) >> 8) & 0xFF;

    payload = 0;

    /*PK hash row 3 LSB*/
    offset = QFPROM_CORR_FUSE_PK_HASH_ROW3_LSB;
    len = 4;
    read_fuse_in_range(offset, &payload , &len);
    cd->eth1addr[0]= payload & 0x000000FF;
    cd->eth1addr[1]= ((payload & 0x0000FF00 ) >> 8 ) & 0xFF;
    cd->eth1addr[2]= ((payload & 0x00FF0000 ) >> 16) & 0xFF;
    cd->eth1addr[3]= ((payload & 0xFF000000 ) >> 24) & 0xFF;

    payload = 0;

    /*PK hash row 3 MSB*/
    offset = QFPROM_CORR_FUSE_PK_HASH_ROW3_LSB + 0x04;
    len = 4;
    read_fuse_in_range(offset, &payload , &len);
    cd->eth1addr[4]= payload & 0x000000FF;
    cd->eth1addr[5]= ((payload & 0x0000FF00) >> 8) & 0xFF;
    payload = 0;

    offset = QFPROM_CORR_FUSE_IMEI_ESN2_LSB;
    len = 4;
    read_fuse_in_range(offset, &payload , &len);
    
    if ( (payload & BOARD_RSM_MASK_PROD) != 0 )
    {    
        cd->production_mode = 1; 
    }    
    else if ( (payload & BOARD_RSM_MASK_DEV ) != 0)
    {    
        if (is_test_bit_set())
            cd->test_mode = 1; 
        else 
            cd->development_mode = 1; 
    }    
    else if ( (payload & BOARD_RSM_MASK_SPEC) != 0 )
    {    
        cd->specials_mode = 1; 
    }    
    else /*No mode/bit set */
    {    
        //check for secure boot enabled
        if(is_secure_boot())
        {
            cd->production_mode = 1; 
        }
        else
        {    
            cd->development_mode = 1; 
        }    
    }    

    return;
}


static int do_hwchar_mmc_write(uint8_t offset,uint8_t * payload, uint8_t size)
{
    return -1;
#if 0
    block_dev_desc_t *mmc_dev;
    struct mmc *mmc;
    disk_partition_t info;
    uint8_t value;
    uint32_t blk, cnt, n;
    memset(&info,0,sizeof(disk_partition_t));

    /*Set the fs0/fs1 userdata partition number environment variable, will be used later*/
    mmc_dev = mmc_get_dev(0);
    if (mmc_dev != NULL && mmc_dev->type != DEV_TYPE_UNKNOWN) {

        if (0 == get_partition_info_by_name(mmc_dev, "chr", &info))
            snprintf(value,sizeof(value),"0x%x",info.part_num);
        else
            snprintf(value,sizeof(value),"0x%x",29); //default value
    }   
    mmc = init_mmc_device(value, false);
    if (!mmc)
    {    
        return -1;
    }    
   
    blk = info.start;
    cnt = 1;
    n = mmc->block_dev.block_write(value, blk, cnt, payload);
    printf("%d blocks read: %s\n", n, (n == cnt) ? "OK" : "ERROR");
    return 0;
#endif
}

static int do_hwchar_mmc_read()
{
    block_dev_desc_t *mmc_dev;
    struct mmc *mmc;
    disk_partition_t info;
    uint8_t value;
    uint32_t blk, n;
    int dev_num = 0;
    uint8_t *buff = NULL;
    /*Default to read one block*/
    uint32_t cnt = 1;

    memset(&info,0,sizeof(disk_partition_t));

    /*Set the fs0/fs1 userdata partition number environment variable, will be used later*/
    mmc_dev = mmc_get_dev(dev_num);
    if (mmc_dev != NULL && mmc_dev->type != DEV_TYPE_UNKNOWN) {

        if (0 != get_partition_info_by_name(mmc_dev, CONFIG_CHARACTERISATION_MMC_PART_NAME, &info))
        {
            printf("Characterisation Partition: %s doesn't exist\n",CONFIG_CHARACTERISATION_MMC_PART_NAME);
            return -1;
        }
    }
    
    /*Blk size smaller than charcterisation size, calculate number of blocks to read*/ 
    if (CONFIG_CHARACTERISATION_SIZE > mmc_dev->blksz)
    {
        cnt = (CONFIG_CHARACTERISATION_SIZE/mmc_dev->blksz) + 1;
    }
    else if ( (mmc_dev->blksz > CONFIG_CHARACTERISATION_SIZE) && (!(buff = malloc(mmc_dev->blksz))) ) /* If block size is greater than charaterisation size, then we need to malloc memory*/
    { 
        /*Malloc failed*/
        printf("Malloc failed");
        return -1;
    }
     
    blk = info.start;
    if (buff) /*if malloced buffer needs to be used*/
    {
        n = mmc_dev->block_read(value, blk, cnt,buff);
        memcpy(serialised_characterisation_data,buff,CONFIG_CHARACTERISATION_SIZE);
    }
    else /*Use charactersation data buffer directly*/
    {
        n = mmc_dev->block_read(value, blk, cnt,serialised_characterisation_data);
    }

    /*Free malloc memory if it was allocated*/
    if (buff)
        free(buff);
    buff = NULL;

    if (n != cnt)
    {
       printf("%d blocks read: %s\n", n, (n == cnt) ? "OK" : "ERROR");
       return -1;
    }
    
    return 0;
}


int deserialise_characterisation_info_mmc(struct characterisation_data_t * cd)
{
    int ret = -1;
    uint8_t *payload;

    payload = serialised_characterisation_data;
    /*Check partition is characterised or not*/
    if ( 0 == strncmp((char *)payload,"IPACCESS",8) ) 
    {
        cd->version = payload[8];

        cd->production_mode = 0;
        cd->test_mode = 0;
        cd->development_mode = 0;
        cd->specials_mode = 0;
        
        cd->eth0addr[0] = payload[9];
        cd->eth0addr[1] = payload[10];
        cd->eth0addr[2] = payload[11];
        cd->eth0addr[3] = payload[12];
        cd->eth0addr[4] = payload[13];
        cd->eth0addr[5] = payload[14];
        
        cd->eth1addr[0] = payload[15];
        cd->eth1addr[1] = payload[16];
        cd->eth1addr[2] = payload[17];
        cd->eth1addr[3] = payload[18];
        cd->eth1addr[4] = payload[19];
        cd->eth1addr[5] = payload[20];
        
        cd->osc = ((payload[21] & 0xC0) >> 6) & 0x3;
        cd->variant = payload[21] & 0x3F;
        cd->variant <<= 4;
        cd->variant |= (((payload[22] & 0xF0) >> 4) & 0xF);
        
        cd->pcbai = payload[23];
        cd->pcbai <<= 8;
        cd->pcbai |= payload[24];
        
        cd->oui[0] = payload[25];
        cd->oui[1] = payload[26];
        cd->oui[2] = payload[27];
        
        cd->serial = ((((uint64_t)(payload[28])) << 32) & 0x300000000) | 
            ((((uint64_t)(payload[29])) << 24) & 0xFF000000) |
            ((((uint64_t)(payload[30])) << 16) & 0x00FF0000) |
            ((((uint64_t)(payload[31])) <<  8) & 0x0000FF00) |
            ((((uint64_t)(payload[32])) <<  0) & 0x000000FF);
        
        cd->parent_serial = ((((uint64_t)(payload[33])) << 32) & 0x300000000) |
            ((((uint64_t)(payload[34])) << 24) & 0xFF000000) |
            ((((uint64_t)(payload[35])) << 16) & 0x00FF0000) |
            ((((uint64_t)(payload[36])) <<  8) & 0x0000FF00) |
            ((((uint64_t)(payload[37])) <<  0) & 0x000000FF);

        cd->hwsw_compat = payload[38];
        
        if ( (payload[239] & BOARD_RSM_MASK_MMC) == 0x80)
            cd->production_mode = 1;
        else if ((payload[239] & BOARD_RSM_MASK_MMC) == 0x40)
            cd->development_mode = 1;
        else if ( (payload[239] & BOARD_RSM_MASK_MMC) == 0x20)
            cd->specials_mode = 1;
        else if ( (payload[239] & BOARD_RSM_MASK_MMC) == 0x01)
            cd->test_mode = 1;
        else 
            cd->development_mode = 1;
        ret = 0;
    }

    return ret;

}

static int is_test_bit_set(void)
{
    uint8_t *payload = serialised_characterisation_data ;

    /*read MMC if mmc charaterisation data is not already read*/
    if ( 0 != strncmp((char *)payload,"IPACCESS",8) )
        do_hwchar_mmc_read();

    /*Check again if we have the mmc characterisation buffer now*/
    if ( 0 == strncmp((char *)payload,"IPACCESS",8) )
        return ((payload[239] & BOARD_RSM_MASK_MMC) == 0x01)?1:0;

    return 0; /*Default return, test mode bit not set*/
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

int is_secure_boot(void)
{
    int val = 0;
    val =  read_secure_boot_fuse();
    return ((val & 0x20)?1:0);
}

static void update_rsm_from_tz(struct characterisation_data_t * cdo)
{

    switch (toeIkeRsm())
    {
        case BL_RSM_PROD:
            {
                cdo->production_mode = cdo->test_mode = cdo->development_mode = cdo->specials_mode = 0;
                cdo->production_mode = 1;
                break;
            }
        case BL_RSM_DEV:
            {
                cdo->production_mode = cdo->test_mode = cdo->development_mode = cdo->specials_mode = 0;
                if (is_test_bit_set())
                    cdo->test_mode = 1;
                else
                    cdo->development_mode = 1;
                break;
            }
        case BL_RSM_SPECIALS:
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
    if(1 == (ret = ipafsm90xx_is_board_fused()))
    {    
        read_characterisation_from_fuses(&cdo);

    }
    else
    {
        /*Try Reading and parsing characterisation info from mmc*/
        if ( (0 != do_hwchar_mmc_read()) || ( 0 != deserialise_characterisation_info_mmc(&cdo)) )
        {
            /* Since this board is not fused and mmc characterisation read or parsing has also failed, It's safe to realise this as dev board*/
            cdo.production_mode = 0;
            cdo.test_mode = 0;
            cdo.development_mode = 1;
            cdo.specials_mode = 0;
        }
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
        setenv("bb_variant_part", lookup_variant(cdo.bb_variant)->part);
        set_ipafsm90xx_ethernet_mac_addresses();
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

        printf("Fused: %s\n", (ipafsm90xx_is_board_fused() ? "Yes" : "No"));
        if (cdo.parent_serial)
            printf("Parent EID:   %02X%02X%02X-%010llu\n", cdo.oui[0], cdo.oui[1], cdo.oui[2], cdo.parent_serial);
    
    }
    else
    {
        printf("Board: %s\n", "Not Characterised");
    }
}


static void set_ipafsm90xx_ethernet_mac_addresses(void)
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


/*Characterisation via u-boot is not implemented yet, so this flag isn't enabled yet.*/
#ifdef CONFIG_CMD_CHARACTERISE_HW
void serialise_characterisation_info_mmc(const struct characterisation_data_t * cd, uint8_t payload[CONFIG_CHARACTERISATION_SIZE])
{
    memset(payload, 0, CONFIG_CHARACTERISATION_SIZE);

    strncpy((char *)payload,"IPACCESS",8);
    
    payload[8] = cd->version;

    payload[ 9] = cd->eth0addr[0];
    payload[10] = cd->eth0addr[1];
    payload[11] = cd->eth0addr[2];
    payload[12] = cd->eth0addr[3];
    payload[13] = cd->eth0addr[4];
    payload[14] = cd->eth0addr[5];
             
    payload[15] = cd->eth1addr[0];
    payload[16] = cd->eth1addr[1];
    payload[17] = cd->eth1addr[2];
    payload[18] = cd->eth1addr[3];
    payload[19] = cd->eth1addr[4];
    payload[20] = cd->eth1addr[5];

    payload[21] = (((cd->osc & 0x3) << 6) & 0xC0) | (((cd->variant & 0x3FF) >> 4) & 0x3F);
    payload[22] = (((cd->variant & 0xF) << 4) & 0xF0); /* remaining four bits are reserved */

    payload[23] = (((cd->pcbai & 0xFF00) >> 8) & 0xFF);
    payload[24] = (cd->pcbai & 0xFF);

    payload[25] = cd->oui[0];
    payload[26] = cd->oui[1];
    payload[27] = cd->oui[2];
    payload[28] = (((cd->serial & 0x300000000) >> 32) & 0xFF);
    payload[29] = (((cd->serial & 0xFF000000) >> 24) & 0xFF);
    payload[30] = (((cd->serial & 0x00FF0000) >> 16) & 0xFF);
    payload[31] = (((cd->serial & 0x0000FF00) >>  8) & 0xFF);
    payload[32] = (((cd->serial & 0x000000FF) >>  0) & 0xFF);
    payload[33] = (((cd->parent_serial & 0x300000000) >> 32) & 0xFF);
    payload[34] = (((cd->parent_serial & 0xFF000000) >> 24) & 0xFF);
    payload[35] = (((cd->parent_serial & 0x00FF0000) >> 16) & 0xFF);
    payload[36] = (((cd->parent_serial & 0x0000FF00) >>  8) & 0xFF);
    payload[37] = (((cd->parent_serial & 0x000000FF) >>  0) & 0xFF);
    payload[38] = cd->hwsw_compat;

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

}

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

static int parse_revocation_count(const char * revocation_count_text, uint8_t *revocation_count )
{

    uint16_t v;

    v = 0;

    if (!revocation_count_text || !revocation_count)
    {
        fprintf(stderr, "%s\n", "Invalid app/loader revocation count type: NULL argument to conversion function.");
        return -1;
    }

    if (strlen(revocation_count_text) < 3)
    {
        fprintf(stderr, "%s\n", "Invalid app/loader revocation count type: Input too short.");
        return -1;
    }

    if (revocation_count_text[0] != '0' || !(revocation_count_text[1] == 'x' || revocation_count_text[1] == 'X'))
    {
        fprintf(stderr, "%s\n", "Invalid app/loader revocation count: Invalid prefix (must be 0x or 0X).");
        return -1;
    }

    if (0 != parse_hex_uint16(revocation_count_text + 2, &v))
    {
        fprintf(stderr, "%s\n", "Invalid app/loader revocation count: failed to parse input.");
        return -1;
    }

    if (v > MAX_REVOCATION_COUNT_VALUE)
    {
        fprintf(stderr, "%s\n", "Invalid app/loader revocation count: Value above Max revocation count value.");
        return -1;
    }
    
    *revocation_count = (uint8_t)(v & 0xFF);
    return 0;
}


static int parse_hwsw_compat(const char * compat_text, uint8_t * compat_number)
{
    char * ep;
    unsigned long ulval;

    if (!compat_text || !compat_number)
    {
        fprintf(stderr, "%s\n", "Invalid hwsw compatibility number: NULL argument to conversion function.");
        return -1;
    }

    if (strlen(compat_text) > 3)
    {
        fprintf(stderr, "%s\n", "Invalid hwsw compatibility number: more than 3 digits in length.");
        return -1;
    }

    errno = 0;
    ulval = simple_strtoul(compat_text, &ep, 10);

    if (ep == compat_text || *ep != '\0')
    {
        fprintf(stderr, "%s\n", "Invalid hwsw compatibility number: not a decimal number.");
        return -1;
    }


    if ( (ulval == 0)||(ulval > 254) )
    {
        fprintf(stderr, "%s\n", "Invalid hwsw compatibility number: valid range is 1-254.");
        return -1;
    }

    *compat_number = (uint8_t)ulval;
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

int characterise_fuses (const struct characterisation_data_t * cd)
{
    printf("\nInvalid operation: Not implemented yet\n");
    return 1;
}

static int usage(const char * progname, int ret)
{
    fprintf((ret ? stderr : stdout), "Usage: %s EEPROM -o OUI -s SERIAL_NUMBER -0 ETH0ADDR -1 ETH1ADDR -t TYPE -p PCB_ASSEMBLY_ISSUE\n", progname);
    fprintf((ret ? stderr : stdout), "Usage: %s FUSE -o OUI -s SERIAL_NUMBER -0 ETH0ADDR -1 ETH1ADDR -t TYPE -p PCB_ASSEMBLY_ISSUE\n", progname);
    fprintf((ret ? stderr : stdout), "       -o OUI                         : Specify the OUI (part of the EID) of this board.\n");
    fprintf((ret ? stderr : stdout), "                                        This is specified as three hexadecimal octets.\n");
    fprintf((ret ? stderr : stdout), "       -s SERIAL_NUMBER               : Specify the serial number (part of the EID, with DAMM digit) of this board.\n");
    fprintf((ret ? stderr : stdout), "                                        This is specified as a 10 digit decimal number made up of the 9\n");
    fprintf((ret ? stderr : stdout), "                                        serial number digits and 1 DAMM check digit.\n");
    fprintf((ret ? stderr : stdout), "       -0 ETH0ADDR                    : Specify the first Ethernet MAC address.\n");
    fprintf((ret ? stderr : stdout), "                                        An Ethernet address is specified as six colon separated hexadecimal octets.\n");
    fprintf((ret ? stderr : stdout), "                                        Octets are left padded to two digits with zeroes.\n");
    fprintf((ret ? stderr : stdout), "                                        For example: CA:FE:BA:BE:B0:0C.\n");
    fprintf((ret ? stderr : stdout), "       -1 ETH1ADDR                    : Specify the second Ethernet MAC address.\n");
    fprintf((ret ? stderr : stdout), "       -t TYPE                        : Specify the board type as a 16 bit decimal value (0x0000-0xFFFF).\n");
    fprintf((ret ? stderr : stdout), "                                        The TYPE argument must correspond to a known board hardware type and oscillator value.\n");
    fprintf((ret ? stderr : stdout), "                                        Unused bits must be set to 0.\n");
    fprintf((ret ? stderr : stdout), "       -p PCB_ASSEMBLY_ISSUE          : Specify the PCB assembly issue as a 16 bit decimal value (0x0000-0xFFFF).\n");
    fprintf((ret ? stderr : stdout), "       -m MODE                        : Specify the board mode (p, t, d, s) = (production, test, development, specials).\n");
    fprintf((ret ? stderr : stdout), "       -a APP_REV_COUNT               : Optional(FUSES only) Specify application revocation count as hexadecimal number (0x00-0x40) \n");
    fprintf((ret ? stderr : stdout), "       -l LOADER_REV_COUNT            : Optional(FUSES only) Specify loader revocation count hexadecimal value(0x00-0x40)\n");
    fprintf((ret ? stderr : stdout), "       -f                             : Force acceptance of a serial number that fails the DAMM algorithm check.\n");
    fprintf((ret ? stderr : stdout), "       -no-prompt                     : Do not prompt user for input before blowing the fuses (must be used when fuse blowing is done via scripts).\n");
    fprintf((ret ? stderr : stdout), "       -dis-boot-license              : Disables boot license feature for this board\n");
    fprintf((ret ? stderr : stdout), "       -S PARENT_SERIAL_NUMBER        : Optional(EEPROM only) Specify the serial number (part of the EID, with DAMM digit) of it's parent assembly.\n");
    fprintf((ret ? stderr : stdout), "                                        This is specified as a 10 digit decimal number made up of the 9\n");
    fprintf((ret ? stderr : stdout), "                                        serial number digits and 1 DAMM check digit. Defaults to 0000000000 if not provided\n");
    fprintf((ret ? stderr : stdout), "       -c HWSW_COMPATIBILITY_NUMBER   : Optional(EEPROM only) Specify the hw sw compatibility number as 8 bit decimal value (1-254). Defaults to 1 if not provided.\n");
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
    const char * app_revocation_count_text;
    const char * loader_revocation_count_text;
    const char * parent_serial_text;
    const char * hwsw_compat_text;
    int prod_mode;
    int test_mode;
    int dev_mode;
    int specials_mode;
    int ignore_damm;
    int prompt_to_user;
    struct characterisation_data_t cd;
    uint8_t serialised[CONFIG_CHARACTERISATION_SIZE];

    ret = 1;
    i = 0;
    oui_text = serial_text = eth0addr_text = eth1addr_text = type_text = pcb_assembly_issue_text = app_revocation_count_text = loader_revocation_count_text=NULL;
    parent_serial_text=hwsw_compat_text=NULL;
    prod_mode = test_mode = dev_mode = specials_mode = 0;
    ignore_damm = 0;
    prompt_to_user = 1;
    memset(&cd, 0, sizeof(cd));
    /*hwsw_compat is optional arg, if not passed it defaults to 1*/
    cd.hwsw_compat= 0x01;
    memset(serialised, 0, CONFIG_CHARACTERISATION_SIZE);

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
        else if (0 == strcmp(argv[i],"-l"))
        {
            loader_revocation_count_text = argv[i+1];
        }
        else if (0 == strcmp(argv[i],"-a"))
        {
            app_revocation_count_text = argv[i+1];
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
        else if (0 == strcmp(argv[i],"-S"))
        {
            parent_serial_text = argv[i+1];
        }
        else if (0 == strcmp(argv[i],"-c"))
        {
            hwsw_compat_text=argv[i+1];
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
    /*All mandatory parameters for both Fuses/EEPROM characterisation*/
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
    fprintf(stdout, "\n");



    if (0 == strcmp(argv[1],"MMC"))
    {
        /*EEPROM: optionals parse here*/
        if ((parent_serial_text && (0 != parse_serial_number(parent_serial_text, &cd.parent_serial, ignore_damm))) ||
           (hwsw_compat_text && (0 != parse_hwsw_compat(hwsw_compat_text, &cd.hwsw_compat))))
        {
            ret = usage(argv[0], 1);
            goto cleanup;

        }
         
        if(cd.parent_serial)
            fprintf(stdout, "            Parent EID: %02X%02X%02X-%010llu\n", cd.oui[0], cd.oui[1], cd.oui[2], cd.parent_serial);
        else
            fprintf(stdout, "            Parent EID: 000000-0000000000\n");    

        fprintf(stdout, "    HWSW Compatibility Number: %u\n", cd.hwsw_compat);       

        if ( 0 == (ret = user_input(prompt_to_user, "EEPROM")) )
        {
            cd.version = CONFIG_CHARACTERISATION_VERSION;
            serialise_characterisation_info_mmc(&cd, serialised);

            /* write the EEPROM in page mode (256 bytes at a time) */
            for (i = 0; i < CONFIG_CHARACTERISATION_SIZE; i += 256)
            {
                if (0 != (ret = do_hwchar_mmc_write(CONFIG_CHARACTERISATION_OFFSET,serialised, 256)))
                {
                    printf("mmc_write returned %d for 8 bytes at offset %d\n", ret, CONFIG_CHARACTERISATION_OFFSET + i);
                    printf("Failure: MMC characterisation failed\n");
                    goto cleanup;
                }

                udelay(5000);
            }
            /*Do not change the text in following print, MTS expects this exact print*/
            printf("Success: MMC characterisation done\n");
            ret = 0;
        }
    }
    else if (0 == strcmp(argv[1],"FUSE"))
    {
        /*Fuses optional parameters: parse if passed*/
        if ( (loader_revocation_count_text && (0 != parse_revocation_count(loader_revocation_count_text, &cd.loader_rev_count ))) ||
             (app_revocation_count_text && (0 != parse_revocation_count(app_revocation_count_text, &cd.app_rev_count ))) )
        {
            ret = usage(argv[0], 1);
            goto cleanup;
        }
        if (loader_revocation_count_text)
            fprintf(stdout, "      Loader rev count: %d\n",cd.loader_rev_count);

        if (app_revocation_count_text)
            fprintf(stdout, " Application rev count: %d\n",cd.app_rev_count);


        if ( 0 == (ret = user_input(prompt_to_user, "FUSE")) )
            ret = characterise_fuses(&cd); 
    }
    else
    {
        ret = usage(argv[0], 1);
    }

cleanup:
    return ret;
}


U_BOOT_CMD(
        characterise_hw, 22, 0, do_characterise,
        "Hw characterisation command to characterise board in eeprom/fuses",
        "<characterise_hw> <args>"
        );

#endif

#endif // defined(CONFIG_CHARACTERISATION)
