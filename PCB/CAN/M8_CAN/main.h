
#ifndef _MAIN_H_
#define _MAIN_H_ 1

#include <avr/io.h>
#include <avr/interrupt.h>

#ifndef F_CPU
	#define F_CPU	7372800UL		/**< Taktfrequenz des ATMega 8 */
#endif
#define UART_BAUD	38400UL			/**< UART Baudrate */


#endif // _MAIN_H_
