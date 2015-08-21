#ifndef __LINUX_MDIO_BITBANG_H
#define __LINUX_MDIO_BITBANG_H

#ifdef LINUX
#include <linux/phy.h>
#else
//
// Map all Linux typedefs here
//
#ifndef u16
#define u16 unsigned short
#endif

#ifndef u32
#define u32 unsigned int
#endif

#ifndef u8
#define u8 unsigned char
#endif

//
// For non-Linux implementation, setup MII bus structure
// and misc. constants from MII module
//

//
// MII defines
//
#define MII_ADDR_C45  0x80000000

struct mii_bus 
{
	struct mdiobb_ctrl * priv;
    int (*read) (struct mii_bus *bus, int phy, int reg);
    int (*write)(struct mii_bus *bus, int phy, int reg, u16 val);
    
};

//
// For non-Linux, for mii_device directly to mii_bus structure
//
#ifndef mii_device
#define mii_device mii_bus
#endif

#endif

struct module;

struct mdiobb_ctrl;

struct mdiobb_ops {
        struct module *owner;

        /* Set the Management Data Clock high if level is one,
         * low if level is zero.
         */
        void (*set_mdc)(struct mdiobb_ctrl *ctrl, int level);

        /* Configure the Management Data I/O pin as an input if
         * "output" is zero, or an output if "output" is one.
         */
        void (*set_mdio_dir)(struct mdiobb_ctrl *ctrl, int output);

        /* Set the Management Data I/O pin high if value is one,
         * low if "value" is zero.  This may only be called
         * when the MDIO pin is configured as an output.
         */
        void (*set_mdio_data)(struct mdiobb_ctrl *ctrl, int value);

        /* Retrieve the state Management Data I/O pin. */
        int (*get_mdio_data)(struct mdiobb_ctrl *ctrl);
};

struct mdiobb_ctrl {
        const struct mdiobb_ops *ops;
};

/* The returned bus is not yet registered with the phy layer. */
struct mii_bus *alloc_mdio_bitbang(struct mdiobb_ctrl *ctrl);

/* The bus must already have been unregistered. */
void free_mdio_bitbang(struct mii_bus *bus);


#if 0
ndef LINUX
/**
 * @brief Function to unit test MDIO bit bang function
 */
void TestMdioBitBangBus(u32 delay_val);

void TempAtherosSwitchDumpStats(void);

#endif

#endif
