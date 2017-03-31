/*
 * Copyright(c) 2007-2014 Intel Corporation. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify 
 * it under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but 
 * WITHOUT ANY WARRANTY; without even the implied warranty of 
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU 
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License 
 * along with this program; if not, write to the Free Software 
 * Foundation, Inc., 51 Franklin St - Fifth Floor, Boston, MA 02110-1301 USA.
 * The full GNU General Public License is included in this distribution 
 * in the file called LICENSE.GPL.
 *
 * Contact Information:
 * Intel Corporation
 */

#include <common.h>
#include <asm/arch/hardware.h>
#include <nand.h>
#include <asm/arch/pcie.h>
#ifndef CONFIG_GT_BOARD
#include <asm/arch/dejitter.h>
#endif
#if defined(CONFIG_CHARACTERISATION_IPAT2K)
#include "characterisation.h"
#endif
#include "secboot.h"
//#include "led.h"

DECLARE_GLOBAL_DATA_PTR;

static ulong timestamp;
static ulong last_count;    /**< Last counter value read from timer for calculations */

static void flash__init (void);
static void ether__init (void);
static void _timer_init(void);
static void gpio_init(void);
static void serdes_init(void);
int dram_init (void);

#if 0
//BSP 5.11 change for GT Board but probably not needed
#ifdef CONFIG_GT_BOARD
#ifdef CONFIG_ADT75
extern void adt75_init(void);
#endif
typedef struct {
    unsigned int    magic;        //GT81
    unsigned int    hw_ver;       // start from 1, so need reg vaule + 1;
    unsigned int    uloader_ver;  // for x.y.z, uloader version is 0x00xxyyzz
    unsigned int    uboot_ver;    // for x.y.z, uboot version is 0x00xxyyzz
    unsigned int    uboot_fs;     //tell uboot, if it is failsafe image
    unsigned int    reserved1;
    unsigned int    reserved2;
    unsigned int    reserved3;
    unsigned int    reserved4;
}FC81VER, *PFC81VER;
/*iram is 256K(0xF4000000 to 0xF4040000), put it to 255k(0xf403fc00), because our loader should be less than 64K.*/
PFC81VER fc81_ver = (PFC81VER)0xf403fc00;
#endif
#endif

#define NAND_CS    4
#define NAND_ADDR_ALE   (1 << 25)
#define NAND_ADDR_CLE   (1 << 24)

#define READ_TIMER (*(volatile u32*) TIMER_COUNT)

#if defined(CONFIG_SHOW_BOOT_PROGRESS)
void show_boot_progress(int progress)
{
    printf("Boot reached stage %d\n", progress);
}
#endif

static inline void delay (unsigned long loops)
{
	__asm__ volatile ("1:\n"
		"subs %0, %1, #1\n"
		"bne 1b":"=r" (loops):"0" (loops));
}

/*
 * Miscellaneous platform dependent initialisations
 */

