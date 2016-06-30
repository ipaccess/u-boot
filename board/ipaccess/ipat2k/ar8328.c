/**
 * @brief Atheros AR8237, AR8238 driver source code
 * Base source code from Atheros
 * Source code modified by Intel Corporation
 *
 * NOTE: In actual deployement, the majority of this sofware is not
 * used, see CfgAtherosSwitch function for actual simpler code used,
 * tested and deployed on T2200 EVM with Atheros switch
 */

#ifdef LINUX
//
// Original includes from Atheros code base
//
#include <common.h>
#include <init.h>
#include <miidev.h>

#else
//
// Includes for portation to Transcede device
//
//#include "syscore.h"
//#include "sysdata.h"
#include <common.h>
#include <asm/arch/mdio_bitbang.h> // Include for MDIO bit bang driver

#ifndef mii_device
/** Map MDIO device structure from Atheros code to mii_bus structure */
#define mii_device mii_bus
#endif

/** Map mdelay call to diagnostics SysDelay functoin */
#ifndef mdelay
#define mdelay SysDelay
#endif

#endif

#include <asm/arch/ar8328.h>

//#define DEBUG_ATHEROS_SWITCH

/** @brief VLAN port for LAN */  #define S17_LAN_PORT_VLAN          1
/** @brief VLAN port for WAN */  #define S17_WAN_PORT_VLAN          2

#define ENET_UNIT_GE1 1
#define ENET_UNIT_GE0 0 /* Connected to the switch */

#ifndef TRUE
#define TRUE    1
#endif

#ifndef FALSE
#define FALSE   0
#endif

#ifndef uint32_t
#define uint32_t unsigned int
#endif

#ifndef uint16_t
#define uint16_t unsigned short
#endif

#ifndef uint8_t
#define uint8_t  unsigned char
#endif
//
// AR8237 has 5 physical ports, addresses 0-4
//
#define S17_PHY0_ADDR   0x0
#define S17_PHY1_ADDR   0x1
#define S17_PHY2_ADDR   0x2
#define S17_PHY3_ADDR   0x3
#define S17_PHY4_ADDR   0x4
#define S17_IND_PHY     4

/**
 * @brief Structure to Track per-PHY port information.
 */
typedef struct {
    BOOL   isEnetPort;         /**< @brief normal enet port */
    BOOL   isPhyAlive;         /**< @brief last known state of link */
    int    ethUnit;            /**< @brief MAC associated with this phy port */
    uint32_t phyBase;
    uint32_t phyAddr;          /**< @brief PHY registers associated with this phy port */
    uint32_t VLANTableSetting; /**< @brief Value to be written to VLAN table */
} athrPhyInfo_t;

/**
 * @brief Per-PHY information, indexed by PHY unit number
 *
 * MAC port 0 - CPU port 0x100
 * All ports are connected to GE0 of the SoC MAC
 * LAN/WAN seperation by VLAN tags (port 0-3 with VLANID 1; port 4 with VLANID 2
 */
static athrPhyInfo_t athrPhyInfo[] = {
    {TRUE,   /* phy port 0 -- MAC port 1 0x200 */
     FALSE,
     ENET_UNIT_GE0,
     0,
     S17_PHY0_ADDR,
     S17_LAN_PORT_VLAN
    },

    {TRUE,   /* phy port 1 -- MAC port 2 0x300 */
     FALSE,
     ENET_UNIT_GE0,
     0,
     S17_PHY1_ADDR,
     S17_LAN_PORT_VLAN
    },

    {TRUE,   /* phy port 2 -- MAC port 3 0x400 */
     FALSE,
     ENET_UNIT_GE0,
     0,
     S17_PHY2_ADDR,
     S17_LAN_PORT_VLAN
    },

    {TRUE,   /* phy port 3 -- MAC port 4 0x500 */
     FALSE,
     ENET_UNIT_GE0,
     0,
     S17_PHY3_ADDR,
     S17_LAN_PORT_VLAN
    },

#ifdef C2K
    //
    // C2K, phy port 4 is connected to RGMII to C2K WAN port
    //
    {TRUE,   /* phy port 4 -- WAN port or MAC port 5 0x600 */
     FALSE,
     ENET_UNIT_GE1,
     0,
     S17_PHY4_ADDR,
     S17_WAN_PORT_VLAN   /* set as WAN port */
    },
#else
    //
    // T2K, PHY port 4 is not connected, set isEnetPort to FALSE)
    //
    {FALSE,   /* phy port 4 -- WAN port or MAC port 5 0x600 */
     FALSE,
     ENET_UNIT_GE0,
     0,
     S17_PHY4_ADDR,
     S17_LAN_PORT_VLAN   /* set as LAN port */
    },
#endif
    //
    // Phy Port 5 is only applicable for AR8328 and not 8327
    // for AR8327, this is set to false
    //
    {FALSE,  /* phy port 5 -- CPU port (no RJ45 connector) */
     TRUE,
     ENET_UNIT_GE0,
     0,
     0x00,
     S17_LAN_PORT_VLAN    /* Send to all ports */
    },
};

