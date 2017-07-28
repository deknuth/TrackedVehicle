
#include "adc.h"


// -------------------------------------------------------------------------
/*
 * Globale Variablen
 */
#if MOVING_AVERAGE
volatile uint16_t ad_tmp[4][4];
#else
volatile uint16_t ad_tmp[4];
#endif

// -------------------------------------------------------------------------
/*
 * Funktionsdefinitionen
 */

void adc_init( void );

uint16_t get_adc( uint8_t channel );

// -------------------------------------------------------------------------
/*
 * Initialisiert den A/D-Wandler
 */
void adc_init( void )
{
	/* AD Wandler einstellen auf externes Aref, Prescaler = 128 */
	ADMUX = 0 | ADC_REF;
	ADCSRA = (1<<ADEN)|(1<<ADSC)|(1<<ADIE)|(1<<ADPS2)|(1<<ADPS1)|(1<<ADPS0);
	SFIOR = 0;

	/* Analog Comperator deaktivieren */
	ACSR = (1<<ACD);
}

// -------------------------------------------------------------------------
/*
 * Gibt den Wert eines A/D-Wandler Kanals zurück
 */
uint16_t get_adc( uint8_t channel )
{
	if (channel > 3)
		return 0;
	
	#if MOVING_AVERAGE
	return ((ad_tmp[channel][0] + ad_tmp[channel][1] + ad_tmp[channel][2] + ad_tmp[channel][3])>>2);
	#else
	return ad_tmp[channel];
	#endif
}

// -------------------------------------------------------------------------
/*
 * AD-Wandler Interruptroutine
 *
 * Abgefragt werden die Kanäle:
 * 	0	ADC0
 * 	1	ADC1
 * 	2	ADC2
 * 	3	ADC3
 *
 * Es lässt sich nur "Single Ended Converstion" benutzen
 *
 * Die Werte können über einen 4-Punkt Moving Average Filter geglättet werden.
 *  ( siehe adc.h )
 */
SIGNAL(SIG_ADC)
{
	static uint8_t ad_kanal = 0;
	#if MOVING_AVERAGE
	static uint8_t ad_filter_punkt = 0;
	#endif

	uint16_t temp = 0;
	uint8_t kanal = ad_kanal;
	
	/* AD-Wandler Resultat auslesen */
	temp = ADC;

	#if MOVING_AVERAGE
	ad_tmp[kanal][ad_filter_punkt] = temp;
	#else
	ad_tmp[kanal] = temp;
	#endif

	/* Nächsten AD Kanal auswählen */
	kanal++;
	if (kanal > 3)
	{
		kanal = 0;
		
		#if MOVING_AVERAGE
		ad_filter_punkt++;
		if (ad_filter_punkt > 3)
			ad_filter_punkt = 0;
		#endif
	}
	
	ADMUX = kanal | ADC_REF;
	
	ad_kanal = kanal;
	
	/* A/D-Wandler neu starten */
	ADCSRA |= (1<<ADSC);
}
