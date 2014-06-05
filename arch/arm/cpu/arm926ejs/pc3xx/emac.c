/*****************************************************************************
 * $picoChipHeaderSubst$
 *****************************************************************************/

/*!
* \file emac.c
* \brief Ethernet driver for the PC302.
*
* Copyright (c) 2006-2011 Picochip Ltd
*
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License version 2 as
* published by the Free Software Foundation.
*
* All enquiries to support@picochip.com
*/

/* Includes ---------------------------------------------------------------- */
#include <common.h>

#ifdef CONFIG_DW_EMAC

#include <malloc.h>
#include <asm/errno.h>
#include <net.h>
#include <miiphy.h>
#include <asm/io.h>
#include <asm/arch/pc302.h>
#include <asm/arch/emac.h>
#include <asm/arch/utilities.h>

/* Macros ------------------------------------------------------------------ */

/*!
 * \brief Timeout value (in uS) for various EMAC operations
 */
#define EMAC_TX_TIMEOUT             (1000)

/*!
 * \brief Timeout value (in mS) for various PHY operations
 */
#define EMAC_PHY_TIMEOUT            (2000)

/*!
 * \brief Length (in bytes) of a MAC address
 */
#define EMAC_LENGTH_OF_MAC_ADDRESS  (6)

/*!
 * \brief Default receive and transmit ring lengths
 */
#define EMAC_RX_NUM_DESCRIPTOR      (8)
#define EMAC_TX_NUM_DESCRIPTOR      (2)
#define EMAC_DESCRIPTOR_BUF_SIZE    (2048)

/*!
 * \brief Values used in the emac_priv structure
 */
#define EMAC_PHY_SPEED_10           (0)
#define EMAC_PHY_SPEED_100          (1)
#define EMAC_PHY_DUPLEX_HALF        (0)
#define EMAC_PHY_DUPLEX_FULL        (1)
#define EMAC_PHY_LINK_DOWN          (0)
#define EMAC_PHY_LINK_UP            (1)
#define EMAC_PHY_AUTO_NEG_COMPLETE  (1)

/*!
 * \brief Macros used to read from, and write to, emac registers
 */
#define EMAC_READ(__offset) \
                 (*((volatile u32*)(PC302_EMAC_BASE + __offset)))

#define EMAC_WRITE(__value, __offset) \
                  (*((volatile u32*)(PC302_EMAC_BASE + __offset)) = __value)

/*!
 * \brief Define the auto-negotiation advertisement register
 *        value.
 *
 * Note: This advertises 100 mpbs capability only.
 */
#define PHY_AUTO_NEG_ADVERT_VALUE   (0x0181)

/*!
 * \brief Align an item to a 64-bit boundary.
 */
#define __align64b	    __attribute__((aligned(8)))

/* Constants --------------------------------------------------------------- */

/*!
 * \brief This structure defines the format of the Ethernet MAC
 *        Tx and Rx descriptors.
 */
struct emac_dma_descriptor
{
    /*! Used to define the Rx/Tx buffer address */
    unsigned int buffer;

    /*! Used to contain the status of the Rx/Tx operation */
    unsigned int status;
};

/*!
 * \brief This structure is used to hold private data for the network code
 */
struct emac_priv
{
    /* Useful phy state */
    unsigned int auto_negotiation;
    unsigned int speed;
    unsigned int duplex;
    unsigned int link;

    /* Rx descriptor count */
    unsigned int rx_desc;

    /* Tx descriptor count */
    unsigned int tx_desc;
};

/*!
 * \brief Assign some memory for the Rx descriptors
 */
static volatile struct
emac_dma_descriptor __align64b rx_descriptor[EMAC_RX_NUM_DESCRIPTOR];

/*!
 * \brief Assign some memory for the Tx descriptors
 */
static volatile struct
emac_dma_descriptor __align64b tx_descriptor[EMAC_TX_NUM_DESCRIPTOR];

/*!
 * \brief Assign some memory for the Rx buffers
 * \n Note: The Rx buffers have to be 64 bit aligned otherwise
 *          very bad things happen.
 */
static unsigned char __align64b
rx_buffer[EMAC_RX_NUM_DESCRIPTOR][EMAC_DESCRIPTOR_BUF_SIZE];