#define S17_PHY_MAX 5

/* Range of valid PHY IDs is [MIN..MAX] */
#define S17_ID_MIN 0
#define S17_ID_MAX (S17_PHY_MAX-1)

/* Convenience macros to access myPhyInfo */
#define S17_IS_ENET_PORT(phyUnit)       (athrPhyInfo[phyUnit].isEnetPort)
#define S17_IS_PHY_ALIVE(phyUnit)       (athrPhyInfo[phyUnit].isPhyAlive)
#define S17_ETHUNIT(phyUnit)            (athrPhyInfo[phyUnit].ethUnit)
#define S17_PHYBASE(phyUnit)            (athrPhyInfo[phyUnit].phyBase)
#define S17_PHYADDR(phyUnit)            (athrPhyInfo[phyUnit].phyAddr)
#define S17_VLAN_TABLE_SETTING(phyUnit) (athrPhyInfo[phyUnit].VLANTableSetting)


/** @brief function to initialize the Atheros Switch */
int athrs17_init(struct mii_device *mdev)
{
    int          addr;
//    unsigned int dummy;

#if 0
    //
    // TEMP FOR NOW, WRITE EXACTLY WHAT WORKED BEFORE WHEN RUNNING EEPROM
    // TO SEE IF WE CAN FIRST RECREATE THOSE RESULTS...
    //
    athrs17_reg_write(mdev, 0x0010, 0x40000000); // Setup as AR8327 mode
    athrs17_reg_write(mdev, 0x0004, 0x00000000); // Disable MAC0
    athrs17_reg_write(mdev, 0x0008, 0x00000000); // Disable MAC5
    athrs17_reg_write(mdev, 0x000C, 0x07600000); // Setup MAC6 for RGMII operations with proper TX/RX delays
//  athrs17_reg_write(mdev, 0x000C, 0x07A00000); // Setup MAC6 for RGMII operations with proper TX/RX delays
//  athrs17_reg_write(mdev, 0x000C, 0x07500000); // Setup MAC6 for RGMII operations with proper TX/RX delays
    athrs17_reg_write(mdev, 0x0624, 0x5E5E5E5E); // Setup register 0x0624 for forwarding to MACs 1-4 and 6, not on 0 or 5
    athrs17_reg_write(mdev, 0x007C, 0x0000007E); // Setup MAC 0 for RGMII connection to Transcede, Fixed Gigabit speed, full duplex, no flow control
    athrs17_reg_write(mdev, 0x0080, 0x00000230);  // Setup MACs 1-5 as operational for actual PHY port with auto-neg turned on, no flow control
    athrs17_reg_write(mdev, 0x0084, 0x00000230);
    athrs17_reg_write(mdev, 0x0088, 0x00000230);
    athrs17_reg_write(mdev, 0x008C, 0x00000230);
    athrs17_reg_write(mdev, 0x0090, 0x00000230);
    athrs17_reg_write(mdev, 0x0094, 0x00000000); // Setup MAC 6 as disabled (not connected to anything)

  //athrs17_reg_write(mdev, 0x0000, 0x80000000); // Software reset
    //
    // SETUP LOOKUP REGISTERS FOR DEFAULT PORT BASED VLAN
    // ON MAC PORTS 1-5 DISABLE LOOKUP ON PORTS 0 AND 6
    //
    athrs17_reg_write(mdev,
                      S17_LOOKUP_CTRL_REG(0),
                      0
                     );
    athrs17_reg_write(mdev,
                      S17_LOOKUP_CTRL_REG(5),
                      0
                     );
    athrs17_reg_write(mdev,
                      S17_LOOKUP_CTRL_REG(1),
                      S17_LOOKUP_SET_PORT_DEFAULT(1)
                     );

    athrs17_reg_write(mdev,
                      S17_LOOKUP_CTRL_REG(2),
                      S17_LOOKUP_SET_PORT_DEFAULT(2)
                     );

    athrs17_reg_write(mdev,
                      S17_LOOKUP_CTRL_REG(3),
                      S17_LOOKUP_SET_PORT_DEFAULT(3)
                     );

    athrs17_reg_write(mdev,
                      S17_LOOKUP_CTRL_REG(4),
                      S17_LOOKUP_SET_PORT_DEFAULT(4)
                     );

    athrs17_reg_write(mdev,
                      S17_LOOKUP_CTRL_REG(6),
                      S17_LOOKUP_SET_PORT_DEFAULT(6)
                     );

#if 0
    //
    // TEMP for debug put RGMII port in loopback
    //
    athrs17_reg_write(mdev,
                      S17_LOOKUP_CTRL_REG(6),
                         S17_LOOKUP_PORT_LOOPBACK_EN
                       | S17_LOOKUP_PORT_STATE_FORWARDING
                       | S17_LOOKUP_SET_PORT_DEFAULT(6)
                       | (1<<6) /* Allow port to send to itself */
                       );
#endif
    //
    // Do the same for priority control, enable port base VLANs on all ports for egress
    // Write 0 to not allow MAC ports 0 and 5
    //
    athrs17_reg_write(mdev,
                      S17_PRI_CTRL_REG(0),
                      0
                     );

    athrs17_reg_write(mdev,
                      S17_PRI_CTRL_REG(5),
                      0
                     );

    athrs17_reg_write(mdev,
                      S17_PRI_CTRL_REG(1),
                      S17_PRI_CTRL_DEFAULT
                     );

    athrs17_reg_write(mdev,
                      S17_PRI_CTRL_REG(2),
                      S17_PRI_CTRL_DEFAULT
                     );

    athrs17_reg_write(mdev,
                      S17_PRI_CTRL_REG(3),
                      S17_PRI_CTRL_DEFAULT
                     );

    athrs17_reg_write(mdev,
                      S17_PRI_CTRL_REG(4),
                      S17_PRI_CTRL_DEFAULT
                     );


    athrs17_reg_write(mdev,
                      S17_PRI_CTRL_REG(6),
                      S17_PRI_CTRL_DEFAULT
                     );


    //
    // Enable MIB counters
    //
    athrs17_reg_write(mdev,
                      S17_MODULE_EN_REG,
                      S17_MODULE_MIB_EN
                     );


#else

    //configure the RGMII
    /* FIXME Configure broadcast ports: the configuration below
     * will broadcast on ports 0-5 of the switch.
     */
    athrs17_reg_write(mdev, S17_GLOFW_CTRL1_REG,                              /* 0x0624 */
            S17_BROAD_DPALL | S17_MULTI_FLOOD_DPALL | S17_UNI_FLOOD_DPALL);   /* C2K 0x003F3F3F, T2K:0x005E5E5E */

    /* Set control register to indicate this is an AR8327,not and AR8328 (different pinout and number of ports) */
    athrs17_reg_write(mdev, S17_PWS_REG, S17_PWS_CHIP_AR8327);                /* 0x0010: 0x40000000 */

    /* Kill load of EEPROM if EEPROM is enabled by setting Reload timer to
     * zero (i.e. switch to full SW control from this point on)
     */
    athrs17_reg_write(mdev, S17_INTF_HIADDR_REG, 0);

#ifdef C2K
    //
    // C2K has:
    // MAC 0 to CPU port via RGMII-0
    // MACs 1-4 to PHY 0-3
    // MAC 5 not connected
    // PHY 4 as WAN port to CPU via RGMII-1
    //
    /* Set delays for MAC0 (0x0004: 0x07500000) */
    athrs17_reg_write(mdev,
                      S17_P0PAD_MODE_REG,                     /* 0x0004:    */
                      S17_MAC0_RGMII_EN          |            /* 0x04000000 */
                      S17_MAC0_RGMII_TXCLK_DELAY |            /* 0x02000000 */
                      S17_MAC0_RGMII_RXCLK_DELAY |            /* 0x01000000 */
                      (1 << S17_MAC0_RGMII_TXCLK_SHIFT) |     /* 0x00400000 */
                      (1 << S17_MAC0_RGMII_RXCLK_SHIFT)       /* 0x00100000 */
                     );
    /* Set bit 24 to enable MAC0 RGMII delay; set MAC6 as PHY mode (PHY4), QCA */
    /* 0x000C: 0x05000000 */
    athrs17_reg_write(mdev,
                      S17_P6PAD_MODE_REG,                     /* 0x000C:    */
                      S17_PHY4_RGMII_EN          |            /* 0x04000000 */
                      S17_MAC6_RGMII_RXCLK_DELAY |            /* 0x01000000 */
                      (0 << S17_MAC6_RGMII_RXCLK_SHIFT)       /* 0x00000000 */
                     );

    /* Disable MAC5 and MAC6 (due to PHY4), QCA */
    athrs17_reg_write(mdev, S17_P5STATUS_REG, 0);             /* 0x0090: 0x00000000 */
    athrs17_reg_write(mdev, S17_P6STATUS_REG, 0);             /* 0x0094: 0x00000000 */

    //
    // Setup port 0 status to default: (register 0x007C: 0x000000FE)
    // Speed Gigabit               0x00000002
    // TX enabled                  0x00000004
    // RX enabled                  0x00000008
    // TX flow control enabled     0x00000010
    // RX flow control enabled     0x00000020
    // Full duplex                 0x00000040
    // TX half duplex flow enabled 0x00000080
    //
    athrs17_reg_write(mdev, S17_P0STATUS_REG, S17_PORT_STATUS_DEFAULT);
#else
    //
    // T2K has:
    // MAC 0 not connected to CPU port via RGMII-0 (NOTE: this may change in future versions of the board
    // MACs 1-4 to PHY 0-3
    // MAC 5 not connected
    // MAC 6 connected as LAN port to CPU via RGMII-1 (NOTE: this may change to MAC0/RGMII-0 in the future)
    //
    /* Disable MACs 0 and 5 */
    athrs17_reg_write(mdev,
                      S17_P0PAD_MODE_REG,                     /* 0x0004: Set to 0   */
                      0
                      );
    athrs17_reg_write(mdev,
                      S17_P5PAD_MODE_REG,                     /* 0x0008: Set to 0   */
                      0
                      );
    //
    // Setup RGMII on MAC 6 (note may change to MAC0 in the future)
    //
    /* Set delays for MAC6 (0x000C: 0x07500000) */
    athrs17_reg_write(mdev,
                      S17_P6PAD_MODE_REG,                     /* 0x000C:    */
                      S17_MAC6_RGMII_EN          |            /* 0x04000000 */
                      S17_MAC6_RGMII_TXCLK_DELAY |            /* 0x02000000 */
                      S17_MAC6_RGMII_RXCLK_DELAY |            /* 0x01000000 */
                      (2 << S17_MAC6_RGMII_TXCLK_SHIFT) |     /* 0x00800000 */
                      (2 << S17_MAC6_RGMII_RXCLK_SHIFT)       /* 0x00200000 */
                     );

    /* Disable MAC0 and MAC5 (not connected) QCA */
    athrs17_reg_write(mdev, S17_P0STATUS_REG, 0);             /* 0x007C: 0x00000000 */
    athrs17_reg_write(mdev, S17_P5STATUS_REG, 0);             /* 0x0090: 0x00000000 */

    //
    // Setup port 6 status to default Gigabit operation: (register 0x007C: 0x000000FE)
    // Speed Gigabit               0x00000002
    // TX enabled                  0x00000004
    // RX enabled                  0x00000008
    // TX flow control enabled     0x00000010
    // RX flow control enabled     0x00000020
    // Full duplex                 0x00000040
    // TX half duplex flow enabled 0x00000080
    //
    athrs17_reg_write(mdev, S17_P6STATUS_REG, S17_RGMII_PORT_STATUS_DEFAULT);
    //
    // Set LAN port defaults, for LAN ports setup speed automatic as well
    //
#if 1
    //
    // Fully enabled port
    //
    athrs17_reg_write(mdev, S17_P1STATUS_REG, S17_LAN_PORT_STATUS_DEFAULT);
    athrs17_reg_write(mdev, S17_P2STATUS_REG, S17_LAN_PORT_STATUS_DEFAULT);
    athrs17_reg_write(mdev, S17_P3STATUS_REG, S17_LAN_PORT_STATUS_DEFAULT);
    athrs17_reg_write(mdev, S17_P4STATUS_REG, S17_LAN_PORT_STATUS_DEFAULT);
#else
    //
    // Fixed gigabit on all ports
    //
    athrs17_reg_write(mdev, S17_P1STATUS_REG, S17_PORT_STATUS_DEFAULT);
    athrs17_reg_write(mdev, S17_P2STATUS_REG, S17_PORT_STATUS_DEFAULT);
    athrs17_reg_write(mdev, S17_P3STATUS_REG, S17_PORT_STATUS_DEFAULT);
    athrs17_reg_write(mdev, S17_P4STATUS_REG, S17_PORT_STATUS_DEFAULT);
#endif

    //
    // Enable Ethernet MIB per port statistics counters
    //
    athrs17_reg_write(mdev,
                      S17_MODULE_EN_REG,
                      S17_MODULE_MIB_EN
                     );

#endif

    /* AR8327/AR8328 v1.0 fixup */
    if ((athrs17_reg_read(mdev, S17_MASK_CTRL_REG) & 0xffff) == S17_CHIPID_V1_0)
    {
#ifdef DEBUG_ATHEROS_SWITCH
        printf("%s:  S17_CHIPID_V1_0 detected \n", __func__);

#endif
        for (addr = 0x0; addr <= S17_PHY_MAX; addr++)
        {
            /* For 100M waveform */
            athrs17_phy_debug_write(mdev, addr, 0x0, 0x02ea);
            /* Turn On Gigabit Clock */
            athrs17_phy_debug_write(mdev, addr, 0x3d, 0x68a0);
        }
    }
#ifdef C2K
    /*
     * Set delays for PHY4 (connected to MAC5 on C2K EVM)
     */
    dummy = athrs17_phy_debug_read(mdev, S17_PHY4_ADDR, 0x0);
    dummy |= (1 << 15); // Enable RGMII Rx clock delay
    athrs17_phy_debug_write(mdev, S17_PHY4_ADDR, 0x0, dummy);
    dummy = athrs17_phy_debug_read(mdev, S17_PHY4_ADDR, 0x5);
    dummy |= (1 << 8); // Enable RGMII Tx clock delay
    athrs17_phy_debug_write(mdev, S17_PHY4_ADDR, 0x5, dummy);
#endif

    /*
     * Setup per port based settings
     *
     * enable HOL by default
     * Setup Control 0 and 1 for each MAC 0-6
     * Control 0:
     * MAC0: 0x0970 : 0x2a666666 Egress priority 6, eg_portq 0x2a
     * MAC1: 0x0978 : 0x2a008888 Egress priority 8, eg_portq 0x2a
     * MAC2: 0x0980 : 0x2a008888 Egress priority 8, eg_portq 0x2a
     * MAC3: 0x0988 : 0x2a008888 Egress priority 8, eg_portq 0x2a
     * MAC4: 0x0990 : 0x2a008888 Egress priority 8, eg_portq 0x2a
     * MAC5: 0x0998 : 0x2a666666 Egress priority 6, eg_portq 0x2a
     * MAC6: 0x09A0 : 0x2a666666 Egress priority 6, eg_portq 0x2a
     *
     * Control 1:
     * MACs 0-7 0x0974, 0x097C, 0x0984, 0x098C, 0x0994, 0x099C, 0x09A4
     * All Set to 0x000000C6
     */
    for (addr = 0; addr < S17_MAC_MAX; addr++)
    {
#ifdef C2K
        switch (addr) {
            case 0:
            case 5:
            case 6:
                athrs17_reg_write(mdev,
                                  S17_PORT0_HOL_CTRL0 + addr * 8,
                                  S17_HOL_CTRL0_WAN /* 0x2a666666 Egress priority 6, eg_portq 0x2a */
                                 );
                break;

            default:
                athrs17_reg_write(mdev,
                                  S17_PORT0_HOL_CTRL0 + addr * 8,
                                  S17_HOL_CTRL0_LAN /* 0x2a008888 Egress priority 8, eg_portq 0x2a */
                                 );
                break;
        }
#else
        //
        // T2K Skip setting of HOL for non-existant MAC ports 0 and 5
        //
        switch (addr) {
            case 0:
            case 5:
                //
                // Set port lookup control off for unused/unconnected port
                //
                athrs17_reg_write(mdev,
                                  S17_LOOKUP_CTRL_REG(addr),
                                  0
                                 );

                break;

            default:
                //
                // Set port lookup control to default dumb switch port based VLAN for T2K
                //
                athrs17_reg_write(mdev,
                                  S17_LOOKUP_CTRL_REG(addr),
                                  S17_LOOKUP_SET_PORT_DEFAULT(addr)
                                 );

                athrs17_reg_write(mdev,
                                  S17_PORT0_HOL_CTRL0 + addr * 8,
                                  S17_HOL_CTRL0_LAN /* 0x2a008888 Egress priority 8, eg_portq 0x2a */
                                 );
                break;
        }
#endif
        athrs17_reg_write(mdev,
                          S17_PORT0_HOL_CTRL1 + addr * 8,
                          0xC6
                         );
    }

    /* LED control */
    athrs17_reg_write(mdev, S17_LED_CTRL0_REG, 0xffb7ffb7); /* 0x0050: 0xFFB7FFB7 */
    athrs17_reg_write(mdev, S17_LED_CTRL1_REG, 0xffb7ffb7); /* 0x0054: 0xFFB7FFB7 */
    athrs17_reg_write(mdev, S17_LED_CTRL2_REG, 0xffb7ffb7); /* 0x0058: 0xFFB7FFB7 */

    printf ("%s:done\n",__func__);

#ifndef C2K
    //
    // For T2K, go ahead and bringup the PHY ports now as well as the default state
    // we want is all LAN ports enabled, including MAC6 going to the CPU
    //
    //athrs17_phy_setup(mdev, S17_PHY0_ADDR); // Indicate PHY0 to setup function, this will setup PHYs 0-3
    //
    // Print out PHY status
    //
    athrs17_phy_stat(mdev);
#if 0
    //
    // TEMP FOR T2K DEBUG (AND EXAMPLE CODE TO ALLOW LOOPBACK ON/OFF IN API, SET RGMII PORT IN LOOPBACK STATE
    //
    athrs17_reg_write(mdev,
                      S17_LOOKUP_CTRL_REG(6),
                      (S17_LOOKUP_PORT_LOOPBACK_EN | S17_LOOKUP_PORT_STATE_FORWARDING)
                     );

#endif
#endif
#endif // #if 1 for forcing simpler code based on EEPROM
    //
    // Initialization completed, always return 0
    //
    return 0;

}

