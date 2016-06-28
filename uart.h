
#ifndef UART_H
#define UART_H 1

void uart_init();
char uart_outbuf_is_empty();
void uart_outbuf_put();
void uart_print_char(char c);
void uart_print_str(const char *str);
void uart_print_hex_byte(uint8_t byte);

#endif