int board_init (void)
{
	gd->bd->bi_arch_number = MACH_TYPE_TRANSCEDE;

	/* adress of boot parameters */
	gd->bd->bi_boot_params = 0x00000100;

	gd->flags = 0;

    //TODO should this be done in dram_init_banksize??
	// DRAM init
	gd->bd->bi_dram[0].start = PHYS_SDRAM_1;
	gd->bd->bi_dram[0].size = PHYS_SDRAM_1_SIZE;

	// icache_enable ();
	gpio_init();
	flash__init ();
	_timer_init();
#ifdef TRANSCEDE_SERDES_INIT
	// for VSEMI debugging
	serdes_init();
#endif
#if 0
//BSP 5.11 change for GT Board but probably not needed
#ifdef CONFIG_GT_BOARD	300
#ifdef CONFIG_ADT75	301
       adt75_init();
#endif
#endif
#endif
	return 0;
}
#ifndef CONFIG_GT_BOARD
void dejitter__init(void)
{
	char*    dejitter_cfg;
	char*    clksource;
	char*    clkoutput_mode_strings[4];
	char*    clkoutput_hz_strings[4];
	char*    clkloophz;
    char*    i2cdac_string;
	unsigned DejitterClockSource;
	unsigned DejitterClockMode;
	unsigned DejitterClockOutputMode[4];
	unsigned DejitterClockOutputHz[4];
	unsigned DejitterClockLoopHz;
    unsigned I2cDacValue;
	int      i;
	int      ValidStringDetected=0;

//#ifdef FULLSIZE_EVM

	//
	// Set defaults (note change this section to optimize better for
	// board type if AD9548 is used, how it is default programmed
	// (example, PLL main loop speed) and how output options are
	// routed/used on the board)
	//
	DejitterClockSource = DEJITTER_CLK_SRC_RADIO;
	DejitterClockMode   = DEJITTER_INIT_RADIO;
	DejitterClockLoopHz = 240000000;

	DejitterClockOutputMode[0] = AD9548_CLK_OUT_MODE_CMOS_BOTH_PINS
	                           | AD9548_CLK_OUT_CMOS_DRIVE_STRENGTH_NORMAL
	                           ;
	DejitterClockOutputHz[0]   = 10000000;

	DejitterClockOutputMode[1] = AD9548_CLK_OUT_MODE_CMOS_BOTH_PINS
	                           | AD9548_CLK_OUT_CMOS_DRIVE_STRENGTH_NORMAL
	                           ;
	DejitterClockOutputHz[1]   = 100;

	DejitterClockOutputMode[2] = AD9548_CLK_OUT_MODE_TRISTATE_BOTH_PINS;
	DejitterClockOutputHz[2]   = 40000000;

	DejitterClockOutputMode[3] = AD9548_CLK_OUT_MODE_CMOS_BOTH_PINS
	                           | AD9548_CLK_OUT_CMOS_DRIVE_STRENGTH_NORMAL
	                           ;
	DejitterClockOutputHz[3]   = 10000000;

    //
    // Get environment variable strings if preset
    // (otherwise local variable is set to NULL)
    //

	dejitter_cfg              = getenv("dejitter");
	clksource                 = getenv("clksrc");
	clkloophz                 = getenv("clkfreerunhz");
	clkoutput_mode_strings[0] = getenv("clkoutmode0");
	clkoutput_mode_strings[1] = getenv("clkoutmode1");
	clkoutput_mode_strings[2] = getenv("clkoutmode2");
	clkoutput_mode_strings[3] = getenv("clkoutmode3");
	clkoutput_hz_strings  [0] = getenv("clkouthz0");
	clkoutput_hz_strings  [1] = getenv("clkouthz1");
	clkoutput_hz_strings  [2] = getenv("clkouthz2");
	clkoutput_hz_strings  [3] = getenv("clkouthz3");

	for (i=0; i<4; i++) {
		if (clkoutput_mode_strings[i] != NULL) {
			DejitterClockOutputMode[i] = simple_strtoul(clkoutput_mode_strings[i],NULL,0);
			if (DejitterClockOutputMode[i] > 255) {
				printf("Invalid clkoutmode%u value:0x%08X, must be byte value, disabling output\n",i,DejitterClockOutputMode[i]);
				DejitterClockOutputMode[i] = AD9548_CLK_OUT_MODE_TRISTATE_BOTH_PINS;;
			} else {
				ValidStringDetected = 1;
			}
		}
		if (clkoutput_hz_strings[i] != NULL) {
			DejitterClockOutputHz[i] = simple_strtoul(clkoutput_hz_strings[i],NULL,0);
			if (DejitterClockOutputHz[i] > 240000000) {
				printf("Invalid clkouthz%u value:%u, to high, setting 10 MHz\n",i,DejitterClockOutputHz[i]);
				DejitterClockOutputHz[i] = 10000000;
			} else {
				ValidStringDetected = 1;
				if (240000000 % DejitterClockOutputHz[i]) {
					printf("Warning clkouthz%u value:%u, exact value not attainable",i,DejitterClockOutputHz[i]);
					DejitterClockOutputHz[i] -= (240000000 % DejitterClockOutputHz[i]);
					printf(" Lowering to %u Hz]n", DejitterClockOutputHz[i]);
				}
			}
			if (clkoutput_mode_strings[i] == NULL) {
				// Hz is specified, but not mode,
				// set to CMOS normal strength on both pins
				DejitterClockOutputMode[i] = AD9548_CLK_OUT_MODE_CMOS_BOTH_PINS
				                           | AD9548_CLK_OUT_CMOS_DRIVE_STRENGTH_NORMAL
				                           ;
	                     ;
			}
		}
	}

	if (clksource) {
            if (strcmp(clksource, "transcede") == 0) {
                    DejitterClockSource = DEJITTER_CLK_SRC_TRANSCEDE;
		    ValidStringDetected = 1;
            }
            else if (strcmp(clksource, "gps1pps") == 0) {
                    DejitterClockSource = DEJITTER_CLK_SRC_GPS_1PPS;
		    ValidStringDetected = 1;
            }
            else if (strcmp(clksource, "gpsosc") == 0) {
                    DejitterClockSource = DEJITTER_CLK_SRC_GPS_OSCILLATOR;
		    ValidStringDetected = 1;
            }
            else if (strcmp(clksource, "external") == 0) {
                    DejitterClockSource = DEJITTER_CLK_SRC_SMA_CONNECTOR;
		    ValidStringDetected = 1;
            }
            else if (strcmp(clksource, "radio") == 0) {
                    DejitterClockSource = DEJITTER_CLK_SRC_RADIO;
		    ValidStringDetected = 1;
            }
	    else {
		    // For testing or prototyping, allow numeric parameter
		    i = simple_strtol(clksource,NULL,0);
		    if ((i > 0) && (i <= AD9548_CLK_SRC_MAX_VALUE))
			    DejitterClockSource = i;
	    }
	}

        /* For femtocell board, a 2 step init process is not needed */
	if (dejitter_cfg) {
	    if (strcmp(dejitter_cfg, "cpri") == 0) {
		DejitterClockMode=DEJITTER_INIT_CPRI;
		ValidStringDetected = 1;
	    }
	    else if (strcmp(dejitter_cfg, "radio") == 0) {
		DejitterClockMode=DEJITTER_INIT_RADIO;
		ValidStringDetected = 1;
	    }
	}
	if (clkloophz) {
		ValidStringDetected = 1;
		DejitterClockLoopHz = simple_strtoul(clkloophz,NULL,0);
		if (DejitterClockLoopHz > 300000000)
			DejitterClockLoopHz = 240000000;
	}

	//
	// Environment variables are all parsed
	// Test if we got any valid strings
	// If so, then we configure the AD9548 chip to requested mode
	// Setting defaults as necessary
	//
	// If no valid inputs, then setup
	//
	if (ValidStringDetected) {
		DejitterInit(DejitterClockMode, DejitterClockSource);
		//
		// Check if freerun loop override (else leave alone, don't program if default)
		//
		if (clkloophz != NULL)
			DejitterSetFreeRunLoopHz(DejitterClockLoopHz);

		for (i=0; i<4; i++) {
			DejitterSetupOutput(i,DejitterClockOutputMode[i],DejitterClockOutputHz[i]);
		}

	}

    //
    // Set I2C Dac value if environment variable is present,
    // Otherwise leave it alone as it may not be present or
    // other software may set it to its base value.
    //
    // If the radio chipset is clocked off of a voltage controlled
    // oscillator controlled by this DAC, then this can set the
    // base frequency offset for radio transmission at a good
    // enough value to start.
    //
    // This environment variable allows storing of the base value on
    // a per individual board basis to try and get it so that
    // the clock disciplining software (if any) can start off
    // with a better, per-board, pre-configured value
    // (to hopefully reduce disciplining time).
    //
    //
	i2cdac_string = getenv("i2cdac");
    if (i2cdac_string != NULL) {
        I2cDacValue = simple_strtoul(i2cdac_string,NULL,0);
        if (I2cDacValue > 0xFFFF) {
            printf("Invalid i2cdac value 0x%08X, too large, setting middle value 0x8000\n",
                   I2cDacValue
                  );
            I2cDacValue = 0x80000;
        }
        SetI2cDacValue(I2cDacValue);
    }
}
#endif
int misc_init_r (void)
{
#if defined(CONFIG_CHARACTERISATION_IPAT2K)
	if (1 != characterisation_init())
		 return 1;
#endif
	if (0 != load_security_requirements())
		return 1;

	if (silent_mode_enabled()) {
		setenv("silent", "1");
		setenv("bootdelay", "0");
	} else {
		setenv("silent", NULL);
		setenv("bootdelay", "3");
	}


	ether__init ();
#ifndef CONFIG_GT_BOARD
	dejitter__init ();
#endif
	setenv("verify", "y");

#if defined(CONFIG_CHARACTERISATION_IPAT2K)
	(void)print_characterisation();
#endif
	return (0);
}