/* Types ------------------------------------------------------------------- */

/* Prototypes--------------------------------------------------------------- */

/*!
 *
 * \brief Read a register in a phy connected to the emac management port.
 *
 * \param phy_id The phy id of the phy to access
 * \param register_number The register to read from
 * \return The value read
 *
 */
static unsigned short emac_mii_read(unsigned int phy_id,
                                    unsigned int register_number);

/*!
 *
 * \brief Write to a register in a phy connected to the emac management port.
 *
 * \param phy_id The phy id of the phy to access
 * \param register_number The register to write to
 * \param data The data to write
 *
 */
static void emac_mii_write(unsigned int phy_id,
                           unsigned int register_number,
                           unsigned short data);

#if defined (CONFIG_CMD_MII)
static int pc302emac_miiphy_read(char *devname,
                                 uchar addr,
                                 uchar reg,
                                 ushort *val);

static int pc302emac_miiphy_write(char *devname,
                                  uchar addr,
                                  uchar reg,
                                  ushort val);
#endif /*CONFIG_CMD_MII */

/*!
 *
 * \brief Initialise the emac mdio interface
 *
 */
static void emac_init_mdio(void);

/*!
 *
 * \brief Initialise the phy connected to the emac management port.
 *
 * \param dev Pointer to the eth_device structure
 *
 */
static int emac_init_phy(struct eth_device *dev);

/*!
 *
 * \brief Obtain the link status from the phy.
 *
 * \param dev Pointer to the eth_device structure
 *
 */
static void emac_phy_get_link_status(struct eth_device *dev);

/*!
 *
 * \brief Set up the MAC address in the emac.
 *
 * \param dev Pointer to the eth_device structure
 * \return Zero on success, non zero on error.
 *
 */
static int emac_set_mac_addr(struct eth_device *dev);

/*!
 *
 * \brief Initialise the emac registers.
 *
 * \param dev Pointer to the eth_device structure
 *
 */
static void emac_startup(struct eth_device *dev);

/*!
 *
 * \brief Initialise the emac buffer descriptors and phy.
 *
 * \param dev Pointer to the eth_device structure
 * \param bis Pointer to the board init structure
 * \return Zero on success, non zero on error
 *
 */
static int emac_open(struct eth_device *dev, bd_t *bis);

/*!
 *
 * \brief Receive a packet
 *
 * \param dev Pointer to the eth_device structure
 * \return Zero on success, non zero on error
 *
 */
static int emac_rx_packet(struct eth_device *dev);

/*!
 *
 * \brief Transmit a packet
 *
 * \param dev Pointer to the eth_device structure
 * \param packet Pointer to the packet data to transmit
 * \param length  Length (in bytes) of the packet to send
 * \return Zero on success, non zero on error
 *
 */
static int emac_tx_packet(struct eth_device *dev,
                          volatile void *packet,
                          int length);

/*!
 *
 * \brief Stop the emac
 *
 * \param dev Pointer to the eth_device structure
 *
 */
static void emac_halt(struct eth_device *dev);

/* Functions --------------------------------------------------------------- */

static unsigned short emac_mii_read(unsigned int phy_id,
                                    unsigned int register_number)
{
    unsigned int write_data = 0x60020000;
    unsigned int phy_management_idle = 0;
    unsigned short value_read = 0;

    /* Mask input parameters */
    phy_id &= EMAC_PHY_ID_MASK;
    register_number &= EMAC_PHY_REG_MASK;

    write_data |= ((phy_id << EMAC_PHY_ID_SHIFT) |
                  (register_number << EMAC_PHY_REG_SHIFT));

    EMAC_WRITE(write_data, EMAC_PHY_MAINTAIN_REG_OFFSET);

    /* Wait for the phy access to complete */
    do
    {
        phy_management_idle = EMAC_READ(EMAC_NETWORK_STATUS_REG_OFFSET);
        phy_management_idle &= EMAC_PHY_MANAGEMENT_IDLE;
    }
    while(!phy_management_idle);

    /* Read back the data obtained from the phy */
    value_read = (unsigned short)EMAC_READ(EMAC_PHY_MAINTAIN_REG_OFFSET);
    return(value_read);
}

