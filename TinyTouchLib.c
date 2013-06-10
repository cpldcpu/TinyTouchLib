/*
 * tinytouchlib.c
 *
 * Created: 09.06.2013 17:25:47			-	v0.1 Initial release (ATtiny 10)
 *			10.06.2013					- 	v0.2 ported to ATtiny 25/45/85 and ATtiny13
 *  Author: Tim (cpldcpu@gmail.com)
 */ 

 
#include "TinyTouchLib.h"
#include <util/delay.h>

// Internal function to read the adc input
uint8_t tinytouch_adc(void);

uint16_t bias;
uint8_t	touch;
#if touch_timeout>0		
uint8_t timer;
#endif
/*
			Capacitive sensing using charge sharing between 
			the S/H capacitor and an external sensing pad			
*/	

void	tinytouch_init(void) {

#ifndef __AVR_ATtiny13__	
	PRR		&=~_BV(PRADC);
#endif	
	ADCSRA	=_BV(ADEN)|_BV(ADPS2)|_BV(ADPS1); // Enable ADC, Set prescaler to 64
		
	bias=tinytouch_adc()<<8;
	touch=0;
}

uint8_t tinytouch_sense(void) {
	uint8_t i;
	uint16_t tmp;
	int16_t delta;

	tmp=0;
	for (i=0; i<16; i++) {
		tmp+=tinytouch_adc();	// average 16 samples
		_delay_us(100);
	}

	delta=tmp-(bias>>4);
			
	if (!touch)	{
		if (delta>touch_threshold_on) {
			touch=1;
#if touch_timeout>0
			timer=0;					
#endif
			return tt_push;
		}

		// update bias only when touch not active
		bias=(bias-(bias>>6))+(tmp>>2);		// IIR low pass
		return tt_off;		
	} else {
		if (delta<touch_threshold_off) {
			touch=0;
			return tt_release;
		}

#if touch_timeout>0		
		if (timer==255) {
			bias=tinytouch_adc()<<8;			
			return tt_timeout;
		} 
		timer++;
#endif						
		return tt_on;
	}
}	

uint8_t tinytouch_adc(void) {	

	uint8_t dat1,dat2;

	// Precharge Low
	ADMUX	=tt_refadc;	// connect S/H cap to reference pin	
	PORTB |= _BV(tt_refpin);		// Charge S/H Cap 
	PORTB &=~_BV(tt_sensepin);		// Discharge Pad (0)
	DDRB  |= _BV(tt_refpin)|_BV(tt_sensepin);
		
	_delay_us(32);
	
	DDRB  &=~(_BV(tt_sensepin));	// float pad input, note that pull up is off.
	
#ifdef __AVR_ATtiny10__
	ADMUX	=tt_senseadc;	// Connect sense input to adc
#else
	ADMUX	=tt_senseadc|_BV(ADLAR);	// Connect sense input to adc
#endif
		
	ADCSRA	|=_BV(ADSC); // Start conversion	
	while (!(ADCSRA&_BV(ADIF)));		
	ADCSRA	|=_BV(ADIF); // Clear ADIF

#ifdef __AVR_ATtiny10__
	dat1=ADCL;
#else
	dat1=ADCH;
#endif

	// Precharge High
	ADMUX  =tt_refadc;	// connect S/H cap to reference pin
	PORTB &=~_BV(tt_refpin);		// Discharge S/H Cap
	PORTB |= _BV(tt_sensepin);		// Charge Pad 
	DDRB  |= _BV(tt_refpin)|_BV(tt_sensepin);
			
	_delay_us(32);

	DDRB  &=~(_BV(tt_sensepin));	// float pad input input
	PORTB &=~_BV(tt_sensepin);		// pull up off

	#ifdef __AVR_ATtiny10__
	ADMUX	=tt_senseadc;	// Connect sense input to adc
#else
	ADMUX	=tt_senseadc|_BV(ADLAR);	// Connect sense input to adc
#endif
							
	ADCSRA	|=_BV(ADSC); // Start conversion	
	while (!(ADCSRA&_BV(ADIF)));
	ADCSRA	|=_BV(ADIF); // Clear ADIF

#ifdef __AVR_ATtiny10__
	dat2=ADCL;
#else
	dat2=ADCH;
#endif
	

	return dat2-dat1;
}