#define RAD_CFG_BASEADDR	0xF5E00000
static void serdes_init(void)
{
	// serdes 0
	REG32(RAD_CFG_BASEADDR + 0x98) = 0x11111; // serdes_cfg_6 Serdes Tx and Rx Rate
	REG32(RAD_CFG_BASEADDR + 0x04) = 0x1;  // SRDS Reset

	// serdes 1
	REG32(RAD_CFG_BASEADDR + 0xC0) = 0x100;
	REG32(RAD_CFG_BASEADDR + 0x08) = 0x1;
	REG32(RAD_CFG_BASEADDR + 0x220) = 0xF;
}

/*
 * Route UART0, I2C and NAND through gpio pins
 * Power off HBI, bootstrap
 */
static void gpio_init(void)
{
//	REG32(GPIO_PIN_SELECT_REG1) |= I2C_SDA | I2C_SCL;
	REG32(GPIO_PIN_SELECT_REG1) &= ~(GPIO_MUX_I2C_SCL | GPIO_MUX_I2C_SDA);	// make sure GPIO function is not selected for I2C SCL\SDA pins
	REG32(AXI_RESET_1) &=  ~(1 << 5);
	REG32(GPIO_PIN_SELECT_REG1) |= EXP_RDY_BSY;

#ifdef CONFIG_CMD_NAND
	*(volatile u32*) GPIO_31_16_PIN_SELECT_REG |= (1 << ((28-16)*2));
	*(volatile u32*) GPIO_OE_REG |= EXP_NAND_CS;

	*(volatile u32*) GPIO_31_16_PIN_SELECT_REG |= (1 << ((29-16)*2));
	*(volatile u32*) GPIO_OE_REG &= ~EXP_NAND_RDY;
#endif
	//TODO: random hangs?
	/* enable both SS0 and SS1, define only one really used */
	*(volatile u32*)  GPIO_31_16_PIN_SELECT_REG &= ~((0x0fUL << 28));

	// HW tracing enable
	REG32(BOOTSTRAP_OVERRIDE_REG) |= 0x18;
        /*Changing the default value of MISC PIN REGISTER
          This will disable default settings for USIM and
          enable use to use GPIO 12 and 13*/
        REG32(MISC_PIN_SELECT_REG)&=1;
}

