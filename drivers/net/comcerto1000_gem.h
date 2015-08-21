#ifndef __COMCERTO_GEM1000_H__
#define __COMCERTO_GEM1000_H__


/* Specify the default PCLK divisor for generating MDC.  This simply defines
 * which of the divisor control bits are active.  By default, the clock
 * division is 32.
 */
#define GEM_DEF_PCLK_DIV (MDC_DIV_96)

////////////////////////////////////////////////////////////
//      GEMAC IP wrapper                                                                                    //
////////////////////////////////////////////////////////////

#define	GEM_SCH_PKT_QUEUED		0x8008

#define GEM_IP				0xE000
#define GEM_CFG				0xF000
/*
#define GEM_MBIST			0xF004
#define GEM_SENSE_AMP_DELAY_ADJ		0xF008
#define GEM_DELAY_ELEMENT_CTRL		0xF00C
*/

#define GEM_SCH_CTRL                    0x8004
#define GEM_SCH_PKT_QUEUED              0x8008

#define GEM_CONF_MODE_SEL_PIN		(0 << 0)
#define GEM_CONF_MODE_SEL_GEM		(1 << 0)

#define GEM_CONF_MODE_GEM_MASK		(7 << 1)
#define GEM_CONF_MODE_GEM_RGMII		(0 << 1)
#define GEM_CONF_MODE_GEM_RMII		(1 << 1)
#define GEM_CONF_MODE_GEM_MII		(2 << 1)
#define GEM_CONF_MODE_GEM_GMII		(3 << 1)

#define GEM_CONF_MODE_PIN_MASK		(7 << 4)
#define GEM_CONF_MODE_PIN_RGMII		(0 << 4)
#define GEM_CONF_MODE_PIN_RMII		(1 << 4)
#define GEM_CONF_MODE_PIN_MII		(2 << 4)
#define GEM_CONF_MODE_PIN_GMII		(3 << 4)

#define GEM_CONF_DUPLEX_SEL_PHY		(0 << 8)
#define GEM_CONF_DUPLEX_SEL_GEM		(1 << 8)
#define GEM_CONF_DUPLEX_GEM_HALF	(0 << 9)
#define GEM_CONF_DUPLEX_GEM_FULL	(1 << 9)
#define GEM_CONF_DUPLEX_PHY_HALF	(0 << 10)
#define GEM_CONF_DUPLEX_PHY_FULL	(1 << 10)
#define GEM_CONF_SPEED_SEL_PHY		(0 << 11)
#define GEM_CONF_SPEED_SEL_GEM		(1 << 11)

#define GEM_CONF_SPEED_MASK		(3 << 12)
#define GEM_CONF_SPEED_GEM_10M		(0 << 12)
#define GEM_CONF_SPEED_GEM_100M		(1 << 12)
#define GEM_CONF_SPEED_GEM_1G		(2 << 12)
#define GEM_CONF_SPEED_PHY_MASK		(3 << 14)
#define GEM_CONF_SPEED_PHY_10M		(0 << 14)
#define GEM_CONF_SPEED_PHY_100M		(1 << 14)
#define GEM_CONF_SPEED_PHY_1G		(2 << 14)

#define GEM_CONF_PHY_LINK_DOWN		(0 << 16)
#define GEM_CONF_PHY_LINK_UP		(1 << 16)
#define GEM_CONF_GEM_LOOPBACK		(1 << 17)


/* Define some bit positions for registers. */

/* Bit positions for network control register */
#define GEM_READ_SNAP       (1<<14)	/* Read snapshot register */
#define GEM_TAKE_SNAP       (1<<13)	/* Take a snapshot */
#define GEM_TX_0Q_PAUSE     (1<<12)	/* Transmit zero quantum pause frame */
#define GEM_TX_PAUSE        (1<<11)	/* Transmit pause frame */
#define GEM_TX_HALT         (1<<10)	/* Halt transmission after curr frame */
#define GEM_TX_START        (1<<9)	/* Start tx (tx_go) */
#define GEM_STATS_WR_EN     (1<<7)	/* Enable writing to stat registers */
#define GEM_STATS_INC       (1<<6)	/* Increment statistic registers */
#define GEM_STATS_CLR       (1<<5)	/* Clear statistic registers */
#define GEM_MDIO_EN         (1<<4)	/* Enable MDIO port */
#define GEM_TX_EN           (1<<3)	/* Enable transmit circuits */
#define GEM_RX_EN           (1<<2)	/* Enable receive circuits */
#define GEM_LB_MAC          (1<<1)	/* Perform local loopback at MAC */
#define GEM_LB_PHY          (1<<0)	/* Perform ext loopback through PHY */

