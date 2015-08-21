#ifndef _M822XX_PCIE_H_
#define _M822XX_PCIE_H_

typedef enum __PCIe_RETCODE {
	RETCODE_OK = 0,
	RETCODE_ERROR,
} PCIe_retcode;

#define MEMRW_TLP_BASEADDR	0x80000000	//<< base address to be used in TLPs (PCIe bus address)
#define MEMRW_TLP_SBLADDR	0xF6000000	//<< address to be used in TLPs for SBL(PCIe bus address)
#define PRIMARY_IMAGE_ADDR	0x00000000	//<< address for Linux kernel region, aligned on 64k (0x10000)
#define SBL_IMAGE_ADDR		0x02000000	//<< default address for SBL region, aligned on 64k (0x10000)
#define SBL_IMAGE_ADDR_HIGH 0x08000000	//<< high address for SBL region, aligned on 64k (0x10000)


#define VIP_BASE_ADDR 0xFA300000
#define VIP_CONFIG_ADDR 0xFA210000
#define PCIE_IRAM_TRGT_ADDR 0xF4200000


// TLP types (PCIe 3.0 spec p60, Table 2-3: Fmt[2:0] and Type[4:0] Field Encodings)
#define PCIE_TLP_TYPE_MEM	0x0	// MRd, MWr
#define PCIE_TLP_TYPE_MRDLK	0x1
#define PCIE_TLP_TYPE_IO	0x2	// IORd, IOWr
#define PCIE_TLP_TYPE_CFG0	0x4	// CfgRd0, CfgWr0
#define PCIE_TLP_TYPE_CFG1	0x5	// CfgRd1, CfgWr1
#define PCIE_TLP_TYPE_CPL 	0xA	// Completion with or without Data
#define PCIE_TLP_TYPE_CPLLK	0xB	// Completion for locked memory read with or without Data

//
//  HARDWARE(D) parameters
//
#define PCIE_IATU_MIN_REGION_SIZE           0x10000     // 65536
#define PCIE_IATU_MIN_REGION_MASK           0x0FFFF

#define PCIE_CX_ATU_NUM_OUTBOUND_REGIONS    8
#define PCIE_CX_ATU_NUM_INBOUND_REGIONS     8

//
// PCIe configuration space and standard capabilities
// This is valid for EP and may be not valid for RC
//
#define PCIE_CFG_BASE(base)			((base) + 0x0)
#define PCIE_PM_CAP_BASE(base)			((base) + 0x040)
#define PCIE_MSI_CAP_BASE(base)			((base) + 0x050)
#define PCIE_PCIE_CAP_BASE(base)		((base) + 0x070)
#define PCIE_MSIX_CAP_BASE(base)		((base) + 0x0B0)
#define PCIE_VPD_CAP_BASE(base)			((base) + 0x0D0)
#define PCIE_AER_CAP_BASE(base)			((base) + 0x100)

// PCIe controller port logic registers
#define PCIE_PL_BASE(base)		((base) + 0x700)
#define PCIE_PL_MSI_BASE(base)		((base) + 0x700 + 0x120)
#define PCIE_PL_iATU_BASE(base)		((base) + 0x700 + 0x200)
#define PCIE_PL_DMA_BASE(base)		((base) + 0x700 + 0x270)

// PF PCI Express Capability Structure
#define PCIE_PCIE_CID_REG(base)		((base) + 0x0 )
#define PCIE_PCIE_DCAP_REG(base)	((base) + 0x04)
#define PCIE_PCIE_DCNT_REG(base)	((base) + 0x08)
#define PCIE_PCIE_LCAP_REG(base)	((base) + 0x0C)
#define PCIE_PCIE_LCNT_REG(base)	((base) + 0x10)
#define PCIE_PCIE_SCAP_REG(base)	((base) + 0x14)
#define PCIE_PCIE_SCNT_REG(base)	((base) + 0x18)
#define PCIE_PCIE_DCAP2_REG(base)	((base) + 0x24)
#define PCIE_PCIE_DCNT2_REG(base)	((base) + 0x28)
#define PCIE_PCIE_LCAP2_REG(base)	((base) + 0x2C)
#define PCIE_PCIE_LCNT2_REG(base)	((base) + 0x30)


/*
 * PCIe registers
 */

//Base Address of PCIE Controller's AXI Slave Space
#define PCIE0_SLV_AXI_BASE 0xF6000000

//Base Address of DWC Registers
#define DWC_CFG_BASE (RAD_BASIC_CFG_BASEADDR + 0x200)


/* =--------------------------------------------------------------= */
/*                    PCIE0 Register Addresses                      */
/* =--------------------------------------------------------------= */
/*                     B A S E   A D D R E S S                      */
/* =--------------------------------------------------------------= */

