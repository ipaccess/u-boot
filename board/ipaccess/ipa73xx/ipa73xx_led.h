/*
 * ipa30ff_led.h
 *
 *  Created on: 6 Feb 2013
 *      Author: sk
 */

#ifndef IPA73XX_LED_H_
#define IPA73XX_LED_H_

typedef enum { LED_OFF, LED_GREEN, LED_RED, LED_BLUE, LED_YELLOW, LED_CYAN, LED_MAGENTA, LED_WHITE, NUM_COLOURS } led_colour;
typedef enum { LED_PWR, LED_NWK, LED_MODE, LED_SVC, LED_GPS, NUM_LEDS } led_index;

int setLED(led_index index, led_colour colour);
void ledc_confidence(void);

#endif /* IPA73XX_LED_H_ */

