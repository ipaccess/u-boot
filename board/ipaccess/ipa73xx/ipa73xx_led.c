/*
 * (C) Copyright 2014 ip.access Ltd
*/

/*
 * IPA73XX platform LED control 
 *
 */

#include <common.h>
#include <command.h>
#include <asm/arch/picoxcell.h>
#include <asm/arch/picoxcell_gpio.h>
#include <asm/arch/axi2cfg.h>
#include "ipa73xx_led.h"

/* GPIO control registers */
#define GPIO_MUX_REG_ADDR (PICOXCELL_AXI2CFG_BASE + AXI2CFG_SHD_GPIO_MUXING_REG_OFFSET)
#define PAI_GPIO_REG_ADDR (PICOXCELL_AXI2CFG_BASE + AXI2CFG_USE_PAI_GPIO_REG_OFFSET)
#define EBI_GPIO_REG_ADDR (PICOXCELL_AXI2CFG_BASE + AXI2CFG_USE_EBI_GPIO_REG_OFFSET)

/* Register write & read functions */
#define IOWRITE32(vALUE, aDDRESS)   *((volatile u32 *)(aDDRESS)) = (vALUE)
#define IOREAD32(aDDRESS)           (*((volatile u32 *)(aDDRESS)))

/* Values for GPIO lines */
#define GPIO_HI 1
#define GPIO_LO 0

/*
 *  LED CONFIGURATION LOGIC
 *  For details, see PC-104068-DS-5-PC3x3_Datasheet (Hoyle 2), section 3.6.3
 *  The INDEX values is used for a quick look-up into the gpio_info table below
 */

/* LED_PWR_GREEN  Arm pin 15, ebi_addr[26} -> bit 12 in EBI GPIO Control register */
#define LED_PWR_GREEN_INDEX         0
#define LED_PWR_GREEN_GPIO          PC3X3_GPIO_PIN_ARM_15
#define LED_PWR_GREEN_MUX_MASK      (1 << LED_PWR_GREEN_GPIO)
#define LED_PWR_GREEN_EBI_MASK      (1 << 12)

/* LED_NWK_RED  Arm pin 5, ebi_addr[23} -> bit 9 in EBI GPIO Control register
                Requires disabling of pai_tx_data[1] -> bit 1 in PAI GPIO control */
#define LED_NWK_RED_INDEX           1
#define LED_NWK_RED_GPIO            PC3X3_GPIO_PIN_ARM_5
#define LED_NWK_RED_MUX_MASK        (1 << LED_NWK_RED_GPIO)
#define LED_NWK_RED_PAI_MASK        (1 << 1)
#define LED_NWK_RED_EBI_MASK        (1 << 9)

/* LED_NWK_GREEN  Arm pin 22, ebi_addr[20} -> bit 6 in EBI GPIO Control register
 *                Requires disabling of pai_rx_data[2] -> bit 10 in PAI GPIO control */
#define LED_NWK_GREEN_INDEX         2
#define LED_NWK_GREEN_GPIO          PC3X3_GPIO_PIN_ARM_22
#define LED_NWK_GREEN_MUX_MASK      (1 << LED_NWK_GREEN_GPIO)
#define LED_NWK_GREEN_PAI_MASK      (1 << 10)
#define LED_NWK_GREEN_EBI_MASK      (1 << 6)

/* LED_MODE_YELLOW  Arm pin 33, ebi_addr[15} -> bit 1 in EBI GPIO Control register */
#define LED_MODE_YELLOW_INDEX       3
#define LED_MODE_YELLOW_GPIO        PC3X3_GPIO_PIN_ARM_33
#define LED_MODE_YELLOW_EBI_MASK    (1 << 1)

/* LED_SVC - Red, Green, Blue, GPIO 44, 47, 48, no muxing, EBI or PAI setup required */
#define LED_SVC_RED_INDEX           4
#define LED_SVC_RED_GPIO            PC3X3_GPIO_PIN_ARM_44

#define LED_SVC_BLUE_INDEX          5
#define LED_SVC_BLUE_GPIO           PC3X3_GPIO_PIN_ARM_47

#define LED_SVC_GREEN_INDEX         6
#define LED_SVC_GREEN_GPIO          PC3X3_GPIO_PIN_ARM_48

/* LED_GPS_GREEN  Arm pin 34, ebi_addr[16] -> bit 2 in EBI GPIO Control register */
#define LED_GPS_GREEN_INDEX         7
#define LED_GPS_GREEN_GPIO          PC3X3_GPIO_PIN_ARM_34
#define LED_GPS_GREEN_EBI_MASK      (1 << 2)

