#include "uart.h"

#define MESSAGE_BUF_MASK	(MESSAGE_BUF_SIZE - 1)
#define RX_BUF_SIZE			(MESSAGE_BUF_SIZE * MAX_LAENGE)
#define TX_BUF_MASK			(TX_BUF_SIZE - 1)

static volatile unsigned char tx_buf[TX_BUF_SIZE];
static volatile unsigned char tx_tail = 0;
static volatile unsigned char tx_head = 0;
void uart_init(void);
void uart_putc(unsigned char data);
void uart_puts_p(const char *progmem_s);
void uart_put_dec(uint16_t wert, unsigned char anzahl, unsigned char fuell);
void uart_put_hex(unsigned char wert);

void uart_init(void)
{
	
	UBRR0H = (unsigned char) (UBRR_BAUD>>8);
    UBRR0L = (unsigned char) UBRR_BAUD & 0x00FF;
    UCSR0B = 1<<RXEN0 | 1<<TXEN0 | 1<<RXCIE0;
    UCSR0C = 1<<UCSZ00 | 1<<UCSZ01;
}


void uart_putc(unsigned char data)
{
    unsigned char tmphead;
    if (data == '\n')
        uart_putc('\r');
    tmphead  = (tx_head + 1) & TX_BUF_MASK;
    while ( tmphead == tx_tail );
    tx_buf[tmphead] = data;
    tx_head = tmphead;
    UCSR0B |= (1<<UDRIE0);
}


void uart_puts_p(const char *progmem_s)
{
    char c;
    while ( (c = pgm_read_byte(progmem_s++)) )
        uart_putc(c);
}


void uart_put_dec(uint16_t wert, unsigned char anzahl, unsigned char fuell)
{
    unsigned char i, s[10];
    for (i = 0; i < anzahl; i++)
    {
        s[anzahl - i - 1] = '0' + (wert % 10);
        wert /= 10;
    }

    for (i=0; i<(anzahl - 1); i++)
    {
        if (s[i] == '0')
            s[i] = fuell;
        else
            break;
    }

    for (i=0; i<anzahl; i++)
        uart_putc(s[i]);
}


void uart_put_hex(unsigned char wert)
{
    unsigned char hi,low;

    /* Obere 4-bit */
    hi = wert & 0xf0;
    hi >>= 4;
    hi += '0';
    if ( hi > '9')
        hi += 7;				// A..F

    /* Untere 4-bit */
    low = ( wert & 0x0f ) + '0';
    if (low > '9')
        low += 7;				// A..F
    uart_putc(hi);
    uart_putc(low);
}

SIGNAL(USART_RX_vect)          // 串口中断接收
{
    unsigned char buffer;
    buffer = UDR0;
}

SIGNAL(USART_TX_vect)           // 中断发送
{
    unsigned char tmptail;
    if ( tx_head != tx_tail )
    {
        tmptail = (tx_tail + 1) & TX_BUF_MASK;
        tx_tail = tmptail;
        UDR0 = tx_buf[tmptail];  /* Starte 躡ertragung */
    }
    else
        UCSR0B &= ~(1<<UDRIE0);
}
