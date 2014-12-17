/* ---------------------------------
 * UART HEADER FILE
 * For 3140 Final Project
 * Taken from TI UART Demo
 * Alvin Adrian Wijaya (aaw39), Xue Rong Shane Soh (xs46)
 * Last updated 4/17/2013
 * --------------------------------- */


#ifndef UART_H_
#define UART_H_
#endif

void init_uart(void);
void uart_putc(char c);
void uart_puts(char *str);
void uart_clear_screen(void);
void uart_out(volatile char a);
void delay (unsigned int d);
