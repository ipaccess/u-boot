/**
 * @file transcede_gem.c
 *
 * @brief Comcerto/Transcede Gigabit Ethernet controller GEMAC driver
 *
 * @copyright Intel Corporation
 * @par From TSEC.c
 * This software may be used and distributed according to the
 * terms of the GNU Public License, Version 2, incorporated
 * herein by reference.
 *
 * @par
 * Copyright 2004 Freescale Semiconductor.
 * (C) Copyright 2003, Motorola, Inc.
 * author Andy Fleming
 *
 * @par
 * Intel modifications copyright
 * Modifications from original TSEC.c 
 *
 * @par
 * license GPL Version 2
 *
 */

#include <common.h>
#include <config.h>
//#include <asm/arch/hardware.h>
#include <malloc.h>
#include <net.h>
#include <command.h>
#include <miiphy.h>

#if defined(CONFIG_COMCERTO_GEMAC)
#include "transcede_gem.h"
#ifdef CONFIG_T2K
#include <asm/arch/serdes.h>
#endif

#if defined(CONFIG_T2K)
#ifndef CONFIG_GT_BOARD
#include <asm/arch/mdio_bitbang.h>
#include <asm/arch/atherosdrv.h>
#include <asm/arch/rtl8363sb.h>
#endif
#include <asm/arch/ar8328.h>
#endif

#if 0
#define MDIO_DEBUG
#endif

DECLARE_GLOBAL_DATA_PTR;

static struct gemac_info_struct gemac_info[] = {
	{GEMAC_BASEADDR,
	 GEMAC0_PHY_ADDR,
	 GEMAC0_CONFIG,
	 GEMAC0_MODE,
	 GEMAC0_PHY_FLAGS,
	 GEMAC0_PHYIDX
	},
#ifdef CONFIG_T2K
	{GEMAC1_BASEADDR,
	 GEMAC1_PHY_ADDR,
	 GEMAC1_CONFIG,
	 GEMAC1_MODE,
	 GEMAC1_PHY_FLAGS,
	 GEMAC1_PHYIDX
	},
#endif
};

#define MAX_GEMACS	2

static struct gemac_dev *gemac_list[MAX_GEMACS];

#define NUM_RX_DESC		16
#define MAX_RX_BUFF_SIZE	2048

static u32 rx_next;

#ifdef CONFIG_T2K
#define PUT_DESC2IRAM
#undef PUT_DESC_DATA2CRAM
#else
#undef PUT_DESC2IRAM
#undef PUT_DESC_DATA2CRAM
#endif

#ifdef PUT_DESC_DATA2CRAM
#define DESC_ALLOC_0		0xf403ff00
#define DESC_ALLOC_1		(DESC_ALLOC_0 + sizeof(struct rx_desc) * NUM_RX_DESC)
#define DESC_ALLOC_2		(DESC_ALLOC_1 + sizeof(struct tx_desc) * 2)
#define DESC_ALLOC_3		(DESC_ALLOC_2 + sizeof(struct rx_desc))
static struct rx_desc *rx_ring = (struct rx_desc *)DESC_ALLOC_0;
static struct tx_desc *tx_fdesc = (struct tx_desc *)DESC_ALLOC_1;
static struct rx_desc *rx_ring_fake = (struct rx_desc *)DESC_ALLOC_2;
static struct tx_desc *tx_ring_fake = (struct tx_desc *)DESC_ALLOC_3;
#else
static struct rx_desc rx_ring[NUM_RX_DESC] __attribute((aligned(128)));
static struct tx_desc tx_fdesc[2] __attribute((aligned(128)));
static struct rx_desc rx_ring_fake[1] __attribute((aligned(128)));
static struct tx_desc tx_ring_fake[1] __attribute((aligned(128)));
#endif

#ifdef PUT_DESC2IRAM
// #define BUFFER_ALLOC_0		(DESC_ALLOC_3 + sizeof(struct tx_desc))
#define BUFFER_ALLOC_0		0xf3010000
#define BUFFER_ALLOC_1		(BUFFER_ALLOC_0 + sizeof(u8) * NUM_RX_DESC * MAX_RX_BUFF_SIZE)
static u8 *rx_ring_data_buff = (u8 *)BUFFER_ALLOC_0;
#else
static u8 rx_ring_data_buff[NUM_RX_DESC * MAX_RX_BUFF_SIZE] __attribute((aligned(128)));
#endif

#ifndef CONFIG_GT_BOARD
#if defined(CONFIG_T2K)
// For Atheros driver, provide Linux like
// MDIO bus structure
struct mii_bus     GemMdioBusData;
#endif
#endif

#if !defined(CONFIG_T2K)
static void default_speed_duplex(struct gemac_dev *gemac, int *speed, int *duplex)
{
  /* Test if SGMII, or if "NO PHY" option is set,
   * if so, Trancede devices only support 1 gigabit full on that interface
   */
  if ((gemac->mode & GEMAC_GEM_MODE_SGMII) || (gemac->phyflags & GEMAC_NO_PHY)) {
	*speed = _1000BASET;
	*duplex = FULL;
	return;
  }

  /* Not SGMII (RGMII), set default speed based on config if set for SW configured speed */
  if (gemac->mode & GEMAC_SW_CONF_SPEED) {
	switch (gemac->mode & GEM_CONF_SPEED_MASK) {
		case GEM_CONF_SPEED_GEM_10M:
			*speed = _10BASET;
			break;

		case GEM_CONF_SPEED_GEM_100M:
			*speed = _100BASET;
			break;

		case GEM_CONF_SPEED_GEM_1G:
		default:
			*speed = _1000BASET;
			break;
	}
  } else {
	/* Not SW configured speed, assume default of gigabit Ethernet */
	*speed = _1000BASET;
  }
  if (gemac->mode & GEMAC_SW_CONF_DUPLEX) {
	if (gemac->mode & GEM_CONF_DUPLEX_GEM_FULL)
		*duplex = FULL;
	else
		*duplex = HALF;
  }
  else {
	*duplex = FULL;
  }
}
#endif /* if !defined(CONFIG_T2K) */

/*! @brief Initialize device structure. 
 *
 * @returns success if PHY initialization succeeded 
 * (i.e. if it recognizes the PHY)
 */
static void gemt_reinit(struct gemac_dev *gemac, int speed, int duplex)
{
  /* Test if SGMII, if so, Trancede devices only support 1 gigabit full on that interface,
   * so leave SGMII alone when requested to reinitialize
   *
   * Also test for if phyflags is set to "NO PHY" option, if so
   * we leave the GEM alone and set to as originally configured
   */
  if ((gemac->mode & GEMAC_GEM_MODE_SGMII) || (gemac->phyflags & GEMAC_NO_PHY))
	return;

  /* Have seen cases where when there is no PHY (e.g. Switch) and/or
   * no link, speed requested set is 10/Half.
   * 10/Half not supported, so force speed and duplex to 1000/FULL
   */
  if ((speed == _10BASET) && (duplex == HALF)) {
		speed  = _1000BASET;
		duplex = FULL;
  }

  /* Not SGMII (RGMII) reinitialize speed based on value passed */
  printf("Reinit PHY Speed:");
  if (gemac->mode & GEMAC_SW_CONF_SPEED) {
	switch (speed) {
	case _1000BASET:
		printf("1000Mbit ");
		gem_set_speed(gemac, SPEED_1000M);
		break;

	case _100BASET:
		printf("100Mbit ");
		gem_set_speed(gemac, SPEED_100M);
		break;

	default:
	case _10BASET:
		printf("10Mbit ");
		gem_set_speed(gemac, SPEED_10M);
		break;
	}
  }
  else
  {
	*(volatile u32 *)(gemac->baseaddr + GEM_CFG) &= ~GEMAC_SW_CONF_SPEED;
	printf("Auto ");
  }
  printf("Duplex:");
  if (gemac->mode & GEMAC_SW_CONF_DUPLEX)
  {

	switch (duplex) {
	case FULL:
		printf("Full\n");
		gem_full_duplex(gemac);
		break;

	default:
	case HALF:
		printf("Half\n");
		gem_half_duplex(gemac);
		break;
	}
  }
  else
  {
	*(volatile u32 *)(gemac->baseaddr + GEM_CFG) &= ~GEMAC_SW_CONF_DUPLEX;
	printf("Auto\n");
  }
}

