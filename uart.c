/* ---------------------------------
 * UART CODE FILE
 * For 3140 Final Project
 * Taken from TI UART Demo
 * Alvin Adrian Wijaya (aaw39), Xue Rong Shane Soh (xs46)
 * Last updated 4/17/2013
 * --------------------------------- */

 #include "uart.h"
 #include "msp430x22x4.h"

/* Initialize the UART for TX (9600, 8N1) */
/* Settings taken from TI UART demo */
void init_uart(void) {
	BCSCTL1 = CALBC1_1MHZ;        /* Set DCO for 1 MHz */
	DCOCTL  = CALDCO_1MHZ;
	P3SEL = 0x30;                 /* P3.4,5 = USCI_A0 TXD/RXD */
	UCA0CTL1 |= UCSSEL_2;         /* SMCLK */
	UCA0BR0 = 104;                /* 1MHz 9600 */
	UCA0BR1 = 0;                  /* 1MHz 9600 */
	UCA0MCTL = UCBRS0;            /* Modulation UCBRSx = 1 */
	UCA0CTL1 &= ~UCSWRST;         /* Initialize USCI state machine */
}

/* Transmit a single character over UART interface */
void uart_putc(char c) {
    while(!(IFG2 & UCA0TXIFG)); /* Wait for TX buffer to empty */
    UCA0TXBUF = c;				/* Transmit character */
}

/* Transmit a nul-terminated string over UART interface */
void uart_puts(char *str) {
	while (*str) {
		/* Replace newlines with \r\n carriage return */
		if(*str == '\n') { uart_putc('\r'); }
		uart_putc(*str++);
	}
}

/* Clear terminal screen using VT100 commands */
/* http://braun-home.net/michael/info/misc/VT100_commands.htm */
void uart_clear_screen(void) {
	uart_putc(0x1B);		/* Escape character */
 	uart_puts("[2J");		/* Clear screen */
 	uart_putc(0x1B);		/* Escape character */
 	uart_puts("[0;0H");		/* Move cursor to 0,0 */
}

void uart_out(volatile char a){
	switch(a) {
		case '1':
		delay(500);
		uart_puts("You have chosen A!\n");
		break;
		case '2':
		delay(500);
		uart_puts("You have chosen B!\n");
		break;
		case '3':
		delay(500);
		uart_puts("You have chosen C!\n");
		break;
		case '4':
		delay(500);
		uart_puts("You have chosen D!\n");
		break;
		default:
		delay(500);
		uart_puts("You have chosen an invalid input!\n");
		break;
	}
}

/* Delay 'd' cycles */
void delay (unsigned int d) {
	int i;
	for (i=0; i < d; i++) {
		__no_operation();
	}
}
