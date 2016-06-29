
	/* (c) 2016 Andrei Nigmatulin */

#include <stdlib.h>
#include <string.h>
#include <stddef.h>

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/eeprom.h>

#include <util/delay.h>

#include "main.h"
#include "uart.h"

	/*
		PB0 port 1 channel A
		PB1 port 1 channel B
		PB2 port 2 channel A
		PB3 port 2 channel B
		PB4 port 3 channel A
		PB5 port 3 channel B
		PB6 port 4 channel A
		PB7 port 4 channel B
	*/

#define prescaler_min      2
#define prescaler_max     10
#define prescaler_default  6

enum msg_e {
	MSG_NONE = 0,
	MSG_OK,
	MSG_ERROR,
	MSG_PRESCALER,
	MSG_PORT1_STATUS,
	MSG_PORT2_STATUS,
	MSG_PORT3_STATUS,
	MSG_PORT4_STATUS,
};

static uint8_t eep_prescaler __attribute__((section(".eeprom"))) = prescaler_default;

uint8_t prescaler;
static volatile enum msg_e msg;

struct wheel_s wheel[4];

static void wheel_update(struct wheel_s *w, uint8_t time, uint8_t two_bits)
{
	while (w->ptr != time) {
		int16_t diff = w->current - w->wbuf[w->ptr];
		w->sum += diff;
		w->wbuf[w->ptr] = w->current;
		w->current = 0;
		w->ptr = (w->ptr + 1) & (pulse_buf_size - 1);
		if (w->ptr == time) {
			break;
		}
		w->skipped_ticks ++;
	}

	switch (w->state | two_bits) {
		case 0b0000:
		case 0b0101:
		case 0b1111:
		case 0b1010:
			/* nothing has changed */
			break;
		case 0b0011:
		case 0b0110:
		case 0b1100:
		case 0b1001:
			/* hope this is just two pulses ahead. let's guess direction */
			if (w->current > 0) {
				w->current += 2;
			} else {
				w->current -= 2;
			}
			w->double_pulses ++;
			break;
		case 0b0001:
		case 0b0111:
		case 0b1110:
		case 0b1000:
			/* one pulse forward */
			w->current ++;
			break;
		case 0b0010:
		case 0b0100:
		case 0b1101:
		case 0b1011:
			/* one pulse backward */
			w->current --;
			break;
	}

	w->state = two_bits << 2;
}

static void wheel_print(const struct wheel_s *w, unsigned n)
{
	uart_print_char('0' + n);
	uart_print_char('=');
	uart_print_hex_byte(w->skipped_ticks);
	uart_print_char(',');
	uart_print_hex_byte(w->double_pulses);
	uart_print_char(',');
	uart_print_hex_byte((w->sum >> 8) & 0xff);
	uart_print_hex_byte((w->sum >> 0) & 0xff);
}

void execute_cmd(const char *cmd)
{
	if (cmd[0] == '\0') {
		if (msg == MSG_NONE) {
			msg = MSG_PORT1_STATUS;
		}
	}

	else 

	if (!strncmp(cmd, "set prescaler ", strlen("set prescaler "))) {
		uint8_t new_value = atoi(cmd + strlen("set prescaler "));
		if (new_value >= prescaler_min && new_value <= prescaler_max) {
			prescaler = new_value;
			eeprom_busy_wait();
			eeprom_write_byte(&eep_prescaler, prescaler);
		}
		if (msg == MSG_NONE) {
			msg = MSG_OK;
		}
	}

	else

	if (!strcmp(cmd, "get prescaler")) {
		if (msg == MSG_NONE) {
			msg = MSG_PRESCALER;
		}
	}

	else

	if (msg == MSG_NONE) {
		msg = MSG_ERROR;
	}
}

static uint16_t read_tcnt1(void)
{
	uint8_t sreg;
	uint16_t i;
	/* Save global interrupt flag */
	sreg = SREG;
	/* Disable interrupts */
	cli();
	/* Read TCNT1 into i */
	i = TCNT1;
	/* Restore global interrupt flag */
	SREG = sreg;
	return i;
}

int main(void)
{
	CLKPR = (1 << CLKPCE);
	CLKPR = 0; /* prescaler = 1, 8MHz total */

	DDRB = 0; /* all in */

	TCCR1A = 0;
	TCCR1B = (1 << CS12) | (0 << CS11) | (1 << CS10); /* /1024 */

	uart_init();

	sei();

	eeprom_busy_wait();
	prescaler = eeprom_read_byte(&eep_prescaler);
	if (prescaler < prescaler_min || prescaler > prescaler_max) {
		prescaler = prescaler_default;
		/* fix it */
		eeprom_busy_wait();
		eeprom_write_byte(&eep_prescaler, prescaler);
	}

	while (1) {
		uint8_t data = PINB;
		uint8_t time = (read_tcnt1() >> prescaler) & (pulse_buf_size - 1);
		wheel_update(&wheel[0], time, (data >> 0) & 3);
		wheel_update(&wheel[1], time, (data >> 2) & 3);
		wheel_update(&wheel[2], time, (data >> 4) & 3);
		wheel_update(&wheel[3], time, (data >> 6) & 3);

		cli();
		if (uart_outbuf_is_empty()) {

			switch (msg) {
				case MSG_NONE:
					break;
				case MSG_PORT1_STATUS:
					wheel_print(&wheel[0], 1);
					msg ++;
					break;
				case MSG_PORT2_STATUS:
					uart_print_char(' ');
					wheel_print(&wheel[1], 2);
					msg ++;
					break;
				case MSG_PORT3_STATUS:
					uart_print_char(' ');
					wheel_print(&wheel[2], 3);
					msg ++;
					break;
				case MSG_PORT4_STATUS:
					uart_print_char(' ');
					wheel_print(&wheel[3], 4);
					uart_print_char('\n');
					msg = MSG_NONE;
					break;
				case MSG_OK:
					uart_print_str("OK\n");
					msg = MSG_NONE;
					break;
				case MSG_ERROR:
					uart_print_str("ERROR\n");
					msg = MSG_NONE;
					break;
				case MSG_PRESCALER:
					if (prescaler >= 10) {
						uart_print_char('0' + prescaler / 10);
					}
					uart_print_char('0' + prescaler % 10);
					uart_print_char('\n');
					msg = MSG_NONE;
					break;
			}

			uart_outbuf_put();
		}
		sei();
	}

	return 0;
}