/***********************************************************/
/*   PF PCI Configuration Space Header - Type 0            */
/***********************************************************/
// ??
#define PCIE0_CFG_BASE                          (PCIE0_CFG_BASEADDR + 0x0 )
#define PCIE0_PM_CAP_BASE                       (PCIE0_CFG_BASEADDR + 0x40)
#define PCIE0_MSI_CAP_BASE                      (PCIE0_CFG_BASEADDR + 0x50)
#define PCIE0_PCIE_CAP_BASE                     (PCIE0_CFG_BASEADDR + 0x70)
#define PCIE0_MSIX_CAP_BASE                     (PCIE0_CFG_BASEADDR + 0xB0)
#define PCIE0_VPD_CAP_BASE                      (PCIE0_CFG_BASEADDR + 0xD0)

#define PCIE0_MSI_CTRL_INTR0_STS_REG     (PCIE0_PL_REG + 0x130)
/***********************************************************/
/*   PF PCI0 Configuration Space Header - Type 0           */
/***********************************************************/
// ??
#define PCIE0_ID_REG                            (PCIE0_CFG_BASE + 0x00)
#define PCIE0_CMD_REG                           (PCIE0_CFG_BASE + 0x04)
#define PCIE0_REV_REG                           (PCIE0_CFG_BASE + 0x08)
#define PCIE0_CACHE_REG                         (PCIE0_CFG_BASE + 0x0C)
#define PCIE0_BAR0_REG                          (PCIE0_CFG_BASE + 0x10)
#define PCIE0_BAR1_REG                          (PCIE0_CFG_BASE + 0x14)
#define PCIE0_BAR2_REG                          (PCIE0_CFG_BASE + 0x18)
#define PCIE0_BAR3_REG                          (PCIE0_CFG_BASE + 0x1C)
#define PCIE0_BAR4_REG                          (PCIE0_CFG_BASE + 0x20)
#define PCIE0_BAR5_REG                          (PCIE0_CFG_BASE + 0x24)
#define PCIE0_CARDBUS_REG                       (PCIE0_CFG_BASE + 0x28)
#define PCIE0_SID_REG                           (PCIE0_CFG_BASE + 0x2C)
#define PCIE0_ROM_BASE_REG                      (PCIE0_CFG_BASE + 0x30)
#define PCIE0_ROM_MASK_REG                      (PCIE0_CFG_BASE + 0x30)
#define PCIE0_CAP_REG                           (PCIE0_CFG_BASE + 0x34)
#define PCIE0_INT_REG                           (PCIE0_CFG_BASE + 0x3C)



// +
typedef struct _PCIE_CONF_SPACE_HEADER_TYPE0 {
//	0x00
	volatile u16	VendorID;
	volatile u16	DeviceID;
//	0x04
	volatile u16	CommandReg;
	volatile u16	StatusReg;
//	0x08
	volatile u8	RevisionID;
	volatile u32	ClassCode:24;
//	0x0C
	volatile u8	CacheLineSize;
	volatile u8	LatencyTimer;
	volatile u8	HeaderType;
	volatile u8	BIST;
//	0x10 - 0x24
	volatile u32	BAR[6];
//	0x28
	volatile u32	CardBusCISPointer;
//	0x2C
	volatile u16	SubsystemVendorID;
	volatile u16	SubsystemID;
//	0x30
	volatile u32	ExpROMBaseAddress;
//	0x34
	volatile u8	CapPtr;
	volatile u32	Reserved_0:24;
//	0x38
	volatile u32	Reserved_1;
//	0x3C
	volatile u8	InterruptLine;
	volatile u8	InturruptPin;
	volatile u8	Min_Grant;
	volatile u8	Max_Latency;
} PCIE_CONF_SPACE_HEADER_TYPE0;


// +
typedef struct _PCIE_CONF_SPACE_HEADER_TYPE1 {
//	0x00
	volatile u16	VendorID;
	volatile u16	DeviceID;
//	0x04
	volatile u16	CommandReg;
	volatile u16	StatusReg;
//	0x08
	volatile u8	RevisionID;
	volatile u32	ClassCode:24;
//	0x0C
	volatile u8	CacheLineSize;
	volatile u8	LatencyTimer;
	volatile u8	HeaderType;
	volatile u8	BIST;
//	0x10 - 0x14
	volatile u32	BAR[2];
//	0x18
	volatile u8	PrimaryBusNum;
	volatile u8	SecondaryBusNum;
	volatile u8	SubordinateBusNum;
	volatile u8	SecondaryLatencyTimer;
//	0x1C
	volatile u8	IOBase;
	volatile u8	IOLimit;
	volatile u16	SecondaryStatus;
//	0x20
	volatile u16	MemBase;
	volatile u16	MemLimit;
//	0x24
	volatile u16	PrefetchMemBase;
	volatile u16	PrefetchMemLimit;
//	0x28
	volatile u32	PrefetchBaseUpper32B;
//	0x2C
	volatile u32	PrefetchLimit32UpperB;
//	0x30
	volatile u16	IOBaseUpper16B;
	volatile u16	IOLimitUpper16B;
//	0x34
	volatile u8	CapPtr;
	volatile u32	Reserved_0:24;
//	0x38
	volatile u32	ExpROMBaseAddr;
//	0x3C
	volatile u8	InterruptLine;
	volatile u8	InturruptPin;
	volatile u16	BridgeControl;
} PCIE_CONF_SPACE_HEADER_TYPE1;

