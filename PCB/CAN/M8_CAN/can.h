/**
 * @file 	can.h
 * 
 * @author 	Fabian Greif
 * @date	22.3.2005
 *
 * @brief	
 */


#ifndef _CAN_H_
#define _CAN_H_ 1

#include "main.h"
#include "mcp2515.h"

#define	CAN_RTR		0x80
#define	CAN_EID		0x40

/**
 * @brief	Sendet eine CAN Nachricht
 *
 * @param	id		Identifier
 * @param	flags	diveres Flags ( CAN_RTR oder CAN_EID )
 * @param	length	Anzahl der Datenbytes
 * @param	*data	Pointer auf die Datenbytes
 * @return	none
 *
 * ... Genauere Beschreibung ...
 */
extern void can_send_message( uint32_t id, uint8_t *data, uint8_t length, uint8_t flags);

#endif 	// _CAN_H