/**
 *
 * @brief athrs17_phy_is_link_alive - test to see if the specified link is alive
 *
 * @return RETURNS:
 *    TRUE  --> link is alive
 *    FALSE --> link is down
 */
BOOL
athrs17_phy_is_link_alive(struct mii_device *mdev,int phyUnit)
{
    uint16_t phyHwStatus;
    uint32_t phyAddr;

    phyAddr = S17_PHYADDR(phyUnit);

    phyHwStatus = mdev->read(mdev, phyAddr, S17_PHY_SPEC_STATUS);

    if (phyHwStatus & S17_STATUS_LINK_PASS)
    {
#ifdef DEBUG_ATHEROS_SWITCH
        printf("phy%u link up\n",phyUnit);
#endif
        return TRUE;
    }

    printf("phy%u link down\n",phyUnit);
    return FALSE;
}

/**
 *
 * @brief Function to read and print the status of all PHY ports that are active
 *
 */

int
athrs17_phy_stat(struct mii_device *mdev)
{
    uint16_t  phyHwStatus;
    uint32_t  phyAddr;
    int       phyUnit;
    int       ii = 200;

    for (phyUnit=0; phyUnit < S17_PHY_MAX; phyUnit++) {

        phyAddr = S17_PHYADDR(phyUnit);

        if (athrs17_phy_is_link_alive(mdev, phyUnit)) {

            do {
                   phyHwStatus = mdev->read(mdev, phyAddr,
                                            S17_PHY_SPEC_STATUS);

                   if(phyHwStatus & S17_STATUS_RESOLVED)
                       break;
                   mdelay(10);
            }while(--ii);

            phyHwStatus = ((phyHwStatus & S17_STATUS_LINK_MASK) >>
                           S17_STATUS_LINK_SHIFT);

            printf("phy%d phyhwstat:%x\n",phyUnit,phyHwStatus);
        }

    }
    return 0;
}