/* Port Logic Register Base */
#define PCIE0_PL_REG                            (PCIE0_CFG_BASE + 0x700)

/***********************************************************/
/* PF Power Management Capability Structure(CFG_PM_CAP)    */
/***********************************************************/
// ??
#define PCIE0_PM_CAP_REG                        (PCIE0_PM_CAP_BASE + 0x0)
#define PCIE0_PMCSR_REG                         (PCIE0_PM_CAP_BASE + 0x4)
// +
typedef struct _PM_CAPABILITY {
//	0x00
	volatile u8	CapabilityID;
	volatile u8	NextCapabilityPtr;
	volatile u16	PMC;	// Power Management Capabilities
//	0x04
	volatile u16	PMCSR;	// Power Management Control Status Register
	volatile u8	PMCSR_BSE;	// Bridge Extensions
	volatile u8	Data;
} PM_CAPABILITY, *PPM_CAPABILITY;

/***********************************************************/
/* PF MSI Capability Structure                             */
/***********************************************************/
// ??
#define PCIE0_MSI_CAP_REG                       (PCIE0_MSI_CAP_BASE + 0x0 )
#define PCIE0_MSI_LADDR_REG                     (PCIE0_MSI_CAP_BASE + 0x4 )
#define PCIE0_MSI_UADDR_REG                     (PCIE0_MSI_CAP_BASE + 0x8 )
#define PCIE0_MSI_DATA_REG                      (PCIE0_MSI_CAP_BASE + 0xC )
#define PCIE0_MSI_MASK_REG                      (PCIE0_MSI_CAP_BASE + 0x10)
#define PCIE0_MSI_PEND_REG                      (PCIE0_MSI_CAP_BASE + 0x14)
// +
typedef struct _MSI_CAPABILITY {
//	0x00
	volatile u8	CapabilityID;
	volatile u8	NextCapabilityPtr;
	volatile u16	MessageControlRegister;
//	0x04
	volatile u32	MSILowerAddressReg;
//	0x08
	volatile u32	MSIUpperAddressReg;
//	0x0C
	volatile u16	MSI_Data;
	volatile u16	Reserved_0;
//	0x10
	volatile u32	MaskBitsReg;
//	0x14
	volatile u32	PendingBitsReg;
} MSI_CAPABILITY, *PMSI_CAPABILITY;

/***********************************************************/
/* PF PCI Express Capability Structure                     */
/***********************************************************/
// +
#define PCIE0_CID_REG		(PCIE0_PCIE_CAP_BASE + 0x0 )
#define PCIE0_DCAP_REG		(PCIE0_PCIE_CAP_BASE + 0x04)
#define PCIE0_DCNT_REG		(PCIE0_PCIE_CAP_BASE + 0x08)
#define PCIE0_LCAP_REG		(PCIE0_PCIE_CAP_BASE + 0x0C)
#define PCIE0_LCNT_REG		(PCIE0_PCIE_CAP_BASE + 0x10)  //9800_0080
#define PCIE0_SCAP_REG		(PCIE0_PCIE_CAP_BASE + 0x14)
#define PCIE0_SCNT_REG		(PCIE0_PCIE_CAP_BASE + 0x18)
#define PCIE0_DCAP2_REG		(PCIE0_PCIE_CAP_BASE + 0x24)
#define PCIE0_DCNT2_REG		(PCIE0_PCIE_CAP_BASE + 0x28)
#define PCIE0_LCAP2_REG		(PCIE0_PCIE_CAP_BASE + 0x2C)
#define PCIE0_LCNT2_REG		(PCIE0_PCIE_CAP_BASE + 0x30)

// +
typedef struct _PCIE_CAPABILITY {
//	0x00
	volatile u8	CapabilityID;
	volatile u8	NextCapabilitiesPtr;
	volatile u16	PCICapabilitiesReg;
//	0x04
	volatile u32	DeviceCapabilities;
//	0x08
	volatile u16	DeviceControl;
	volatile u16	DeviceStatus;
//	0x0C
	volatile u32	LinkCapabilities;
//	0x10
	volatile u16	LinkControl;
	volatile u16	LinkStatus;
//	0x14 - 0x20
	volatile u32	reserved_0[4];	// not described in doc at all (Table 5-5 PF PCI Express Capability Structure (p538))
//	0x24
	volatile u32	DeviceCapabilities2;
//	0x28
	volatile u16	DeviceContror2;
	volatile u16	reserved_1;
//	0x2C
	volatile u32	LinkCapabilities2;
//	0x30
	volatile u16	LinkControl2;
	volatile u16	LinkStatus2;
} PCIE_CAPABILITY;


