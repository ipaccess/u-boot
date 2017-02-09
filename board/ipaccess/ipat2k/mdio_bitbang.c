/*
* Bitbanged MDIO support.
*
* Author: Scott Wood <scottwood@freescale.com>
* Copyright (c) 2007 Freescale Semiconductor
*
* Based on CPM2 MDIO code which is:
*
* Copyright (c) 2003 Intracom S.A.
*  by Pantelis Antoniou <panto@intracom.gr>
*
* 2005 (c) MontaVista Software, Inc.
* Vitaly Bordug <vbordug@ru.mvista.com>
*
* This file is licensed under the terms of the GNU General Public License
* version 2. This program is licensed "as is" without any warranty of any
* kind, whether express or implied.
*/
#ifndef NULL
#define NULL 0
#endif

#ifdef LINUX
	#include <linux/module.h>
	#include <linux/mdio-bitbang.h>
	#include <linux/types.h>
	#include <linux/delay.h>
#else
	#include <common.h>
	#include <asm/arch/hardware.h>
	#include <asm/arch/mdio_bitbang.h>

	#ifdef ATHEROS_SWITCH_TEST_ENABLED
	#include "ar8328.h"
	#endif
#endif

//#define DEBUG_MDIO_BIT_BANG
//#define MDIO_BIT_BANG_TEST

#define MDIO_READ 2
#define MDIO_WRITE 1

#define MDIO_C45 (1<<15)
#define MDIO_C45_ADDR (MDIO_C45 | 0)
#define MDIO_C45_READ (MDIO_C45 | 3)
#define MDIO_C45_WRITE (MDIO_C45 | 1)

#define MDIO_SETUP_TIME 10
#define MDIO_HOLD_TIME 10

/* Minimum MDC period is 400 ns, plus some margin for error.  MDIO_DELAY
* is done twice per period.
*/
//#define MDIO_DELAY 250
//#define MDIO_DELAY 2500
#define MDIO_DELAY 15
/* The PHY may take up to 300 ns to produce data, plus some margin
* for error.
*/
//#define MDIO_READ_DELAY 350
//#define MDIO_READ_DELAY 3500
#define MDIO_READ_DELAY 20
#ifdef LINUX
#else

//
// For transcede portation, use static structures
//
struct mii_bus     MdioBitBangBusData;
//
// Map ndelay to local expansion bus timer on Transcede
//
void ndelay (u32 Delay)
{
	udelay(Delay);
	/*
	u32 ticks;
	u32 StartTicks;
	//
	// Get current expansion bus systime
	// Adjust up one tick to make sure minimum time is reached
	//
	ticks = (Delay/EXPBUSTICKS_IN_1_US) + 1;
	StartTicks = SysTimerGetTick();
	//
	// Wait for timer to expire
	//
	while ((SysTimerGetTick() - StartTicks) < ticks);
	//
	// Time reached, return
	//
	*/
}

//#ifdef T2K_EVM
#ifdef ATHEROS_SWITCH_TEST_ENABLED /* assume T2K EVM if Atheros switch test enabled */
/** @brief mapping for MDIO Data (MDIO) GPIO for T2200 EVM board, also mapped to debug LED 1 (0 for on) */
#define MDIO_GPIO 4

/** @brief mapping for MDIO Clock (MDC) GPIO for T2200 EVM board */
#define MDC_GPIO  5

#else
//
// T2K CHECKOUT BOARD
//
/** @brief mapping for MDIO Data (MDIO) GPIO for T2200 EVM board, also mapped to debug LED 1 (0 for on) */
#define MDIO_GPIO 20

#define MDIO_GPIO_SELECT_MASK (1 << ((MDIO_GPIO - 16) * 2))
#define MDIO_GPIO_SELECT_REG  GPIO_31_16_PIN_SELECT_REG

/** @brief mapping for MDIO Clock (MDC) GPIO for T2200 EVM board */
#define MDC_GPIO  21
#define MDC_GPIO_SELECT_MASK (1 << ((MDC_GPIO - 16) * 2))
#define MDC_GPIO_SELECT_REG  GPIO_31_16_PIN_SELECT_REG

#endif

#define MDIO_GPIO_BIT (1<<MDIO_GPIO)
#define MDC_GPIO_BIT  (1<<MDC_GPIO)

