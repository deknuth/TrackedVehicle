#include "uart.h"

#define MESSAGE_BUF_MASK	(MESSAGE_BUF_SIZE - 1)
#define RX_BUF_SIZE			(MESSAGE_BUF_SIZE * MAX_LAENGE)
#define TX_BUF_MASK			(TX_BUF_SIZE - 1)

static volatile uint8_t tx_buf[TX_BUF_SIZE];
static volatile uint8_t tx_tail = 0;
static volatile uint8_t tx_head = 0;
void uart_init(void);
void uart_putc(uint8_t data);
void uart_puts_p(const char *progmem_s);
void uart_put_dec(uint16_t wert, uint8_t anzahl, uint8_t fuell);
void uart_put_hex(uint8_t wert);

void uart_init(void)
{
    UBRRH = (uint8_t) (UBRR_BAUD>>8);
    UBRRL = (uint8_t) UBRR_BAUD & 0x00FF;
    UCSRB = (1<<RXCIE)|(1<<RXEN)|(1<<TXEN);
    UCSRC = (1<<URSEL)|(1<<UCSZ1)|(1<<UCSZ0);
}


void uart_putc(uint8_t data)
{
    uint8_t tmphead;
    if (data == '\n')
        uart_putc('\r');
    tmphead  = (tx_head + 1) & TX_BUF_MASK;
    while ( tmphead == tx_tail );
    tx_buf[tmphead] = data;
    tx_head = tmphead;
    UCSRB |= (1<<UDRIE);
}


void uart_puts_p(const char *progmem_s)
{
    char c;
    while ( (c = pgm_read_byte(progmem_s++)) )
        uart_putc(c);
}


void uart_put_dec(uint16_t wert, uint8_t anzahl, uint8_t fuell)
{
    uint8_t i, s[10];
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


void uart_put_hex(uint8_t wert)
{
    uint8_t hi,low;

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

SIGNAL(SIG_UART_RECV)          // 串口中断接收
{
    uint8_t buffer;
    buffer = UDR;
}

SIGNAL(SIG_UART_DATA)           // 中断发送
{
    uint8_t tmptail;
    if ( tx_head != tx_tail )
    {
        tmptail = (tx_tail + 1) & TX_BUF_MASK;
        tx_tail = tmptail;
        UDR = tx_buf[tmptail];  /* Starte 躡ertragung */
    }
    else
        UCSRB &= ~(1<<UDRIE);
}
