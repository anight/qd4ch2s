#include <avr/io.h>
#include <util/twi.h>
#include <avr/interrupt.h>

#include "I2C_slave.h"
#include "main.h"
#include "uart.h"

volatile uint8_t tx_in = 0, tx_out = 0;
volatile uint8_t txbuffer[4];


void I2C_init(uint8_t address)
{
	// load address into TWI address register
	TWAR = address << 1;
	// set the TWCR to enable address matching and enable TWI, clear TWINT, enable TWI interrupt
	TWCR = (1<<TWIE) | (1<<TWEA) | (1<<TWINT) | (1<<TWEN);
}

void I2C_stop(void)
{
	// clear acknowledge and enable bits
	TWCR &= ~( (1<<TWEA) | (1<<TWEN) | (1<<TWIE) );
}

static uint8_t rr_request_byte(uint8_t byte)
{
	switch (byte) {
		case 0x00:
			txbuffer[0] = prescaler;
			tx_out = 0;
			tx_in = 1;
			break;
		case 0x81:
		case 0x82:
		case 0x83:
		case 0x84:
		{
			uint8_t port_id = byte - 0x81;
			struct wheel_s *this_wheel = &wheel[port_id];
			txbuffer[0] = this_wheel->skipped_ticks;
			txbuffer[1] = this_wheel->double_pulses;
			txbuffer[2] = (this_wheel->sum >> 8) & 0xff;
			txbuffer[3] = (this_wheel->sum >> 0) & 0xff;
			tx_out = 0;
			tx_in = 4;
			break;
		}
		default:
			return -1;
	}

	return 0;
}

static uint8_t __attribute__((always_inline)) rr_response_byte(uint8_t *byte)
{
	if (tx_in == tx_out) {
		return -1;
	}

	*byte = txbuffer[tx_out++];
	if (tx_out == tx_in) {
		tx_out = 0;
		tx_in = 0;
	}
	return 0;
}

//#define I2C_DEBUG 1
#define I2C_DUMMY 1

ISR(TWI_vect)
{
	uint8_t state = TWSR & 0xE8;

#if I2C_DUMMY
	static uint8_t b;
#endif

	if (__builtin_expect(state == 0xA8, 1)) {
		TWDR = b;
		TWCR |= (1<<TWEA) | (1<<TWINT);// | (1<<TWEN) | (1<<TWIE);
		return;
	}

	state = TWSR & 0xF8;

#ifdef I2C_DEBUG
	uart_print_hex_byte(state);
#endif

	switch (state) {

		case TW_SR_SLA_ACK: /* 0x60 */

			break;

		case TW_SR_DATA_ACK: /* 0x80 */
		{
			uint8_t byte = TWDR;
#ifdef I2C_DEBUG
			uart_print_char(' ');
			uart_print_hex_byte(byte);
#endif
#ifdef I2C_DUMMY
			b = byte;
#else
			if (0 != rr_request_byte(byte)) {
				goto reject;
			}
#endif
			break;
		}

		case TW_SR_STOP: /* 0xA0 */

			break;

		case TW_ST_SLA_ACK: /* 0xA8 */
		case TW_ST_DATA_ACK: /* 0xB8 */
		{
			uint8_t byte;
#ifdef I2C_DUMMY
			byte = b;
#else
			if (0 != rr_response_byte(&byte)) {
				goto reject;
			}
#endif
#ifdef I2C_DEBUG
			uart_print_char(' ');
			uart_print_hex_byte(byte);
#endif
			TWDR = byte;
			break;
		}

		case TW_ST_DATA_NACK: /* 0xc0 */
			break;

		default:
			break;
	} 
#ifdef I2C_DEBUG
	uart_print_char(' ');
	uart_print_char('A');
	uart_print_char('\n');
	uart_outbuf_put();
#endif
	TWCR |= (1<<TWEA) | (1<<TWINT);// | (1<<TWEN) | (1<<TWIE);
	return;

reject:
#ifdef I2C_DEBUG
	uart_print_char(' ');
	uart_print_char('R');
	uart_print_char('\n');
	uart_outbuf_put();
#endif
	TWCR |= (0<<TWEA) | (1<<TWINT);// | (1<<TWEN) | (1<<TWIE);
}