/***********************************************************/
/* Port Logic Registers                                    */
/***********************************************************/
// +
#define PCIE0_PL_ALRT_REG		(PCIE0_PL_REG)
#define PCIE0_PL_AFL0L1_REG		(PCIE0_PL_REG + 0xC)
#define PCIE0_PL_SYMNUM_REG		(PCIE0_PL_REG + 0x18)
#define PCIE0_PL_G2CTRL_REG		(PCIE0_PL_REG + 0x10C)
#define PCIE0_PL_CTL_REG		(PCIE0_PL_REG + 0x10)

/***********************************************************/
/* MSI Interrupt Controller Registers                      */
/***********************************************************/
#define PCIE0_MSI_CTRL_ADDR_REG         (PCIE0_PL_REG + 0x120)
#define PCIE0_MSI_CTRL_INTR0_EN_REG     (PCIE0_PL_REG + 0x128)
#define PCIE1_MSI_CTRL_INTR0_STS_REG    (PCIE1_PL_REG + 0x130)
/***********************************************************/
/* iATU Registers                                          */
/***********************************************************/
#define PCIE0_iATU_VIEW_REG              (PCIE0_PL_REG + 0x200)
#define PCIE0_iATU_CTL1_REG              (PCIE0_PL_REG + 0x204)
#define PCIE0_iATU_CTL2_REG              (PCIE0_PL_REG + 0x208)
#define PCIE0_iATU_RLBA_REG              (PCIE0_PL_REG + 0x20C)
#define PCIE0_iATU_RUBA_REG              (PCIE0_PL_REG + 0x210)
#define PCIE0_iATU_RLIA_REG              (PCIE0_PL_REG + 0x214)
#define PCIE0_iATU_RLTA_REG              (PCIE0_PL_REG + 0x218)
#define PCIE0_iATU_RUTA_REG              (PCIE0_PL_REG + 0x21C)

typedef struct tag_IATU_REGISTERS
{
	volatile u32 Viewport;            // +0x200, viewport
	volatile u32 Ctrl1;               // +0x204, Control1
	volatile u32 Ctrl2;               // +0x208, Control1
	volatile u32 LowBaseAddr;         // +0x20C, Low base Address
	volatile u32 HiBaseAddr;          // +0x210, High base Address
	volatile u32 LimitAddr;           // +0x214, Limit Address
	volatile u32 LowTargetAddr;       // +0x218, Low Target Address
	volatile u32 HiTargetAddr;        // +0x21C, High Target Address
}IATU_REGISTERS, *PIATU_REGISTERS;

/***********************************************************/
/* DMA Registers                                           */
/***********************************************************/
#define PCIE0_DMA_WR_ENG_EN_REG			(PCIE0_PL_REG + 0x27C)
#define PCIE0_DMA_WR_INT_STAT_REG		(PCIE0_PL_REG + 0x2BC)
#define PCIE0_DMA_WR_INT_MSK_REG		(PCIE0_PL_REG + 0x2C4)
#define PCIE0_DMA_WR_INT_CLR_REG		(PCIE0_PL_REG + 0x2C8)
#define PCIE0_DMA_WR_DRBL_REG			(PCIE0_PL_REG + 0x280)
#define PCIE0_DMA_WR_ERR_STAT			(PCIE0_PL_REG + 0x2CC)

#define PCIE0_DMA_WR_DN_IMWR_ADDRL_REG		(PCIE0_PL_REG + 0x2D0)
#define PCIE0_DMA_WR_DN_IMWR_ADDRH_REG		(PCIE0_PL_REG + 0x2D4)

#define PCIE0_DMA_WR_ABT_IMWR_ADDRL_REG		(PCIE0_PL_REG + 0x2D8)
#define PCIE0_DMA_WR_ABT_IMWR_ADDRH_REG		(PCIE0_PL_REG + 0x2DC)

#define PCIE0_DMA_WR_CNL0_IMWR_DATA_REG		(PCIE0_PL_REG + 0x2E0)
#define PCIE0_DMA_WR_LL_ERR_EN_REG		(PCIE0_PL_REG + 0x300)

#define PCIE0_DMA_RD_ENG_EN_REG			(PCIE0_PL_REG + 0x29C)
#define PCIE0_DMA_RD_INT_STAT_REG		(PCIE0_PL_REG + 0x310)
#define PCIE0_DMA_RD_INT_MSK_REG		(PCIE0_PL_REG + 0x318)
#define PCIE0_DMA_RD_INT_CLR_REG		(PCIE0_PL_REG + 0x31C)
#define PCIE0_DMA_RD_DRBL_REG			(PCIE0_PL_REG + 0x2A0)