#if 1 // defined(CONFIG_MII) || (CONFIG_COMMANDS & CFG_CMD_MII)

static int gemt_PHY_timeout(struct gemac_dev *gemac, int timeout)
{
	while (!gem_phy_man_idle(gemac)) {

		if (timeout-- <= 0) {
			//			printf("PHY MDIO read/write timeout\n");
			return -1;
		}
#ifndef	CONFIG_RTSM_ONLY
		udelay(1);
#endif
	}

	return 0;
}

struct gemac_dev *get_gemac(char *devname)
{
	int i;

	for (i = 0; i < MAX_GEMACS; i++) {
		if (!strcmp(gemac_list[i]->dev->name, devname))
			return gemac_list[i];
	}

	return NULL;
}

/*! @brief Function to perform a U-boot compliant MII write function */
static int transcede_miiphy_write(char *         devname, /**< Device driver name   */
                                  unsigned char  addr,    /**< MDIO PHY address     */
                                  unsigned char  reg,     /**< PHY register address */
                                  unsigned short value    /**< Value to write       */
                                 )
{
	struct gemac_dev *gemac = get_gemac(devname);

	if (!gemac) {
		printf("%s: Unknown device %s\n", __func__, devname);
		return -1;
	}

	if ((addr > 31) || (reg > 31))
		return -1;

	gem_phy_man_wr(gemac, addr, reg, value);
	if (gemt_PHY_timeout(gemac, MDIO_TIMEOUT))
		return -1;

#ifdef MDIO_DEBUG
    printf("%s: %s: Reg: 0x%02X, Addr:0x%02X, Val:0x%04X\n",
           __func__,
           devname,
           addr,
           reg,
           value
          );
#endif

	return 0;
}

/*! @brief Function to perform a U-boot compliant MII read function */
static int transcede_miiphy_read(char *           devname, /**< Device driver name   */
                                 unsigned char    addr,    /**< MDIO PHY address     */
                                 unsigned char    reg,     /**< PHY register address */
                                 unsigned short * value    /**< Ponter to variable to store read data to */
                                )
{
	struct gemac_dev *gemac = get_gemac(devname);

    *value = 0xFFFF; // Force bad value initially in case read fails for some reason

	if (!gemac) {
		printf("%s: Unknown device %s\n", __func__, devname);
		return -1;
	}

	if ((addr > 31) || (reg > 31))
		return -1;

	gem_phy_man_rd(gemac, addr, reg);
	if (gemt_PHY_timeout(gemac, MDIO_TIMEOUT))
		return -1;

	*value = gem_phy_man_data(gemac);

#ifdef MDIO_DEBUG
    printf("%s:  %s: Reg: 0x%02X, Addr:0x%02X, Val:0x%04X\n",
           __func__,
           devname,
           addr,
           reg,
           *value
          );
#endif


	return 0;
}
#ifndef CONFIG_GT_BOARD
#if defined(CONFIG_T2K)

/*! @brief Function to change device flags based on name of GEM device
 *         passed and new flag values
 */
int gem_set_phy_flags(char *   devname,      /**< Device driver name       */
                      unsigned phy_flags     /**< New PHY flags to set     */
                     )
{
	struct gemac_dev *gemac = get_gemac(devname);

	if (!gemac) {
		printf("%s: Unknown device %s\n", __func__, devname);
		return -1;
	}
    gemac->phyflags = phy_flags;

	return 0;
}

/** @brief Linux like MDIO read function for GEM */
int gem_mdio_read(struct mii_bus *bus, int phy, int reg)
{
    unsigned short ret;

    if (transcede_miiphy_read("gemac0", phy, reg, &ret) == 0)
    {
        return (int) ret;
    }
    else
    {
        return(0xFFFF);
    }
}

/** @brief Linux like MDIO write function for GEM */
int gem_mdio_write(struct mii_bus *bus, int phy, int reg, u16 val)
{
    int ret;

    ret = transcede_miiphy_write("gemac0", phy, reg, val);

    return(ret);
}

/** @brief Linux like MDIO alloc function for GEM */
struct mii_bus *alloc_gem_mdio(struct mdiobb_ctrl *ctrl)
{

    struct mii_bus * bus;

#ifdef LINUX
    bus = mdiobus_alloc();
    if (!bus)
        return NULL;

    __module_get(ctrl->ops->owner);

    bus->read = mdiobb_read;
    bus->write = mdiobb_write;
    bus->priv = ctrl;

#else
    bus            = &GemMdioBusData;
    bus->priv      = NULL;
    bus->read      = &gem_mdio_read;
    bus->write     = &gem_mdio_write;
#endif

    return bus;
}

/** @brief Linux like MDIO free function for GEM */
void free_gem_mdio(struct mii_bus *bus)
{
#ifdef LINUX
    struct mdiobb_ctrl *ctrl = bus->priv;

    module_put(ctrl->ops->owner);
    mdiobus_free(bus);
#endif
}

#endif /* if defined(CONFIG_T2K) */
#else /*ifndef CONFIG_GT_BOARD*/
/*=============================================================================
 *
 * NAME: gemt_config_PHY
 *
 * PARAMETERS:
 *   net_device *dev    -INOUT
 *
 * DESCRIPTION
 *   Reconfigure PHY
 *   This function will set up the PHY device.  This is a required external
 *   support routine.
 *   The parameters set up the maximum desired advertised ability for the
 *   device.
 *   TODO - read back negotiated ability and set MAC appropriately.
 *          duplex configuration to be done.
 *
 * ASSUMPTIONS
 *   None
 *
 * SIDE EFFECTS / STATIC DATA USE
 *   None
 *
 * RETURNS:
 *   int        0       -successful
 *              <0      -failed
 *
 *===========================================================================*/
static int gemt_config_PHY(struct gemac_dev *gemac, int phy_addr, MAC_SPEED speed, u8 duplex)
{
#ifndef CONFIG_TURNER
	unsigned short anar; /* Value for Auto Negotiation Advertising Register */
	unsigned short bmcr; /* Value for Basic Management Control Register */
	unsigned short btcr; /* Value for BaseT Control register */
#endif

	/* Test if SGMII, if so, Trancede devices only support 1 gigabit full on that interface,
	 * so leave SGMII alone when requested to reinitialize
	 */
	if (gemac->mode & GEMAC_GEM_MODE_SGMII)
		return 0;

	/* Reset the PHY device, return if error */
	if (miiphy_reset(gemac->dev->name, gemac->phyaddr))
		return -1;

#ifndef CONFIG_TURNER
	/* PHY has been reset, figure out register settings to send
	 * to PHY based on speed
	 */
	switch (speed) {
		case _10BASET:
			if (duplex == FULL) {
				anar = PHY_ANAR_10FD | PHY_ANAR_10;
				btcr = 0x0;
				bmcr = PHY_BMCR_DPLX | PHY_BMCR_10_MBPS;
			} else {
				anar = PHY_ANAR_10;
				btcr = 0x0;
				bmcr = PHY_BMCR_10_MBPS;
			}

			break;

		case _100BASET:
		default:
			if (duplex == FULL) {
				anar = PHY_ANAR_TXFD | PHY_ANAR_TX | PHY_ANAR_10FD | PHY_ANAR_10;
				btcr = 0x0;
				bmcr = PHY_BMCR_DPLX | PHY_BMCR_100_MBPS;
			} else {
				anar = PHY_ANAR_TX | PHY_ANAR_10;
				btcr = 0x0;
				bmcr = PHY_BMCR_100_MBPS;
			}

			break;

		case _1000BASET:
			if (duplex == FULL) {
				anar = PHY_ANAR_TXFD | PHY_ANAR_TX | PHY_ANAR_10FD | PHY_ANAR_10;
				btcr = PHY_1000BTCR_1000FD | PHY_1000BTCR_1000HD;
				bmcr = PHY_BMCR_DPLX | PHY_BMCR_1000_MBPS;
			} else {
				anar = PHY_ANAR_TX | PHY_ANAR_10;
				btcr = PHY_1000BTCR_1000HD;
				bmcr = PHY_BMCR_1000_MBPS;
			}

			break;
	}

	/* Test if PHY flag options setup for PHY autonegotiation or not.
	 * Based on that, write to standard PHY registers to
	 * setup speed
	 */

	if (gemac->phyflags & GEMAC_PHY_AUTONEG) {
		if (miiphy_write(gemac->dev->name, phy_addr, PHY_ANAR, anar | PHY_ANAR_PSB_802_3))
			return -1;

		if (miiphy_supports_1000base_t(gemac->dev->name, phy_addr)) {
			if (miiphy_write(gemac->dev->name, phy_addr, PHY_1000BTCR, btcr))
				return -1;
		}

		if (miiphy_write(gemac->dev->name, phy_addr, PHY_BMCR, PHY_BMCR_AUTON | PHY_BMCR_RST_NEG | bmcr))
			return -1;
	} else {
		if (miiphy_write(gemac->dev->name, phy_addr, PHY_BMCR, bmcr))
			return -1;
	}
#endif
	return 0;
}
#endif /*ifndef CONFIG_GT_BOARD*/


