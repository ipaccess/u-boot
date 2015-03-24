#ifndef IPA_LED_H_20150324
#define IPA_LED_H_20150324

typedef enum { LED_OFF, LED_GREEN, LED_RED, LED_BLUE, LED_YELLOW, LED_CYAN, LED_MAGENTA, LED_WHITE, NUM_COLOURS } led_colour;
typedef enum { LED_PWR, LED_NWK, LED_MODE, LED_SVC, LED_GPS, NUM_LEDS } led_index;

extern int setLED(led_index index, led_colour colour);
extern void ledc_confidence(void);

#endif
