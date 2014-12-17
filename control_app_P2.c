/* ---------------------------------
 * CONTROLLER CODE FILE FOR PLAYER 2
 * For 3140 Final Project
 * Alvin Adrian Wijaya (aaw39), Xue Rong Shane Soh (xs46)
 * Last updated 5/3/2013
 * --------------------------------- */

#include "3140final.h"
#include "uart.h"
#include "bsp.h"
#include "mrfi.h"
#include "radios/family1/mrfi_spi.h"
#include <string.h>

volatile char num_press = '0';		//Number of times the button is pressed in a 5-sec interval
const char curPlayer = '2';	// Player 2

/* Define state of sender; default to 1
 * 0 - idle
 * 1 - attempt to connect to receiver
 * 2 - polling buzzer to determine which player answers question
 * 3 - accepting answer input
 * 4 - blocked from answering
 * 5 - polling for final round wager points
 * 6 - accepting final wager input
 */
volatile char state = '1';

/* Useful #defines */
#define RED_LED 		0x01
#define GREEN_LED		0x02

/* Function prototypes */
void sleep(unsigned int count);

/* Main function for transmit application */
void main(void) {
	/* Set a filter address for packets received by the radio
	 *   This should match the "destination" address of
	 *   the packets sent by the transmitter. */
	uint8_t address[] = {0x12,0x34,0xab,0xcd};
	
	/* Filter setup return value: success or failure */
	unsigned char status;
  	
  	/* Sets up interrupt and timer initialization*/
	init();
	
	/* Perform board-specific initialization */
	BSP_Init();
	
	/* Initialize minimal RF interface, wake up radio */
	MRFI_Init();
	MRFI_WakeUp();
	
	/* Attempt to turn on address filtering
	 *   If unsuccessful, turn on both LEDs and wait forever */
	status = MRFI_SetRxAddrFilter(address);	
	MRFI_EnableRxAddrFilter();
	if( status != 0) {
		P1OUT = RED_LED | GREEN_LED;
		while(1);
	}
	
	/* Turn on the radio receiver */
	MRFI_RxOn();
	
	__bis_SR_register(GIE);
	
	/* Main (infinite) transmit loop */
	while(1){
		while ((state == '0') || (state == '4')) { // idle or blocked. Activate low power mode.
			sw_stat = 0;
			P1OUT &= ~GREEN_LED;//Turn off green LED
			_BIS_SR(OSCOFF);	//Turn off oscillator
			_BIS_SR(SCG0);		//Turn off system clock generator 0
		}
		
		while (state == '1'){ // attempt to connect to receiver
			sleep(40000);
			send_msg('r','x',curPlayer); //  signal that buzzer is ready
		}
		
		if (state == '2') { // polling buzzer
			sw_stat = 0;
			/* Wait indefinitely until someone presses buzzer */
			while ((sw_stat == 0) && (state == '2'));
			if(sw_stat == 1) {
				debounce_buzzer();						//Switch debouncing
				send_msg('z','x',curPlayer);			//Send buzzer signal
			}
			P1OUT &= ~GREEN_LED;	//Stop green LED
		}
		
		if (state == '3') { // accepting answer input
			/* Increments num_press according to number of times button is pressed */
			sw_stat = 0;
			timerpress_msec = 0; //Reset button timer
			num_press = '0';
			timerhbt_msec = 0;
			while(timerpress_msec < TIMER_PRESS) {
				if(timerhbt_msec > 500) {P1OUT ^= GREEN_LED; timerhbt_msec = 0;}		//Heartbeat code
				if(sw_stat == 1) {debounce_numpress();}									//Switch debouncing
			}		
			P1OUT &= ~GREEN_LED; // stop green LED
			send_msg('a',num_press, curPlayer);
			num_press = '0';				//Reset button counter
			state = '0'; 					//Set itself to idle
		}
		
		if (state == '5') { // accepting wager input
			/* Increments num_press according to number of times button is pressed */
			sw_stat = 0;
			timerpress_msec = 0; //Reset button timer
			num_press = '0';
			timerhbt_msec = 0;
			while(timerpress_msec < WAGER_PERIOD) {
				if(timerhbt_msec > 500) {P1OUT ^= GREEN_LED; timerhbt_msec = 0;}		//Heartbeat code
				if(sw_stat == 1) {debounce_numpress();}									//Switch debouncing
			}		
			P1OUT &= ~GREEN_LED; // stop green LED
			send_msg('w',num_press, curPlayer);
			num_press = '0';				//Reset button counter
			state = '0'; 					//Set itself to idle
		}
		
		if (state == '6') { // accepting wager answer
			/* Increments num_press according to number of times button is pressed */
			sw_stat = 0;
			timerpress_msec = 0; //Reset button timer
			num_press = '0';
			timerhbt_msec = 0;
			while(timerpress_msec < WAGER_PERIOD) {
				if(timerhbt_msec > 500) {P1OUT ^= GREEN_LED; timerhbt_msec = 0;}		//Heartbeat code
				if(sw_stat == 1) {debounce_numpress();}									//Switch debouncing
			}		
			P1OUT &= ~GREEN_LED; // stop green LED
			send_msg('f',num_press, curPlayer);
			num_press = '0';				//Reset button counter
			state = '0'; 					//Set itself to idle
		}
	}
}

