TinyTouchLib
============

Library for touch-button input on ATtiny AVR microcontrollers with integrated ADC.

// Library initialization
// Call this once to initialize the library functions and the ADC converter
void  tinytouch_init(void);

// The sense function evaluates the button state and performs internal 
// housekeeping. It should be polled at least 30 times per second to
// update the internal logic. Please note that each call performs 32
// analog to digital conversions with active waiting. This may take 
// several ms.

// Possible return values are:
//		tt_off=0	No touch sensed
//		tt_on		Touch button is active and touch is sensed.
//		tt_push		Touch button is pushed. Use this to initiate one time events.
//		tt_release	Touch button is released. Use this to initiate one time events.
//		tt_timeout	Touch button has been active too long and internal bias was	reset. 
uint8_t tinytouch_sense(void);