/* Bit positions for network configuration register */
#define GEM_RX_NO_PAUSE     (1<<23)	/* Do not copy pause frames to memory */
#define GEM_AHB_WIDTH1      (1<<22)     /* Bit 1 for defining AHB width */
#define GEM_AHB_WIDTH0      (1<<21)     /* Bit 0 for defining AHB width */
#define GEM_MDC_DIV2        (1<<20)	/* PCLK divisor for MDC, bit 2 */
#define GEM_MDC_DIV1        (1<<19)	/* PCLK divisor for MDC, bit 1 */
#define GEM_MDC_DIV0        (1<<18)	/* PCLK divisor for MDC, bit 0 */
#define GEM_RX_NO_FCS       (1<<17)	/* Discard FCS from received frames. */
#define GEM_RX_LEN_CHK      (1<<16)	/* Receive length check. */
#define GEM_RX_PAUSE_EN     (1<<13)	/* Enable pause reception */
#define GEM_RETRY_TEST      (1<<12)	/* Retry test for speeding up debug */
#define GEM_PCS_SEL         (1<<11)	/* Select PCS */
#define GEM_GIG_MODE        (1<<10)	/* Gigabit mode enable */
#define GEM_EAM_EN          (1<<9)	/* External address match enable */
#define GEM_FRAME_1536      (1<<8)	/* Enable 1536 byte frames reception */
#define GEM_UNICAST_EN      (1<<7)	/* Receive unicast hash frames */
#define GEM_MULTICAST_EN    (1<<6)	/* Receive multicast hash frames */
#define GEM_NO_BROADCAST    (1<<5)	/* Do not receive broadcast frames */
#define GEM_COPY_ALL        (1<<4)	/* Copy all frames */
#define GEM_RX_JUMBO        (1<<3)	/* Allow jumbo frame reception */
#define GEM_VLAN_ONLY       (1<<2)	/* Receive only VLAN frames */
#define GEM_FULL_DUPLEX     (1<<1)	/* Enable full duplex */
#define GEM_SPEED_100       (1<<0)	/* Set to 100Mb mode */

/* Bit positions for network status register */
#define GEM_PHY_IDLE        (1<<2)	/* PHY management is idle */
#define GEM_MDIO_IN         (1<<1)	/* Status of mdio_in pin */
#define GEM_LINK_STATUS     (1<<0)	/* Status of link pin */

/* Bit positions for transmit status register */
#define GEM_TX_HRESP        (1<<8)	/* Transmit hresp not OK */
#define GEM_LATE_COL        (1<<7)	/* Late collision */
#define GEM_TX_URUN         (1<<6)	/* Transmit underrun occurred */
#define GEM_TX_COMPLETE     (1<<5)	/* Transmit completed OK */
#define GEM_TX_BUF_EXH      (1<<4)	/* Transmit buffs exhausted mid frame */
#define GEM_TX_GO           (1<<3)	/* Status of tx_go internal variable */
#define GEM_TX_RETRY_EXC    (1<<2)	/* Retry limit exceeded */
#define GEM_TX_COL          (1<<1)	/* Collision occurred during frame tx */
#define GEM_TX_USED         (1<<0)	/* Used bit read in tx buffer */

/* Bit positions for receive status register */
#define GEM_RX_HRESP        (1<<3)	/* Receive hresp not OK */
#define GEM_RX_ORUN         (1<<2)	/* Receive overrun occurred */
#define GEM_RX_DONE         (1<<1)	/* Frame successfully received */
#define GEM_RX_BUF_USED     (1<<0)	/* Receive buffer used bit read */

/* Bit positions for interrupts */
#define GEM_IRQ_PCS_AN      (1<<16)	/* PCS autonegotiation complete */
#define GEM_IRQ_EXT_INT     (1<<15)	/* External interrupt pin triggered */
#define GEM_IRQ_PAUSE_TX    (1<<14)	/* Pause frame transmitted */
#define GEM_IRQ_PAUSE_0     (1<<13)	/* Pause time has reached zero */
#define GEM_IRQ_PAUSE_RX    (1<<12)	/* Pause frame received */
#define GEM_IRQ_HRESP       (1<<11)	/* hresp not ok */
#define GEM_IRQ_RX_ORUN     (1<<10)	/* Receive overrun occurred */
#define GEM_IRQ_PCS_LINK    (1<<9)	/* Status of PCS link changed */
#define GEM_IRQ_TX_DONE     (1<<7)	/* Frame transmitted ok */
#define GEM_IRQ_TX_ERROR    (1<<6)	/* Transmit err occurred or no buffers */
#define GEM_IRQ_RETRY_EXC   (1<<5)	/* Retry limit exceeded */
#define GEM_IRQ_TX_URUN     (1<<4)	/* Transmit underrun occurred */
#define GEM_IRQ_TX_USED     (1<<3)	/* Tx buffer used bit read */
#define GEM_IRQ_RX_USED     (1<<2)	/* Rx buffer used bit read */
#define GEM_IRQ_RX_DONE     (1<<1)	/* Frame received ok */
#define GEM_IRQ_MAN_DONE    (1<<0)	/* PHY management operation complete */
#define GEM_IRQ_ALL         (0xFFFFFFFF)	/* Everything! */

/* Revision ID Register */
#define GEM_REV_ID_MODEL_MASK   (0x000F0000)	/* Model ID */
#define GEM_REV_ID_MODEL_BASE   (16)	/* For Shifting */
#define GEM_REV_ID_REG_MODEL    (0x00020000)	/* GEM module ID */
#define GEM_REV_ID_REV_MASK     (0x0000FFFF)	/* Revision ID */

/* Define some memory offsets for easier direct access to memory map. */
#define GEM_NET_CONTROL         (0x00)
#define GEM_NET_CONFIG          (0x04)
#define GEM_NET_STATUS          (0x08)
#define GEM_USER_IO             (0x0C)
#define GEM_DMA_CONFIG              (0x10)
#define GEM_TX_STATUS           (0x14)

#ifndef CONFIG_T2K
#define GEM_RX_QPTR             (0x18)
#define GEM_RX_OFFSET           (0x1C)
#else
#define GEM_RX_QUEUE0           (0x18)
#define GEM_TX_QUEUE0           (0x1C)
#endif

#define GEM_RX_STATUS           (0x20)
#define GEM_IRQ_STATUS          (0x24)
#define GEM_IRQ_ENABLE          (0x28)
#define GEM_IRQ_DISABLE         (0x2C)
#define GEM_IRQ_MASK            (0x30)
#define GEM_PHY_MAN             (0x34)
#define GEM_RX_PAUSE_TIME       (0x38)
#define GEM_TX_PAUSE_QUANT      (0x3C)
#define GEM_TX_PART_STORE_FORW  (0x40)
#define GEM_RX_PART_STORE_FORW  (0x40)

