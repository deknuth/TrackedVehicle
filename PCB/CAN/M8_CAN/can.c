#include "can.h"
void can_send_message( uint32_t id, uint8_t *data, uint8_t length, uint8_t flags);

void can_send_message( uint32_t id, uint8_t *data, uint8_t length, uint8_t flags)
{
    uint8_t temp[4];

    if (length > 8)
        length = 0;

    /* Nachrichten Buffer auf Höchste Priorität einstellen */
    mcp2515_bit_modify( TXB0CTRL, (1<<TXP1)|(1<<TXP0), (1<<TXP1)|(1<<TXP0) );

    /* Identifier einstellen */
    if (flags & CAN_EID)
    {
        temp[0] = (uint8_t) (id>>21);
        temp[1] = (((uint8_t) (id>>16)) & 0x03) | (((uint8_t) (id>>13)) & 0xE0) | (1<<EXIDE);
        temp[2] = (uint8_t) (id>>8);
        temp[3] = (uint8_t) id;
    }
    else
    {
        temp[0] = (uint8_t) (id>>3);
        temp[1] = (uint8_t) (id<<5);
        temp[2] = 0;
        temp[3] = 0;
    }
    mcp2515_write_register_p( TXB0SIDH, temp, 4 );


    if (flags & CAN_RTR)
        mcp2515_write_register( TXB0DLC, length | (1<<RTR) );
    else
        mcp2515_write_register( TXB0DLC, length );

    mcp2515_write_register_p( TXB0D0, data, length );

    PORTB &= ~(1<<SPI_CS);
    spi_putc(SPI_RTS | 0x01);
    PORTB |= (1<<SPI_CS);
}