/**
 *
 * @brief athrs17_phy_init - reset and setup the PHY associated
 *
 * Resets the associated PHY port.
 *
 * @return RETURNS:
 *    TRUE  --> associated PHY is alive
 *    FALSE --> no LINKs on this ethernet unit
 */

int
athrs17_phy_init(struct mii_device *mdev, int phyUnit)
{
    uint16_t  phyHwStatus;
    int       timeout;
    int       liveLinks = 0;
    uint32_t  phyBase   = 0;
    uint32_t  phyAddr   = 0;

    printf("athrs17_phy_init %d\n", phyUnit);

    phyBase = S17_PHYBASE(phyUnit);
    phyAddr = S17_PHYADDR(phyUnit);

    mdev->write(mdev, phyAddr, S17_AUTONEG_ADVERT,
                S17_ADVERTISE_ALL);

    mdev->write(mdev, phyAddr, S17_1000BASET_CONTROL,
                S17_ADVERTISE_1000FULL);

    /* Reset PHYs*/
    mdev->write(mdev,
                phyAddr,
                S17_PHY_CONTROL,
                S17_CTRL_AUTONEGOTIATION_ENABLE
              | S17_CTRL_SOFTWARE_RESET
               );

    /*
     * After the phy is reset, it takes a little while before
     * it can respond properly.
     */
    //FIXME: Now since this function is called multiple time for each LAN PHY the overrall delay
    //will also increase. Maybe this is not required as the phy status is being check
    //So this is subjected to change during the bringup
    mdelay(5000);


    /*
     * Wait up to 3 seconds for ALL associated PHYs to finish
     * autonegotiation.  The only way we get out of here sooner is
     * if ALL PHYs are connected AND finish autonegotiation.
     */

    timeout=20;
    for (;;) {
        phyHwStatus = mdev->read(mdev, phyAddr, S17_PHY_CONTROL);

        if (S17_RESET_DONE(phyHwStatus)) {
            printf("Port %d, Negotiation Success\n", phyUnit);

            break;
        }
        if (--timeout <= 0) {
            printf("Port %d, Negogiation timeout\n", phyUnit);
            break;
        }

        mdelay(150);
    }

#ifdef C2K
    /*
     * The PHY have had adequate time to autonegotiate.
     * Now initialize software status.
     *
     * It's possible that some ports may take a bit longer
     * to autonegotiate; but we can't wait forever.  They'll
     * get noticed by mv_phyCheckStatusChange during regular
     * polling activities.
     */
    /* fine-tune PHY 0 and PHY 1*/
    if ((phyUnit == 0) || (phyUnit == 1))
    {
        mdev->write(mdev, S17_PHYADDR(phyUnit), S17_MMD_CTRL_REG, 0x3);
        mdev->write(mdev, S17_PHYADDR(phyUnit), S17_MMD_DATA_REG, 0x8007);
        mdev->write(mdev, S17_PHYADDR(phyUnit), S17_MMD_CTRL_REG, 0x4003);
        mdev->write(mdev, S17_PHYADDR(phyUnit), S17_MMD_DATA_REG, 0x8315);
    }

    /* fine-tune PHYs */
    mdev->write(mdev, S17_PHYADDR(phyUnit), S17_MMD_CTRL_REG, 0x3);
    mdev->write(mdev, S17_PHYADDR(phyUnit), S17_MMD_DATA_REG, 0x800d);
    mdev->write(mdev, S17_PHYADDR(phyUnit), S17_MMD_CTRL_REG, 0x4003);
    mdev->write(mdev, S17_PHYADDR(phyUnit), S17_MMD_DATA_REG, 0x103f);

    athrs17_phy_debug_write(mdev, S17_PHYADDR(phyUnit), 0x3d, 0x6860);

    /* for PHY4, QCA */
    if (phyUnit == S17_PHY4_ADDR)
    {
        athrs17_phy_debug_write(mdev, S17_PHYADDR(phyUnit), 0x12, 0x4c0c);
        athrs17_phy_debug_write(mdev, S17_PHYADDR(phyUnit), 0x0, 0x82ee);
        athrs17_phy_debug_write(mdev, S17_PHYADDR(phyUnit), 0x5, 0x3d46);
        athrs17_phy_debug_write(mdev, S17_PHYADDR(phyUnit), 0xb, 0xbc20);
    }
#endif

    if (athrs17_phy_is_link_alive(mdev, phyUnit)) {
#ifdef DEBUG_ATHEROS_SWITCH
        printf("Phy Init %u is alive\n",phyUnit);
#endif
        liveLinks++;
        S17_IS_PHY_ALIVE(phyUnit) = TRUE;
    } else {
#ifdef DEBUG_ATHEROS_SWITCH
        printf("Phy Init %u is not alive\n",phyUnit);
#endif
        S17_IS_PHY_ALIVE(phyUnit) = FALSE;
    }

#ifdef DEBUG_ATHEROS_SWITCH
      printf("eth%d: Phy Specific Status=%4.4x\n", phyUnit, mdev->read(mdev, S17_PHYADDR(phyUnit),S17_PHY_SPEC_STATUS));
//    printk("Phy setup Complete\n");
#endif

    return S17_IS_PHY_ALIVE(phyUnit);


}

