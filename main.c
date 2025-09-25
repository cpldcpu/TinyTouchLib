/*
 v0.1	06/10/2013
 
 TinyTouchLibrary Example
 
 Hardware:
	- Attiny 13A/25/45/85
	- LED on PB4
	- Touchbutton on PB3
 */ 

 
#include <avr/io.h>
#include <util/delay.h>
#include "TinyTouchLib.h"

int main(void)
{
	CLKPR=_BV(CLKPCE);
	CLKPR=0;			// set clock prescaler to 1 (attiny 25/45/85/24/44/84/13/13A)		

	DDRB=_BV(PB4);		// Enable LED output pin
	
	tinytouch_init();
	
	while(1) {
		if (tinytouch_sense()==tt_push) {PORTB^=_BV(PB4);}  // Toggle LED on touch event		
		_delay_ms(10);	
	}	
}

