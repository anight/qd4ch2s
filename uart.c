
#include <avr/interrupt.h>

#include "main.h"
#include "uart.h"

static char uart_outbuf[64];
static char * volatile uart_outbuf_in = uart_outbuf;
static char * volatile uart_outbuf_out = uart_outbuf;

void uart_print_char(char c)
{
	if (uart_outbuf_in == uart_outbuf + sizeof(uart_outbuf)) {
		PORTC |= (1 << 3);
		return;
	}
	*uart_outbuf_in = c;
	uart_outbuf_in ++;
}

void uart_print_str(const char *str)
{
	while (*str) {
		uart_print_char(*str);
		str ++;
	}
}

void uart_print_hex_byte(uint8_t byte)
{
	uart_print_char("0123456789abcdef"[(byte >> 4) & 0xf]);
	uart_print_char("0123456789abcdef"[byte & 0xf]);
}

void uart_outbuf_put()
{
	if (uart_outbuf_in != uart_outbuf_out) {
		UCSR0B |= (1 << UDRIE0);
	}
}

char uart_outbuf_is_empty()
{
	return uart_outbuf_in == uart_outbuf_out;
}

ISR(USART_RX_vect)
{
	static char inbuf[32];
	static char *ptr = inbuf;
	static char skip_this_line = 0;

	char byte = UDR0;

	if (byte == '\n') {
		if (skip_this_line) {
			skip_this_line = 0;
		} else {
			if (ptr > inbuf && ptr[-1] == '\r') {
				ptr --;
			}
			*ptr = '\0';
			execute_cmd(inbuf);
			ptr = inbuf;
		}
	} else {
		if (!skip_this_line) {
			if (ptr == inbuf + sizeof(inbuf) - 1) { /* the last byte, should be always '\n' */
				skip_this_line = 1;
				ptr = inbuf;
			} else {
				*ptr++ = byte;
			}
		}
	}
}

ISR(USART_UDRE_vect)
{
	if (uart_outbuf_in != uart_outbuf_out) {
		UDR0 = *uart_outbuf_out++;
		if (uart_outbuf_in == uart_outbuf_out) {
			uart_outbuf_in = uart_outbuf;
			uart_outbuf_out = uart_outbuf;
			UCSR0B &= ~(1 << UDRIE0);
		} else {
			UCSR0B |= (1 << UDRIE0);
		}
	} else {
		UCSR0B &= ~(1 << UDRIE0);
	}
}

void uart_init()
{
#define USE_2X 1
#define BAUD 1000000
//#define BAUD 9600
#include <util/setbaud.h>

	UBRR0H = UBRRH_VALUE;
	UBRR0L = UBRRL_VALUE;

	UCSR0A = (USE_2X << U2X0); /* Double the USART Transmission Speed */

	UCSR0B = (1 << RXCIE0) /* RX Complete Interrupt Enable */
		| (0 << TXCIE0) /* TX Complete Interrupt Enable */
		| (0 << UDRIE0) /* USART Data Register Empty Interrupt Enable */
		| (1 << RXEN0) /* Receiver Enable */
		| (1 << TXEN0) /* Transmitter Enable */
		| (0 << UCSZ02); /* 8 bits of data */

	UCSR0C = (0 << UMSEL01) | (0 << UMSEL00) /* Asynchronous USART */
		| (0 << UPM01) | (0 << UPM00) /* No parity */
		| (0 << USBS0) /* 1 stop bit */
		| (1 << UCSZ01) | (1 << UCSZ00); /* 8 bits of data */
}