//
// Transcede GPIO to bit bang I/O routines
//
/* Set the Management Data Clock high if level is one,
 * low if level is zero.
 */
void SetMdc(struct mdiobb_ctrl *ctrl, int level)
{
	if (level)
	{
		REG32(GPIO_OUTPUT_REG) |=  MDC_GPIO_BIT;
	}
	else
	{
		REG32(GPIO_OUTPUT_REG) &=  ~(MDC_GPIO_BIT);
	}
}

/**
 * @brief Configure the Management Data I/O pin as an input if
 * "output" is zero, or an output if "output" is one.
 */
void SetMdioDir(struct mdiobb_ctrl *ctrl, int output)
{
    //
    // Set MDIO GPIO as output
	// Use this opportunity to also make sure clock (MDC) is properly set as output
	//	
	if (output)
	{
		REG32(GPIO_OE_REG) |=  MDIO_GPIO_BIT;
	}
	else
	{
		REG32(GPIO_OE_REG) &=  ~(MDIO_GPIO_BIT);
	}

	REG32(GPIO_OE_REG) |= MDC_GPIO_BIT;

#ifdef MDIO_GPIO_SELECT_MASK
    //
    // If MDIO GPIO is multiplexed, also setup as GPIO instead of alternate function
    //
    REG32(MDIO_GPIO_SELECT_REG) |= MDIO_GPIO_SELECT_MASK;
#endif

#ifdef MDC_GPIO_SELECT_MASK
    //
    // If MDIO GPIO is multiplexed, also setup as GPIO instead of alternate function
    //
    REG32(MDC_GPIO_SELECT_REG) |= MDC_GPIO_SELECT_MASK;
#endif
}

/**
 * @brief Set the Management Data I/O pin high if value is one,
 * low if "value" is zero.  This may only be called
 * when the MDIO pin is configured as an output.
 */
void SetMdioData(struct mdiobb_ctrl *ctrl, int value)
{
	if (value)
	{
		REG32(GPIO_OUTPUT_REG) |=  MDIO_GPIO_BIT;
	}
	else
	{
		REG32(GPIO_OUTPUT_REG) &=  ~(MDIO_GPIO_BIT);
	}
}

/** @brief Retrieve the state of the Management Data I/O pin. */
int GetMdioData(struct mdiobb_ctrl *ctrl)
{
    if (REG32(GPIO_INPUT_REG) & MDIO_GPIO_BIT)
    {
        return 1;
    }
    else
    {
        return 0;
    }
}

const struct mdiobb_ops  MdioBitBangBusOpsData =
{
    NULL, // struct module *owner;

    /* Set the Management Data Clock high if level is one,
     * low if level is zero.
     */
    &SetMdc, //void (*set_mdc)(struct mdiobb_ctrl *ctrl, int level);

    /* Configure the Management Data I/O pin as an input if
     * "output" is zero, or an output if "output" is one.
     */
    &SetMdioDir, //void (*set_mdio_dir)(struct mdiobb_ctrl *ctrl, int output);

    /* Set the Management Data I/O pin high if value is one,
     * low if "value" is zero.  This may only be called
     * when the MDIO pin is configured as an output.
     */
    &SetMdioData, // void (*set_mdio_data)(struct mdiobb_ctrl *ctrl, int value);

    /* Retrieve the state Management Data I/O pin. */
    &GetMdioData  // int (*get_mdio_data)(struct mdiobb_ctrl *ctrl);		
};

struct mdiobb_ctrl MdioBitBangBusOpsPointer =
{
	&MdioBitBangBusOpsData	
};

#endif


/* MDIO must already be configured as output. */
static void mdiobb_send_bit(struct mdiobb_ctrl *ctrl, int val)
{
    const struct mdiobb_ops *ops = ctrl->ops;

    ops->set_mdio_data(ctrl, val);
    ndelay(MDIO_DELAY);
    ops->set_mdc(ctrl, 1);
    ndelay(MDIO_DELAY);
    ops->set_mdc(ctrl, 0);
}

/* MDIO must already be configured as input. */
static int mdiobb_get_bit(struct mdiobb_ctrl *ctrl)
{
    const struct mdiobb_ops *ops = ctrl->ops;

    ndelay(MDIO_DELAY);
    ops->set_mdc(ctrl, 1);
    ndelay(MDIO_READ_DELAY);
    ops->set_mdc(ctrl, 0);

    return ops->get_mdio_data(ctrl);
}

