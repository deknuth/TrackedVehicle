/**
 * @file 	uart.h
 *
 * @author 	Fabian Greif
 * @date	22.3.2005
 *
 * @brief	enthät Funktionen zur Ansteuerung des UART
 *
 * 
 */

#ifndef _UART_H_
#define _UART_H_ 1

#include "main.h"
#include <avr/pgmspace.h>

/**
 * @brief	Anzahl der Bytes des Sendepuffers ( 4, 8, 16, 32, 64 usw. ... )
 */
#define TX_BUF_SIZE			32

/**
 * @name	Nützliche Makros
 */
 
/**
 * @brief	Erzeugt aus den Werten für die Taktfrequenz und Baudraten
 *			den den entsprechnden Wert für das Register UBRR
 */
#define UBRR_BAUD	((F_CPU/(16L*UART_BAUD))-1)

/*@}*/

/**
 * @brief	Macro um einen konstanten String automatisch im Programmspeicher abzulegen
 *
 * Beispiel:
 * \code
 * uart_puts_P( "Hallo Welt!");
 * \endcode
 *
 * @see		uart_puts_p
 */
#define uart_puts_P(__s)         uart_puts_p(PSTR(__s))

/**
 * @brief	Initialisiert den UART des AVRs
 *
 * Stellt die Baudrate ein, aktiviert den UART und initialisiert den Empfangspuffer
 */
extern void uart_init(void);

/**
 * @brief	Schiebt ein Zeichen in den Ringbuffer
 *
 * @param	data	Zeichen das gespeichert werden soll
 * @return	none
 *
 * Funktion stammt von Peter Fleury ( http://jump.to/fleury ) aus seiner UART Libary
 *
 * Die Zeichen im Ringbuffer werden per Interrupt automatisch ausgegeben.
 */
extern void uart_putc(uint8_t data);

/**
 * @brief	Sendet einen String aus dem Programmspeicher über die serielle Schnittstelle
 *
 * @param	progmem_s Pointer auf einen String im Flashspeicher
 * @return	none
 *
 * Funktion stammt von Peter Fleury ( http://jump.to/fleury ) aus seiner UART Libary
 *
 * @see		uart_puts_P
 */
extern void uart_puts_p(const char *progmem_s);

/**
 * @brief 	Gibt einen Integer ( 0..65535 ) mit oder ohne führende Nullen aus
 *
 * @param	wert	Zahl die ausgegeben werden soll
 * @param	anzahl	Anzahl der Zeichen die ausgegeben werden ( maximal 10 )
 * @param	fuell	Füllzeichen die am Anfang ausgegeben werden
 *
 * @return	none
 *
 * Beispiel :
 * \code
 * uart_put_dec(12345,6,'0')  ->  '012345'
 * uart_put_dec(12345,5,'')   ->  '12345'
 * uart_put_dec(12345,6,' ')  ->  ' 12345'
 * \endcode
 */
extern void uart_put_dec(uint16_t wert, uint8_t anzahl, uint8_t fuell);

/**
 * @brief	Gibt ein Byte Hexadezimal über den UART aus
 *
 * @param	wert	Zahl die ausgegeben werden soll
 * @return	none
 */
extern void uart_put_hex(uint8_t wert);

#endif 	// _UART_H
