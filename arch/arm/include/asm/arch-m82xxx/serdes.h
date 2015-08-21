#ifndef _SERDES_H
#define _SERDES_H

#define SERDES_X1_RESET_REG (RAD_BASIC_CFG_BASEADDR + 0x4)
#define SERDES_X4_RESET_REG (RAD_BASIC_CFG_BASEADDR + 0x8)

/* Exported Functions */
void  Serdes1SgmiiInit(u32 SrdsLoopback);

u32 SerdesRegsInit_PCIe(u32 SrdsId);
u32 Serdes0PcieInit(void);


#endif