/* MDIO must already be configured as output. */
static void mdiobb_send_num(struct mdiobb_ctrl *ctrl, u16 val, int bits)
{
    int i;

    for (i = bits - 1; i >= 0; i--)
        mdiobb_send_bit(ctrl, (val >> i) & 1);
}

/* MDIO must already be configured as input. */
static u16 mdiobb_get_num(struct mdiobb_ctrl *ctrl, int bits)
{
    int i;
    u16 ret = 0;

    for (i = bits - 1; i >= 0; i--) {
        ret <<= 1;
        ret |= mdiobb_get_bit(ctrl);
    }

    return ret;
}

/* Utility to send the preamble, address, and
* register (common to read and write).
*/
static void mdiobb_cmd(struct mdiobb_ctrl *ctrl, int op, u8 phy, u8 reg)
{
    const struct mdiobb_ops *ops = ctrl->ops;
    int i;

    ops->set_mdio_dir(ctrl, 1);

    /*
     * Send a 32 bit preamble ('1's) with an extra '1' bit for good
     * measure.  The IEEE spec says this is a PHY optional
     * requirement.  The AMD 79C874 requires one after power up and
     * one after a MII communications error.  This means that we are
     * doing more preambles than we need, but it is safer and will be
     * much more robust.
     */

    for (i = 0; i < 32; i++)
        mdiobb_send_bit(ctrl, 1);

    /* send the start bit (01) and the read opcode (10) or write (10).
       Clause 45 operation uses 00 for the start and 11, 10 for
       read/write */
    mdiobb_send_bit(ctrl, 0);
    if (op & MDIO_C45)
        mdiobb_send_bit(ctrl, 0);
    else
        mdiobb_send_bit(ctrl, 1);
    mdiobb_send_bit(ctrl, (op >> 1) & 1);
    mdiobb_send_bit(ctrl, (op >> 0) & 1);

    mdiobb_send_num(ctrl, phy, 5);
    mdiobb_send_num(ctrl, reg, 5);
}

/* In clause 45 mode all commands are prefixed by MDIO_ADDR to specify the
   lower 16 bits of the 21 bit address. This transfer is done identically to a
   MDIO_WRITE except for a different code. To enable clause 45 mode or
   MII_ADDR_C45 into the address. Theoretically clause 45 and normal devices
   can exist on the same bus. Normal devices should ignore the MDIO_ADDR
   phase. */

static int mdiobb_cmd_addr(struct mdiobb_ctrl *ctrl, int phy, u32 addr)
{
    unsigned int dev_addr = (addr >> 16) & 0x1F;
    unsigned int reg = addr & 0xFFFF;
    mdiobb_cmd(ctrl, MDIO_C45_ADDR, phy, dev_addr);

    /* send the turnaround (10) */
    mdiobb_send_bit(ctrl, 1);
    mdiobb_send_bit(ctrl, 0);

    mdiobb_send_num(ctrl, reg, 16);

    ctrl->ops->set_mdio_dir(ctrl, 0);
    mdiobb_get_bit(ctrl);

    return dev_addr;
}

int mdiobb_read(struct mii_bus *bus, int phy, int reg)
{
    struct mdiobb_ctrl *ctrl = bus->priv;
    int ret, i;

    if (reg & MII_ADDR_C45) {
        reg = mdiobb_cmd_addr(ctrl, phy, reg);
        mdiobb_cmd(ctrl, MDIO_C45_READ, phy, reg);
    } else
        mdiobb_cmd(ctrl, MDIO_READ, phy, reg);

    ctrl->ops->set_mdio_dir(ctrl, 0);

    /* check the turnaround bit: the PHY should be driving it to zero */
    if (mdiobb_get_bit(ctrl) != 0) {
        /* PHY didn't drive TA low -- flush any bits it
         * may be trying to send.
         */
        for (i = 0; i < 32; i++)
        mdiobb_get_bit(ctrl);

        return 0xffff;
    }

    ret = mdiobb_get_num(ctrl, 16);
    mdiobb_get_bit(ctrl);

#ifdef DEBUG_MDIO_BIT_BANG
    printf("mdiobb_read  phy:0x%4.4X register: 0x%4.4X value 0x%4.4X\n",phy, reg, ret);
    mdelay(10); // Delay for print
#endif
    return ret;
}