static void flash__init (void)
{
#ifndef CONFIG_SYS_NO_FLASH
    unsigned int nor_cs = 0;
    unsigned int delay;

    REG32(EXP_CLOCK_DIV_R) = 12; // change expansion bus clock value to 10 to cover highest bus frequency 250 MHz (~20 MHz)
    delay = 1000;
    while (delay--)
        asm("nop"); // 10000 cycles delay

    *(volatile u32*) EXP_CSx_CFG_R(nor_cs) |= 1 << 1;       /* 16bit */
    *(volatile u32*) EXP_CSx_SEG_R(nor_cs) = (CONFIG_SYS_FLASH_BASE >> 12) | 0x7FFF;
    *(volatile u32*) EXP_CSx_BASE_R(nor_cs) = CONFIG_SYS_FLASH_BASE >> 12;
    *(volatile u32*) EXP_CSx_TMG1_R(nor_cs) = 0x02021002;
    *(volatile u32*) EXP_CSx_TMG2_R(nor_cs) = 0x01010101;
    *(volatile u32*) EXP_CSx_TMG3_R(nor_cs) = 0x1;
    *(volatile u32*) EXP_CS_EN_R = EXP_CLK_EN | (1 << (nor_cs + 1));
#endif

#ifdef CONFIG_CMD_NAND
        /*
         * NAND flash CS# init
         */
        *(volatile u32*) EXP_CSx_BASE_R(NAND_CS) = CONFIG_SYS_NAND_BASE >> 12;
        *(volatile u32*) EXP_CSx_SEG_R(NAND_CS) =  ((CONFIG_SYS_NAND_BASE | NAND_ADDR_ALE | NAND_ADDR_CLE) >> 12) - 1;
        *(volatile u32*) EXP_CSx_TMG1_R(NAND_CS) = 0x02021002;
        *(volatile u32*) EXP_CSx_TMG2_R(NAND_CS) = 0x01010101;
        *(volatile u32*) EXP_CSx_TMG3_R(NAND_CS) = 0x1;
        *(volatile u32*) EXP_CSx_CFG_R(NAND_CS) = 1 << 9;
        *(volatile u32*) EXP_CS_EN_R |= (1 << (NAND_CS + 1)) | EXP_CLK_EN;
#endif
}