/* Function to send char data in a wireless payload*/
void send_msg(volatile char cmd, volatile char data, const char playerID) {
		mrfiPacket_t 	packet;
		__disable_interrupt();
			/* Construct a packet to send over the radio.
			 * 
			 *  Packet frame structure:
			 *  ---------------------------------------------------
			 *  | Length (1B) | Dest (4B) | Source (4B) | Payload |
			 *  ---------------------------------------------------
			 */
			 
			 
			/* First byte of packet frame holds message length in bytes */
			packet.frame[0] = 8+8;	/* Includes 8-byte address header */
			
			/* Next 8 bytes are addresses, 4 each for source and dest. */
			packet.frame[1] = 0xab;		/* Destination */
			packet.frame[2] = 0xcd;
			packet.frame[3] = 0x12;
			packet.frame[4] = 0x34;
			
			packet.frame[5] = 0x02;		/* Source */
			packet.frame[6] = 0x00;
			packet.frame[7] = 0x01;
			packet.frame[8] = 0x02;
			
			packet.frame[9] = cmd; 
			packet.frame[10] = data;
			packet.frame[11] = playerID;
			packet.frame[12] = NULL;
			
			/* Transmit the packet over the radio */
			MRFI_Transmit(&packet , MRFI_TX_TYPE_FORCED);
			
			/* Flash red LED after transmitting */
			P1OUT ^= RED_LED;
			sleep(10000);
			P1OUT ^= RED_LED;
			__enable_interrupt();
}

/* Function to execute upon receiving a packet
 *   Called by the driver when new packet arrives */
void MRFI_RxCompleteISR(void) {
	char data;
	char playerID;
	char cmd;
	
	/* Read the received data packet */
	mrfiPacket_t	packet;
	__disable_interrupt();
	MRFI_Receive(&packet);
	
	/* Define content of packet */
	cmd = packet.frame[9];
	data = packet.frame[10];
	playerID = packet.frame[11];
	
	/* Receive state change command */
	if (cmd == 's' && playerID == curPlayer) {
		state = data;
		_BIC_SR(OSCOFF);		//Turn on oscillator
		_BIC_SR(SCG0);			//Turn on system clock generator 0. Leave low power mode.
	}
	__enable_interrupt();
}

/* Parameterized "sleep" helper function */
void sleep(unsigned int count) {
	int i;
	for (i = 0; i < 10; i++) {
		while(count > 0) {
			count--;
			__no_operation();
		}
	}
}
/* ---------------- INIT METHOD -------------------
 * Sets up timer interrupts and switch interrupt*/
 void init(void) {
 	WDTCTL = WDTPW + WDTHOLD;	// turn off watchdog */
	P1OUT &= ~0x03;				//Set the output bit of the LEDs LOW.
	P1DIR |= 0x03;				//Set the RED and GREEN LED to output
	P1DIR &= ~0x04;				//Set the switch to input
	P1REN |= 0x04;				//Attach a resistor to switch
	P1OUT |= 0x04;				//Set the output bit of the switch HIGH
 	
 	P1IE |= 0x04;				//Enabled interrupts on the switch
	P1IFG &= ~0x04;				//Reset all interrupt flags
	P1IES |= 0x04;				//The interrupt is on a HIGH to LOW edge
 	
	/* Set up Timer A (triggers context switch) */
	TACCR0  = TIMERA_PERIOD;
	TACTL   = TACLR|MC_1|TASSEL_2|ID_2;
	TACCTL0 = CCIE;
	
	/* Set up Timer B (1000 cycles = 1 millisecond) */
  	TBCCR0 = TIMERB_PERIOD;
 	TBCTL = TBCLR|MC_1|TBSSEL_2|ID_0;
  	TBCCTL0 = CCIE;
  	
  	/* Set switch status to be unpressed */
  	sw_stat = 0;
  	
  	/* Set heartbeat timer to be 0 */
  	timerhbt_msec = 0;
  	timerpress_msec = 0;
  	global_msec = 0;
  	__enable_interrupt();		//Set global interrupt
}


/* -------------Switch debouncing state to determine number of button presses------------- */
void debounce_numpress(void) {
	timerdeb_msec = 0;
	while(timerdeb_msec != DEBOUNCE_DELAY);	//Wait for proper debouncing
	if((P1IN&0x04) == 0) {		//If switch is properly pressed and debounced
		num_press= (char)(num_press + 1);
	}
	sw_stat = 0;				//Return from 'debouncing' state to 'normal' state	
}

/* -------------Switch debouncing state to determine buzzer press------------- */
void debounce_buzzer(void) {
	timerdeb_msec = 0;
	while(timerdeb_msec != DEBOUNCE_DELAY);	//Wait for proper debouncing
	if((P1IN&0x04) == 0) {		//If switch is properly pressed and debounced
		P1OUT ^= 0x02;
	}
}

/* -------------Interrupt vector for Timer A----------- */
#pragma vector=TIMERA0_VECTOR
__interrupt void timerA (void)
{
}

/* -------------Interrupt vector for time increments------------ (Timer B) */
#pragma vector=TIMERB0_VECTOR
__interrupt void timerB (void)
{
	global_msec++;
	timerhbt_msec++;
	timerdeb_msec++;
	timerpress_msec++;
	if(global_msec == 60000) {
		global_msec = 0;
	}
	if(timerhbt_msec == 60000) {
		timerhbt_msec = 0;
	}
	if(timerdeb_msec == 60000) {
		timerdeb_msec = 0;
	}
	if(timerpress_msec == 60000) {
		timerpress_msec = 0;
	}
}

/* -------------Interrupt vector for switch -----------------*/
#pragma vector=PORT1_VECTOR
__interrupt void my_handler (void)
{
	if(((P1IFG&0x04) != 0) && (state != '0') && (state != '4')) { 		//This is switch interrupt
		sw_stat = 1;					//Move the system to 'debouncing' state
		P1IFG &= ~0x04;					//Reset the interrupt flag
	} 
}