static void emac_mii_write(unsigned int phy_id,
                           unsigned int register_number,
                           unsigned short data)
{
    unsigned int write_data = 0x50020000;
    unsigned int phy_management_idle = 0;

    /* Mask input parameters */
    phy_id &= EMAC_PHY_ID_MASK;
    register_number &= EMAC_PHY_REG_MASK;

    write_data |= ((phy_id << EMAC_PHY_ID_SHIFT) |
                   (register_number << EMAC_PHY_REG_SHIFT) | data);
    EMAC_WRITE(write_data, EMAC_PHY_MAINTAIN_REG_OFFSET);

    /* Wait for the phy access to complete */
    do
    {
        phy_management_idle = EMAC_READ(EMAC_NETWORK_STATUS_REG_OFFSET);
        phy_management_idle &= EMAC_PHY_MANAGEMENT_IDLE;
    }
    while (!phy_management_idle);
}

#if defined (CONFIG_CMD_MII)
static int pc302emac_miiphy_read(char *devname, uchar addr, uchar reg, ushort *val)
{
    /* Make sure the mdio bus is initialised */
    emac_init_mdio();

    /* Perform the mii bus read */
    *val = emac_mii_read((unsigned int)addr,(unsigned int)reg);
    return 0;
}

static int pc302emac_miiphy_write(char *devname, uchar addr, uchar reg, ushort val)
{
    /* Make sure the mdio bus is initialised */
    emac_init_mdio();

    /* Perform the mdio bis write */
    emac_mii_write((unsigned int)addr,(unsigned int)reg, (unsigned short)val);
    return 0;
}
#endif /*CONFIG_CMD_MII */

static void emac_init_mdio(void)
{
    unsigned int network_control_register = 0;
    unsigned int network_config_register = 0;

    /* Set phy management MDC Clock to 200 MHz (pclk) / 96 */
    network_config_register = EMAC_READ(EMAC_NETWORK_CFG_REG_OFFSET);
    network_config_register &= ~(EMAC_MDC_CLOCK_DIV_MASK);
    network_config_register |= EMAC_MDC_CLOCK_DIV_96;
    EMAC_WRITE(network_config_register, EMAC_NETWORK_CFG_REG_OFFSET);

    /* Enable phy management */
    network_control_register = EMAC_READ(EMAC_NETWORK_CTRL_REG_OFFSET);
    network_control_register |= EMAC_MDIO_ENABLE;
    EMAC_WRITE(network_control_register, EMAC_NETWORK_CTRL_REG_OFFSET);
}

static int emac_init_phy(struct eth_device *dev)
{
    struct emac_priv *priv = dev->priv;

    unsigned int device_id = pc302_read_device_id();
    unsigned int device_rev = pc302_read_device_revision();

    /* Setup the MDIO bus for Phy communications */
    emac_init_mdio ();

    /* If we are running on PC3x2 Rev D silicon and we are using a
     * Reduced MII (RMII) connected Ethernet Phy then we need the
     * link speed to be 100 mbps.
     */
    if (((device_id == PC302_DEVICE_ID) || (device_id == PC312_DEVICE_ID)) &&
         (device_rev == PC3X2_REV_D) &&
         pc302_get_rmii_enabled())
    {
        /* Are we already set for 100 mpbs ? */
        emac_phy_get_link_status(dev);
        if (priv->speed == EMAC_PHY_SPEED_100)
        {
            /* No need to do anything */
        }
        else
        {
            /* Setup the phy auto-negotiation advertisement register */
            emac_mii_write(CONFIG_PHY_ADDR, MII_ADVERTISE,
                           PHY_AUTO_NEG_ADVERT_VALUE);

            /* Re-start auto-negotiation */
            emac_mii_write(CONFIG_PHY_ADDR, MII_BMCR,
                           (BMCR_ANENABLE | BMCR_ANRESTART));

            /* Allow some time for the auto-negotiation process to start */
            udelay(100);
        }
    }

    /* Get the phy status */
    emac_phy_get_link_status(dev);

    if (priv->link == EMAC_PHY_LINK_DOWN)
    {
        /* Oops, no valid link established, time to bail out */
        printf("%s: Link down !\n", dev->name);
        return (-1);
    }
    else
    {
        /* We have a valid link established
         * so we can report the phy setup
         */
        printf("%s: Link up\n", dev->name);

        if (priv->auto_negotiation == EMAC_PHY_AUTO_NEG_COMPLETE)
        {
            printf("%s: Auto-Negotiation complete\n", dev->name);
        }

        if (priv->speed == EMAC_PHY_SPEED_100)
        {
            printf("%s: 100Mbps\n", dev->name);
        }
        else
        {
            printf("%s: 10Mbps\n", dev->name);
        }

        if (priv->duplex == EMAC_PHY_DUPLEX_FULL)
        {
            printf("%s: Full-duplex\n", dev->name);
        }
        else
        {
            printf("%s: Half-duplex\n", dev->name);
        }
    }
    return(0);
}