#endif /* defined(CONFIG_MII) || (CONFIG_COMMANDS & CFG_CMD_MII) */

/*! 
 *
 * @brief NAME: gemt_reset_gem, Reset MAC Hardware
 *
 * PARAMETERS: \n
 *   net_device *dev    -INOUT
 *
 * DESCRIPTION \n
 *   Reset MAC hardware
 *   This function will reset MAC hardware after stop the MAC for both
 *   transmission and reception.
 *
 * ASSUMPTIONS \n
 *   None
 *
 * SIDE EFFECTS / STATIC DATA USE \n
 *   None
 *
 * @returns Nothing (void function)
 *
 *===========================================================================*/
static void gemt_reset_gem(struct gemac_dev *gemac)
{
	gem_abort_tx(gemac);
	gem_disable_rx(gemac);

	gem_mask_irq(gemac, GEM_IRQ_ALL);
	gem_get_irq_stat(gemac);

	if (gem_reset(gemac) != 0)
		printf("Failed to reset device!\n");
}

#if defined(CONFIG_COMCERTO_100)
static void gemac_rx_ring_init(struct gemac_dev *gemac)
{
	u32 i;
	u8 *pU8 = rx_ring_data_buff;

	for (i = 0; i < NUM_RX_DESC - 1; i++) {
		rx_ring[i].next = &(rx_ring[i + 1]);
		rx_ring[i].system = i;
		rx_ring[i].fstatus = 0;
		rx_ring[i].fcontrol = IDMA_FCONTROL_FREADY;
		rx_ring[i].bdesc.bptr = pU8;
		rx_ring[i].bdesc.bcontrol = 0;
		pU8 += MAX_RX_BUFF_SIZE;
	}

	rx_ring[i].next = &(rx_ring[0]);
	rx_ring[i].system = i;
	rx_ring[i].fstatus = 0;
	rx_ring[i].fcontrol = 0;
	rx_ring[i].bdesc.bptr = pU8;
	rx_ring[i].bdesc.bcontrol = 0;

	rx_next = 0;
}

static int gemac_send(struct eth_device *dev, volatile void *packet, int length)
{
	struct gemac_dev *gemac = (struct gemac_dev *)dev->priv;
	int i;

	if (length <= 0) {
		printf("Comcerto_Emac: bad packet size: %d\n", length);
		return (-1);
	}

	memset(&tx_fdesc, 0, sizeof(struct fdesc));

	// build the tx frame descriptor here
	tx_fdesc.fcontrol = IDMA_FCONTROL_FREADY | IDMA_FCONTROL_FLAST;
	tx_fdesc.fstatus = 0;
	tx_fdesc.bdesc.bptr = (u8 *)packet;
	tx_fdesc.bdesc.bcontrol = length | IDMA_BCONTROL_BLAST;

	// Check if DMA Stopped
	if (!(*(volatile u32 *)(gemac->idma_baseaddr + MMEM_START) & IDMA_START)) {
		*(volatile u32 *)(gemac->idma_baseaddr + MMEM_HEAD) = (u32) &tx_fdesc;
		*(volatile u32 *)(gemac->idma_baseaddr + MMEM_START) |= IDMA_START;
	} else {
		printf("Emac: tx EDMA busy!\n");
		return (-1);
	}

	i = 0;
	while ((tx_fdesc.fstatus & IDMA_FSTATUS_FRAME_DONE_MASK) == 0) {
		udelay(100);
		i++;
		if (i == 50000) {
			printf("Emac: tx timed out!\n");
			return (-1);
		}
	}

	if (*(volatile u32 *)(gemac->idma_baseaddr + MMEM_START) & IDMA_START) {
		printf("Error! Emac: tx did not stop after sending a packet!\n");
	}

	return (length);
}


static int gemac_recv(struct eth_device *dev)
{
	struct gemac_dev *gemac = (struct gemac_dev *)dev->priv;
	int rx_prev;
	int length;
	int total_length = 0;
	int bcontrol;

	// loop thru rx FDescs
	while (1) {
		// Check if there is a frame available, if not, then exit loop
		if ((rx_ring[rx_next].fstatus & IDMA_FSTATUS_FRAME_DONE_MASK) == 0)
			break;

		//
		// Frame is available
		// mark current position not usable so the frame is not overwritten
		//
		rx_ring[rx_next].fcontrol = 0;

		// Get length and check if too large
		bcontrol = rx_ring[rx_next].bdesc.bcontrol
		length   = bcontrol & 0x0000ffff;
		if (length > MAX_RX_BUFF_SIZE) {
			printf("\nGEMAC: frame too big (%d bytes)!\n", length);
			length = MAX_RX_BUFF_SIZE; // Truncate size to maximum length
		}

		//
		// Pass the packet up to the protocol layers if CRC is OK,
		// (bit 13 is 1 for CRC error) otherwise drop this packet
		// and continue
		//
		if (bcontrol & (1 << 13) == 0) {
		    NetReceive(rx_ring[rx_next].bdesc.bptr, length);
		    total_length += length;
		}

		// rx_prev can be used now
		if (rx_next == 0)
			rx_prev = NUM_RX_DESC - 1;
		else
			rx_prev = rx_next - 1;

		rx_ring[rx_prev].fstatus = 0;
		rx_ring[rx_prev].fcontrol = IDMA_FCONTROL_FREADY;

		rx_next++;
		if (rx_next == NUM_RX_DESC)
			rx_next = 0;
	}

	// Check if DMA Stopped
	// if RX is stopped, restart
	if (!(*(volatile u32 *)(gemac->idma_baseaddr + EMMM_START) & IDMA_START)) {
		*(volatile u32 *)(gemac->idma_baseaddr + EMMM_HEAD) = (u32) (&(rx_ring[rx_next]));
		*(volatile u32 *)(gemac->idma_baseaddr + EMMM_START) |= IDMA_START;
	}

	return (total_length);
}

/*! @brief Function to stop the interface */
static void gemac_halt(struct eth_device *dev)
{
	struct gemac_dev *gemac = (struct gemac_dev *)dev->priv;

	*(volatile u32 *)(gemac->idma_baseaddr + MMEM_SOFT_RESET) = 1;
	*(volatile u32 *)(gemac->idma_baseaddr + EMMM_SOFT_RESET) = 1;

	gem_stop_tx(gemac);
	gem_disable_rx(gemac);
}