#define GEM_HASH_BOT            (0x80)
#define GEM_HASH_TOP            (0x84)
#define GEM_LADDR1_BOT          (0x88)
#define GEM_LADDR1_TOP          (0x8C)
#define GEM_LADDR2_BOT          (0x90)
#define GEM_LADDR2_TOP          (0x94)
#define GEM_LADDR3_BOT          (0x98)
#define GEM_LADDR3_TOP          (0x9C)
#define GEM_LADDR4_BOT          (0xA0)
#define GEM_LADDR4_TOP          (0xA4)

#ifndef CONFIG_T2K

#define GEM_ID_CHECK1           (0x320)
#define GEM_ID_CHECK2           (0x324)
#define GEM_ID_CHECK3           (0x328)
#define GEM_ID_CHECK4           (0x32C)

#else

#define GEM_ID_CHECK1           (0xA8)
#define GEM_ID_CHECK2           (0xAC)
#define GEM_ID_CHECK3           (0xB0)
#define GEM_ID_CHECK4           (0xB4)

#define GEM_WAKE_ON_LAN         (0xB8)
#define GEM_IPG_STRETCH         (0xBC)
#define GEM_STACKED_VLAN        (0xC0)
#define GEM_TX_PFC_PAUSE        (0xC4)

#define GEM_SPEC_ADDR1_MASK_BOTTOM  (0xC8)
#define GEM_SPEC_ADDR1_MASK_TOP     (0xCC)
#define GEM_AHB_RX_ADDR_MASK    (0xD0)

#endif

#define GEM_REV_ID              (0xFC)

#define GEM_OCT_TX_BOT          (0x100)
#define GEM_OCT_TX_TOP          (0x104)
#define GEM_STATS_FRAMES_TX     (0x108)
#define GEM_BROADCAST_TX        (0x10C)
#define GEM_MULTICAST_TX        (0x110)
#define GEM_STATS_PAUSE_TX      (0x114)
#define GEM_FRAME64_TX          (0x118)
#define GEM_FRAME65_TX          (0x11C)
#define GEM_FRAME128_TX         (0x120)
#define GEM_FRAME256_TX         (0x124)
#define GEM_FRAME512_TX         (0x128)
#define GEM_FRAME1024_TX        (0x12C)
#define GEM_FRAME1519_TX        (0x130)
#define GEM_STATS_TX_URUN       (0x134)
#define GEM_STATS_SINGLE_COL    (0x138)
#define GEM_STATS_MULTI_COL     (0x13C)
#define GEM_STATS_EXCESS_COL    (0x140)
#define GEM_STATS_LATE_COL      (0x144)
#define GEM_STATS_DEF_TX        (0x148)
#define GEM_STATS_CRS_ERRORS    (0x14C)
#define GEM_OCT_RX_BOT          (0x150)
#define GEM_OCT_RX_TOP          (0x154)
#define GEM_STATS_FRAMES_RX     (0x158)
#define GEM_BROADCAST_RX        (0x15C)
#define GEM_MULTICAST_RX        (0x160)
#define GEM_STATS_PAUSE_RX      (0x164)
#define GEM_FRAME64_RX          (0x168)
#define GEM_FRAME65_RX          (0x16C)
#define GEM_FRAME128_RX         (0x170)
#define GEM_FRAME256_RX         (0x174)
#define GEM_FRAME512_RX         (0x178)
#define GEM_FRAME1024_RX        (0x17C)
#define GEM_FRAME1519_RX        (0x180)
#define GEM_STATS_USIZE_FRAMES  (0x184)
#define GEM_STATS_EXCESS_LEN    (0x188)
#define GEM_STATS_JABBERS       (0x18C)
#define GEM_STATS_FCS_ERRORS    (0x190)
#define GEM_STATS_LENGTH_ERRORS (0x194)
#define GEM_STATS_RX_SYM_ERR    (0x198)
#define GEM_STATS_ALIGN_ERRORS  (0x19C)
#define GEM_STATS_RX_RES_ERR    (0x1a0)
#define GEM_STATS_RX_ORUN       (0x1a4)

#define GEM_REG_TOP             (0x23C)

#define	GEM_QUEUE_BASE0		(0x300)
#define	GEM_QUEUE_BASE1		(0x304)
#define	GEM_QUEUE_BASE2		(0x308)
#define	GEM_QUEUE_BASE3		(0x30C)
#define	GEM_QUEUE_BASE4		(0x310)
#define	GEM_QUEUE_BASE5		(0x314)
#define	GEM_QUEUE_BASE6		(0x318)
#define	GEM_QUEUE_BASE7		(0x31C)

#define GEM_ID_CHECK5           (0x320)
#define GEM_ID_CHECK6           (0x324)
#define GEM_ID_CHECK7           (0x328)
#define GEM_ID_CHECK8           (0x32C)

#define GEM_IRQ_STATUS_PQUEUE(i) (0x400 + 4*((i)-1)) /* i = 1..7 */
#define GEM_TX_PQUEUE(i)         (0x440 + 4*((i)-1)) /* i = 1..7 */
#define GEM_RX_PQUEUE(i)         (0x480 + 4*((i)-1)) /* i = 1..7 */
#define GEM_RX_BUF_SIZE_PQUEUE(i)(0x4A0 + 4*((i)-1)) /* i = 1..7 */
#define GEM_IRQ_ENABLE_PQUEUE(i) (0x600 + 4*((i)-1)) /* i = 1..7 */
#define GEM_IRQ_DISABLE_PQUEUE(i)(0x620 + 4*((i)-1)) /* i = 1..7 */
#define GEM_IRQ_MASK_PQUEUE(i)   (0x640 + 4*((i)-1)) /* i = 1..7 */

