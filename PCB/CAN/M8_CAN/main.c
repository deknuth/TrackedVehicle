/*
 * CAN Testboard mit ATMega8 + MCP2515
 *
 *  Copyright (C) 2005 Fabian Greif
 *    email@kreatives-chaos.com
 */

#include "main.h"
#include "adc.h"
#include "uart.h"
#include "mcp2515.h"
#include "can.h"
#include "lcd.h"

void init(void);
void delay(uint16_t ms);

#define	UART	0

int main(void)
{
    adc_init();
    uart_init();
    mcp2515_init();
    init();

    //uint8_t daten[8] = { 0x7B, 0x0F, 0x8E, 0x15, 0xC2, 0xAA, 0xAA, 0xFF };
    /*while(1)
    {
        //can_send_message( 0x048DABCD, daten, 8, CAN_EID );
        can_send_message( 0x00000123, daten, 5, CAN_RTR );
        daten[0] = daten[0] + 3;
        delay(1000);
    }*/
    while(1)
    {
        uint8_t temp, rtr = 0;
        temp = mcp2515_read_register( CANINTF );

        // Ñ­»·¶Ácan
        if ( temp & ((1<<RX0IF)|(1<<RX1IF)) )
        {
            uint8_t i;
            uint16_t iTemp = 0;

            uart_puts_P("eine neue Nachricht wurde empfangen : \n");
            temp = mcp2515_read_register( RXB0SIDL );       
            if ( temp & (1<<IDE) )
            {
                uart_puts_P("Extended Identifier : ");
                uart_put_hex(mcp2515_read_register( RXB0SIDL) & 0x03 );
                uart_put_hex(mcp2515_read_register( RXB0EID8) );
                uart_put_hex(mcp2515_read_register( RXB0EID0) );
                uart_putc('\n');
                uart_puts_P("Standard Identifier : ");
                iTemp = ( (uint16_t) mcp2515_read_register( RXB0SIDH ))<<3;
                iTemp |= (temp>>5) & 0x07;

                uart_put_hex((uint8_t) (iTemp>>8));
                uart_put_hex((uint8_t) iTemp & 0x00ff);
                uart_putc('\n');

                if ( mcp2515_read_register( RXB0DLC ) & (1<<RTR))
                    uart_puts_P("Remote Transmit Request\n");
            }
            else
            {
                uart_puts_P("Standard Identifier : ");
                iTemp = ( (uint16_t) mcp2515_read_register( RXB0SIDH ))<<3;
                iTemp |= (temp>>5) & 0x07;

                /* Identifier ausgeben */
                uart_put_hex((uint8_t) (iTemp>>8));
                uart_put_hex((uint8_t) iTemp & 0x00ff);

                uart_putc('\n');

                /* Überprüfen ob es ein Standard Remote Transmit Request Frame ist */
                if ( temp & (1<<SRR))
                {
                    uart_puts_P("Remote Transmit Request\n");
                    rtr = 1;
                }
                else
                {
                    ;
                }
            }
            /* Datenlänge usw. einlesen */
            temp = mcp2515_read_register( RXB0DLC ) & 0x0F;
            if (rtr == 0)
            {
                /* Datenbytes ausgeben */
                for (i = 0;i < temp; i++ )
                {
                    uint8_t daten = mcp2515_read_register( RXB0D0 + i );
                    uart_puts_P("DByte ");
                    uart_putc(i + '0');
                    uart_puts_P(" : ");
                    uart_put_hex( daten );
                    uart_putc('\n');

                }
            }
            uart_putc('\n');
            /* Interrupt Flag löschen */
            mcp2515_bit_modify( CANINTF, 0x01, 0x00 );
        }
    }
    return 0;
}

void init(void)
{
    /* Eingang für Interruptleitung des MCP2515 + Pullup aktivieren */
    DDRD &= ~(1<<PD3);
    PORTD |= (1<<PD3);

    /* Externen Interrupt 0 aktivieren ( fallende Flanke ) */
    //	MCUCR |= (1<<ISC11);
    //	GICR |= (1<<INT1);

    sei();
}

void delay(uint16_t ms)
{
    uint16_t zaehler;

    while (ms)
    {
        zaehler = F_CPU / 5000;
        while (zaehler)
        {
            asm volatile ("nop");
            zaehler--;
        }
        ms--;
    }
}