#define PCIE0_DMA_RD_ERR_STATL_REG		(PCIE0_PL_REG + 0x324)
#define PCIE0_DMA_RD_ERR_STATH_REG		(PCIE0_PL_REG + 0x328)

#define PCIE0_DMA_RD_CNL0_IMWR_DATA_REG		(PCIE0_PL_REG + 0x34C)
#define PCIE0_DMA_RD_LL_ERR_EN_REG		(PCIE0_PL_REG + 0x334)

#define PCIE0_DMA_RD_DN_IMWR_ADDRL_REG		(PCIE0_PL_REG + 0x33C)
#define PCIE0_DMA_RD_DN_IMWR_ADDRH_REG		(PCIE0_PL_REG + 0x340)

#define PCIE0_DMA_RD_ABT_IMWR_ADDRL_REG		(PCIE0_PL_REG + 0x344)
#define PCIE0_DMA_RD_ABT_IMWR_ADDRH_REG		(PCIE0_PL_REG + 0x348)

#define PCIE0_DMA_CNL_CXT_IND_REG		(PCIE0_PL_REG + 0x36C)
#define PCIE0_DMA_CNL_CTRL_REG			(PCIE0_PL_REG + 0x370)
#define PCIE0_DMA_TR_SIZE_REG			(PCIE0_PL_REG + 0x378)
#define PCIE0_DMA_SAR_LOW_REG			(PCIE0_PL_REG + 0x37C)
#define PCIE0_DMA_SAR_HIGH_REG			(PCIE0_PL_REG + 0x380)
#define PCIE0_DMA_DAR_LOW_REG			(PCIE0_PL_REG + 0x384)
#define PCIE0_DMA_DAR_HIGH_REG			(PCIE0_PL_REG + 0x388)

#define PCIE0_DMA_LLP_LOW_REG			(PCIE0_PL_REG + 0x38C)
#define PCIE0_DMA_LLP_HIGH_REG			(PCIE0_PL_REG + 0x390)

/*******************************************************/
/* BAR MASK Registers (uses dbi_cs2)                   */
/*******************************************************/
// ?? dbi_cs2 bit position; if it is 0x100000 - then it is correct
#define PCIE0_BAR0_MASK_REG             (PCIE0_CFG_BASE + 0x100010)

/* Transmit and receive start address */
// ??
#define TRANS_START_ADDR         PCIE0_SLV_AXI_BASE

/*******************************************************/
/* X1C Configuration Registers                         */
/*******************************************************/
#define X1C_CFG_REG0	(RAD_BASIC_CFG_BASEADDR + 0x78)
#define X1C_CFG_REG1	(RAD_BASIC_CFG_BASEADDR + 0x9C)

typedef struct __PCIeX1_CONTROLLER_CONFIG_REG0 {
	volatile u32 device_type:4;	// 3:0	R/W	0x0	Device Type (Page 333 of PCIe Ref. Manual)
					// 	0x0 – PCI Express Endpoint
					// 	0x1 – Legacy PCI Express Endpoint
					// 	0x4 – Root port of PCI Express Root Complex
					// 	0x5 – Upstream port of Switch
					// 	0x6 – Downstream port of Switch
	volatile u32 reserved_0:3;	// 6:4	R/W	0x0	Reserved
	volatile u32 sys_int:1;		// 7	R/W	0x0	sys_int (Page 335 of PCIe Ref. Manual)
	volatile u32 sys_aux_pwr_det:1;	// 8	R/W	0x0	sys_aux_pwr_det (Page 342 of PCIe Ref. Manual)
	volatile u32 reserved_1:23;	// 31:9	R/W	0x0	Reserved
} PCIeX1_CONTROLLER_CONFIG_REG0;

typedef struct __PCIeX1_CONTROLLER_CONFIG_REG1 {
	volatile u32 app_ltssm_enable:1;	// 0	R/W	0x0	Connected to app_ltssm_enable port (Page 333 of Synopsys PCIe Controller reference manual) of X1 PCIe Controller and clock reset block.
	volatile u32 phy_clk_req_n:1;		// 1	R/W	0x0	Connected to phy_clk_req_n port of PCIe X1 Clock Reset block.
	volatile u32 app_clk_req_n:1;		// 2	R/W	0x0	Connected to app_clk_req_n port of PCIe X1 Clock Reset block.
	volatile u32 sys_aux_pwr_det:1;		// 3	R/W	0x0	Connected to sys_aux_pwr_det port of PCIe X1 Clock Reset block.
	volatile u32 reserved:28;		// 32:4
} PCIeX1_CONTROLLER_CONFIG_REG1;


/*******************************************************/
/* X1C Status Registers                                */
/*******************************************************/
#define X1C_STS_REG0	(RAD_BASIC_CFG_BASEADDR + 0x100)
#define X1C_STS_REG1	(RAD_BASIC_CFG_BASEADDR + 0x104)
#define X1C_STS_REG2	(RAD_BASIC_CFG_BASEADDR + 0x110)
#define X1C_STS_REG5	(RAD_BASIC_CFG_BASEADDR + 0x114)
#define X1C_STS_REG6	(RAD_BASIC_CFG_BASEADDR + 0x118)