//
// Network Control Register offset 0x000
//
#define GEM_NETCTRL_STORE_L4_HDR_OFFSET (1 << 22)   /* Store UDP / TCP offset to memory */
#define GEM_NETCTRL_ALT_SGMII_MODE      (1 << 21)   /* Alternative sgmii mode */
#define GEM_NETCTRL_PTP_UNICAST_ENABLE  (1 << 20)   /* Enable detection of unicast PTP unicast frames */
#define GEM_NETCTRL_LPI_TX_ENABLE       (1 << 19)   /* Enable LPI transmission */
#define GEM_NETCTRL_FLUSH_PKT_RX_DPRAM  (1 << 18)   /* Flush the next packet from the external RX DPRAM */
#define GEM_NETCTRL_TX_PFC_PAUSE_FRAME  (1 << 17)   /* Transmit PFC Priority Based Pause Frame */
#define GEM_NETCTRL_PFC_PAUSE_RX_ENABLE (1 << 16)   /* Enable PFC Priority Based Pause Reception */
#define GEM_NETCTRL_STORE_TIMESTAMP     (1 << 15)   /* Setting this bit stores RX 1588 timestamps to CRC field */
#define GEM_NETCTRL_READ_SNAPSHOT       (1 << 14)   /* Read Snapshot (1) or current (0) statsistics register */
#define GEM_NETCTRL_SNAPSHOT            (1 << 13)   /* Record current statistics in snapshot registers, clear stats registers  */
#define GEM_NETCTRL_TX_ZERO_PAUSE       (1 << 12)   /* Transmit a single pause frame with zero quantum */
#define GEM_NETCTRL_TX_PAUSE            (1 << 11)   /* Transmit a single pause frame */
#define GEM_NETCTRL_HALT_TX             (1 << 10)   /* Halt  transmission after current TX frame if any (1 stops  TX) */
#define GEM_NETCTRL_START_TX            (1 << 9)    /* Start transmission (1 starts TX) */
#define GEM_NETCTRL_BACK_PRESSURE       (1 << 8)    /* Back pressure in 10M or 100M half duplex */
#define GEM_NETCTRL_STAT_WR_ENB         (1 << 7)    /* Allow write access for statistics registers (test only) */
#define GEM_NETCTRL_INC_STAT            (1 << 6)    /* Increment all statistics register by one    (test only) */
#define GEM_NETCTRL_CLR_STAT            (1 << 5)    /* Clear statistics */
#define GEM_NETCTRL_MDIO_ENB            (1 << 4)    /* MDIO     enable */
#define GEM_NETCTRL_TX_ENB              (1 << 3)    /* Transmit enable */
#define GEM_NETCTRL_RX_ENB              (1 << 2)    /* Receive  enable */
#define GEM_NETCTRL_RX_LB               (1 << 1)    /* Receive  loopback */
#define GEM_NETCTRL_TX_LB               (1 << 0)    /* Transmit loopback */

//
// Network Configuration Register offset 0x004
//
#define GEM_NETCFG_100BM                (1 << 0)    /* Speed - set to logic one to indicate 100Mbps */
#define GEM_NETCFG_FULL_DUPLEX          (1 << 1)    /* Full duplex */
#define GEM_NETCFG_DISCARD_NON_VLAN     (1 << 2)    /* Discard non-VLAN frames */
#define GEM_NETCFG_JUMBO_ENB            (1 << 3)    /* Jumbo frames */
#define GEM_NETCFG_COPY_ALL             (1 << 4)    /* Copy all frames (i.e. promiscuous mode) */
#define GEM_NETCFG_NO_BROADCAST         (1 << 5)    /* No broadcast */
#define GEM_NETCFG_MULTICAST_ENB        (1 << 6)    /* Multicast hash enable */
#define GEM_NETCFG_UNICAST_ENB          (1 << 7)    /* Unicast hash enable */
#define GEM_NETCFG_1536_ENB             (1 << 8)    /* Receive 1536 byte frames */
#define GEM_NETCFG_EXT_ADDR_ENB         (1 << 9)    /* External address match enable */
#define GEM_NETCFG_1GB_MODE             (1 << 10)   /* setting this bit configures the GEM for 1000 Mbps operation */
#define GEM_NETCFG_TBI_ENB              (1 << 11)   /* PCS select 1: TBI enabled, 0: GMII/MII enabled */
#define GEM_NETCFG_RETRY_TEST           (1 << 12)   /* Retry test, must be set to zero for normal operation */
#define GEM_NETCFG_PAUSE_ENB            (1 << 13)   /* Pause enable */

#define GEM_NETCFG_RX_OFFS_MASK         (3 << 14)   /* Receive buffer offset mask */
#define GEM_NETCFG_RX_OFFS_BYTES(x)     (x << 14)   /* Reciver buffer offset value (0-3 bytes from base address) */

#define GEM_NETCFG_NO_LEN_ERR           (1 << 16)   /* Length field error frame discard */
#define GEM_NETCFG_FCS_REMOVE           (1 << 17)   /* FCS remove */

