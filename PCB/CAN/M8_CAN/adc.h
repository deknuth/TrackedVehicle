/**
 * @file 	adc.h
 * 
 * @author 	Fabian Greif
 * @date	20.2.2005
 *
 * @brief	enthät Funktionen zur Ansteuerung des Analog/Digital-Wandlers
 *
 * Der AD-Wandler sampelt in einer Art selbstgeschriebenem Free-Running-Modus vor sich hin.
 * Die Werte können dabei über einen 4-Punkt-Moving-Average Filter geglättet werden (abschaltbar).
 * 
 * @see		MOVING_AVERAGE
 *
 * Man kann auf die A/D-Werte über die Funktion get_adc() zugreifen.
 *
 * Es dauert ca. 600 µSekunden bis alle Kanäle abgefragt wurden. 
 */

#ifndef _ADC_H_
#define _ADC_H_ 1

#include "main.h"

/**
 * @brief	Aktivieren/deaktivieren des 4-Punkt-Moving-Average Filters
 *
 * Wird der Wert auf Null gesetzt, so wird keine Filterung der Werte vorgenommen.
 */
#define	MOVING_AVERAGE	1

/**
 * @brief	Einstellen der ADC Referenzspannung
 *
 * Über diesen Wert kann die Referenzspannung angepasst werden: \n
 * \b 0x00	-  externe Spannung an Aref \n
 * \b 0x40  -  AVCC mit einem Kondensator an Aref \n
 * \b 0x80  -  Reserviert ( nicht benutzten ) \n
 * \b 0xC0  -  interne 2,56 Volt Referenzspannung mit Kondensator an Aref \n
 *
 * @see		Datenblatt des ATMega16, Seite 211
 */
#define	ADC_REF		0x40

/**
 * @brief	Initialisiert den A/D-Wandler
 *
 * Prescaler = 128 -> f_adc = 125 kHz bei 16 MHz
 */
extern void adc_init(void);

/**
 * @brief	Gibt den Wert eines A/D-Wandler Kanals zurück
 *
 * @param	channel	Kanal des A/D-Wandlers der abgefragt werden soll \n
 *			\b 0 ->	ADC0 \n
 * 			\b 1 ->	ADC1 \n 
 * 			\b 2 ->	ADC2 \n
 * 			\b 3 ->	ADC3 \n 
 * @return	ad-wert
 *
 * @see		MOVING_AVERAGE
 *
 */
extern uint16_t get_adc( uint8_t channel );

#endif 	// _ADC_H