typedef struct __PCIeX1_CONTROLLER_STATUS_REG0 {
	volatile u32 reserved_0:8;	// 7:0		RO	0x0	 Reserved
	volatile u32 pm_curnt_state:3;	// 10:8		RO	0x0	pm_curnt_state[2:0], indicates current power state, used for debugging. (Page 371of Synopsys PCIe Controller Ref. Manual)
	volatile u32 reserved_1:5;	// 15:11	RO	0x0	Reserved
	volatile u32 rdlh_link_up:1;	// 16		RO	0x0	rdlh_link_up, Data link layer up/down indicator
					//			1 – Link is up
					//			0 – Link is down
					//			(Page 371of Synopsys PCIe Controller Ref. Manual)
	volatile u32 xmlh_link_up:1;	// 17		RO	0x0	xmlh_link_up , PHY link up/down indicator
					// 			1 – Link is up
					//			0 – Link is down
					//			(Page 370 of Synopsys PCIe Controller Ref. Manual)
	volatile u32 reserved_2:14;	// 31:18	RO	0x0	Reserved
} PCIeX1_CONTROLLER_STATUS_REG0;


/*******************************************************/
/* DWC Configuration Registers                         */
/*******************************************************/
// +
#define DWC_CFG_REG0    (DWC_CFG_BASE + 0x00)
#define DWC_CFG_REG1    (DWC_CFG_BASE + 0x04)
#define DWC_CFG_REG2    (DWC_CFG_BASE + 0x08)
#define DWC_CFG_REG3    (DWC_CFG_BASE + 0x0C)
#define DWC_CFG_REG4    (DWC_CFG_BASE + 0x10)
#define DWC_CFG_REG5    (DWC_CFG_BASE + 0x14)
#define DWC_CFG_REG6    (DWC_CFG_BASE + 0x18)
#define DWC_CFG_REG7    (DWC_CFG_BASE + 0x1C)
#define DWC_CFG_REG20   (DWC_CFG_BASE + 0x20)

