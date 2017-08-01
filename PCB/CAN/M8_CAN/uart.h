
#ifndef _UART_H_
#define _UART_H_ 1

#include "main.h"
#include <avr/pgmspace.h>

#define TX_BUF_SIZE			32

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

extern void uart_putc(uint8_t data);

extern void uart_puts_p(const char *progmem_s);

/**
 * @brief 	Gibt einen Integer ( 0..65535 ) mit oder ohne führende Nullen aus
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
