#ifndef IPAFSM92XX_PM8058_H_20140118
#define IPAFSM92XX_PM8058_H_20140118

#include <common.h>

int pm8058_init();
int pm8058_read(u32 address, u8 * value);
int pm8058_write(u32 address, u8 value);

#endif