int mdiobb_write(struct mii_bus *bus, int phy, int reg, u16 val)
{
    struct mdiobb_ctrl *ctrl = bus->priv;

#ifdef DEBUG_MDIO_BIT_BANG
    printf("mdiobb_write phy:0x%4.4X register: 0x%4.4X value 0x%4.4X\n",phy, reg, val);
    mdelay(10); // Delay for print
#endif

    if (reg & MII_ADDR_C45) {
        reg = mdiobb_cmd_addr(ctrl, phy, reg);
        mdiobb_cmd(ctrl, MDIO_C45_WRITE, phy, reg);
    } else
        mdiobb_cmd(ctrl, MDIO_WRITE, phy, reg);

    /* send the turnaround (10) */
    mdiobb_send_bit(ctrl, 1);
    mdiobb_send_bit(ctrl, 0);

    mdiobb_send_num(ctrl, val, 16);

    ctrl->ops->set_mdio_dir(ctrl, 0);
    mdiobb_get_bit(ctrl);
    return 0;
}

struct mii_bus *alloc_mdio_bitbang(struct mdiobb_ctrl *ctrl)
{

    struct mii_bus *     bus;
    
#ifdef LINUX
    bus = mdiobus_alloc();
    if (!bus)
        return NULL;
    
    __module_get(ctrl->ops->owner);
    
    bus->read = mdiobb_read;
    bus->write = mdiobb_write;
    bus->priv = ctrl;
    
#else
    bus            = &MdioBitBangBusData;
    bus->priv      = &MdioBitBangBusOpsPointer;
    bus->read      = &mdiobb_read;
    bus->write     = &mdiobb_write;    
#endif

    return bus;
}
#ifdef LINUX
EXPORT_SYMBOL(alloc_mdio_bitbang);
#endif

void free_mdio_bitbang(struct mii_bus *bus)
{
#ifdef LINUX
    struct mdiobb_ctrl *ctrl = bus->priv;

    module_put(ctrl->ops->owner);
    mdiobus_free(bus);
#endif
}

u32 ScanMdioBus(void)
{
	struct mdiobb_ctrl ctrl;
	struct mii_bus *     bus;
	int    addr;
	u16    reg[4];
	u16    val;
	u32    cnt = 0;

	bus = alloc_mdio_bitbang(&ctrl);
	if (bus==NULL)
	{
		printf("Bit Bang Bus Allocation Error\n");
		return 1;
	}

	for(addr = 0; addr < 0x20; addr++)
	{
		reg[0] = bus->read(bus, addr, 0);
		reg[1] = bus->read(bus, addr, 1);
		reg[2] = bus->read(bus, addr, 2);
		reg[3] = bus->read(bus, addr, 3);
		if(reg[0] != 0xffff || reg[1] != 0xffff || reg[2] != 0xffff || reg[3] != 0xffff)
		{
			printf("MDIO 0x%02X: %04X %04X %04X %04X\n", addr, reg[0], reg[1], reg[2], reg[3]);
			cnt++;
		}
	}
	printf("Detected %d MDIO device(s)\n", cnt);

	free_mdio_bitbang(bus);
    
    return 0;
}


#ifdef LINUX
EXPORT_SYMBOL(free_mdio_bitbang);

MODULE_LICENSE("GPL");

#else

#if(0)
//
// TEMP CODE HERE, WILL MOVE TO ETH DRIVER LATER
//

//
// Diagnostic test and example code
//

void WriteAtherosDebugRegister(struct mii_bus * bus, u16 phy, u16 DebugRegister, u16 Value)
{
    //
    // Setup Debug register to target value
    //
    mdiobb_write(bus,phy,0x1D,DebugRegister);
    //
    // Debug register selected, write value to 0x1E to put value into that
    // debug register.
    //
    mdiobb_write(bus,phy,0x1E,Value);
}

u16 ReadAtherosDebugRegister(struct mii_bus * bus, u16 phy, u16 DebugRegister)
{
    //
    // Setup Debug register to target value
    //
    mdiobb_write(bus,phy,0x1D,DebugRegister);
    //
    // Debug register selected, read value from 0x1E to get value from that
    // debug register.
    //
    return(mdiobb_read(bus,phy,0x1E));
}

