#ifndef IPA_LED_H_20150324
#define IPA_LED_H_20150324

typedef enum { LED_OFF, LED_GREEN, LED_RED, LED_YELLOW, NUM_COLOURS } led_colour;
typedef enum { LED_NWK, LED_SVC, LED_GPS, NUM_LEDS } led_index;

extern int setLED(led_index index, led_colour colour);
extern void led_confidence(void);

#endif