int
athrs17_phy_setup(struct mii_device *mdev, int phyUnit)
{
    int i;

    if(phyUnit == S17_PHY4_ADDR)
    {//WAN PHY
            athrs17_phy_init(mdev, phyUnit);
    }
    else if(phyUnit < S17_PHY4_ADDR)
    {//LAN PHYs
            for(i = 0 ; i <S17_PHY4_ADDR; i++)
            {
                athrs17_phy_init(mdev, i);
            }
    }
    return TRUE;
}


unsigned int athrs17_reg_read(struct mii_device *mdev, unsigned int reg_addr)
{
    uint32_t reg_word_addr;
    uint32_t phy_addr, tmp_val, reg_val;
    uint16_t phy_val;
    uint8_t  phy_reg;

    /* change reg_addr to 16-bit word address, 32-bit aligned */
    reg_word_addr = (reg_addr & 0xfffffffc) >> 1;

    /* configure register high address */
    phy_addr = 0x18;
    phy_reg  = 0x0;
    phy_val  = (uint16_t) ((reg_word_addr >> 8) & 0x3ff);  /* bit16-8 of reg address */
    mdev->write(mdev, phy_addr, phy_reg, phy_val);

    /* For some registers such as MIBs, since it is read/clear, we should */
    /* read the lower 16-bit register then the higher one */

    /* read register in lower address */
    phy_addr = 0x10 | ((reg_word_addr >> 5) & 0x7); /* bit7-5 of reg address */
    phy_reg  = (uint8_t) (reg_word_addr & 0x1f);   /* bit4-0 of reg address */
    reg_val  = (uint32_t) mdev->read(mdev, phy_addr, phy_reg);

    /* read register in higher address */
    reg_word_addr++;
    phy_addr = 0x10 | ((reg_word_addr >> 5) & 0x7); /* bit7-5 of reg address */
    phy_reg  = (uint8_t) (reg_word_addr & 0x1f);   /* bit4-0 of reg address */
    tmp_val  = (uint32_t) mdev->read(mdev, phy_addr, phy_reg);
    reg_val |= (tmp_val << 16);

#ifdef DEBUG_ATHEROS_SWITCH
    printf("%s:  reg 0x%4.4X val 0x%8.8X\n",__func__, reg_addr, reg_val);
#endif
    return reg_val;
}