typedef struct __PCIeX4_CONTROLLER_CONFIG_REGISTERS {
//	Register 0	0x200
	volatile u32 device_type:4;		// 3:0		R/W	0x0	Connected to device_type port  of PCIe X4 controller
	volatile u32 dbi_csysreq:1;		// 4		R/W	0x0	Connected to dbi_ csysreq port of PCIe X4 controller
	volatile u32 slv_csysreq:1;		// 5		R/W	0x0	Connected to slv_csysreq port of PCIe X4 controller
	volatile u32 mstr_csysreq:1;		// 6		R/W	0x0	Connected to mstr_ csysreq port of PCIe X4 controller
	volatile u32 mstr_resp_err_map:2;	// 8:7		R/W	0x0	Connected to mstr_resp_err_map port of PCIe X4 controller
	volatile u32 slv_resp_err_map:6;	// 14:9		R/W	0x0	Connected to slv_resp_err_map port of PCIe X4 controller
	volatile u32 mstr_rmisc_info:13;	// 27:15	R/W	0x0	Connected to mstr_rmisc_info port of PCIe X4 controlller
	volatile u32 reserved_1:4;		// 31:28

//	Register 1	0x204
	volatile u32 slv_armisc_info:22;	// 21:0		R/W	0x0	Connected to slv_armisc_info port of PCIe X4 controller
	volatile u32 reserved_2:10;		// 31:22

//	Register 2	0x208
	volatile u32 mstr_bmisc_info:13;	// 12:0		R/W	0x0	Connected to mstr_bmisc_info port of PCIe X4 controller
	volatile u32 reserved_3:19;		// 31:13

//	Register 3	0x20C
	volatile u32 slv_awmisc_info:22;	// 21:0		R/W	0x0	Connected to slv_awmisc_info port of PCIe X4 controller
	volatile u32 reserved_4:10;		// 31:22

//	Register 4	0x210
	volatile u32 outband_pwrup_cmd:1;		// 0		R/W	0x0	Connected to outband_pwrup_cmd port of X4 controller
	volatile u32 rx_lane_flip_en:1;			// 1		R/W	0x0	Connected to rx_lane_flip_en port of X4 controller
	volatile u32 tx_lane_flip_en:1;			// 2		R/W	0x0	Connected to tx_lane_flip_en port of X4 controller
	volatile u32 sys_atten_button_pressed:1;	// 3		R/W	0x0	Connected to sys_atten_button_pressed port of X4 controller
	volatile u32 sys_aux_pwr_det:1;			// 4		R/W	0x0	Connected to sys_aux_pwr_det  port of X4 controller
	volatile u32 sys_cmd_cpled_int:1;		// 5		R/W	0x0	Connected to sys_cmd_cpled_int port of X4 controller
	volatile u32 sys_eml_interlock_engaged:1;	// 6		R/W	0x0	Connected to sys_eml_interlock_engaged port of X4 controller
	volatile u32 sys_int:1;				// 7		R/W	0x0	Connected to sys_int port of X4 controller
	volatile u32 sys_mrl_sensor_chged:1;		// 8		R/W	0x0	Connected to sys_mrl_sensor_chged port of X4 controller
	volatile u32 sys_mrl_sensor_chged_1:1;		// 9		R/W	0x0	Connected to sys_mrl_sensor_chged port of X4 controller
	volatile u32 sys_pre_det_chged:1;		// 10		R/W	0x0	Connected to sys_pre_det_chged port of X4 controller
	volatile u32 sys_pre_det_state:1;		// 11		R/W	0x0	Connected to sys_pre_det_state port of X4 controller
	volatile u32 sys_pwr_fault_det:1;		// 12		R/W	0x0	Connected to sys_pwr_fault_det port of X4 controller
	volatile u32 diag_ctrl_bus:3;			// 15:13	R/W	0x0	Connected to diag_ctrl_bus port of X4 controller
	volatile u32 reserved_5:16;			// 31:16

//	Register 5	0x214
	volatile u32 app_init_rst:1;		// 0	R/W	0x0	Connected to app_init_rst port of X4 controller
	volatile u32 app_ltssm_enable:1;	// 1	R/W	0x0	Connected to app_ltssm_enable port of X4 controller
	volatile u32 app_ready_entr_l32:1;	// 2	R/W	0x0	Connected to app_ready_entr_l32  port of X4 controller
	volatile u32 app_req_entr_l1:1;		// 3	R/W	0x0	Connected to app_req_entr_l1 port of X4 controller
	volatile u32 app_req_exit_l1:1;		// 4	R/W	0x0	Connected to app_req_exit_l1 port of X4 controller
	volatile u32 app_req_retry_en:1;	// 5	R/W	0x0	Connected to app_req_retry_en  port of X4 controller
	volatile u32 app_unlock_msg:1;		// 6	R/W	0x0	Connected to app_unlock_msg port of X4 controller
	volatile u32 apps_pm_xmt_pme:1;		// 7	R/W	0x0	Connected to apps_pm_xmt_pme port of X4 controller
	volatile u32 apps_pm_xmt_turnoff:1;	// 8	R/W	0x0	Connected to apps_pm_xmt_turnoff  port of X4 controller
	volatile u32 reserved_6:23;		// 31:9

//	Register 6	0x218
	volatile u32 phy_cfg_status;		// 31:0	R/W	0x0	 Connected to phy_cfg_status port of X4 controller

//	Register 7	0x21A
	volatile u32 phy_clk_req_n:1;		// 0	R/W	0x0	Connected to phy_clk_req_n port of PCIe clock reset
	volatile u32 app_clk_req_n:1;		// 1	R/W	0x0	Connected to app_clk_req_n port of PCIe clock reset
	volatile u32 reserved_7:30;		// 31:2

//	Register 20	0x220
	volatile u32 IRST_PIPE_RST_L0_B_A:1;	// 0	R/W	0x0	Connected to IRST_PIPE_RST_L0_B_A port of X4 Serdes
	volatile u32 IRST_PIPE_RST_L1_B_A:1;	// 1	R/W	0x0	Connected to IRST_PIPE_RST_L1_B_A port of X4 Serdes
	volatile u32 IRST_PIPE_RST_L2_B_A:1;	// 2	R/W	0x0	Connected to IRST_PIPE_RST_L2_B_A port of X4 Serdes
	volatile u32 IRST_PIPE_RST_L3_B_A:1;	// 3	R/W	0x0	Connected to IRST_PIPE_RST_L3_B_A port of X4 Serdes
	volatile u32 reserved_8:28;		// 31:4

} PCIeX4_CONTROLLER_CONFIG_REGISTERS;


#define DWC_STS_REG_BASE     (DWC_CFG_BASE + 0x24)
#define DWC_STS_REG0         (DWC_CFG_BASE + 0x24)
#define DWC_STS_REG1         (DWC_CFG_BASE + 0x28)
#define DWC_STS_REG2         (DWC_CFG_BASE + 0x2C)
#define DWC_STS_REG3         (DWC_CFG_BASE + 0x30)
#define DWC_STS_REG4         (DWC_CFG_BASE + 0x34)
#define DWC_STS_REG5         (DWC_CFG_BASE + 0x38)
#define DWC_STS_REG6         (DWC_CFG_BASE + 0x3C)
#define DWC_STS_REG7         (DWC_CFG_BASE + 0x5C)