/* Use this to limit array access below */
#define NUM_GPIO_LINES              8


/* Mask for setting bits in GPIO muxing register */
#define LED_GPIO_MUX_MASK  (LED_PWR_GREEN_MUX_MASK \
                          + LED_NWK_RED_MUX_MASK   \
                          + LED_NWK_GREEN_MUX_MASK)
                          
/* Mask for clearing bits in PAI GPIO control register */
#define LED_PAI_MASK (~(LED_NWK_RED_PAI_MASK     \
                      + LED_NWK_GREEN_PAI_MASK))

/* Mask for setting bits in EBI GPIO control register */
#define LED_EBI_MASK (LED_PWR_GREEN_EBI_MASK   \
                    + LED_NWK_RED_EBI_MASK     \
                    + LED_NWK_GREEN_EBI_MASK   \
                    + LED_MODE_YELLOW_EBI_MASK \
                    + LED_GPS_GREEN_EBI_MASK)


/* For ledc sequences */
#define MAX_STEPS 6    /* Number of allowed states in a cycle */

/* Min, mxx and default repetitions of a sequence */
#define MIN_REPS 1
#define MAX_REPS 100
#define DEF_REPS 3

/* Min, mxx and default period between steps in a sequence (ms) */
#define MIN_PERIOD 100   
#define MAX_PERIOD 2000
#define DEF_PERIOD 1000


/* Map of GPIO address and states */
typedef struct
{
    const int addr;
    int state;
} ledc_gpio;

/* Table which keeps a record of GPIO states - note that the indexes in the
 * comments correspond to the _INDEX values defined above, so DON'T CHANGE!
 */
static ledc_gpio gpio_info[NUM_GPIO_LINES] = 
{
    {LED_PWR_GREEN_GPIO,   GPIO_LO},  /* 0  (red when off) */
    {LED_NWK_RED_GPIO,     GPIO_LO},  /* 1 */
    {LED_NWK_GREEN_GPIO,   GPIO_LO},  /* 2 */
    {LED_MODE_YELLOW_GPIO, GPIO_LO},  /* 3  (green when off, off when pwr led is off) */
    {LED_SVC_RED_GPIO,     GPIO_LO},  /* 4 */
    {LED_SVC_BLUE_GPIO,    GPIO_LO},  /* 5 */
    {LED_SVC_GREEN_GPIO,   GPIO_LO},  /* 6 */
    {LED_GPS_GREEN_GPIO,   GPIO_LO}   /* 7 */
};

/* Set maximum GPIO lines associated with one physical LED 
 * Only the Service LED actually has this many 
 */
#define GPIO_LINES_PER_LED 3
#define GPIO_NA -1

/* Look up table for GPIO line addresses for each LED */
typedef int led_lines[GPIO_LINES_PER_LED];

static const led_lines led_addr_table[NUM_LEDS] = 
{
    { LED_PWR_GREEN_INDEX,   GPIO_NA,            GPIO_NA             }, /* PWR  */
    { LED_NWK_GREEN_INDEX,   LED_NWK_RED_INDEX,  GPIO_NA             }, /* NWK  */
    { LED_MODE_YELLOW_INDEX, GPIO_NA,            GPIO_NA             }, /* MODE */
    { LED_SVC_GREEN_INDEX,   LED_SVC_RED_INDEX,  LED_SVC_BLUE_INDEX, }, /* SVC  */
    { LED_GPS_GREEN_INDEX,   GPIO_NA,            GPIO_NA             }, /* GPS  */
};

/* MASTER LED LOOK-UP TABLE
 *  This table contains bitmaps indicating the GPIO states required to obtain a given colour for any given LED
 *  Bit 0 indicates the first line state, bit 1 the second and bit 2 the third
 *  If the colour is not available for that LED, the entry is 0xFF.
 */
typedef unsigned char led_settings[NUM_COLOURS];  /* How to get each colour for a given LED */
 
static const led_settings led_lookup[NUM_LEDS] =
{   /*  Off     Green   Red     Blue    Yellow  Cyan    Magenta White */ 
    {   0xFF,   0x01,   0x00,   0xFF,   0xFF,   0xFF,   0xFF,   0xFF  }, /* PWR */
    {   0x00,   0x01,   0x02,   0xFF,   0x03,   0xFF,   0xFF,   0xFF  }, /* NWK  */
    {   0xFF,   0x00,   0xFF,   0xFF,   0x01,   0xFF,   0xFF,   0xFF  }, /* MODE */
    {   0x00,   0x01,   0x02,   0x04,   0x03,   0x05,   0x06,   0x07  }, /* SVC  */
    {   0x00,   0x01,   0xFF,   0xFF,   0xFF,   0xFF,   0xFF,   0xFF  }  /* GPS  */
};