void athrs17_reg_write(struct mii_device *mdev, unsigned int reg_addr, unsigned int reg_val)
{
    uint32_t reg_word_addr;
    uint32_t phy_addr;
    uint16_t phy_val;
    uint8_t  phy_reg;

#ifdef DEBUG_ATHEROS_SWITCH
    printf("%s: reg 0x%4.4X val 0x%8.8X\n",__func__, reg_addr, reg_val);
    mdelay(10); // Delay for print to complete
#endif

    /* change reg_addr to 16-bit word address, 32-bit aligned */
    reg_word_addr = (reg_addr & 0xfffffffc) >> 1;

    /* configure register high address */
    phy_addr = 0x18;
    phy_reg  = 0x0;
    phy_val  = (uint16_t) ((reg_word_addr >> 8) & 0x3ff);  /* bit16-8 of reg address */
    mdev->write(mdev, phy_addr, phy_reg, phy_val);

    /* For S17 registers such as ARL and VLAN, since they include BUSY bit */
    /* in higher address, we should write the lower 16-bit register then the */
    /* higher one */

    /* write register in lower address */
    phy_addr = 0x10 | ((reg_word_addr >> 5) & 0x7); /* bit7-5 of reg address */
    phy_reg  = (uint8_t) (reg_word_addr & 0x1f);   /* bit4-0 of reg address */
    phy_val  = (uint16_t) (reg_val & 0xffff);
    mdev->write(mdev, phy_addr, phy_reg, phy_val);

    /* read register in higher address */
    reg_word_addr++;
    phy_addr = 0x10 | ((reg_word_addr >> 5) & 0x7); /* bit7-5 of reg address */
    phy_reg  = (uint8_t) (reg_word_addr & 0x1f);   /* bit4-0 of reg address */
    phy_val  = (uint16_t) ((reg_val >> 16) & 0xffff);
    mdev->write(mdev, phy_addr, phy_reg, phy_val);

}