void DumpPhyRegisters(struct mii_bus * bus, u32 phy)
{
    u32 reg;
    u16 val;

	printf("\nPHY 0x%2.2X:\n",phy);
	for (reg=0; reg<=0xF; reg++)
	{
		// Read register
		val = mdiobb_read(bus, phy, reg);
		// Print it
		printf(" 0x%4.4X",val);
		//mdiobb_write(bus,phy,reg,val);
	}
	printf("\n");
	for (; reg<=0x1F; reg++)
	{
		// Read register
		val = mdiobb_read(bus, phy, reg);
		// Print it
		printf(" 0x%4.4X",val);
		//mdiobb_write(bus,phy,reg,val);
	}
    printf("\n");
}

void DumpPhyDebugRegisters(struct mii_bus * bus, u32 phy)
{
    u32 reg;
    u16 val;

	printf("\nPHY 0x%2.2X DEBUG:\n",phy);
	for (reg=0; reg<=0xF; reg++)
	{
		// Read register
		val = ReadAtherosDebugRegister(bus, phy, reg);
		// Print it
		printf(" 0x%4.4X",val);
	}
	printf("\n");
	for (; reg<=0x1F; reg++)
	{
		// Read register
		val = ReadAtherosDebugRegister(bus, phy, reg);
		// Print it
		printf(" 0x%4.4X",val);
	}
    printf("\n");
}

//
// TEMP DEFINES HERE FOR TEST (NEED TO MOVE TO ATHEROS DRIVER
//

#ifdef ATHEROS_SWITCH_TEST_ENABLED
#define ATHEROS_SGMII_PHY_ADDRESS 6 // T2K EVMBOARD ATHEROS SGMII PHY ADDRESS (DIFFERENT FOR OTHER BOARDS)
#else
#define ATHEROS_SGMII_PHY_ADDRESS 4 // T2K CHECKOUT  BOARD ATHEROS SGMII PHY ADDRESS (DIFFERENT FOR OTHER BOARDS)
#endif
#define ATHEROS_RGMII_PHY_ADDRESS 5 // T2K CHECKOUT  BOARD ATHEROS RGMII PHY ADDRESS (DIFFERENT FOR OTHER BOARDS)

#define ATHEROS_RGMII_GTX_DLY_MASK         (3 << 5)
#define ATHEROS_RGMII_GTX_DLY_VAL_250_PS   (0 << 5)
#define ATHEROS_RGMII_GTX_DLY_VAL_1300_PS  (1 << 5)
#define ATHEROS_RGMII_GTX_DLY_VAL_2400_PS  (2 << 5)
#define ATHEROS_RGMII_GTX_DLY_VAL_3400_PS  (3 << 5)

#define ATHEROS_CONFIG_REG_TYPE_MASK                    (0xF << 0)
#define ATHEROS_CONFIG_TYPE_BASET_RGMII                 0       /* 1000 BASE-T, RGMII */
#define ATHEROS_CONFIG_TYPE_BASET_SGMII                 1       /* 1000 BASE-T, SGMII */
#define ATHEROS_CONFIG_TYPE_BX1000_RGMII_50             2       /* 1000 BASE-X, RGMII, 50 ohms */
#define ATHEROS_CONFIG_TYPE_BX1000_SGMII_75             3       /* 1000 BASE-X, SGMII, 75 ohms */
#define ATHEROS_CONFIG_TYPE_BX1000_CONV_1000_BASET_50   4       /* Converter mode between 1000 BASE-X  and 1000 BASE-T media, 50 ohm */
#define ATHEROS_CONFIG_TYPE_BX1000_CONV_1000_BASET_75   5       /* Converter mode between 1000 BASE-X  and 1000 BASE-T media, 75 ohm */
#define ATHEROS_CONFIG_TYPE_FX100_RGMII_50              6       /* 100 BASE-FX, RGMII 50 ohm */
#define ATHEROS_CONFIG_TYPE_FX100_CONV_100BASET_50      7       /* Converter mode between  100 BASE-FX and 1000 BASE-T media, 50 ohm */
#define ATHEROS_CONFIG_TYPE_RGMII_AUTO_COPPER_FIBER    11       /* RGMII, copper fiber auto-detection */
#define ATHEROS_CONFIG_TYPE_FX100_RGMII_75             14       /* 100 BASE-FX, RGMII mode, 75 ohm */
#define ATHEROS_CONFIG_TYPE_FX100_CONV_100_BASET_75    15       /* Converter mode between  100 BASE-FX and  100 BASE-T media, 75 ohm */