/** @brief T2200/T3300 EVMs board specific initialization for Ethernet */
extern struct mii_bus * alloc_gem_mdio(void * ctrl);
extern void             free_gem_mdio(struct mii_bus * bus);
extern int              gem_set_phy_flags(char *       devname,      /* Device driver name       */
                                          unsigned int phy_flags     /* New PHY flags to set     */
                                         );

static void ether__init (void)
{
	volatile u32       val;

	*(volatile u32*) EXTPHY1_RESET &= ~1;        // PHY
	*(volatile u32*) GEMTX_RESET    = 0;         // GEM TX
	*(volatile u32*) AXI_RESET_2   &= ~(1 << 6); // GEM1

	for (val=100; val; val--) {}			// small delay

	*(volatile u32*) PAD_CONF5_REG = 0x00033333;	// SET MAX DRIVE STRENGTH (3) AND FAST SLEW (0) ON ALL RGMII SIGNALS
}


int dram_init (void)
{
	//gd->bd->bi_dram[0].start = PHYS_SDRAM_1;
	//gd->bd->bi_dram[0].size  = PHYS_SDRAM_1_SIZE;
    gd->ram_size = PHYS_SDRAM_1_SIZE; //DDR0_SIZE; //TODO which one??
	return 0;
}

#ifdef CONFIG_RTSM_ONLY
static void _timer_init(void)
{
	*(volatile u32*) TIMER_CONTROL = 0;
	*(volatile u32*) TIMER_STATUS = 0;
	*(volatile u32*) TIMER_LOAD = AXICLK_HZ * 2;
	*(volatile u32*) TIMER_CONTROL |=  TIMER_CTRL_ENABLE | TIMER_CTRL_AUTO;
	reset_timer_masked();
}

void reset_timer(void)
{
	// reset_timer_masked();
}

/* delay x useconds AND perserve advance timstamp value */
/* ASSUMES timer is ticking at 1 msec			*/
void __udelay (unsigned long usec)
{
	ulong tmo, tmp;

	tmo = usec/1000;

	tmp = get_timer (0);		/* get current timestamp */

	if( (tmo + tmp + 1) < tmp )	/* if setting this forward will roll time stamp */
		reset_timer_masked ();	/* reset "advancing" timestamp to 0, set last_count value */
	else
		tmo += tmp;		/* else, set advancing stamp wake up time */


	while (get_timer_masked () < tmo)/* loop till event */
	{

		/*NOP*/;
	}

}

ulong get_timer (ulong base)
{
	return get_timer_masked () - base;
}

void reset_timer_masked (void)
{
	/* reset time */
	last_count = READ_TIMER/1000;  /* capure current decrementer value time */
	timestamp = 0;	       	    /* start "advancing" time stamp from 0 */
}

/* ASSUMES 1MHz timer */
ulong get_timer_masked (void)
{
	ulong now = READ_TIMER/1000;	/* current tick value @ 1 tick per msec */

	if (last_count >= now) {		/* normal mode (non roll) */
		/* normal mode */
		timestamp += last_count - now; /* move stamp forward with absolute diff ticks */
	} else {			/* we have overflow of the count down timer */
		/* nts = ts + ld + (TLV - now)
		 * ts=old stamp, ld=time that passed before passing through -1
		 * (TLV-now) amount of time after passing though -1
		 * nts = new "advancing time stamp"...it could also roll and cause problems.
		 */
		timestamp += last_count + TIMER_LOAD_VAL - now;
	}
	last_count = now;

	return timestamp;
}