unsigned int athrs17_phy_debug_read(struct mii_device *mdev, unsigned int phy_addr, unsigned int reg_addr)
{
    uint32_t reg_val;
    mdev->write(mdev, phy_addr, S17_DEBUG_PORT_ADDRESS, reg_addr);
    reg_val = mdev->read(mdev, phy_addr, S17_DEBUG_PORT_DATA);

#ifdef DEBUG_ATHEROS_SWITCH
    printf("%s:  phy_addr 0x%4.4X reg 0x%4.4X val 0x%4.4X\n",__func__, phy_addr, reg_addr, reg_val);
    mdelay(10); // Delay for print to complete
#endif

    return (unsigned int) reg_val;
}
void athrs17_phy_debug_write(struct mii_device *mdev, unsigned int phy_addr, unsigned int reg_addr, unsigned int reg_val)
{
#ifdef DEBUG_ATHEROS_SWITCH
    printf("%s: phy_addr 0x%4.4X reg 0x%4.4X val 0x%4.4X\n",__func__, phy_addr, reg_addr, (uint16_t)reg_val);
    mdelay(10); // Delay for print to complete
#endif
    mdev->write(mdev, phy_addr, S17_DEBUG_PORT_ADDRESS, reg_addr);
    mdev->write(mdev, phy_addr, S17_DEBUG_PORT_DATA,    reg_val);
}