typedef struct __PCIeX4_CONTROLLER_STATUS_REGISTERS {
//	Register 0	0x224
	volatile u32 dbi_cactive:1;		// 0		RO	0x0	Connected to dbi_cactive port of X4 controller
	volatile u32 slv_cactive:1;		// 1		RO	0x0	Connected to slv_cactive port of X4 controller
	volatile u32 mstr_cactive:1;		// 2		RO	0x0	Connected to mstr_cactive port of X4 controller
	volatile u32 dbi_csysack:1;		// 3		RO	0x0	Connected to dbi_csysack port of X4 controller
	volatile u32 slv_csysack:1;		// 4		RO	0x0	Connected to slv_csysack port of X4 controller
	volatile u32 mstr_csysack:1;		// 5		RO	0x0	Connected to mstr_csysack port of X4 controller
	volatile u32 reserved_1:8;		// 13:6		RO	0x0	Reserved
	volatile u32 pm_xtlh_block_tlp:1;	// 14		RO	0x0	Connected to pm_xtlh_block_tlp port of X4 controller
	volatile u32 xmlh_link_up:1;		// 15		RO	0x0	Connected to xmlh_link_up port of X4 controller
	volatile u32 rdlh_link_up:1;		// 16		RO	0x0	Connected to rdlh_link_up port of X4 controller
	volatile u32 pm_curnt_state:3;		// 19:17	RO	0x0	Connected to pm_curnt_state port of X4 controller
	volatile u32 reserved_2:12;		// 31:20

//	Register 1	0x228
	volatile u32 cxpl_debug_info_l;		// 31:0		RO	0x0	Connected to cxpl_debug_info [31:0] port of X4 controller

//	Register 2	0x22C
	volatile u32 cxpl_debug_info_h;		// 31:0		RO	0x0	Connected to cxpl_debug_info [63:32] port of X4 controller

//	Register 3	0x230
	volatile u32 mstr_armisc_info:24;	// 23:0		RO	0x0	Connected to mstr_armisc_info port of X4 Controller
	volatile u32 mstr_armisc_info_dma:6;	// 29:24	RO	0x0	Connected to mstr_armisc_info_dma port of X4 controller
	volatile u32 reserved_3:2;		// 31:30

//	Register 4	0x234
	volatile u32 mstr_awmisc_info:24;	// 23:0		RO	0x0	Connected to mstr_awmisc_info port of X4 Controller
	volatile u32 mstr_awmisc_info_dma:6;	// 29:24	RO	0x0	Connected to mstr_awmisc_info_dma port of X4 controller
	volatile u32 reserved_4:2;		// 31:30

//	Register 5	0x238
	volatile u32 slv_bmisc_info:11;		// 10:0		RO	0x0	Connected to slv_bmisc_info port of X4 controller
	volatile u32 slv_rmisc_info:11;		// 21:11	RO	0x0	Connected to slv_rmisc_info port of X4 controller
	volatile u32 cfg_pwr_ind:2;		// 23:22	RO	0x0	Connected to cfg_pwr_ind port of X4 controller
	volatile u32 atten_ind:2;		// 25:24	RO	0x0	Connected to atten_ind port of X4 controller
	volatile u32 reserved_5:6;		// 31:26

//	Register 6	0x23C
	volatile u32 mac_phy_clk_req_n:1;	// 0		RO	0x0	Connected to mac_phy_clk_req_n port of X4 PCIe clock reset block
	volatile u32 ref_clk_req_n:1;		// 1		RO	0x0	Connected to ref_clk_req_n port of X4 PCIe clock reset block
	volatile u32 reserved_6:30;		// 31:2

} PCIeX4_CONTROLLER_STATUS_REGISTERS;

// create COMPLETER ID (BDF)
#define COMBINE_BDF(bus_n, device_n, fun_n) ( (((bus_n) & 0xff) << 8) + (((device_n) & 0x1f) << 3) + ((fun_n) & 0x07) )

/* Exported Functions */

void PCIeDrvSetOBMemAT(u32 PCIeBaseAddr, u32 RegionID, u32 StartAddr, u32 LimitAddr, u32 TargetAddr);
void PCIeDrvSetOBCfg0AT(u32 PCIeBaseAddr,  u32 RegionID, u32 StartAddr, u16 BDF);
void PCIeDrvSetIBBarAT(u32 PCIeBaseAddr, u32 RegionID, u32 BarNum, u32 TargetAddr);
void PCIeDrvSetIBMemAT(u32 PCIeBaseAddr, u32 RegionID, u32 BaseAddr, u32 LimitAddr, u32 TargetAddr);
int PCIeDrvX4Init(int RC_mode, int PCIeRate, int lanes);

void start_PCIEX1_link_up(void);
void start_PCIEX4_link_up(void);

void wait_PCIEX1_link_up(void);
void wait_PCIEX4_link_up(void);

PCIe_retcode pcie_read(u32 eeprom_address, u8 *target_address, u32 len);
PCIe_retcode pcie_write(u32 src, u32 dst, u32 len);

#endif // _M822XX_PCIE_H_
