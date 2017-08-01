
#include "main.h"
#include "uart.h"
#include "mcp2515.h"
#include "can.h"

void init(void);
void delay(unsigned int ms);

#define	UART	0

int main(void)
{
    mcp2515_init();

    //unsigned char daten[8] = { 0x7B, 0x0F, 0x8E, 0x15, 0xC2, 0xAA, 0xAA, 0xFF };
    /*while(1)
    {
        //can_send_message( 0x048DABCD, daten, 8, CAN_EID );
        can_send_message( 0x00000123, daten, 5, CAN_RTR );
        daten[0] = daten[0] + 3;
        delay(1000);
    }*/
    while(1)
    {
        unsigned char temp, rtr = 0;
        temp = mcp2515_read_register( CANINTF );

        // Ñ­»·¶Ácan
        if ( temp & ((1<<RX0IF)|(1<<RX1IF)) )
        {
            unsigned char i;
            unsigned int iTemp = 0;

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
                iTemp = ( (unsigned int) mcp2515_read_register( RXB0SIDH ))<<3;
                iTemp |= (temp>>5) & 0x07;

                uart_put_hex((unsigned char) (iTemp>>8));
                uart_put_hex((unsigned char) iTemp & 0x00ff);
                uart_putc('\n');

                if ( mcp2515_read_register( RXB0DLC ) & (1<<RTR))
                    uart_puts_P("Remote Transmit Request\n");
            }
            else
            {
                uart_puts_P("Standard Identifier : ");
                iTemp = ( (unsigned int) mcp2515_read_register( RXB0SIDH ))<<3;
                iTemp |= (temp>>5) & 0x07;

                uart_put_hex((unsigned char) (iTemp>>8));
                uart_put_hex((unsigned char) iTemp & 0x00ff);

                uart_putc('\n');

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
                    unsigned char daten = mcp2515_read_register( RXB0D0 + i );
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

void delay(unsigned int ms)
{
    unsigned int zaehler;
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