static const char* led_strings[NUM_LEDS] =
    { "pwr", "nwk", "mode", "svc", "gps"};

static const char* colour_strings[NUM_COLOURS] =
    { "off", "green", "red", "blue", "yellow", "cyan", "magenta", "white" };




static void set_gpio(int index, int state)
{
    if (gpio_info[index].state != state)
    {
        picoxcell_gpio_set_value(gpio_info[index].addr, state);
        gpio_info[index].state = state;
    }
};


static int set_one_led(led_index index, led_colour colour)
{
    unsigned char setting;
    int i;
    
    if ( (index < NUM_LEDS) && (colour < NUM_COLOURS) )
    {
        setting = led_lookup[index][colour];
        if (setting == 0xFF)
        {
            return 1; /* No can do - quietly ignore */
        }
        
        for (i = 0; i < GPIO_LINES_PER_LED; ++i)
        {
            int gpio_index = led_addr_table[index][i];
            if (gpio_index == GPIO_NA)
            {
                break; /* No mode lines for this LED */
            }
            
            set_gpio(gpio_index, (setting & 0x1) ? GPIO_HI : GPIO_LO);
            
            setting >>= 1;
        }
    }
    else
    {
        printf("set_one_led out of range error for index %d and colour %d\n", index, colour);
        return 1;
    }
    return 0;
}


static void ledGpioInit(void)
{
    int i;
    ulong pinmux;
    
    /* Setup AXI2CFG shared gpio muxing register */
    pinmux = IOREAD32(GPIO_MUX_REG_ADDR);
    pinmux |= LED_GPIO_MUX_MASK;
    IOWRITE32(pinmux, GPIO_MUX_REG_ADDR);
    
    /* To be able to use certain EBI pins, we must first ensure corresponding PAI register bits are clear */ 
    pinmux = IOREAD32(PAI_GPIO_REG_ADDR);
    pinmux &= LED_PAI_MASK;
    IOWRITE32(pinmux, PAI_GPIO_REG_ADDR);
    
    /* Setup AXI2CFG use ebi gpio register */
    pinmux = IOREAD32(EBI_GPIO_REG_ADDR);
    pinmux |= LED_EBI_MASK;
    IOWRITE32(pinmux, EBI_GPIO_REG_ADDR);
    
    /* Request pins and set direction for output */
    for (i = 0; i < NUM_GPIO_LINES; ++i)
    {
        picoxcell_gpio_request(gpio_info[i].addr);
        picoxcell_gpio_direction_output(gpio_info[i].addr, gpio_info[i].state);
    }    
}


static void ledGpioFinish(void)
{
    int i;
    
    /* Free up the pins we requested */
    for (i = 0; i < NUM_GPIO_LINES; ++i)
    {
        picoxcell_gpio_free(gpio_info[i].addr);
    }    
}


int setLED(led_index index, led_colour colour)
{
    int ret;
    
    ledGpioInit();

    ret = set_one_led(index, colour);
    
    ledGpioFinish();
    
    return ret;
}



#ifdef CONFIG_CMD_LEDSET

/*---------------------------------------------------------------------------*/
/* run the set_led command.
 */
int do_set_led (cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
    int i;
    int result = 1;
    led_index  index;
    led_colour colour;

    if (argc == 3)
    {
        for (i = 0; i < NUM_LEDS; i++)
        {
            if (strcmp(argv[1], led_strings[i]) == 0)
            {
                index = (led_index)i;
                break;
            }
        }
        
        if (i < NUM_LEDS)
        {
            for (i = 0; i < NUM_COLOURS; i++)
            {
                if (strcmp(argv[2], colour_strings[i]) == 0)
                {
                    colour = (led_colour)i;
                    result = 0;
                    break;
                }
            }
        }
        
        if (result == 0)
        {
            if (setLED(index, colour))
            {
                printf("\"%s\" is not a valid setting for LED \"%s\"\n", argv[2], argv[1]);
            }
        }
    }

    if (result)
    {
        return CMD_RET_USAGE;
    }

    return CMD_RET_SUCCESS;
}

/*===========================================================================*/

