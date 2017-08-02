#include "mcp2515.h"

#define SPI_CS_L	PORTB&=~(1<<SPI_CS)
#define SPI_CS_H	PORTB|=(1<<SPI_CS)

void spi_init(void);
unsigned char spi_putc( unsigned char data );
void mcp2515_init(void);
void mcp2515_write_register(unsigned char data, unsigned char adress);
unsigned char mcp2515_read_register(unsigned char adress);
void mcp2515_bit_modify(unsigned char data, unsigned char mask, unsigned char adress);
void mcp2515_write_register_p( unsigned char adress, unsigned char *data, unsigned char length );

void spi_init(void)
{
    DDRB |= (1<<SPI_CS)|(1<<SPI_SCK)|(1<<SPI_MOSI);
    PORTB |= (1<<SPI_CS);
    PORTB &= ~(1<<SPI_SCK)|(1<<SPI_MOSI)|(1<<SPI_MISO);
    /* set SPI master interfaces, fosc = fclk / 4 */
    SPCR = (1<<SPE)|(1<<MSTR)/*|(1<<SPIE)*/;
    //SPSR = (1<<SPI2X);
}

void mcp2515_init(void)
{
    spi_init();
    SPI_CS_L;
    spi_putc( SPI_RESET );
    SPI_CS_H;
	_delay_ms(1);
    mcp2515_bit_modify( CANCTRL, 0xE0, (1<<REQOP2) );

/*
    mcp2515_write_register( CNF1, (1<<BRP0)|(1<<BRP1) );		   	// SJW = 1*TO; BRP=3  TQ = 2*(BRP+1)/FOSC  
    mcp2515_write_register( CNF2, (1<<BTLMODE)|(1<<PHSEG11) );		// PS1 = 3*TQ; RSEG= 2*T  
    mcp2515_write_register( CNF3, (1<<PHSEG21)|(1<<PHSEG20) );		// PS2 = 4*TQ;   
    mcp2515_write_register( CANINTE, (1<<RX1IE)|(1<<RX0IE) );				
    mcp2515_write_register( RXB0CTRL, (1<<RXM1)|(1<<RXM0) );
    mcp2515_write_register( RXB1CTRL, (1<<RXM1)|(1<<RXM0) );
  */

    mcp2515_write_register(CNF1, 0x03);               // SJW = 1*TO; BRP=3  TQ = 2*(BRP+1)/FOSC  
    mcp2515_write_register(CNF2, 0x91);               // PS1 = 3*TQ; RSEG= 2*T  
    mcp2515_write_register(CNF3, 0x03);               // PS2 = 4*TQ;   
	mcp2515_write_register( RXB0CTRL, (1<<RXM1)|(1<<RXM0) );
    mcp2515_write_register( RXB1CTRL, (1<<RXM1)|(1<<RXM0) );
    mcp2515_write_register(TXRTSCTRL , 0x00);          //Disable RTS PINs  

/*  
    unsigned char temp[4] = { 0, 0, 0, 0 };
    
    mcp2515_write_register_p( RXF0SIDH, temp, 4 );
    mcp2515_write_register_p( RXF1SIDH, temp, 4 );
    
    // empty  Buffer 1
    mcp2515_write_register_p( RXF2SIDH, temp, 4 );
    mcp2515_write_register_p( RXF3SIDH, temp, 4 );
    mcp2515_write_register_p( RXF4SIDH, temp, 4 );
    mcp2515_write_register_p( RXF5SIDH, temp, 4 );
    mcp2515_write_register_p( RXM0SIDH, temp, 4 );
    mcp2515_write_register_p( RXM1SIDH, temp, 4 );
    
    // Deaktivieren der Pins RXnBF Pins ( High Impedance State )
    mcp2515_write_register( BFPCTRL, 0 );
    // TXnRTS Bits als Inputs schalten 
    mcp2515_write_register( TXRTSCTRL, 0 );
	*/
	
//	mcp2515_bit_modify( CANCTRL, 0XE0, (1<<REQOP1) );		// 回环模式测试

    mcp2515_bit_modify( CANCTRL, 0xE0, 0 );		// set to normal mode		
}

unsigned char spi_putc( unsigned char data )
{
    SPDR = data;
    while( !( SPSR & (1<<SPIF) ) );
    return SPDR;
}

void mcp2515_write_register( unsigned char adress, unsigned char data )
{
    SPI_CS_L;
    spi_putc(SPI_WRITE);
    spi_putc(adress);
    spi_putc(data);
    SPI_CS_H;
}


unsigned char mcp2515_read_register(unsigned char adress)
{
    unsigned char data;
    SPI_CS_L;
    spi_putc(SPI_READ);
    spi_putc(adress);
    data = spi_putc(0xff);
    SPI_CS_H;
    return data;
}


unsigned char mcp2515_read_rx_buffer(unsigned char adress)
{
    unsigned char data;
    /* 躡erpr黤en ob die Adresse richtig ist */
    if (adress & 0xF9)
        return 0;
    SPI_CS_L;
    spi_putc(SPI_READ_RX | adress);
    data = spi_putc(0xff);
    SPI_CS_H;
    return data;
}


void mcp2515_bit_modify(unsigned char adress, unsigned char mask, unsigned char data)
{
    SPI_CS_L;
    spi_putc(SPI_BIT_MODIFY);
    spi_putc(adress);
    spi_putc(mask);
    spi_putc(data);
    SPI_CS_H;
}

/*
 *	Beschreibt mehrere Register auf einmal
 */
void mcp2515_write_register_p( unsigned char adress, unsigned char *data, unsigned char length )
{
    unsigned char i;
    SPI_CS_L;
    spi_putc(SPI_WRITE);
    spi_putc(adress);
    for (i=0; i<length ;i++ )
        spi_putc(*data++);
    SPI_CS_H;
}


void mcp2515_read_register_p( unsigned char adress, unsigned char *data, unsigned char length )
{
    unsigned char i;
    SPI_CS_L;
    spi_putc(SPI_READ);
    spi_putc(adress);
    for (i=0; i<length ;i++ )
        *data++ = spi_putc(0xff);
    SPI_CS_H;
}