/* Register 0x004 R/W 20:18 MDC clock division - set according to pclk speed
000: divide pclk by 8 (pclk up to 20 MHz)
001: divide pclk by 16 (pclk up to 40 MHz)
010: divide pclk by 32 (pclk up to 80 MHz) (reset value)
011: divide pclk by 48 (pclk up to 120MHz)
100: divide pclk by 64 (pclk up to 160 MHz)
101: divide pclk by 96 (pclk up to 240 MHz)
110: divide pclk by 128 (pclk up to 320 MHz)
111: divide pclk by 224 (pclk up to 540 MHz)
*/
#define GEM_NETCFG_MDC_CLOCK_DIV_MASK   (7 << 18)
#define GEM_NETCFG_MDC_CLOCK_DIV_8      (0 << 18)
#define GEM_NETCFG_MDC_CLOCK_DIV_16     (1 << 18)
#define GEM_NETCFG_MDC_CLOCK_DIV_32     (2 << 18)
#define GEM_NETCFG_MDC_CLOCK_DIV_48     (3 << 18)
#define GEM_NETCFG_MDC_CLOCK_DIV_64     (4 << 18)
#define GEM_NETCFG_MDC_CLOCK_DIV_96     (5 << 18)
#define GEM_NETCFG_MDC_CLOCK_DIV_128    (6 << 18)
#define GEM_NETCFG_MDC_CLOCK_DIV_224    (7 << 18)


/** @brief Register 0x004 R/W Bits 22:21 Data bus width
- 00: 32 bit AMBA AHB/AXI data bus width
- 01: 64 bit AMBA AHB/AXI data bus width
- 10: 128 bit AMBA AHB/AXI data bus width
- 11: 128 bit AMBA AHB/AXI data bus width
*/
#define GEM_NETCFG_BUS_WIDTH_MASK       (3 << 21)
#define GEM_NETCFG_BUS_WIDTH_32_BIT     (0 << 21)
#define GEM_NETCFG_BUS_WIDTH_64_BIT     (1 << 21)
#define GEM_NETCFG_BUS_WIDTH_128_BIT    (2 << 21)
#define GEM_NETCFG_BUS_WIDTH_128_BITS   (3 << 21)

#define GEM_NETCFG_DONOT_COPY_PAUSE     (1 << 23)   /* Do not copy Ethernet pause frames */
#define GEM_NETCFG_CALC_CHKS_ENB        (1 << 24)   /* Receive checksum offload enable */
#define GEM_NETCFG_RX_HALF_ENB          (1 << 25)   /* Enable frames to be received in half-duplex mode while transmitting */
#define GEM_NETCFG_IGNORE_RX_FCS        (1 << 26)   /* Ignore RX FCS - when set frames with FCS/CRC errors will not be rejected */
#define GEM_NETCFG_SGMII_MODE           (1 << 27)   /* SGMII mode enable */
#define GEM_NETCFG_IPG                  (1 << 28)   /* IPG stretch enable */
#define GEM_NETCFG_BAD_PREAMBLE         (1 << 29)   /* Receive bad preamble */
#define GEM_NETCFG_IGNORE_RX_ERR        (1 << 30)   /* Ignore RX_ER when RX_DV is low */
#define GEM_NETCFG_UNIDIR               (1 << 31)   /* Uni-direction-enable */

//
// Network Status Register, offset 0x008
//
#define GEM_NETSTAT_RESERVED            (0 << 8)    /* Bits 31:8 Reserved, read as zero, ignored on write */
#define GEM_NETSTAT_LPI_DETECTED        (1 << 7)    /* Low Power Idle (LPI) Indication */
#define GEM_NETSTAT_PFC_NEGOTIATED      (1 << 6)    /* Set when PFC Priority Based Pause has been negotiated */
#define GEM_NETSTAT_PCS_AUTONEG_TX      (1 << 5)    /* PCS auto-negotiation pause transmit resolution */
#define GEM_NETSTAT_PCS_AUTONEG_RX      (1 << 4)    /* PCS auto-negotiation pause receive resolution */
#define GEM_NETSTAT_PCS_AUTONEG_DUPLEX  (1 << 3)    /* PCS auto-negotiation duplex resolution */
#define GEM_NETSTAT_PHY_MANGEMENT_IDLE  (1 << 2)    /* PHY management logic is idle (i.e. has completed)*/
#define GEM_NETSTAT_MDIO_IN_STATUS      (1 << 1)    /* Returns status of the mdio_in pin */
#define GEM_NETSTAT_PCS_LINK_STATE      (1 << 0)    /* Returns status of PCS link state */

//
// DMA Configuration Register
//
#define GEM_DMA_BURST_MASK              (0x1F << 0)
#define GEM_DMA_BURST_SINGLE            (1 << 0)    /* 00001: Always use SINGLE AHB bursts */
#define GEM_DMA_BURST_INCR4             (1 << 2)    /* 001xx: Attempt to use INCR4 AHB bursts */
#define GEM_DMA_BURST_INCR8             (1 << 3)    /* 01xxx: Attempt to use INCR8 AHB bursts */
#define GEM_DMA_BURST_INCR16            (1 << 4)    /* 1xxxx: Attempt to use INCR16 AHB bursts */

#define GEM_DMA_BIG_ENDIAN_DESC_ACCESS  (1 << 6)    /* AHB endian swap mode for management decristor accesses enable */
#define GEM_DMA_BIG_ENDIAN              (1 << 7)    /* AHB endian swap mode for packet data accesses enable */

#define GEM_DMA_RX_PB_SIZE_MASK         (3 << 8)
#define GEM_DMA_RX_PB_SIZE_1K           (0 << 8)    /* 00: Do not use top three address bits (1 Kb) */
#define GEM_DMA_RX_PB_SIZE_2K           (1 << 8)    /* 01: Do not use top two address bits (2 Kb) */
#define GEM_DMA_RX_PB_SIZE_4K           (2 << 8)    /* 10: Do not use top address bit (4 Kb) */
#define GEM_DMA_RX_PB_SIZE_8K           (3 << 8)    /* 11: Use full configured addressable space (8 Kb) */

