/*
 * tinytouchlib.c
 *
 * Created: 09.06.2013 17:25:47			-	v0.1 Initial release
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
			
			Reference	Analog0	= PB0
			Sense		Analog1 = PB1
*/	

void	tinytouch_init(void) {
	
	PRR		&=~_BV(PRADC);
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
	ADMUX	=0x00;	// Charge S/H cap from Analog0		
	PORTB |= _BV(PB0);		// Charge S/H Cap 
	PORTB &=~_BV(PB1);		// Discharge Pad (0)
	DDRB  |= _BV(PB0)|_BV(PB1);
		
	_delay_us(32);
	
	DDRB  &=~(_BV(PB1));	// float pad input, note that pull up is off.
	ADMUX	=0x01;			// Read Cext from Analog2
							// additional delay due to ADC logic
		
	ADCSRA	|=_BV(ADSC); // Start conversion	
	while (!(ADCSRA&_BV(ADIF)));		
	ADCSRA	|=_BV(ADIF); // Clear ADIF

	dat1=ADCL;

	// Precharge High
	ADMUX	=0x00;	// Charge S/H cap from Analog0
	PORTB &=~_BV(PB0);		// Discharge S/H Cap
	PORTB |= _BV(PB1);		// Charge Pad 
	DDRB  |= _BV(PB0)|_BV(PB1);
			
	_delay_us(32);

	DDRB  &=~(_BV(PB1));	// float pad input input
	PORTB &=~_BV(PB1);		// pull up off
	ADMUX	=0x01;			// Read Cext from Analog1
							// additional delay due to ADC logic
							
	ADCSRA	|=_BV(ADSC); // Start conversion	
	while (!(ADCSRA&_BV(ADIF)));
	ADCSRA	|=_BV(ADIF); // Clear ADIF

	dat2=ADCL;

	return dat2-dat1;
}