#define ATHEROS_CONFIG_TYPE_RG_AUTO_MDET        11


//#define T2K_EVM_RGMII_GTX_DLY_VAL   ATHEROS_RGMII_GTX_DLY_VAL_250_PS // Seeing abuot 45% CRC errors and gaps where no frames are seen at all
#define T2K_EVM_RGMII_GTX_DLY_VAL   ATHEROS_RGMII_GTX_DLY_VAL_1300_PS // Still seeing CRC errors on board 9, but not many, about .0016 percent (still to high for actual system)
//#define T2K_EVM_RGMII_GTX_DLY_VAL   ATHEROS_RGMII_GTX_DLY_VAL_2400_PS // Still seeing CRC errors on board 9, about 12% on small packets
//#define T2K_EVM_RGMII_GTX_DLY_VAL   ATHEROS_RGMII_GTX_DLY_VAL_3400_PS // Seeing very few good frames (over 99% are CRC error at this delay value)

void SetRgmiiAtherosPhyForT2K(struct mii_bus * bus, u32 new_val)
{
    u16 val;
    u32 phy = 5; //ATHEROS_RGMII_PHY_ADDRESS;
    //
    // For RGMII mode, need to change RGMII timing to delay TX (delay for RX defaulted OK)
    //
    // First set TX Value to default (4 values possible, see defines)
    //
    val = ReadAtherosDebugRegister(bus, phy, 0xB);
    printf("##### RGMII: %x\n", (val&ATHEROS_RGMII_GTX_DLY_MASK) >> 5);
    printf("##### NEW:   %x\n", new_val & 3);
    val &= ~ATHEROS_RGMII_GTX_DLY_MASK;  // Clear bits
    //val |= T2K_EVM_RGMII_GTX_DLY_VAL;
    val |= (new_val & 3) << 5;
    WriteAtherosDebugRegister(bus, phy, 0xB, val);


    
    //
    // Get current value from debug register 5
    //
    val = ReadAtherosDebugRegister(bus, phy, 5);
    //
    // TX clock delay is enabled by setting bit 8
    //
    val |= (1<<8);
    //
    // Write it back
    //
    WriteAtherosDebugRegister(bus, phy, 5, val);
    //
    // Reset the PHY using standard MII register 0
    // Set bit 15 high (self clearing bit)
    //
    val = mdiobb_read(bus,phy,0);
    val |= 0x8000; // Set reset bit
    mdiobb_write(bus, phy, 0, val);
}