#else
static void gemac_rx_ring_init(struct gemac_dev *gemac)
{
	u8 *pU8;
	int i;

//	printf("%s()\n", __func__);

	memset(rx_ring, 0, NUM_RX_DESC * sizeof(struct rx_desc));
	pU8 = rx_ring_data_buff;

	for (i = 0; i < NUM_RX_DESC - 1; i++) {
		rx_ring[i].data = (u32)pU8;
		pU8 += MAX_RX_BUFF_SIZE;
	}

	rx_ring[i].data = (u32)pU8;
#ifndef CONFIG_T2K
        rx_ring[i].status |= GEMRX_WRAP;        // last descriptor in the list
#else
	rx_ring[i].data |= GEM_RX_DESCR_WRAP;	// last descriptor in the list
#endif
	rx_next = 0;
#ifndef CONFIG_T2K
        *(volatile u32 *)(gemac->registers + GEM_RX_QPTR) = (u32)rx_ring;
#else
	*(volatile u32 *)(gemac->registers + GEM_RX_QUEUE0) = (u32)rx_ring;
#endif


#ifdef CONFIG_T2K
	/* fake queues to ensure they are not used */
	memset(rx_ring_fake, 0, 1 * sizeof(struct rx_desc));
	memset(tx_ring_fake, 0, 1 * sizeof(struct tx_desc));
	rx_ring_fake[0].data = 0 | GEM_RX_DESCR_WRAP | GEM_RX_DESCR_OWNERSHIP;
//	tx_ring_fake[0].data = 0 | GEM_TX_DESCR_USED | GEM_TX_DESCR_WRAP | GEM_TX_DESCR_LAST_BUF_BIT;
	tx_ring_fake[0].ctl = 0 | GEM_TX_DESCR_USED | GEM_TX_DESCR_WRAP;
	for (i = 1; i <= 7; i++) {
		REG32(gemac->registers + GEM_RX_PQUEUE(i)) = (u32)rx_ring_fake;
		REG32(gemac->registers + GEM_TX_PQUEUE(i)) = (u32)tx_ring_fake;
	}
#endif
}

#ifndef CONFIG_T2K
/**
 * @brief Function to send a single Ethernet frame to the GEMAC and wait for completion
 */
static int gemac_send(struct eth_device *dev, volatile void *packet, int length)
{
	struct gemac_dev *gemac = (struct gemac_dev *)dev->priv;
	int i, rc = -1;

	//
	// First test for valid positive frame length, error return if
	// zero or negative.
	//
	if (length <= 0) {
		printf("Comcerto_Emac: bad packet size: %d\n", length);
		return (-1);
	}

	//
	// build the tx frame descriptor here for starting state
	// with descriptors 0 and 1 empty and owned by CPU,
	// descriptor 0 with wrap bit.
	// Do not clear used bit until all data is ready for transmit.
	//
	memset(&tx_fdesc, 0, 2 * sizeof(struct tx_desc));

	// DD tx_fdesc[0].ctl  = TX_DESC_WORD1_USED | TX_DESC_WORD1_LAST | TX_DESC_WORD1_FCS;
	tx_fdesc[0].ctl  = TX_DESC_WORD1_LAST | TX_DESC_WORD1_FCS;
	// DD tx_fdesc[1].ctl  = TX_DESC_WORD1_USED | TX_DESC_WORD1_LAST | TX_DESC_WORD1_FCS | TX_DESC_WORD1_WRAP;
	tx_fdesc[0].data = (u32)packet;
	// DD tx_fdesc[1].data = 0;
	tx_fdesc[0].ctl |= length & TX_DESC_WORD1_LEGTH_MASK;
	tx_fdesc[1].ctl |= TX_DESC_WORD1_WRAP | TX_DESC_WORD1_USED;

	//
	// Descriptors OK now, now clear used bit to pass ownership over
	// from CPU to GEMAC
	//
	//DD tx_fdesc[0].ctl &= ~TX_DESC_WORD1_USED;
	//
	// just enable queue0, let the other queues uninitialized
	//
	*(volatile u32 *)(gemac->registers + GEM_QUEUE_BASE0) = (u32)&tx_fdesc[0];
	//
	// Start transmitter (if not already started)
	//
	gem_start_tx(gemac);

	//
	// Wait for transmission to complete
	// Used bit should go from 0 to 1 when completed by GEMAC
	//
	i = 0;
	while ((tx_fdesc[0].ctl & TX_DESC_WORD1_USED) == 0) {
		//
		// Check transmit status bits for error
		//
		u32 tx_status = *(volatile u32*)0xfe0de014;
		if (tx_status & ((1 << 4) | (1 << 2) | (1 << 1) | (1<<6) | (1<<7) | (1<<8))) {
		    //
		    // Transmit status error, restart the transmission
		    //
		    printf("restart %04X\n", tx_status);
		    *(volatile u32*)0xfe0de000 &= ~8;       // Stop the transmitter
		    *(volatile u32*)0xfe0de014 = tx_status; // Clear the error status
		    //
		    // Reinitialize the transmit descriptors
		    //
		    tx_fdesc[0].ctl  = TX_DESC_WORD1_LAST | TX_DESC_WORD1_FCS | TX_DESC_WORD1_USED;
		    tx_fdesc[1].ctl  = TX_DESC_WORD1_LAST | TX_DESC_WORD1_FCS | TX_DESC_WORD1_USED | TX_DESC_WORD1_WRAP;
		    tx_fdesc[0].data = (u32)packet;
		    tx_fdesc[1].data = 0;
		    tx_fdesc[0].ctl |= length & TX_DESC_WORD1_LEGTH_MASK;
		    tx_fdesc[1].ctl &= ~TX_DESC_WORD1_USED;

		    *(volatile u32 *)(gemac->registers + GEM_QUEUE_BASE0) = (u32)&tx_fdesc[0];
		    gem_start_tx(gemac);
		}
		udelay(100);
		i++;
		if (i == 50) {
			printf("Emac: tx timed out!\n");
			goto out;
		}
	}

	//
	// Normal return, Ethernet Frame sent OK
	//
	rc = length;
//        printf("gemac_send done \n");
	return rc;
out:
	//
	// Error return, transmit timeout, kill transmitter
	//
	gem_stop_tx(gemac);

	return rc;
}

static int gemac_recv(struct eth_device *dev)
{
        int length = 0;
        int total_length = 0;
        u32 rx_status;
        u32 rx_data;
        u32 rx_extstatus;

        // loop thru rx FDescs
        while (1) {

                rx_extstatus = rx_ring[rx_next].extstatus;
                if (((rx_extstatus & GEMRX_OWN) == 0)) {
//                      printf("done %d index %d\n",total_length, rx_next);
                        break;
                }

                rx_data = rx_ring[rx_next].data;
                rx_status = rx_ring[rx_next].status;
                length = (rx_status & RX_STA_LEN_MASK) /*>>RX_STA_LEN_POS */ ;

                // Pass the packet up to the protocol layers.
                if (!(rx_status & RX_CHECK_ERROR)) {
                        NetReceive((u8 *)rx_data, length);
                        total_length += length;
                }

                //clear bits... the buffer can be reused
                rx_ring[rx_next].status &= GEMRX_WRAP;
                rx_ring[rx_next].extstatus = 0;
                length = 0;

                rx_next++;
                if (rx_next == NUM_RX_DESC)
                        rx_next = 0;
        }

        return (total_length);
}


#else

/* T2200/T3300 send and receive functions */

