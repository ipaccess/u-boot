/*
 * ipa30ff_led.h
 *
 *  Created on: 6 Feb 2013
 *      Author: sk
 */

#ifndef IPA73XX_LED_H_
#define IPA73XX_LED_H_

#define IOWRITE32(vALUE, aDDRESS)   *((volatile u32 *)(aDDRESS)) = (vALUE)
#define IOREAD32(aDDRESS)           (*((volatile u32 *)(aDDRESS)))

typedef enum { LED_GREEN = 0x10, LED_RED = 0x11 } led_col;

void setLED(led_col col);

#endif /* IPA73XX_LED_H_ */