void SetSgmiiAtherosPhyForT2K(struct mii_bus * bus)
{
    u16 val;
    u32 phy = ATHEROS_SGMII_PHY_ADDRESS;
    u32 ResetNeeded = 0;
    //
    // For SGMII mode, need to change mode register to SGMII (tests are showing this as not being set for some reason)
    //
    // Get Atheros Chip configure regsiter (register 0x1F)
    // Mask off current type and set to SGMII copper mode
    //
    val = mdiobb_read(bus, phy, 0x1F);
#if DEBUG_MDIO_BIT_BANG
    printf("SGMII PHY 0x%02X read  0x1F: 0x%04X\n",phy,val);  // TEMP PRINT
#endif
    //
    // Test if already running SGMII, if so, nothing to do
    //
    if ((val & ATHEROS_CONFIG_REG_TYPE_MASK) == /*ATHEROS_CONFIG_TYPE_BX1000_CONV_1000_BASET_75*/ ATHEROS_CONFIG_TYPE_BASET_SGMII)
    {
        printf("SGMII PHY already running SGMII, leaving alone...\n");
    }
    else
    {
        val &= ~(ATHEROS_CONFIG_REG_TYPE_MASK);   // (0xF << 0)
        val |=   /*ATHEROS_CONFIG_TYPE_BX1000_CONV_1000_BASET_75;*/ ATHEROS_CONFIG_TYPE_BASET_SGMII; //  1
#if DEBUG_MDIO_BIT_BANG
        printf("SGMII PHY 0x%02X write 0x1F: 0x%04X\n",phy,val);  // TEMP PRINT
#endif
        mdiobb_write(bus, phy, 0x1F, val);

        val = mdiobb_read(bus, phy, 0x1F);// TEMP READ BACK
#if DEBUG_MDIO_BIT_BANG
        printf("SGMII PHY 0x%02X read  0x1F: 0x%04X\n",phy,val);  // TEMP PRINT
#endif

        ResetNeeded = 1;
    }
#if 0
    //
    // FOR TEST, SHUTDOWN ENERGY EFFICIENT ETHERNET FEATURE 
    // TO HOPEFULLY GET LINK MORE STABLE FOR IXIA TESTING
    //
    // Get current value of extended MDIO register 0x805D
    // and clear energy effecient ethernet bit
    //
    mdiobb_write(bus, phy, 0x0D, 0x0003); // Function=address, set device address to Request MMD3 exteneded GPIO data
    mdiobb_write(bus, phy, 0x0E, 0x805D); // Address with energy effecient Ethernet option
    mdiobb_write(bus, phy, 0x0D, 0x4003); // Function=data, keep the device address the same for MMD3 and previous set register
    val = mdiobb_read(bus, phy, 0x0E);    // Get data from extended register
    val &= ~(1<<8);                       // Turn off bit 8 to disable energy efficient ethernet
    mdiobb_write(bus, phy, 0x0E, val);    // Write back data with bit disabled
    //ResetNeeded = 1;  // Request reset (lpi_en bit is retained in software reset)
#endif
    if (ResetNeeded)
    {
        //
        // Reset the PHY using standard MII register 0
        // Set bit 15 high (self clearing bit)
        //
        val = mdiobb_read(bus,phy,0);
        val |= 0x8000; // Set reset bit
        mdiobb_write(bus, phy, 0, val);
    }
}