#else /* CONFIG_RTSM_ONLY */

#define TIMER_LOAD_VAL ((u32)0xFFFFFFFF)

static void _timer_init(void)
{
    /* Reset the timer */
        reset_timer_masked();

    /* Configure ARM private timer */
        *(volatile u32*) TIMER_CONTROL = 0;
        *(volatile u32*) TIMER_STATUS = 0;
        *(volatile u32*) TIMER_LOAD = TIMER_LOAD_VAL;
        *(volatile u32*) TIMER_CONTROL |=  TIMER_CTRL_ENABLE | TIMER_CTRL_AUTO;

    /* Setup expansion bus timer (this timer runs independently of
     * all ARM processors)
     */
    REG32(TIMER0_CNTR_REG) = (u32)0xFFFFFFFF;   /* Set Timer 0 count to max (Timer 0 not used) */

    REG32(TIMER3_LBOUND_REG) = 0;                  /* Set Timer 3 low  bound register to 0 */
    REG32(TIMER3_HBOUND_REG) = (u32)0xFFFFFFFF; /* Set Timer 3 high bound register to max */

    REG32(TIMER3_CNTR_REG) = 1;                    /* chain Timer 3 with Timer 1's timeout */

    REG32(TIMER1_CNTR_REG) = (u32)TIMER1_CNTR_VALUE;    /* Set Timer 1 counter register */

    gd->arch.tbu = 0;
    gd->arch.tbl = 0;
}

void disable_timer(void)
{
       *(volatile u32*) TIMER_CONTROL = 0;
       *(volatile u32*) TIMER_STATUS = 0;
}

/**
 * timer without interrupts
 */
void reset_timer (void)
{
        reset_timer_masked ();
}

/**
 * This function must return milliseconds
 * For Integrator/CP 1 tick == 1 millisecond
 */
ulong get_timer (ulong base_ticks)
{
        return get_timer_masked () - base_ticks;
}

/**
 * Return raw timer count for timing intervals
 * less than 1 tick e.g. usecs
 * Caller must test for wrap.....
 */
u32 get_timer_raw(void)
{
        return READ_TIMER;
}

/**
 * Function to delay for 1 microsecond
 */
void __udelay (unsigned long usec)
{
    u32 start_time;

    usec *= TIMER0_TICKS_PER_MICROSECOND;
    start_time = *(volatile u32*)TIMER0_CURR_COUNT;
    while ( (*(volatile u32*)TIMER0_CURR_COUNT - start_time)  < usec) {
       asm("nop");
    }

}

/**
 * Function to reset the timer back to zero
 */
void reset_timer_masked (void)
{
    last_count = *(volatile u32 *)TIMER3_CURR_COUNT;
        /* start "advancing" time stamp from 0 */
        timestamp = 0L;
}

/**
 * Function to convert the timer reading to U-Boot ticks
 * the timestamp is the number of ticks since reset
 * returns ticks
 */
ulong get_timer_masked (void)
{
        /* get current millisecond count from timer 3 */
        ulong now = *(volatile u32 *)TIMER3_CURR_COUNT;

    /* adjust timestamp based on difference from last time timer count taken */
    timestamp += now - last_count;

    /* Save current timestamp for future calculation */
    last_count = now;

    /* Return u-boot time */
        return timestamp;
}

/**
 * Function to waits for a specified delay value and reset the timer timestamp
 */
void udelay_masked (unsigned long usec)
{
        udelay(usec);
        reset_timer_masked();
}

/**
 * This function is derived from PowerPC code (read timebase as long long).
 * On ARM it just returns the timer value since the timer decrements at CFG_HZ.
 */
unsigned long long get_ticks(void)
{
        /* get current millisecond count from timer 3 */
        ulong now = *(volatile u32 *)TIMER3_CURR_COUNT;

        /* increment tbu if tbl has rolled over */
        if (now < gd->arch.tbl)
           gd->arch.tbu++;
        gd->arch.tbl = now;
        return (((unsigned long long)gd->arch.tbu) << 32) | gd->arch.tbl;
}

/*
 * This function is derived from PowerPC code (timebase clock frequency).
 * On ARM it returns the number of timer ticks per second.
 */
