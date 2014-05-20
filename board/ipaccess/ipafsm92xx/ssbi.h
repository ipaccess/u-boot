#ifndef IPAFSM92XX_SSBI_H_20140118
#define IPAFSM92XX_SSBI_H_20140118

#include <common.h>

#define SSBI2_0_BASE (0x94080000)
#define SSBI2_1_BASE (0x94090000)
#define SSBI2_2_BASE (0x940C0000)

#define SSBI_MODE_1             (0)
#define SSBI_MODE_2             (1)


int ssbi_init(u32 base);
int ssbi_ready(u32 base);
int ssbi_mode_set(u32 base, u32 mode, u32 width);
int ssbi_mode_get(u32 base, u32 * mode, u32 * width);
int ssbi_read(u32 base, u32 addr, u32 * value);
int ssbi_write(u32 base, u32 addr, u32 value);

#endif