#define GEM_DMA_TX_PB_SIZE_2K           (0 << 10)   /* 0: Do not use top address bit (2 Kb) */
#define GEM_DMA_TX_PB_SIZE_4K           (1 << 10)   /* 1: Use full configured addressable space (4 Kb) */

#define GEM_DMA_CHK_SUM_ENB             (1 << 11)   /* Transmitter IP, TCP and UDP checksum generation offload enable */

/** @brief Register 0x010 R/W Bits 23:16 DMA receive buffer size in external AHB or AXI system memory.
@details
The value defined by these bits determines the size
of buffer to use in main system memory when writing
received data.
The value is defined in multiples of 64 bytes such
that a value of 0x01 corresponds to buffers of 64
bytes, 0x02 corresponds to 128 bytes etc.

For example:
- 0x02: 128 byte
- 0x18: 1536 byte (1*max length frame/buffer)
- 0xA0: 10240 byte (1*10K jumbo frame/buffer)

Note that this value should never be written as zero.
*/
#define GEM_DMA_RX_SIZE_MASK            (0xFF << 16)
#define GEM_DMA_DEF_RX_SIZE(size)       ((((size)   >>  6) & 0xFF) << 16)
#define GEM_DMA_GET_RX_SIZE(dmareg)     ((((dmareg) >> 16) & 0xFF) *  64)
#define GEM_DMA_RX_64                   (1 << 16)
#define GEM_DMA_RX_128                  (2 << 16)
#define GEM_DMA_RX_192                  (3 << 16)
#define GEM_DMA_RX_256                  (4 << 16)
#define GEM_DMA_RX_320                  (5 << 16)
#define GEM_DMA_RX_384                  (6 << 16)
#define GEM_DMA_RX_448                  (7 << 16)
#define GEM_DMA_RX_512                  (8 << 16)
#define GEM_DMA_RX_1024                 (16<< 16)
#define GEM_DMA_RX_1536                 (24<< 16)
#define GEM_DMA_RX_2048                 (32<< 16)
#define GEM_DMA_RX_4096                 (64<< 16)
#define GEM_DMA_RX_8192                (128<< 16)
#define GEM_DMA_RX_10240               (160<< 16)

#define GEM_DMA_RX_DISCARD_BUS_RESOURCE (1<<24) /* Discard RX packets when no Bus resources */
#define GEM_DMA_FORCE_RX_MAX_LEN_BURSTS (1<<25) /* Force the RX DMA to always issue max length */
#define GEM_DMA_FORCE_TX_MAX_LEN_BURSTS (1<<26) /* Always Force TX maximum length bursts */

struct gemac_info_struct {
	unsigned int baseaddr;
	unsigned int phyaddr;
	unsigned int gemacconfig;
	u32 mode;
	u32 phyflags;		// to indicate if gigabit supported or not
	unsigned int phyregidx;
};

#ifdef CONFIG_T2K
struct rx_desc {
	volatile u32 data;
	volatile u32 ctl;
};
#else
struct rx_desc {
        volatile u32 data;
        volatile u32 status;
        volatile u32 extstatus;
        u32 pad;
};
#endif
//
//  Core configuration register
//
#define GEM_CFG_MODE_SEL_PIN                (0 << 0)
#define GEM_CFG_MODE_SEL_GEM                (1 << 0)
#define GEM_CFG_MODE_GEM_RGMII              (0 << 3)
#define GEM_CFG_MODE_GEM_SGMII              (1 << 3)

#define GEM_CFG_DUPLEX_SEL_PHY              (0 << 8)
#define GEM_CFG_DUPLEX_SEL_GEM              (1 << 8)
#define GEM_CFG_DUPLEX_GEM_HALF             (0 << 9)
#define GEM_CFG_DUPLEX_GEM_FULL             (1 << 9)
#define GEM_CFG_DUPLEX_PHY_HALF             (0 << 10)
#define GEM_CFG_DUPLEX_PHY_FULL             (1 << 10)
#define GEM_CFG_SPEED_SEL_PHY               (0 << 11)
#define GEM_CFG_SPEED_SEL_GEM               (1 << 11)
#define GEM_CFG_SPEED_MASK                  (3 << 12)
#define GEM_CFG_SPEED_GEM_10M               (0 << 12)
#define GEM_CFG_SPEED_GEM_100M              (1 << 12)
#define GEM_CFG_SPEED_GEM_1G                (2 << 12)
#define GEM_CFG_SPEED_PHY_10M               (0 << 14)
#define GEM_CFG_SPEED_PHY_100M              (1 << 14)
#define GEM_CFG_SPEED_PHY_1G                (2 << 14)
#define GEM_CFG_PHY_LINK_DOWN               (0 << 16)
#define GEM_CFG_PHY_LINK_UP                 (1 << 16)
#define GEM_CFG_GEM_LOOPBACK                (1 << 17)


#ifdef CONFIG_T2K
/******************************************************************************
*       Definition of the GEM DMA buffer descriptor and macros                *
*******************************************************************************/

// RX DESCRIPTOR WORD-0
#define GEM_RX_DESCR_BUF_ADDR(x)              ((x) & 0xFFFFFFFC)/* 31:2 Address of beginning of buffer */
#define GEM_RX_DESCR_WRAP                     (1 << 1)          /* Wrap - marks last descriptor in receive buffer descriptor list */
#define GEM_RX_DESCR_OWNERSHIP                (1 << 0)          /* needs to be zero for the GEM to write data to the receive buffer */