static void emac_phy_get_link_status(struct eth_device *dev)
{
    struct emac_priv *priv = dev->priv;
    unsigned short phy_control = 0;
    unsigned short phy_status = 0;
    unsigned short ana = 0;
    unsigned short anlpa = 0;
    unsigned int timebase = 0;

    /* Initialise the phy status parameters in the private data structure */
    priv->auto_negotiation = ~(EMAC_PHY_AUTO_NEG_COMPLETE);
    priv->link = EMAC_PHY_LINK_DOWN;
    priv->speed = EMAC_PHY_SPEED_10;
    priv->duplex = EMAC_PHY_DUPLEX_HALF;

    /* Check to make sure the phy has auto-negotiation enabled */
    phy_control = emac_mii_read(CONFIG_PHY_ADDR, MII_BMCR);
    if (phy_control & BMCR_ANENABLE)
    {
        /* Auto-negotiation is enabled
         * now need to check on auto-negotiation progress
         */
        phy_status = emac_mii_read(CONFIG_PHY_ADDR, MII_BMSR);
        if (phy_status & BMSR_ANEGCAPABLE)
        {
            /* The phy is auto-negotiation capable */
            timebase = get_timer(0);
            do
            {
                phy_status = emac_mii_read(CONFIG_PHY_ADDR, MII_BMSR);
                if (phy_status & BMSR_ANEGCOMPLETE)
                {
                    /* Auto-negotiation has completed */
                    priv->auto_negotiation = EMAC_PHY_AUTO_NEG_COMPLETE;
                    break;
                }
            }
            while (get_timer (timebase) < EMAC_PHY_TIMEOUT);

            if (phy_status & BMSR_LSTATUS)
            {
                /* We have a valid link established */
                priv->link = EMAC_PHY_LINK_UP;
            }
            else
            {
                /* We do not have a valid link established */
                priv->link = EMAC_PHY_LINK_DOWN;
            }

            /* Read the auto-negotiation advertisement register */
            ana = emac_mii_read(CONFIG_PHY_ADDR, MII_ADVERTISE);

            /* Read the auto-negotiation link partner ability register */
            anlpa = emac_mii_read(CONFIG_PHY_ADDR, MII_LPA);

            anlpa &= ana;

            if (anlpa & (LPA_100FULL | LPA_100HALF))
            {
                priv->speed = EMAC_PHY_SPEED_100;

                if (anlpa & LPA_100FULL)
                {
                    priv->duplex = EMAC_PHY_DUPLEX_FULL;
                }
            }
            else
            {
                if (anlpa & LPA_10FULL)
                {
                    priv->duplex = EMAC_PHY_DUPLEX_FULL;
                }
            }
        }
    }

    if (priv->auto_negotiation != EMAC_PHY_AUTO_NEG_COMPLETE)
    {
        /* Auto-negotiation is not enabled or failed for some reason
         * so we just get phy setup from the status and control registers
         */
        phy_control = emac_mii_read(CONFIG_PHY_ADDR, MII_BMCR);
        phy_status = emac_mii_read(CONFIG_PHY_ADDR, MII_BMSR);
        if (phy_status & BMSR_LSTATUS)
        {
            priv->link = EMAC_PHY_LINK_UP;
        }
        else
        {
            priv->link = EMAC_PHY_LINK_DOWN;
        }

        if (phy_control & BMCR_SPEED100)
        {
            priv->speed = EMAC_PHY_SPEED_100;
        }
        else
        {
            priv->speed = EMAC_PHY_SPEED_10;
        }

        if (phy_control & BMCR_FULLDPLX)
        {
            priv->duplex = EMAC_PHY_DUPLEX_FULL;
        }
        else
        {
            priv->duplex = EMAC_PHY_DUPLEX_HALF;
        }
    }
}