void DumpAtherosStatistics(struct mii_device *mdev, int MacPortNumber)
{
    int i;
    unsigned int val;
    unsigned int reg;

    printf("Atheros MIB statistics port %u:",MacPortNumber);

    for (i=0; i<0xA8; i+=4)
    {
        reg = S17_MIB_PORT(MacPortNumber) + i;
        if (i % 0xF == 0)
        {
            printf("\nReg 0x%4.4X:",reg);
        }
        val = athrs17_reg_read(mdev, reg);
        printf(" %8.8X",val);
    }
    printf("\n");
}


/**
    @brief Configure Atheros swith for T2200 EVM revision 2.
*/
uint32_t CfgAtherosSwitch(struct mii_bus * bus)
{
    athrs17_reg_write(bus, 0x0010, 0x40000000);  // global Power up register to 8237 (148 pin package) mode
    athrs17_reg_write(bus, 0x0004, 0x07800000);  // MAC0 for RGMII operation into switch 2.1 ns TX delay for MAC
    athrs17_reg_write(bus, 0x0008, 0x00000000);  // MAC5 used by embedded phy in switch, set all bits to zero to disable use by RGMII/SGMII/etc.
    athrs17_reg_write(bus, 0x000C, 0x01000000);  // MAC6 not used, set all bits to zero, except RGMII delay bit 24 which is needed to set MAC 0 delay
    athrs17_reg_write(bus, 0x0624, 0x3f3f3f3f);  // for forwarding to MACs 0-5 (0: RGMII and 1-5: PHY ports, MACs 6 RGMII port disabled 0x3F3F3F3F
    athrs17_reg_write(bus, 0x007C, 0x0000007E);  // Setup MAC 0 for RGMII connection to Transcede, Fixed Gigabit speed, full duplex, no flow control
    athrs17_reg_write(bus, 0x0080, 0x00000230);  // Setup MACs 1-5 as operational for actual PHY port with auto-neg turned on, no flow control
    athrs17_reg_write(bus, 0x0084, 0x00000230);
    athrs17_reg_write(bus, 0x0088, 0x00000230);
    athrs17_reg_write(bus, 0x008C, 0x00000230);
    athrs17_reg_write(bus, 0x0090, 0x00000230);
    athrs17_reg_write(bus, 0x0094, 0x00000000); // Setup MAC 6 as disabled (not connected to anything)

    athrs17_reg_write(bus, 0x0030, 0x00000001);  // For debug purposes, enable MIB counters in Module enable register
    // Leave priority as default for now (no register writes)
    // Leave LEDs as default for now (no register writes)
    // Writing 0 to 0x000C will make RGMII to stop working (used for test purposes)

    return 0;

}




#ifdef LINUX
EXPORT_SYMBOL(athrs17_init)
EXPORT_SYMBOL(athrs17_phy_setup)
EXPORT_SYMBOL(athrs17_phy_stat)
EXPORT_SYMBOL(athrs17_reg_read)
EXPORT_SYMBOL(athrs17_reg_write)
EXPORT_SYMBOL(athrs17_phy_debug_read)
EXPORT_SYMBOL(athrs17_phy_debug_write)
#endif