ulong get_tbclk(void)
{
    return CONFIG_SYS_HZ;
}


#endif /* CONFIG_RTSM_ONLY */

/*
 * Write the system control status register to cause reset
 */
void reset_cpu(ulong addr){
#ifdef CONFIG_GT_BOARD
	REG32(GPIO_OUTPUT_REG) &= ~(1<< 2);
	REG32(GPIO_OE_REG) |= (1 << 2);
	udelay(200000);
	REG32(GPIO_OUTPUT_REG) |= (1<< 2);
#else
	REG32(PLLS_GLOBAL_CNTRL) |= 0xf;
	REG32(DEVICE_RST_CNTRL) = 1;
#endif
}


/*
 *  u32 get_board_rev() for ARM supplied development boards
 */
u32 get_board_rev(void)
{
    return -1;
}

#ifdef CONFIG_SYS_FLASH_CFI

ulong board_flash_get_legacy(ulong base,  int banknum, flash_info_t* info)
{
	if ( banknum==0 ) {
	    info->portwidth = 1;
	    info->chipwidth = 1;
	    info->interface = FLASH_CFI_X8;
	    return 1;
	} else
	    return 0;
}

#endif /* CONFIG_SYS_FLASH_CFI */

#ifdef CONFIG_CMD_NAND
static u32 addr_mask = 0x0;
static void t2k_nand_control(struct mtd_info *mtd, int cmd, unsigned int ctrl)
{
	struct nand_chip* this = mtd->priv;

	if (ctrl & NAND_CTRL_CHANGE)
	{
		if (ctrl & NAND_NCE)
			*(volatile u32*) GPIO_OUTPUT_REG &= ~EXP_NAND_CS;
		else
			*(volatile u32*) GPIO_OUTPUT_REG |= EXP_NAND_CS;

		addr_mask = 0x0;
		if ( (ctrl & NAND_CTRL_CLE) == NAND_CTRL_CLE ) {// old bitmask
			addr_mask |= NAND_ADDR_CLE;
			ctrl &= ~NAND_CTRL_CLE;
		}

		if ( (ctrl & NAND_CTRL_ALE) == NAND_CTRL_ALE ) {// old bitmask
			addr_mask |= NAND_ADDR_ALE;
			ctrl &= ~NAND_CTRL_ALE;
		}
	}

	if (cmd != NAND_CMD_NONE)
		REG8((u32)this->IO_ADDR_W | addr_mask) = cmd;

}

static int t2k_nand_ready(struct mtd_info* mtd)
{
#ifdef CONFIG_RTSM_ONLY
//	return REG32(GPIO_INPUT_REG) & 0x00010000;	// this bit is used in RTSM
#endif
	return REG32(GPIO_INPUT_REG) & PIN_EXP_NAND_RDY;
}

int board_nand_init(struct nand_chip* nand)
{
	nand->cmd_ctrl = t2k_nand_control;
#ifdef CONFIG_RTSM_ONLY
	nand->ecc.mode = NAND_ECC_NONE;
#else
	nand->ecc.mode = NAND_ECC_SOFT;
//	nand->ecc.mode = NAND_ECC_HW_SYNDROME; // we should use this when HW ECC block will be tested
#endif
	nand->chip_delay = 20;
	nand->dev_ready = t2k_nand_ready;
	nand->options = 0;
	return 0;
}
#endif // CONFIG_CMD_NAND

int transcede_gemac_initialize(bd_t * bis, int index, char *devname);

int board_eth_init(bd_t *bis)
{
    int ret = 0;

#if defined(CONFIG_COMCERTO_GEMAC)
	transcede_gemac_initialize(bis, 0, "gemac0");
#ifdef CONFIG_T2K
	transcede_gemac_initialize(bis, 1, "gemac1");
#endif
#endif
#if 0
    ret = transcede_gemac_initialize(bis, 0, "gemac0");
    if(0 != ret)
    {
        printf("%s failed to init gemac0\n", __func__);
        return ret;
    }
    ret = transcede_gemac_initialize(bis, 1, "gemac1");
    if(0 != ret)
    {
        printf("%s failed to init gemac1\n", __func__);
        return ret;
    }
#endif
    return ret;
}