static int emac_set_mac_addr(struct eth_device *dev)
{
    unsigned int mac_addr_bottom = 0;
    unsigned int mac_addr_top = 0;

    mac_addr_bottom = dev->enetaddr[0] |
                      dev->enetaddr[1] << 8 |
                      dev->enetaddr[2] << 16 |
                      dev->enetaddr[3] << 24;

    mac_addr_top = dev->enetaddr[4] |
                   dev->enetaddr[5] << 8;

    EMAC_WRITE(mac_addr_bottom, EMAC_SPEC_ADDR_1_BOT_31_0_REG_OFFSET);
    EMAC_WRITE(mac_addr_top, EMAC_SPEC_ADDR_1_TOP_47_32_REG_OFFSET);

    return 0;
}

static void emac_startup(struct eth_device *dev)
{
    struct emac_priv *priv = dev->priv;
    unsigned int network_config_register = 0;
    unsigned int network_control_register = 0;
    unsigned int dma_config_register = 0;

    /* Make sure the Tx & Rx are halted */
    network_control_register = EMAC_READ(EMAC_NETWORK_CTRL_REG_OFFSET);
    network_control_register &= ~(EMAC_RX_ENABLE | EMAC_TX_ENABLE);
    EMAC_WRITE(network_control_register, EMAC_NETWORK_CTRL_REG_OFFSET);

    /* Get the MAC address from environment variables
     *
     * Note: We do this again here just incase the MAC
     * address has been changed since U-Boot has started up
     */
    eth_getenv_enetaddr("ethaddr", dev->enetaddr);

    /* Set the hardware MAC address
     *
     * Note: We do this again here just incase the MAC
     * address has been changed since U-Boot has started up
     */
    (void)emac_set_mac_addr(dev);

    /* Setup the Rx Buffer Queue Base Address */
    EMAC_WRITE((unsigned int)&rx_descriptor,
                EMAC_RX_BUFF_Q_BASE_ADDR_REG_OFFSET);

    /* Setup the Tx Buffer Queue Base Address */
    EMAC_WRITE((unsigned int)&tx_descriptor,
                EMAC_TX_BUFF_Q_BASE_ADDR_REG_OFFSET);

    /* Setup the size of the DMA Receive Buffer */
    dma_config_register = EMAC_READ(EMAC_DMA_CFG_REG_OFFSET);
    dma_config_register &= ~(EMAC_DMA_RX_BUFFER_SIZE_MASK);
    dma_config_register |= EMAC_DMA_RX_BUFFER_SIZE;
    EMAC_WRITE(dma_config_register, EMAC_DMA_CFG_REG_OFFSET);

    /* Setup the Network Configuration Register */
    network_config_register = EMAC_READ(EMAC_NETWORK_CFG_REG_OFFSET);
    network_config_register |= EMAC_64_BIT_AMBA_DATA_BUS_WITDH;
    network_config_register |= EMAC_LENGTH_FIELD_ERROR_FRAME_DISCARD;
    network_config_register |= EMAC_FCS_REMOVE;

    if (priv->duplex == EMAC_PHY_DUPLEX_FULL)
    {
        network_config_register |= EMAC_FULL_DUPLEX;
    }
    else
    {
        network_config_register &= ~(EMAC_FULL_DUPLEX);
    }

    if (priv->speed == EMAC_PHY_SPEED_100)
    {
        network_config_register |= EMAC_SPEED_100_MBPS;
    }
    else
    {
        network_config_register &= ~(EMAC_SPEED_100_MBPS);
    }

    EMAC_WRITE(network_config_register, EMAC_NETWORK_CFG_REG_OFFSET);

    /* Setup the Network Control Register */
    network_control_register = EMAC_READ(EMAC_NETWORK_CTRL_REG_OFFSET);
    network_control_register |= (EMAC_RX_ENABLE | EMAC_TX_ENABLE);
    EMAC_WRITE(network_control_register, EMAC_NETWORK_CTRL_REG_OFFSET);
}