U_BOOT_CMD(
    setled,   3,   1,     do_set_led,
    "setled - change colour of single LED",
    "  setled [pwr | nwk | mode | svc | gps] [off | green | red | blue | yellow | cyan | magenta | white]"
);

#endif /* CONFIG_CMD_LEDSET */

#if defined CONFIG_CMD_LEDC

/*
 *  ledc command for U-Boot
 * 
 *   This command provides a flexible way to control a set of multi-coloured LEDs.
 *   The standard U-Boot LED command only handles two-state LEDs, and the existing extensions
 *   for colours are not really suitable for a bank of LEDs. This command is designed to
 *   deal with 7 colours, and is easily extensible to deal with more. It also permits
 *   a fixed sequence sto be applied to one or more of the LEDs, though more complex patterns
 *   where not all LEDs behave the same will still need to be programmed manually.
 * 
 *   General syntax:   
 * 
 *   ledc <ledspec> <sequence> [<reps> [<period>]]
 *      <ledspec> = <led_id> [<ledspec>]   (Between 1 and 5 LED ids)
 *      <led_id> =  pwr | nwk | mode | svc | gps | all
 *      <sequence> = <state> [<sequence>]  (Between 1 and 6 LED states)
 *      <state> = off|red|green|yellow
 *      1 <= <reps> <= 100                 (number of times to repeat the sequence)
 *      100 <= <period> <= 2000            (time in ms between steps)
 * 
 *   Examples:
 * 
 *    ledc 1 red                                     Turn led 1 red
 *    ledc 2 3 yellow                                Turn leds 2 and 3 yellow
 *    ledc all green                                 Turn all leds green
 *    ledc 2 3 1 off                                 Turn leds 1 2 3 off (order doesn't matter)
 *    ledc 1 3 off red off green off yellow 10 200   Step leds 1 and 3 through sequence
 *                                                     10 times, 200ms between steps
 * 
 *   After running a sequence, leds are left in the final state of the sequence
 * 
 *   Note re. picochip ipa73xx board:
 *   Instruction to go to a specific colour not supported by a particular LED are ignored
 *   - thus "ledc all blue" will turn the svc led blue but leave all others unchanged
 */

/* Summary of above as a help text */
static const char detailed_help[] =
"ledc <ledspec> <sequence> [<reps> [<period>]]\n"
"  <ledspec> = <led_id> [<ledspec>]   (Between 1 and 5 LED ids)\n"
"  <led_id> = pwr|nwk|mode|svc|gps|all\n"
"  <sequence> = <state> [<sequence>]  (Between 1 and 6 LED states)\n"
"  <state> = off|red|green|blue|yellow|cyan|magenta|white\n"
"  1 <= <reps> <= 100                 (number of times to repeat the sequence)\n"
"  100 <= <period> <= 2000            (time in ms between steps)\n\n"
"   Examples:\n"
"    ledc nwk red                                   Turn network led red\n"
"    ledc nwk mode yellow                           Turn network and mode leds yellow\n"
"    ledc all green                                 Turn all leds green\n"
"    ledc gps nwk svc off                           Turn gps network and service leds off (order doesn't matter)\n"
"    ledc svc off red off green off blue 10 200     Step service led through sequence\n\n";


/* Map of mask values for LED Ids */
typedef struct
{
    char* arg;
    int mask;
} led_mask_entry;

static const led_mask_entry led_mask_table[] =
{
    { "pwr",    0x01 },
    { "nwk",    0x02 },
    { "mode",   0x04 },
    { "svc",    0x08 },
    { "gps",    0x10 },
    { "all",    0x1F },
    { "",       -1  }
};

/* Table for cycling round colours */
led_colour led_sequence[MAX_STEPS];         


/* Apply specified states to all LEDs specified by the mask */
void ledc_set_leds(int mask, led_colour colour)
{
    int i, im;
    
    for (i = 0, im = 1; i < NUM_LEDS; i++, im <<= 1)
    {
        if ( (mask & im) != 0)
        {
            set_one_led((led_index)i, colour);
        }
    }
}


