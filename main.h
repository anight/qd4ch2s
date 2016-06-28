#ifndef MAIN_H
#define MAIN_H 1

#define pulse_buf_size 16

struct wheel_s {
	uint8_t ptr;
	uint8_t state;
	uint8_t double_pulses;
	uint8_t skipped_ticks;
	int16_t current;
	int16_t sum;
	int16_t wbuf[pulse_buf_size];
};

extern struct wheel_s wheel[4];

extern uint8_t prescaler;

void execute_cmd(const char *cmd);

#endif
