#include "mcp2515.h"

void spi_init(void);
uint8_t spi_putc( uint8_t data );
void mcp2515_init(void);
void mcp2515_write_register(uint8_t data, uint8_t adress);
uint8_t mcp2515_read_register(uint8_t adress);
void mcp2515_bit_modify(uint8_t data, uint8_t mask, uint8_t adress);
void mcp2515_write_register_p( uint8_t adress, uint8_t *data, uint8_t length );


void spi_init(void)
{
    DDRB |= (1<<SPI_CS)|(1<<SPI_SCK)|(1<<SPI_MOSI);
    PORTB |= (1<<SPI_CS);
    PORTB &= ~(1<<SPI_SCK)|(1<<SPI_MOSI)|(1<<SPI_MISO);

    /* Aktivieren des SPI Master Interfaces, fosc = fclk / 4 */
    SPCR = (1<<SPE)|(1<<MSTR)/*|(1<<SPIE)*/;
    //SPSR = (1<<SPI2X);
}


void mcp2515_init(void)
{
    /* SPI Interface initialisieren */
    spi_init();

    /* MCP2515 per Software Reset zurücksetzten */
    PORTB &= ~(1<<SPI_CS);
    spi_putc( SPI_RESET );
    PORTB |= (1<<SPI_CS);

    /* Device in den Konfigurations Modus versetzten, CLKOUT Pin deaktivieren */
    mcp2515_bit_modify( CANCTRL, 0xE0, (1<<REQOP2) );

    /*
         *	Einstellen des Bit Timings
         *
     *	Fosc 	   = 16MHz
         *	BRP        = 7					( teilen durch 8 )
         *	TQ = 2 * (BRP + 1) / Fosc		( => 1 µS )
         *
         *	Sync Seg   = 1TQ
         *	Prop Seg   = ( PRSEG + 1 ) * TQ  = 1 TQ
         *	Phase Seg1 = ( PHSEG1 + 1 ) * TQ = 3 TQ
         *	Phase Seg2 = ( PHSEG2 + 1 ) * TQ = 3 TQ
         *
         *	Bus speed  = 1 / (Total # of TQ) * TQ
         *			   = 1 / 8 * TQ = 125 kHz
         */

    /* BRP = 7 */
    mcp2515_write_register( CNF1, (1<<BRP0)|(1<<BRP1)|(1<<BRP2) );

    /* Prop Seg und Phase Seg1 einstellen */
    mcp2515_write_register( CNF2, (1<<BTLMODE)|(1<<PHSEG11) );

    /* Wake-up Filter deaktivieren, Phase Seg2 einstelen */
    mcp2515_write_register( CNF3, (1<<PHSEG21) );

    /* Aktivieren der Rx Buffer Interrupts */
    mcp2515_write_register( CANINTE, /*(1<<RX1IE)|(1<<RX0IE)*/ 0 );

    /*
         *	Einstellen der Filter
         */

    /* Buffer 0 : Empfangen aller Nachrichten */
    mcp2515_write_register( RXB0CTRL, (1<<RXM1)|(1<<RXM0) );

    /* Buffer 1 : Empfangen aller Nachrichten */
    mcp2515_write_register( RXB1CTRL, (1<<RXM1)|(1<<RXM0) );

    uint8_t temp[4] = { 0, 0, 0, 0 };

    /* Filter für Buffer 0 */
    mcp2515_write_register_p( RXF0SIDH, temp, 4 );
    mcp2515_write_register_p( RXF1SIDH, temp, 4 );

    /* Filter für Buffer 1 */
    mcp2515_write_register_p( RXF2SIDH, temp, 4 );
    mcp2515_write_register_p( RXF3SIDH, temp, 4 );
    mcp2515_write_register_p( RXF4SIDH, temp, 4 );
    mcp2515_write_register_p( RXF5SIDH, temp, 4 );

    /* Maske für Buffer 0 */
    mcp2515_write_register_p( RXM0SIDH, temp, 4 );

    /* Maske für Buffer 1 */
    mcp2515_write_register_p( RXM1SIDH, temp, 4 );

    /*
         *	Einstellen der Pin Funktionen
         */

    /* Deaktivieren der Pins RXnBF Pins ( High Impedance State ) */
    mcp2515_write_register( BFPCTRL, 0 );

    /* TXnRTS Bits als Inputs schalten */
    mcp2515_write_register( TXRTSCTRL, 0 );

    /* Device zurück in den normalen Modus versetzten */
    mcp2515_bit_modify( CANCTRL, 0xE0, 0);
}

// -------------------------------------------------------------------------
/*
 *
 */
uint8_t spi_putc( uint8_t data )
{
    /* Sendet ein Byte */
    SPDR = data;
    /* Wartet bis Byte gesendet wurde */
    while( !( SPSR & (1<<SPIF) ) )
        ;
    return SPDR;
}

// -------------------------------------------------------------------------
/*
 *
 */
void mcp2515_write_register( uint8_t adress, uint8_t data )
{
    /* CS low */
    PORTB &= ~(1<<SPI_CS);

    spi_putc(SPI_WRITE);

    spi_putc(adress);

    spi_putc(data);

    /* CS high */
    PORTB |= (1<<SPI_CS);
}

// -------------------------------------------------------------------------
/*
 *
 */
uint8_t mcp2515_read_register(uint8_t adress)
{
    uint8_t data;

    /* CS low */
    PORTB &= ~(1<<SPI_CS);

    spi_putc(SPI_READ);

    spi_putc(adress);

    data = spi_putc(0xff);

    /* CS high */
    PORTB |= (1<<SPI_CS);

    return data;
}

// -------------------------------------------------------------------------
/*
 *
 */
uint8_t mcp2515_read_rx_buffer(uint8_t adress)
{
    uint8_t data;

    /* Überprüfen ob die Adresse richtig ist */
    if (adress & 0xF9)
        return 0;

    /* CS low */
    PORTB &= ~(1<<SPI_CS);

    spi_putc(SPI_READ_RX | adress);

    data = spi_putc(0xff);

    /* CS high */
    PORTB |= (1<<SPI_CS);

    return data;
}

// -------------------------------------------------------------------------
/*
 *
 */
void mcp2515_bit_modify(uint8_t adress, uint8_t mask, uint8_t data)
{
    /* CS low */
    PORTB &= ~(1<<SPI_CS);

    spi_putc(SPI_BIT_MODIFY);

    spi_putc(adress);

    spi_putc(mask);

    spi_putc(data);

    /* CS high */
    PORTB |= (1<<SPI_CS);
}

// -------------------------------------------------------------------------
/*
 *	Beschreibt mehrere Register auf einmal
 */
void mcp2515_write_register_p( uint8_t adress, uint8_t *data, uint8_t length )
{
    uint8_t i;

    /* CS low */
    PORTB &= ~(1<<SPI_CS);

    spi_putc(SPI_WRITE);

    spi_putc(adress);

    for (i=0; i<length ;i++ )
        spi_putc(*data++);

    /* CS high */
    PORTB |= (1<<SPI_CS);
}

// -------------------------------------------------------------------------
/*
 *	Beschreibt mehrer Register auf einmal
 */
void mcp2515_read_register_p( uint8_t adress, uint8_t *data, uint8_t length )
{
    uint8_t i;

    /* CS low */
    PORTB &= ~(1<<SPI_CS);

    spi_putc(SPI_READ);

    spi_putc(adress);

    for (i=0; i<length ;i++ )
        *data++ = spi_putc(0xff);

    /* CS high */
    PORTB |= (1<<SPI_CS);
}