static int emac_open(struct eth_device *dev, bd_t *bis)
{
    struct emac_priv *priv = dev->priv;
    unsigned int i = 0;;
    unsigned int buffer_address = 0;
    unsigned int ret = 0;

    /* Initialise the Rx descriptor count */
    priv->rx_desc = 0;

    /* Initialise the Tx descriptor count */
    priv->tx_desc = 0;

    /* Initialise the Rx descriptors */
    for (i = 0; i < EMAC_RX_NUM_DESCRIPTOR; i++)
    {
        buffer_address = (unsigned int)&rx_buffer[i][0];
        if (i == (EMAC_RX_NUM_DESCRIPTOR - 1))
        {
            /* we are on the last descriptor entry */
            buffer_address |= EMAC_RX_DESC_WRAP;
        }
        rx_descriptor[i].buffer = buffer_address;
        rx_descriptor[i].status = 0;
    }

    /* Initialise the Tx descriptors */
    for (i = 0; i < EMAC_TX_NUM_DESCRIPTOR; i++)
    {
        tx_descriptor[i].buffer = 0;
        tx_descriptor[i].status = EMAC_TX_DESC_HOST_OWN;

        if (i == (EMAC_TX_NUM_DESCRIPTOR - 1))
        {
            /* we are on the last descriptor entry */
            tx_descriptor[i].status |= EMAC_TX_DESC_WRAP;
        }
    }

    /* Check out the phy status */
    ret = emac_init_phy(dev);
    if (ret != 0)
    {
        /* Oops, we've had an error */
        return(ret);
    }

    /* Initialise the emac registers */
    emac_startup(dev);

    return (0);
}

static int emac_rx_packet(struct eth_device *dev)
{
    struct emac_priv *priv = dev->priv;
    unsigned int start_of_frame = 0;
    unsigned int end_of_frame = 0;
    unsigned int frame_error = 0;
    unsigned int length = 0;
    void * buffer_start;

    if (rx_descriptor[priv->rx_desc].buffer & EMAC_RX_DESC_HOST_OWN)
    {
        /* We have some received data */

        /* Make sure we have a whole frame */
        start_of_frame = rx_descriptor[priv->rx_desc].status &
                         EMAC_RX_DESC_START_OF_FRAME;
        end_of_frame = rx_descriptor[priv->rx_desc].status &
                       EMAC_RX_DESC_END_OF_FRAME;
        if (start_of_frame && end_of_frame)
        {
            /* We have a complete frame */
            length = rx_descriptor[priv->rx_desc].status &
                     EMAC_RX_DESC_LENGTH_MASK;

            /* Noodle the buffer start address for the higher level
             * network stack. The start address should be 8 byte aligned,
             * also bits 0 & 1 can be set by the emac, so these need to
             * be masked out as well
             */

            buffer_start = (void *)
                           (rx_descriptor[priv->rx_desc].buffer & 0xFFFFFFF8);

            /* Send received packet to the higher network layers */
            NetReceive(buffer_start, length);
        }
        else
        {
            /* Oops, not a complete frame */
            frame_error++;
        }

        /* Reclaim the buffer just used */
        rx_descriptor[priv->rx_desc].buffer &= ~(EMAC_RX_DESC_HOST_OWN);

        /* Increment the Rx descriptor counter */
        priv->rx_desc++;
        if (priv->rx_desc == EMAC_RX_NUM_DESCRIPTOR)
        {
            /* We have exhausted the supply of Rx descriptors */
            priv->rx_desc = 0;
        }
    }

    if (frame_error)
    {
        printf ("frame error\n");
        return (1);
    }

    return(0);
}

