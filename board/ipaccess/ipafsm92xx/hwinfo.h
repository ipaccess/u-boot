#ifndef IPAFSM92XX_HWINFO_H_20140124
#define IPAFSM92XX_HWINFO_H_20140124

#include <common.h>

extern void print_ipa_hardware_info(void);
extern int read_eid(u32 * oui, u32 * serial);
extern int read_hardware_info(u8 * oscillator, u16 * hw_variant, u16 * pcb_assembly);

#endif