static int gemac_send(struct eth_device *dev, volatile void *packet, int length)
{
	struct gemac_dev *gemac = (struct gemac_dev *)dev->priv;
	int i, rc = -1;

	if (length <= 0) {
		printf("Comcerto_Emac: bad packet size: %d\n", length);
		return (-1);
	}

	memset(tx_fdesc, 0, 2 * sizeof(struct tx_desc));

	// build the tx frame descriptor here
	tx_fdesc[0].ctl = GEM_TX_DESCR_LAST_BUF_BIT;  //FCS is generated automatically

#ifdef PUT_DESC2IRAM
	tx_fdesc->data = (u32)BUFFER_ALLOC_1;
	memcpy((void *)tx_fdesc->data, (void *)packet, length);
#else
	tx_fdesc->data = (u32)packet;
#endif

	tx_fdesc[0].ctl |= length & GEM_TX_DESCR_LEN_MASK;
	tx_fdesc[1].ctl |= GEM_TX_DESCR_WRAP | GEM_TX_DESCR_USED;

	// just enable queue0, let the other queues uninitialized
	REG32(gemac->registers + GEM_TX_QUEUE0)    = (u32)&tx_fdesc[0];

	gem_start_tx(gemac);

	i = 0;
	while ((tx_fdesc[0].ctl & GEM_TX_DESCR_USED) == 0) {
		udelay(100);
		i++;
		if (i == 50) {
			printf("Emac: tx timed out!\n");
			REG32(gemac->registers + GEM_NET_CONTROL) |= GEM_NETCTRL_HALT_TX;
			goto out;
		}
	}

	rc = length;
//        printf("gemac_send done \n");
out:
//	gem_stop_tx(gemac);

	return rc;
}


static int gemac_recv(struct eth_device *dev)
{
	//int i = 0;
	int length = 0;
	int total_length = 0;
	u32 rx_data;
	u32 rx_ctl;
	//u32 *addr;
	//volatile int p = 1;

	// loop through rx FDescs
	while (1) {
		rx_data = rx_ring[rx_next].data;
		if (((rx_data & GEM_RX_DESCR_OWNERSHIP) == 0)) {
			break;
		}
		rx_ctl = rx_ring[rx_next].ctl;

		length = (rx_ctl & GEM_TX_DESCR_LEN_MASK) /*>>RX_STA_LEN_POS */ ;

		// Pass the packet up to the protocol layers.
		if (!(rx_ctl & GEM_RX_DESCR_FCS_STAT)) {
			/*
			printf("gemac_recv: rx_next: %i, data_addr:%p, len:%i, rx_ctl: %x\n", rx_next, (void *)rx_data, length, rx_ctl);
			addr = GEM_RX_DESCR_BUF_ADDR(rx_data);
			printf("gemac_recv: data_content:\n");
			for (i = 0; i<length; i++) {
				printf("%02x ", ((u8 *)addr)[i]);
			}
			*/
			NetReceive((u8 *)GEM_RX_DESCR_BUF_ADDR(rx_data), length);
			total_length += length;
		}

		//clear bits... the buffer can be reused
		length = 0;

		rx_ring[rx_next].data &= ~GEM_RX_DESCR_OWNERSHIP;
		rx_ring[rx_next].ctl = 0;

		rx_next++;
		if (rx_next == NUM_RX_DESC)
			rx_next = 0;
	}

	return (total_length);
}
#endif // CONFIG_T2K

/* Stop the interface */
static void gemac_halt(struct eth_device *dev)
{
	struct gemac_dev *gemac = (struct gemac_dev *)dev->priv;

//	printf("%s()\n", __func__);

	gem_stop_tx(gemac);
	gem_abort_tx(gemac);
	gem_disable_rx(gemac);
}
#endif

/*! @brief Function to intialize the GEMAC
 *
 * Initializes data structures and registers for the controller,
 * and brings the interface up.	 Returns the link status, meaning
 * that it returns success if the link is up, failure otherwise.
 * This allows u-boot to find the first active controller.
 */
static int gemac_init(struct eth_device *dev, bd_t * bd)
{
	struct gemac_dev *gemac = (struct gemac_dev *)dev->priv;
	MAC_ADDR enet_addr;
	int                speed  = _1000BASET;
	int                duplex = FULL;

#ifdef MDIO_DEBUG
    printf("Begin %s: %s\n",
           __func__,
           dev->name
          );
#endif


	/* Make sure the controller is stopped */
	gemac_halt(dev);

#if defined(CONFIG_MII) || defined(CONFIG_CMD_MII)
	/* Standard U-boot MII/MDIO operations used: */

	/* Test if SGMII, if so, Trancede devices only support 1 gigabit full on that interface,
	 * so leave SGMII alone when requested to reinitialize
     *
     * Also test if there is no PHY on this GEM, if so, assume we are connected
     * to a switch or other fixed speed device and setup for Gigabit speed
     * and full duplex.
	 */
	if ((gemac->mode & GEMAC_GEM_MODE_SGMII) || (gemac->phyflags & GEMAC_NO_PHY)) {
		speed  = 1000;
		duplex = FULL;
	} else {
		/* If RGMII, Get speed and duplex from the PHY */
		speed  = miiphy_speed (gemac->dev->name, gemac->phyaddr);
		duplex = miiphy_duplex(gemac->dev->name, gemac->phyaddr);
	}

#else
	/* Standard U-boot MII/MDIO operations not used */
#if !defined(CONFIG_T2K)
	default_speed_duplex(gemac, &speed, &duplex);
#endif
#endif // #if defined(CONFIG_MII) || defined(CONFIG_CMD_MII)

#if !defined(CONFIG_T2K)
	/* T3000/T4000, use abstracted GEM API calls to setup various options */
	gem_disable_copy_all(gemac);
	gem_allow_broadcast(gemac);
	gem_enable_unicast(gemac);
	gem_disable_multicast(gemac);
	gem_disable_fcs_rx(gemac);
#else
	/* T2200/T3300, setup Network config register directly */
	if (!(gemac->mode & GEMAC_GEM_MODE_SGMII)) {
		REG32(gemac->registers + GEM_NET_CONFIG) = // 0x040224d2;
					GEM_NETCFG_MULTICAST_ENB |
					GEM_NETCFG_UNICAST_ENB |
					GEM_NETCFG_COPY_ALL |
					GEM_NETCFG_FULL_DUPLEX |
					GEM_NETCFG_1GB_MODE |
//					GEM_NETCFG_FCS_REMOVE |
					GEM_NETCFG_BUS_WIDTH_64_BIT;
	} else if (gemac->mode & GEMAC_GEM_MODE_SGMII) {
		REG32(gemac->registers + GEM_NET_CONFIG) = // 0x040224d2;
					GEM_NETCFG_MULTICAST_ENB |
					GEM_NETCFG_UNICAST_ENB |
					GEM_NETCFG_COPY_ALL |
					GEM_NETCFG_FULL_DUPLEX |
					GEM_NETCFG_1GB_MODE |
//					GEM_NETCFG_FCS_REMOVE |
					GEM_NETCFG_TBI_ENB |
#ifdef SGMII_AUTONEG_MODE
					GEM_NETCFG_SGMII_MODE |
#endif
					GEM_NETCFG_BUS_WIDTH_64_BIT;
	}
#endif

#if defined(CONFIG_MII) || defined(CONFIG_CMD_MII)
	if (!(gemac->mode & GEMAC_GEM_MODE_SGMII)) {
		gemt_reinit(gemac, speed, duplex);
	}
#endif

	gem_enet_addr_byte_mac(dev->enetaddr, &enet_addr);
	gem_set_laddr1(gemac, &enet_addr);

	gemac_rx_ring_init(gemac);

	/* Ready the device for tx/rx */
	gem_enable_rx(gemac);

#ifdef MDIO_DEBUG
    printf("End   %s: %s\n",
           __func__,
           dev->name
          );
#endif

	return (1);
}