// RX DESCRIPTOR WORD-1
#define GEM_RX_DESCR_BROADCAST                ((U32)1L << 31)   /* Global all ones broadcast address detected */
#define GEM_RX_DESCR_MULTICAST                (1 << 30)         /* Multicast Hash match */
#define GEM_RX_DESCR_UNICAST                  (1 << 29)         /* Unicast Hash match */

#define GEM_RX_DESCR_EXTERNAL_ADDR_MATCH      (1 << 28)         /* External address match */
#define GEM_RX_DESCR_SPECIFIC_ADDR_MATCH      (1 << 27)         /* Specific address match */
#define GEM_RX_DESCR_SPEC_ADDR_MASK           (3 << 25)         /* Specific address register match found */

#define GEM_RX_DESCR_EXTRA_SPECIFIC_ADDR_MATCH (1 << 28)         /* Specific address match if EXTRA (>4) registers used*/
#define GEM_RX_DESCR_EXTRA_SPEC_ADDR_MASK      (7 << 25)         /* Specific address register match found if EXTRA (>4) registers used*/

/** @brief Bit 24 Type ID register match or SNAP/CFI Info (User guide version 25 or later)
@details
This bit has a different meaning depending on whether RX checksum
offloading is enabled.

@par With RX checksum offloading disabled:
(bit 24 clear in Network Configuration)
Type ID register match found, bit 22 and bit 23 indicate which type ID register
causes the match.
@par With RX checksum offloading enabled:
(bit 24 set in Network Configuration)
- 0 - the frame was not SNAP encoded and/or had a VLAN tag with the CFI bit
set.
- 1 - the frame was SNAP encoded and had either no VLAN tag or a VLAN tag
with the CFI bit not set.
*/
#define GEM_RX_DESCR_TYPE_MATCH_OR_PKT_INFO   (1 << 24)

/** @brief Bits 23:22
@details
These bits have different meaning depending on whether RX checksum
offloading is enabled.

@par With RX checksum offloading disabled:
(bit 24 clear in Network Configuration)
Type ID register match. Encoded as follows:
- 00 - Type ID register 1 match
- 01 - Type ID register 2 match
- 10 - Type ID register 3 match
- 11 - Type ID register 4 match
If more than one Type ID is matched only one is indicated with priority 4 down
to 1.
@par With RX checksum offloading enabled:
(bit 24 set in Network Configuration)
- 00 - Neither the IP header checksum nor the TCP/UDP checksum was
checked.
- 01 - The IP header checksum was checked and was correct. Neither the
TCP nor UDP checksum was checked.
- 10 - Both the IP header and TCP checksum were checked and were correct.
- 11 - Both the IP header and UDP checksum were checked and were correct.
*/
#define GEM_RX_DESCR_TYPE_ID_OR_CHKSUM_INFO_MASK (3 << 22)

#define GEM_RX_DESCR_VLAN                     (1 << 21) /* VLAN tag detected <97> type ID of 0x8100 */
#define GEM_RX_DESCR_PRIOR                    (1 << 20) /* Priority tag detected <97> type ID of 0x8100 and null VLAN identifier */
#define GEM_RX_DESCR_VLAN_PRIOR_MASK          (7 << 17) /* VLAN priority - only valid if bit 21 is set */
#define GEM_RX_DESCR_CFI                      (1 << 16) /* Canonical format indicator (CFI) bit - only valid if bit 21 is set */
#define GEM_RX_DESCR_EOF                      (1 << 15)   /* end of frame bit */
#define GEM_RX_DESCR_SOF                      (1 << 14) /* Start of frame */
#define GEM_RX_DESCR_FCS_STAT                 (1 << 13) /* FCS status (1 - bad FCS), valid if FCS ignore enabled and jumbo disabled */
#define GEM_RX_DESCR_JUMBO_LEN_MASK           (0x03FFF)
#define GEM_RX_DESCR_LEN_MASK                 (0x01FFF)


#else
// gemac rx controls
// Wrap flag - marks last descriptor in a queue when set
// goes to the status word (offset 0x4)
#define GEMRX_WRAP 	(1<<28)
// Ownership flag - when 0 gem can use the descriptor
// goes to the extended status word (offset 0x8)
#define GEMRX_OWN	(1<<15)

// gemac rx status
#define RX_STA_BCAST		(1UL<<31)
#define RX_STA_MCAST		(1<<30)
#define RX_STA_UM		(1<<29)
#define RX_MAC_MATCH_FLAG 	(0x4<<25)
#define RX_MAC_MATCH_NUM_MASK	(0x3<<25)
#define RX_MAC_MATCH_POS 	25
#define RX_INT 			(1<<24)
#define RX_IPSEC_OUT		(1<<23)
#define RX_IPSEC_IN	 	(1<<22)
#define RX_STA_VLAN 		(1<<21)
#define RX_STA_VLAN_802p 	(1<<20)
#define RX_STA_VLAN_PRI_MASK	(7<<17)
#define RX_STA_VLAN_PRI_POS	17
#define RX_STA_VLAN_CFI		(1<<16)
#define RX_STA_SOF		(1<<15)
#define RX_STA_EOF		(1<<14)
#define RX_STA_PACKET		(RX_STA_SOF|RX_STA_EOF)
#define RX_STA_CRCERR		(1<<13)
#define RX_STA_LEN_MASK		0x1fff
#define RX_STA_LEN_POS		0
#define	RX_CHECK_ERROR		RX_STA_CRCERR
// gemac rx extended status(word2)
#define RX_STA_L4OFF_MASK	(0xff<<24)
#define RX_STA_L4OFF_POS	24
#define RX_STA_L3OFF_MASK	(0xff<<16)
#define RX_STA_L3OFF_POS	16