void TestMdioBitBangBus(u32 delay_value)
{
	struct mdiobb_ctrl ctrl;
	struct mii_bus *     bus;
	int    phy;
	int    reg;
	u16    val;


#if(1)
    //
    // Get current value of Misc. Control register and make sure
    // MDIO is set to 2.5V
    //
    reg = REG32(GPIO_MISC_CTRL_REG);
#if DEBUG_MDIO_BIT_BANG
    printf("Reading GPIO_MISC_CTRL_REG: 0x%8.8X\n",reg);
#endif
    reg &= ~(3<<20); // Clear current bits
//  reg |=  (0<<20); // Set as 3.3V
//  reg |=  (1<<20); // Set as 1.8V
    reg |=  (2<<20); // Set as 2.5V
//  reg |=  (3<<20); // Set as 3.0V
#if DEBUG_MDIO_BIT_BANG
    printf("Setting GPIO_MISC_CTRL_REG: 0x%8.8X\n",reg);
#endif
    REG32(GPIO_MISC_CTRL_REG) = reg;

    //
    // TEMP RUN TEST TO GET VALUES OF PAD CONFIG REGISTER 5
    // FOR PATCHING LATER TO TRY AND IMPROVE RGMII SIGNALS
    //
    reg = REG32(PAD_CONF5_REG);
#if DEBUG_MDIO_BIT_BANG
    printf("Reading PAD CONFIG REGISTER 5: 0x%8.8X = 0x%8.8X\n",PAD_CONF5_REG,reg);
    printf("Values slew/strength: REFCLK = %u:%u TXC = %u:%u TXD = %u:%u RXC = %u:%u RXD = %u:%u\n",
           (reg >>  2) & 1, // REFCLK SLEW
           (reg >>  0) & 3, // REFCLK STRENGTH
           (reg >>  6) & 1, // TXC    SLEW
           (reg >>  4) & 3, // TXC    STRENGTH
           (reg >> 10) & 1, // TXD    SLEW
           (reg >>  8) & 3, // TXD    STRENGTH
           (reg >> 14) & 1, // RXC    SLEW
           (reg >> 12) & 3, // RXC    STRENGTH
           (reg >> 18) & 1, // RXD    SLEW
           (reg >> 16) & 3  // RXC    STRENGTH
          );
#endif

    //
    // SET TO LOOK LIKE WHAT IT APPEARS SHOULD HAVE BEEN DEFAULTS,
    // SLOW SLEW AND DRIVE STRENGTH 2 (DRIVE STRENGTH RANGE 0-3)
    //
    //reg = 0x00066666;
    //
    // TRYING MAX DRIVE STRENGTH (3) AND FAST (0)
    //
    reg = 0x00033333;
#if DEBUG_MDIO_BIT_BANG
    printf("Setting PAD CONFIG REGISTER 5: 0x%8.8X = 0x%8.8X\n",PAD_CONF5_REG,reg);
#endif
    REG32(PAD_CONF5_REG) = reg;

#endif
	
	bus = alloc_mdio_bitbang(&ctrl);
	if (bus==NULL)
	{
		printf("Bit Bang Bus Allocation Error\n");
		return;
	}


#ifdef ATHEROS_SWITCH_TEST_ENABLED
    //
    // TEMP FOR NOW, PUT ATHEROS SWITCH TEST CODE HERE (WILL MOVE TO PROPER PLACE LATER)
    //
    //
    // MDIO bus initialized, call Atheros initialization
    //
    printf("\nTesting Atheros Switch Initialization...\n");
    athrs17_init(bus);
    printf("\nAtheros Switch initialization complete\n");
    //
    // T2K EVM, ALSO TEST IF ABLE TO SETUP SGMII PHY FOR SGMII OPERATION
    //
    printf("\nSetting Atheros 8031 PHY for SGMII operation\n");
    SetSgmiiAtherosPhyForT2K(bus);


#endif




#if MDIO_BIT_BANG_TEST
	//
	// Read and write all possible normal range PHY (0-7), extended MDIO addresses (0x8 - 0x1F, example, Atheros Switch) and normal range registers (0-0x1F)
	//
#ifdef ATHEROS_SWITCH_TEST_ENABLED
	for (phy = 0; phy<=0x1F; phy++)
	//for (phy = 4; phy<=5; phy++)  // TEMP ONLY TEST ATHEROS PHY
	{
        DumpPhyRegisters(bus,phy);
	}
    printf("\nRegister 1st read test complete, waiting 5 seconds\n");
#else
    DumpPhyRegisters(bus,4);
    DumpPhyDebugRegisters(bus,4);
    DumpPhyRegisters(bus,5);
    DumpPhyDebugRegisters(bus,5);
    printf("\nRegister 1st read test complete, setting Atheros PHY for RGMII and SGMII and waiting 5 seconds\n");
    //
    // Put PHY into proper mode for operation for RGMII and SGMII 
    // (override defaults where defaults do not work)
    //
//    SetRgmiiAtherosPhyForT2K(bus);
//    SetSgmiiAtherosPhyForT2K(bus);
#endif

#endif

    SetRgmiiAtherosPhyForT2K(bus, delay_value);

#if 1
    //
    // TEMP DELAY AND READ AGAIN
    //
    mdelay(1000);
#ifdef ATHEROS_SWITCH_TEST_ENABLED
	for (phy = 0; phy<=0x1F; phy++)
	//for (phy = 4; phy<=5; phy++)  // TEMP ONLY TEST ATHEROS PHY
	{
        DumpPhyRegisters(bus,phy);
	}
#else
    DumpPhyRegisters(bus,4);
    DumpPhyDebugRegisters(bus,4);
    DumpPhyRegisters(bus,5);
    DumpPhyDebugRegisters(bus,5);
#endif
    printf("\nRegister 2nd read test complete\n");

#endif
    //
    // Test complete, shutdown bit bang driver
    //
	free_mdio_bitbang(bus);
}


#if(0)
void TempAtherosSwitchDumpStats(void)
{
	struct mdiobb_ctrl ctrl;
	struct mii_bus *     bus;
	int    phy;
	int    reg;
	u16    val;

	bus = alloc_mdio_bitbang(&ctrl);
	if (bus==NULL)
	{
		printf("Bit Bang Bus Allocation Error\n");
		return;
	}

    //
    // Dump statistics for MAC port 6 (hooked up to CPU)
    //
    DumpAtherosStatistics(bus, 6);

    //
    // Test complete, shutdown bit bang driver
    //
	free_mdio_bitbang(bus);
}
#endif

#endif

#endif // ifdef LINUX, else clause