int transcede_gemac_initialize(bd_t * bis, int index, char *devname)
{
	struct eth_device *dev;
	struct gemac_dev *gemac;
	int i;
#ifndef CONFIG_GT_BOARD
#if defined(CONFIG_T2K)
	int gem_mdio_ok         = 0;
	int gem_switch_detected = 0;
    u32                regval1=0xFFFF;
    u32                regval2=0xFFFF;
    u32                regval3=0xFFFF;
    u32                regval4=0xFFFF;
	struct mdiobb_ctrl ctrl;
	struct mii_bus *   gem_bus=NULL;
    struct mii_bus *   bit_bang_bus=NULL;
    struct mii_bus *   valid_bus=NULL;
#endif
#endif
#ifdef MDIO_DEBUG
    printf("Begin: %s: index:%d devname:%s\n", __func__, index, devname);
#endif

	/* Allocate ethernet device structure memory */
	dev = (struct eth_device *)malloc(sizeof(struct eth_device));
	if (!dev)
		return -1;

	/* Clear Ethernet device structure memory */
	memset(dev, 0, sizeof(struct eth_device));

	/* Allocate GEMAC private structure memory */
	gemac = (struct gemac_dev *)malloc(sizeof(struct gemac_dev));
	if (!gemac)
		return -1;

	/* Setup standard Ethernet device structure and private
	 * Transcede GEMAC structure fields */
	gemac_list[index]   = gemac;
	gemac->baseaddr     = (void *)gemac_info[index].baseaddr;
	gemac->registers    = (void *)(gemac_info[index].baseaddr + GEM_IP);
	gemac->phyregisters = (void *)(gemac_info[gemac_info[index].phyregidx].baseaddr + GEM_IP);

	gemac->phyaddr  = gemac_info[index].phyaddr; // Get default PHY address from config
	gemac->mode     = gemac_info[index].mode;
	gemac->phyflags = gemac_info[index].phyflags;

	sprintf(dev->name, devname);
	dev->iobase = 0;
	dev->priv   = gemac;
	gemac->dev  = dev;
	dev->init   = gemac_init;
	dev->halt   = gemac_halt;
	dev->send   = gemac_send;
	dev->recv   = gemac_recv;

	/* Tell u-boot to get the addr from the environment */
	for (i = 0; i < 6; i++)
		dev->enetaddr[i] = 0;

	/* Register Ethernet device with U-boot */
	eth_register(dev);
#ifndef CONFIG_GT_BOARD
	// TODO: GEM_DMA_CONFIG - DMA receive buffer size in external AHB or AXI system memory
	// following code doesnt loks like it;

	/* configure DMA register */
	/* disable scheduler */
	*(volatile u32 *)(gemac->registers + GEM_DMA_CONFIG) &= ~(1UL << 31);

	/* enable 16 bytes aligned bursts */
	*(volatile u32 *)(gemac->registers + GEM_DMA_CONFIG) |= (1UL << 27);

	/* enable software buffer allocation (legacy mode) */
	*(volatile u32 *)(gemac->registers + GEM_DMA_CONFIG) |= (1UL << 26);
	*(volatile u32 *)(gemac->registers + GEM_DMA_CONFIG) |= (1UL << 25);

	*(volatile u32 *)(gemac->registers + GEM_DMA_CONFIG) &= ~(0x00FF001F);

	/* set buffer size to 2048 bytes */
	*(volatile u32 *)(gemac->registers + GEM_DMA_CONFIG) |= 0x00200000;

	/* attempt to use INCR16 AHB bursts */
	*(volatile u32 *)(gemac->registers + GEM_DMA_CONFIG) |= 0x00000010;

	/* disable admittance manager */
	*(volatile u32 *)(gemac->registers + GEM_DMA_CONFIG) &= ~(1UL << 12);
#endif
	/* Reset the MAC */
	gemt_reset_gem(gemac);

	/* Setup the GEMCORE registers */
	*(volatile u32*)(gemac->baseaddr + GEM_CFG) = gemac->mode; /* Get GEMCORE configuration bits from options based on board type */
	//
	// Setup GEM IP DMA configuration register
	//

#if defined(CONFIG_T2K)
	*(volatile u32*)(gemac->registers + GEM_DMA_CONFIG) = // 0x06180704;
				GEM_DMA_RX_1536 |
                                GEM_DMA_BURST_INCR16 |
                                GEM_DMA_RX_PB_SIZE_8K |
                                GEM_DMA_TX_PB_SIZE_4K |
                                GEM_DMA_FORCE_RX_MAX_LEN_BURSTS |
                                GEM_DMA_FORCE_TX_MAX_LEN_BURSTS;
#else
	*(volatile u32*)(gemac->baseaddr + GEM_IP + GEM_DMA_CONFIG) = // 0x06180704;
		0
		| (   1<<26) // Bit 26: Transmit buffers are software allocated
		| (   1<<25) // Bit 25: Receive  buffers are software allocated
		| (0x18<<16) // Bits 23:16 DMA size (in 64 byte increments, 0x18 -> 1536)
		| (   1<<10) // Bit  10: 0b1 TX Use full configured addressable space (4 kB)
		| (   3<< 8) // Bit  8,9: 0b11, RX Use full configured addressable space (8 kB)
		| (   1<< 4) // Bit  4: BURST_INCR16 Attemp to use AHB INCR16 Bursts
		;
#endif

	//
	// Setup GEM IP Network configuration register
	// based on interface and default speed of 1 Gigabit per second
	// and default duplex of full duplex
	//
#if defined(CONFIG_T2K)
	if (!(gemac->mode & GEMAC_GEM_MODE_SGMII)) { // GEMAC_GEM_MODE_RGMII
		// printf("%s:%d RGMII configuration for device index = %i\n", __FUNCTION__, __LINE__, index);
		REG32(gemac->registers + GEM_NET_CONFIG) = // 0x040224d2;
					GEM_NETCFG_MULTICAST_ENB |
					GEM_NETCFG_UNICAST_ENB |
					GEM_NETCFG_COPY_ALL |
					GEM_NETCFG_FULL_DUPLEX |
					GEM_NETCFG_1GB_MODE |
//					GEM_NETCFG_FCS_REMOVE |
					GEM_NETCFG_BUS_WIDTH_64_BIT;
	} else if (gemac->mode & GEMAC_GEM_MODE_SGMII) {
		// printf("%s:%d SGMII configuration for device index = %i\n", __FUNCTION__, __LINE__, index);
#ifdef SGMII_AUTONEG_MODE
#ifndef CONFIG_GT_BOARD
		Serdes1SgmiiInit(1); // Init Serdes in internal loopback
#else
		Serdes1SgmiiInit(0); // Init Serdes, no internal loopback
#endif
#else
		Serdes1SgmiiInit(0); // Init Serdes, no internal loopback
#endif
		REG32(gemac->registers + GEM_NET_CONFIG) = // 0x040224d2;
					GEM_NETCFG_MULTICAST_ENB |
					GEM_NETCFG_UNICAST_ENB |
					GEM_NETCFG_COPY_ALL |
					GEM_NETCFG_FULL_DUPLEX |
					GEM_NETCFG_1GB_MODE |
//					GEM_NETCFG_FCS_REMOVE |
					GEM_NETCFG_TBI_ENB |
#ifdef SGMII_AUTONEG_MODE
					GEM_NETCFG_SGMII_MODE |
#endif
					GEM_NETCFG_BUS_WIDTH_64_BIT;
	}
#else
	*(volatile u32*)(gemac->baseaddr + GEM_IP + GEM_NET_CONFIG) = // 0x040224d2;
		0
	//      | (1<<24) // Bit 24: Ignore FCS errors
		| (1<<17) // Bit 17: Remove FCS from buffer
		| (1<<13) // Bit 13: Enable pause frames
		| (1<<10) // Bit 10: 1 gigabit mode (overriden by GEMCORE config or PHY)
		| (1<< 7) // Bit  7: Unicast   enable
		| (1<< 6) // Bit  6: Multicast enable
		| (1<< 4) // Bit  4: Copy all (promiscuous mode receive all frames regardless of address)
		| (1<< 1) // Bit  1: Full duplex mode (overriden by GEMCOR config or PHY)
		;
#endif
#ifndef CONFIG_GT_BOARD
#if defined(CONFIG_T2K)

    //
    // New code for RGMII for X2, try and detect if running GEM based MDIO
    // or if running MDIO bit bang
    // Run this code only when RGMII port is being initialized
    //
    gem_enable_MDIO(gemac);
    //
    // Setup MDIO for slowest rate (U-boot we don't need speed, we need reliability)
    //
    gem_set_mdc_div(gemac, MDC_DIV_224); // Alternate Divisors: 8, 16, 32, 48, 64, 96, 128, 224

    //
    // Autodetect which MDIO bus is used, GEM MDIO or GPIO bit bang
    // and also autodetect which PHYs are present to determine
    // how to setup the EVM
    //

	// Setup to try GEM MDIO bus and GPIO bit bang bus for autodetect

    gem_bus      = alloc_gem_mdio(NULL);
    bit_bang_bus = alloc_mdio_bitbang(&ctrl);
    if (gem_bus != NULL)
    {
        // Bus structure OK for GEM based Linux
        // Like MDIO calls
        // Read PHY addresses for 0, 3, 4 and 5
        // to autodetect MSPD reference design and EVM standard PHY/Switch addresses

        // Check for Realtek PHY and/or Switch on MSPD EVM

        regval1 = (u32)gem_bus->read(gem_bus, 0, 3);
        regval4 = (u32)gem_bus->read(gem_bus, 3, 3);

        //
        // Setup PHY or switch based on detected/read values
        // Start with RealTek PHY first as before we can do
        // anything useful on the PC73300 board, we have to get
        // that PHY so it only repondes to address 3
        //
        if (((regval1 & 0xFFF0)== 0xC910) || ((regval4 & 0xFFF0) == 0xC910))
        {
            // RealTek PHY detected, assuming PC73300 board
            valid_bus = gem_bus;
            regval2 = 0xFFFF;
            regval3 = 0xFFFF;
            if ((regval1 & 0xFFF0)== 0xC910)
            {
                //
                // RealTek PHY responding to addresses 0 and 3
                // Send commands to disable it
                // from responding to address 0
                //
                printf("Realtek PHY detected using addresses 0 and 3, disabling use of address 0\n");
                valid_bus->write(valid_bus,3,31,0x0007);
                valid_bus->write(valid_bus,3,30,0x002C);
                regval1 = valid_bus->read(valid_bus,3,27);
                regval1 &= ~(1<<2);
                valid_bus->write(valid_bus,3,27,  regval1);
                valid_bus->write(valid_bus,3,31,0x0000);
                //
                // If Realtek PHY detected, skip other detection for address 0
                //
                regval1 = 0xFFFF;
            }
        }
        else
        {
            // Realtek PHY not detected, check for Atheros PHYs on T2200 or T3300 EVMs
            regval2 = (u32)gem_bus->read(gem_bus, ATHEROS_RGMII_PHY_ADDRESS, 3);
            regval3 = (u32)gem_bus->read(gem_bus, T3300_EVM_ATHEROS_SGMII_PHY_ADDRESS, 3);
        }
        //
        // Check if any valid reads for expected devices on the EVM
        //
        if (
                ((regval1 != 0x0000) && (regval1 != 0xFFFF))
            ||  ((regval2 != 0x0000) && (regval2 != 0xFFFF))
            ||  ((regval3 != 0x0000) && (regval3 != 0xFFFF))
            ||  ((regval4 != 0x0000) && (regval4 != 0xFFFF))
           )
        {
            //
            // Valid PHY or Switch read on one of the
            // MDIO bus addresses using GEM MDIO
            //
            gem_mdio_ok = 1;
            valid_bus = gem_bus;
            //
            // Add status message to Net: display
            // to inform user which type of MDIO is running
            //
            if (index == 0)
                printf("GEM MDIO: ");
        }
        else
        {
            //
            // Valid device not found using GEM MDIO
            // try GPIO bit bang bus
            //
            if (bit_bang_bus != NULL)
            {
                // Read PHY addresses for 0, 4 and 5
                // to autodetect EVM standard PHY addresses
                regval1 = (u32)bit_bang_bus->read(bit_bang_bus, 0, 3);
                regval2 = (u32)bit_bang_bus->read(bit_bang_bus, ATHEROS_RGMII_PHY_ADDRESS, 3);
                regval3 = (u32)bit_bang_bus->read(bit_bang_bus, T3300_EVM_ATHEROS_SGMII_PHY_ADDRESS, 3);
                regval4 = 0xFFFF;
                //
                // Check if any valid reads for expected devices on the EVM
                //
                if (
                        ((regval1 != 0x0000) && (regval1 != 0xFFFF))
                    ||  ((regval2 != 0x0000) && (regval2 != 0xFFFF))
                    ||  ((regval3 != 0x0000) && (regval3 != 0xFFFF))
                   )
                {
                    //
                    // Valid PHY or Switch read on one of the
                    // MDIO bus addresses using GPIO bit bang MDIO
                    //
                    valid_bus = bit_bang_bus;
                    //
                    // Add status message to Net: display
                    // to inform user which type of MDIO is running
                    // Print on SGMII init so it only prints once
                    //
                    if (index == 0)
                        printf("GPIO bit bang MDIO: ");
                }
            }
        }
    }
    if (valid_bus != NULL)
    {
#ifdef MDIO_DEBUG
        // TEMP DEBUG PRINT, DUMP PHY REGISTER 3 READS
        printf("Index %u: 1:%04X, 2:%04X, 3:%04X, 4:%04X\n",
               index,
               regval1,
               regval2,
               regval3,
               regval4
               );
#endif
        // Valid bus with valid devices found, initialize
        // devices based on what was detected
        //
        // Test if SGMII
        //
        if (gemac->mode & GEMAC_GEM_MODE_SGMII)
        {
            // SGMII mode, currently all MSPD boards require no
            // initialization for SGMII devices (they boot up OK
            // with strapping options if present)
            gemac->phyflags = GEMAC_NO_PHY; // SGMII fixed @ 1 gigabit
            if ((regval4 & 0xFFF0) == 0xC910)
            {
                printf("\n       SGMII RTL8211DN: ");
                gemac->phyflags = GEMAC_NO_PHY; // SGMII fixed @ 1 gigabit
            }
            //
            // Check if Atheros SGMII PHY(s) are present on the board
            //
            if ((regval3 & 0xFFF0) == 0xD070)
            {
                printf("\n       SGMII AR8031: ");
                if ((valid_bus->read(valid_bus,
                                     ATHEROS_SGMII_PHY_ADDRESS_SLAVE,
                                     3
                                    ) & 0xFFF0
                    ) == 0xD070
                   )
                {
                    // Slave PHY also detected on T3300 EVM
                    printf("\n       Slave SGMII AR8031: ");
                }
            }
            else if ((regval1 & 0xFFF0) == 0xD030)
            {
                // Atheros Switch detected, if so, assume T2200 EVM
                // where the SGMII PHY if present
                // uses address 6
                if ((valid_bus->read(valid_bus,
                                         T2200_EVM_ATHEROS_SGMII_PHY_ADDRESS,
                                         3
                                        ) & 0xFFF0
                        ) == 0xD070
                       )
                {
                    // Slave PHY also detected on T3300 EVM
                    printf("\n       SGMII AR8031: ");
                }
            }
        }
        //
        // Not SGMII port init, check RGMII info:
        // Check for Atheros Switch
        //
        else if ((regval1 & 0xFFF0) == 0xD030)
        {
            // Atheros RGMII Switch detected
            gem_switch_detected = 1;
            if (!(gemac->mode & GEMAC_GEM_MODE_SGMII))
            {
                printf("\n       RGMII AR8327: ");
                CfgAtherosSwitch(valid_bus);
                gemac->phyflags = GEMAC_NO_PHY;
            }
        }
        else if ((regval2 & 0xFFF0) == 0xD070)
        {
            // Atheros RGMII PHY detected on T3300 RGMII PHY address
            if (!(gemac->mode & GEMAC_GEM_MODE_SGMII))
            {
                //
                // U-boot initialization for RGMII port (GEM1), Atheros PHY detected:
                //
                printf("\n       RGMII AR8031: ");
                SetRgmiiAtherosPhy(valid_bus,
                                   ATHEROS_RGMII_PHY_ADDRESS,
                                   ATHEROS_RGMII_GTX_DLY_VAL_1300_PS
                                  );
                gemac->phyflags = GEMAC_PHY_AUTONEG;
                //
                // Check if T3300 EVM by also reading slave RGMII phy address
                // If present, also initialize it so it can be used
                // by slave processor Linux later...
                //
                if ((valid_bus->read(valid_bus,ATHEROS_RGMII_PHY_ADDRESS_SLAVE,3) & 0xFFF0) == 0xD070)
                {
                    printf("\n       Slave RGMII AR8031: ");
                    SetRgmiiAtherosPhy(valid_bus,
                                       ATHEROS_RGMII_PHY_ADDRESS_SLAVE,
                                       ATHEROS_RGMII_GTX_DLY_VAL_1300_PS
                                      );
                }
            }
        }
        else
        {
            //
            // RGMII port and no known detectable EVM Ethernet PHY or Switch detected
            //
            gemac->phyflags = GEMAC_NO_PHY;
            //
            // Check if Realtek RGMII PHY detected, assume this
            // is a PC73300 board
            //
            if ((regval4 & 0xFFF0) == 0xC910)
            {
                //
                // Realtek PHY detected, assume PC73300 board and also
                // try and setup the RealTek Switch on board
                // (which cannot be detected by reading regiter 3 like other
                // "normal" devices).
                //
                printf("\n       RGMII RTL8363SB: ");
                CfgRealTekSwitch(valid_bus);
            }
        }
    }
    else
    {
        //
        // No detectable MDIO devices using MDIO reads
        // Assume running GEMAC MDIO bus and bus is OK
        // Assume fixed function PHY or Switch a 1 GBPS
        //
        valid_bus = gem_bus;
        gem_mdio_ok = 1;
        gemac->phyflags = GEMAC_NO_PHY;
        //
        // Test if SGMII (index 0) or RGMII (index 1)
        //
        if (index == 0)
        {
            //
            // SGMII port and no detectable MDIO devices
            //
            // Print out message if first port initialized
            // (so only one message is printed at startup)
            //
            printf("\n       %s: devname:%s No detectable supported MDIO devices detected, assuming GEM MDIO bus",
                   __func__,
                   devname
                  );
        }
        else
        {
            //
            // RGMII port and no detectable MDIO devices
            //
            // All MDIO device autodetect failed, but this still could be a case for a PC73300
            // board where there is a working RealTek Switch, but the PHY is not
            // working or disconnected on a PC73300 board.
            //
            // Try initializing the Realtek switch.
            // The initialize function does have an autodetect (writing multiple registers
            // and reading back).
            //
            // Only do this for RGMII port
            //
            printf("\n       ");
            CfgRealTekSwitch(valid_bus);
        }
    }
    if (bit_bang_bus != NULL)
	    free_mdio_bitbang(bit_bang_bus);

    if (gem_bus != NULL)
        free_gem_mdio(gem_bus);

#endif // #if defined (CONFIG_T2K)

#if (defined(CONFIG_MII) || defined(CONFIG_CMD_MII))
#if defined(CONFIG_BITBANGMII)
//    if (     (gemac->mode & GEMAC_GEM_MODE_SGMII)
//        || ((!(gemac->mode & GEMAC_GEM_MODE_SGMII)) && (gem_switch_detected == 0))
      if (1
       ) {
        /* Register U-boot MII read and write routines with miiphy library/driver */
        if (gem_mdio_ok) {
            miiphy_register(dev->name, transcede_miiphy_read, transcede_miiphy_write);
        } else {
            miiphy_register(dev->name, bb_miiphy_read, bb_miiphy_write);
        }
    }
#else
//    if (     (gemac->mode & GEMAC_GEM_MODE_SGMII)
//        || ((!(gemac->mode & GEMAC_GEM_MODE_SGMII)) && (gem_switch_detected == 0))
    if (1
       ) {
        /* Register Transcede GEM controller based MII read and write routines with miiphy library/driver */
        miiphy_register(dev->name, transcede_miiphy_read, transcede_miiphy_write);
    }
#endif
#endif

#if !defined(CONFIG_T2K)
	// MSPD T3000/T4000 EVMs have Marvell PHY on GEM0 and GEM1,
	// MSPD T3000/T4000 Femtocell demostration board has Broadcom PHY on GEM0 and Broadcom Switch on GEM1
	// Code below assumes Marvell PHY
	// Add 2ns delay to Marvell PHY (Marvell PHY defaults to 0ns delay, Broadcom PHY
	// defaults to 2ns delay, so it doesn't need this, but tests have shown, this
	// doesn't hurt)
        gem_enable_MDIO(gemac);

        *(volatile u32 *)(gemac->registers + GEM_NET_CONFIG) &= ~(7 << 18);
        *(volatile u32 *)(gemac->registers + GEM_NET_CONFIG) |= (3 << 18);

        {
            unsigned short val;
            transcede_miiphy_write(gemac->dev->name, 0, 27, 0x848B); //RGMII to Copper

            if ( ((*(volatile unsigned *)0xFE070050 >> 1) & 0x7FF) == 0x22D)
            {
                transcede_miiphy_read(gemac->dev->name, 0, 20, &val);
                val |= (1 << 7); // RGMII Receive timing
                val |= (1 << 1); // RGMII Trasmit timing (set if required)
                transcede_miiphy_write(gemac->dev->name, 0, 20, val);
            }
    	    // Software reset
    	    transcede_miiphy_read(gemac->dev->name, 0, 0, &val);
            val |= (1 << 15);
    	    transcede_miiphy_write(gemac->dev->name, 0, 0, val);

//          transcede_miiphy_read(gemac->dev->name, 0, 20, &val);
//          printf("phy val=%04x\n", val);
        }
#endif /*#if !defined(CONFIG_T2K)*/
#else /*#ifndef CONFIG_GT_BOARD*/
#if defined(CONFIG_T2K)

    //
    // New code for RGMII for X2, try and detect if running GEM based MDIO
    // or if running MDIO bit bang
    // Run this code only when RGMII port is being initialized
    //
	gem_enable_MDIO(gemac);
    //
    // Setup MDIO for slowest rate (U-boot we don't need speed, we need reliability)
    //
	gem_set_mdc_div(gemac, MDC_DIV_224); // Alternate Divisors: 8, 16, 32, 48, 64, 96, 128, 224

	miiphy_register(dev->name, transcede_miiphy_read, transcede_miiphy_write);
	//here enable SFP GPIO
	REG32(GPIO_31_16_PIN_SELECT_REG) |= ((1 << 16) | (1 << 20) | (1 << 22));
	REG32(GPIO_OE_REG) |= ((1 << 24) | (1 << 26) | (1 << 27));
	REG32(GPIO_OUTPUT_REG) &=  ~((1 << 24) | (1 << 26) | (1 << 27));
	//printf("PIN Select:%08x, OE:%08x, OUTPUT:%08x\n", REG32(GPIO_31_16_PIN_SELECT_REG),REG32(GPIO_OE_REG),REG32(GPIO_OUTPUT_REG));

//	if(index == 0){ //only init once.
//		CfgAtherosSwitch(dev->name);
//	}
	if(index == 1){
		unsigned short phy_tmp;
		transcede_miiphy_read(dev->name, gemac->phyaddr, 0x03, &phy_tmp);
		if(phy_tmp != 0xD072){	//factory test code, check phy connection.
			printf("Can not find PHY\n");
			while(1);
		}
		transcede_miiphy_write(dev->name, gemac->phyaddr, 0x1d, 5);
		transcede_miiphy_read(dev->name, gemac->phyaddr, 0x1e, &phy_tmp);
		transcede_miiphy_write(dev->name, gemac->phyaddr, 0x1d, 5);
		transcede_miiphy_write(dev->name, gemac->phyaddr, 0x1e, phy_tmp | 0x100);
	}
#endif /*#ifdef CONFIG_T2K*/
#endif /*#ifndef CONFIG_GT_BOARD*/
#ifndef CONFIG_GT_BOARD
#ifdef MDIO_DEBUG
    printf("End:   %s: index:%d devname:%s\n", __func__, index, devname);
#endif
    if (index !=0)
    {
        printf("\n       ");
    }
#endif
	return 0;
}

#endif /* CONFIG_COMCERTO_GEMAC */