static int emac_tx_packet(struct eth_device *dev,
                          volatile void *packet,
                          int length)
{
    struct emac_priv *priv = dev->priv;
    unsigned int i = 0;
    unsigned int network_control_register = 0;

    /* Set up the Tx descriptor */

    /* Make sure the wrap bit is set for the last descriptor */
    if (priv->tx_desc == (EMAC_TX_NUM_DESCRIPTOR - 1))
    {
        /* we are on the last descriptor entry */
        tx_descriptor[priv->tx_desc].status = EMAC_TX_DESC_WRAP;
    }

    tx_descriptor[priv->tx_desc].status |= length & EMAC_TX_BUFFER_LENGTH_MASK;
    tx_descriptor[priv->tx_desc].status |= EMAC_TX_LAST_BUFFER;
    tx_descriptor[priv->tx_desc].status &= ~(EMAC_TX_NO_CRC_APPEND);
    tx_descriptor[priv->tx_desc].status &= ~(EMAC_TX_DESC_HOST_OWN);

    /* Setup the Tx descriptor buffer */
    tx_descriptor[priv->tx_desc].buffer = (unsigned int)packet;

    /* Start the packet transmission */
    network_control_register = EMAC_READ(EMAC_NETWORK_CTRL_REG_OFFSET);
    network_control_register |= EMAC_START_TX;
    EMAC_WRITE(network_control_register, EMAC_NETWORK_CTRL_REG_OFFSET);

    /* Wait for transmission to complete */
    for (i = 0; i <= EMAC_TX_TIMEOUT; i++)
    {
	if (tx_descriptor[priv->tx_desc].status & EMAC_TX_DESC_HOST_OWN)
        {
            /* The emac has completed transmission */
            break;
        }
	udelay(1);
    }

    /* Increment the Tx descriptor counter */
    priv->tx_desc++;
    if (priv->tx_desc == EMAC_TX_NUM_DESCRIPTOR)
    {
        /* We have exhausted the supply of Tx descriptors */
        priv->tx_desc = 0;
    }

    /* We could add some error reporting in here, but no one cares anyway */
    return(0);
}

static void emac_halt(struct eth_device *dev)
{
    unsigned int status_register = 0;

    /* Halt the Tx & Rx */
    EMAC_WRITE(0, EMAC_NETWORK_CTRL_REG_OFFSET);

    /* Clear the statistics counters */
    EMAC_WRITE(EMAC_CLEAR_STATS_REGISTERS, EMAC_NETWORK_CTRL_REG_OFFSET);

    /* Clear the Tx status registers */
    status_register = EMAC_READ(EMAC_TX_STATUS_REG_OFFSET);
    EMAC_WRITE(status_register, EMAC_TX_STATUS_REG_OFFSET);

    /* Clear the Rx status registers */
    status_register = EMAC_READ(EMAC_RX_STATUS_REG_OFFSET);
    EMAC_WRITE(status_register, EMAC_RX_STATUS_REG_OFFSET);
}

int pc302_eth_register(bd_t *bis)
{
    struct eth_device *dev = NULL;
    struct emac_priv *priv = NULL;

    /* Create some storage for useful structures */
    dev = (struct eth_device *) malloc(sizeof (*dev));
    if (dev == NULL)
    {
        /* Oops, no memory available */
        return -ENOMEM;
    }

    priv = (struct emac_priv *) malloc(sizeof (*priv));
    if (priv == NULL)
    {
        /* Oops, no memory available */
        free (dev);
        return -ENOMEM;
    }

    dev->priv = priv;

    /* Reset the private data */
    memset(priv, 0, sizeof(struct emac_priv));

    /* Define our name */
    sprintf(dev->name, "pc302_emac");

    dev->init = emac_open;
    dev->recv = emac_rx_packet;
    dev->send = emac_tx_packet;
    dev->halt = emac_halt;
    dev->write_hwaddr = emac_set_mac_addr;

    /* Register our emac driver with the networking environment */
    (void)eth_register(dev);

    /* Get the MAC address from environment variables */
    eth_getenv_enetaddr("ethaddr", dev->enetaddr);

    /* Set the hardware MAC address */
    (void)emac_set_mac_addr(dev);

#if defined (CONFIG_CMD_MII)
    miiphy_register(dev->name, pc302emac_miiphy_read, pc302emac_miiphy_write);
#endif

    return 0;
}

#endif /* CONFIG_DW_EMAC */