int do_ledc(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
    int i;
    int argi;
    int mask;
    int steps;
    int reps = DEF_REPS;
    int period_us = (DEF_PERIOD * 1000);
    
    /* Check first for a detailed help request */
    if ( (argc > 0) && (strcmp("?", argv[1]) == 0) )
    {
        printf(detailed_help);
        return CMD_RET_SUCCESS;
    }
    
    /* Validate arguments */
    if ( (argc < 3) || (argc > (8 + MAX_STEPS)) )
    {
        printf("Incorrect number of arguments\n");
        return CMD_RET_USAGE;
    }

    /* Find mask for specified LED(s) - step through args until we reach a non-led value */
    i = 0; argi = 1; mask = 0;
    while ((led_mask_table[i].mask >= 0) && (argi < argc))
    {
        if (strcmp(led_mask_table[i].arg, argv[argi]) == 0)
        {
            mask |= led_mask_table[i].mask;
            ++argi; i = 0; /* Next arg... */
            continue;
        }
        ++i;
    }
    
    if (mask <= 0)
    {
        printf("No valid LED id specified\n");
        return CMD_RET_USAGE;
    }
    
    /* Step through next args until we run out or hit a non-colour */
    i = 0; steps = 0;
    while ( (i < NUM_COLOURS) && (argi < argc) && (steps < MAX_STEPS))
    {
        if (strcmp(colour_strings[i], argv[argi]) == 0)
        {
            led_sequence[steps] = i; /* Each sequence step contains the index of the colour table entry */
            i = 0; ++steps; ++argi;
            continue;
        }
        ++i;
    }
    
    /* If no colours detected, oops */
    if (steps == 0)
    {
        printf("Invalid LED id, or no valid LED colour/state specified\n");
        return CMD_RET_USAGE;
    }
    
    /* If we only have one colour, just set it - no sequence */
    if (steps == 1)
    {
        ledc_set_leds(mask, led_sequence[0]);
        return CMD_RET_SUCCESS;
    }
    
    /* Read reps if present */
    if (argi < argc)
    {
        int new_reps = simple_strtoul(argv[argi], NULL, 0);
        if ( (new_reps < MIN_REPS) || ( new_reps > MAX_REPS) )
        {
            printf("Number of sequence repetitions must be >= %d and <= %d\n", MIN_REPS, MAX_REPS);
            return CMD_RET_USAGE;
        }
        reps = new_reps;
        ++argi;
    }
    
    /* Read period if present */
    if (argi < argc)
    {
        int new_period_ms = simple_strtoul(argv[argi], NULL, 0);
        if ( (new_period_ms < MIN_PERIOD) || (new_period_ms > MAX_PERIOD) )
        {
            printf("Step period must be >= %dms and <= %dms\n", MIN_PERIOD, MAX_PERIOD);
            return CMD_RET_USAGE;
        }
        period_us = 1000 * new_period_ms;
        ++argi;
    }
    
    /* Should be no more args! */
    if (argi < argc)
    {
        printf("Extra arguments found\n");
        return CMD_RET_USAGE;
    }
    /* All OK, initialise GPIO and go for it */
    ledGpioInit();
    
    /* Now go through the sequence the requisite number of times */
    while (reps > 0)
    {
        for (i=0; i < steps; i++)
        {
            ledc_set_leds(mask, led_sequence[i]);
            udelay(period_us);
        }
        --reps;
    }
    
    /* Tidy up GPIO settings */
    ledGpioFinish();
    
    return CMD_RET_SUCCESS;
}


U_BOOT_CMD(
    ledc, (7 + MAX_STEPS), 1, do_ledc,
           "Control of multicoloured LEDs - set a single state or step through several",
           "<ledspec> <sequence> [<reps> [<period>]] - type \"ledc ?\" for detailed help"
);

#endif /* defined CONFIG_CMD_LEDC */


void led_confidence(void)
{
    /* LED confidence sequence */
    ledGpioInit();
    set_one_led(LED_PWR,  LED_RED);   /* As many red as possible */
    set_one_led(LED_NWK,  LED_RED);
    set_one_led(LED_SVC,  LED_RED);    
    mdelay(500);
    set_one_led(LED_PWR,  LED_GREEN); /* All green */
    set_one_led(LED_NWK,  LED_GREEN);
    set_one_led(LED_MODE, LED_GREEN);
    set_one_led(LED_SVC,  LED_GREEN);    
    set_one_led(LED_GPS,  LED_GREEN);    
    mdelay(500);
    set_one_led(LED_MODE, LED_YELLOW); /* Other colours */
    set_one_led(LED_SVC,  LED_BLUE);    
    mdelay(500);
    set_one_led(LED_NWK,  LED_OFF);    /* All off that can go off */
    set_one_led(LED_SVC,  LED_OFF);    
    set_one_led(LED_GPS,  LED_OFF);    
    set_one_led(LED_MODE, LED_GREEN);
    set_one_led(LED_PWR,  LED_GREEN);
    ledGpioFinish();
}