#define RX_STA_L3_CKSUM		(1<<11)
#define RX_STA_L3_GOOD		(1<<12)
#define RX_STA_L4_CKSUM		(1<<13)
#define RX_STA_L4_GOOD		(1<<14)

#define RX_STA_TCP		(1<<9)
#define RX_STA_UDP		(1<<8)
#define RX_STA_IPV6		(1<<7)
#define RX_STA_IPV4		(1<<6)
#define RX_STA_PPPOE		(1<<5)
#define RX_STA_WILLHANDLE	(RX_STA_IPV6 | RX_STA_IPV4)
#define RX_STA_QinQ		(1<<4)
#define RX_STA_TYPEID_MATCH_FLAG (0x8 << 0)
#define RX_STA_TYPEID		(0x7 << 0)
#define RX_STA_TYPEID_POS	0

#endif

struct tx_desc {
	volatile u32 data;
	volatile u32 ctl;
};

#ifdef CONFIG_T2K

// TX DESCRIPTOR WORD-0 - data buffer address

// TX DESCRIPTOR WORD-1

#define GEM_TX_DESCR_USED                     ((u32)1L << 31)   /* Must be zero for the GEM to read data to the transmit buffer */
#define GEM_TX_DESCR_WRAP                     (1 << 30)   /* Last descriptor */
#define GEM_TX_DESCR_RETRY_EXCEEDED           (1 << 29)   /* Retry limit exceeded, transmit error detected */
//#define GEM_TX_DESCR_IRQ_ENB_BIT              (1 << 29)   /* TODO: is used in this revision? */
#define GEM_TX_DESCR_UNDERRUN_BIT             (1 << 28)     /* Transmit under run */
#define GEM_TX_DESCR_FRM_CORRUPT_BIT          (1 << 27)     /* Transmit frame corruption due to AHB or AXI error */
#define GEM_TX_DESCR_LATE_COLLISION_BIT       (1 << 26)     /* Late collision, transmit error detected */

/** @brief Bits 22:20 Transmit IP/TCP/UDP checksum generation offload errors:
@details
- 000 No Error
- 001 The Packet was identified as a VLAN type, but the header was not fully
complete, or had an error in it
- 010 The Packet was identified as a SNAP type, but the header was not fully
complete, or had an error in it
- 011 The Packet was not of an IP type, or the IP packet was invalidly short, or
the IP was not of type IPv4/IPv6
- 100 The Packet was not identified as VLAN, SNAP or IP
- 101 Non supported packet fragmentation occurred. For IPv4 packets, the IP
checksum was generated and inserted
- 110 Packet type detected was not TCP or UDP. TCP/UDP checksum was
therefore not generated. For IPv4 packets, the IP checksum was
generated and inserted
- 111 A premature end of packet was detected and the TCP/UDP checksum
could not be generated
*/
#define GEM_TX_DESCR_CHECKSUM_RESULT_MASK     (7 << 20)
#define GEM_TX_DESCR_CHECKSUM_NO_ERROR        (0 << 20)
#define GEM_TX_DESCR_CHECKSUM_VLAN_ERROR      (1 << 20)
#define GEM_TX_DESCR_CHECKSUM_SNAP_ERROR      (2 << 20)
#define GEM_TX_DESCR_CHECKSUM_NOT_IP_TYPE     (3 << 20)
#define GEM_TX_DESCR_CHECKSUM_NOT_IDENTIFIED  (4 << 20)
#define GEM_TX_DESCR_CHECKSUM_FRAGMENTED      (5 << 20)
#define GEM_TX_DESCR_CHECKSUM_NOT_TCP_UDP     (6 << 20)
#define GEM_TX_DESCR_CHECKSUM_PREMATURE_END   (7 << 20)

#define GEM_TX_DESCR_NO_CRC_BIT               (1 << 16)   /* No CRC to be appended by MAC */
#define GEM_TX_DESCR_LAST_BUF_BIT             (1 << 15)   /* Last buffer in the frame */
#define GEM_TX_DESCR_LEN_MASK                 (0x01FFF)   /* Length of buffer [12:0] */

#else

#define TX_DESC_WORD1_USED	(1 << 31)
#define TX_DESC_WORD1_WRAP	(1 << 30)
#define TX_DESC_WORD1_INT	(1 << 29)
#define TX_DESC_WORD1_ERR1	(1 << 28)
#define TX_DESC_WORD1_ERR2	(1 << 27)
#define TX_DESC_WORD1_TXCSUM_MASK	((1 << 26)| (1 << 25))
#define	TX_DESC_CSUM(a)		((a&(TX_DESC_WORD1_TXCSUM_MASK))<<25)
#define TX_DESC_WORD1_FCS	(1 << 24)
#define TX_DESC_WORD1_LAST	(1 << 15)
#define TX_DESC_WORD1_POOLB	(1 << 14)
#define TX_DESC_WORD1_BUFRET	(1 << 13)
#define TX_DESC_WORD1_LEGTH_MASK (0x1FFF)

#endif

/* This is a structure that will be passed and used for all HAL operations, it
 * consists of pointers to the various MAC structures such as the MAC register
 * block and the first descriptor element for the rx and tx buffer queues.
 * Other internal variables declared for use in function calls and to keep track
 * of where things are.
 */
struct gemac_dev {
	void *registers;	/* Pointer to the MAC address space. */
	void *phyregisters;	/* Pointer to the MAC address space controlling the phy */

	void *baseaddr;
	struct phy_info *phyinfo;
	u32 phyaddr;
	u32 mode;
	u32 phyflags;
	struct eth_device *dev;
#if defined (CONFIG_TURNER)
  struct tx_fdesc * descripter_to_use;
#endif 

};

#endif
