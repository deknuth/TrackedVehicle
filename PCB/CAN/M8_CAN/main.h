/**
 * @file 	main.h
 * 
 * @author 	Fabian Greif
 * @date	20.3.2005
 *
 * @brief	enth‰t Funktionen zur Ansteuerung eines MCP2515
 *
 * Es wird der Hardware SPI Bus des AVRs benutzt.
 * 
 * /CS des MCp2515 muss dabei an den SS Pin des SPI Interfaces angeschloﬂen sein.
 *
 */

#ifndef _MAIN_H_
#define _MAIN_H_ 1

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/signal.h>

/**
 * @name  Grundeinstellungen
 *
 */

#ifndef F_CPU
	#define F_CPU	7372800UL		/**< Taktfrequenz des ATMega 8 */
#endif
#define UART_BAUD	38400UL			/**< UART Baudrate */

/*@}*/

#endif // _MAIN_H_
