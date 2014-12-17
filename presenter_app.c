/* ---------------------------------
 * PRESENTER CODE FILE
 * For 3140 Final Project
 * Alvin Adrian Wijaya (aaw39), Xue Rong Shane Soh (xs46)
 * Last updated 5/3/2013
 * --------------------------------- */
 
#include "3140final.h"
#include "uart.h"
#include "bsp.h"
#include "mrfi.h"
#include "radios/family1/mrfi_spi.h"

volatile unsigned int num_press = 0;		//Number of times the button is pressed in a 5-sec interval
volatile char active_player = '0'; 			//Set active player (ie player who can answer question) to "0", i.e. no active player
unsigned int p1_ready = 0;					//1 if P1 is connected and ready, 0 otherwise
unsigned int p2_ready = 0;					//1 is P2 is connected and ready, 0 otherwise
volatile unsigned int player1_score = 0;	//initialize player 1 score to 0
volatile unsigned int player2_score = 0;	//initialize player 2 score to 0
volatile char P1wager = 'w';				//arbitrarily set player 1 wager to 'w' before it is updated
volatile char P2wager = 'w';				//arbitrarily set player 2 wager to 'w' before it is updated
volatile unsigned int P1wager_int = 0;		//arbitrarily set player 1 wager (in integer) to 0 before it is updated
volatile unsigned int P2wager_int = 0;		//arbitrarily set player 1 wager (in integer) to 0 before it is updated
volatile char P1f_ans = 't';				//arbitrarily set player 1 final round answer to 't' before it is updated
volatile char P2f_ans = 't';				//arbitrarily set player 2 final round answer to 't' before it is updated

/* Define state of receiver; default to 1
 * 0 - idle
 * 1 - waiting for players to connect
 * 2 - displaying question
 * 3 - accepting buzzer input
 * 4 - accepting answer input
 * 5 - accepting final wager input
 * 6 - accepting final wager answer
 * 7 - game over, low power mode
 * and more for bonus sections?
 */
volatile char state = '1';

/* Content of packet */
volatile char data;
volatile char playerID;
volatile char cmd;
	
/* Useful #defines */
#define GREEN_LED   0x02
#define RED_LED 	0x01

/* Function prototypes */
void sleep(unsigned int count);

/* Main function for receive application */
void main(void) {
	/* Set a filter address for packets received by the radio
	 *   This should match the "destination" address of
	 *   the packets sent by the transmitter. */
	uint8_t address[] = {0xab,0xcd,0x12,0x34};
	
	/* Filter setup return value: success or failure */
	unsigned char status;
	init_uart();
	
	init(); 	//Setup timer
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
		
	/* 	Red and green LEDs are output, green starts on */
	P1DIR = RED_LED | GREEN_LED;
	P1OUT = GREEN_LED;
	
	/* Turn on the radio receiver */
	MRFI_RxOn();
	
	/* Main loop */
	__bis_SR_register(GIE);
	uart_puts("Starting... Waiting for players to connect..\n\n");		//Starting
	
	/* Set up questions */
	createQuestion("When was Cornell founded?\n", "1855\n", "1865\n", "1875\n", "1885\n\n", '2',0);
	createQuestion("Where did Prof. Martinez earn his Ph.D?\n", "MIT\n", "Cornell\n", "UIUC\n", "The School of Hard Knocks\n\n", '3',1);
	createQuestion("The word 'engineer' comes from a\nLatin word meaning...\n", "Cleverness\n", "Engine\n", "Complex\n", "Social awkwardness\n\n", '1',0);
	createQuestion("The Arts Quad statue of Ezra Cornell depicts\nhim with what piece of equipment?\n", "A Mars rover\n", "A telegraph receiver\n", "A steam engine\n", "MSP430\n\n", '2',0);
	createQuestion("Robert C. Baker (December 29, 1921 – March 13, 2006)\nwas a Cornell University professor who invented the...\n", "Telegraph\n", "Dounce homogenizer\n", "Term 'power nap'\n", "Chicken nugget\n\n", '4',0);
	
	while(state < '5'){
		/* heart beat */
		sleep(60000);
		P1OUT ^= GREEN_LED;
		
		if ((p1_ready) && (p2_ready) && (state == '1')) { state = '2'; } // if both players are ready, set state to 2
		
		while (state == '0'); // idle
		
		/* Display questions */
		if ((questions_left > 1) && (state == '2')) { // while there are questions left and system is in state 2
			sleep(60000);
			delay(60000);
			getQuestion();
			state = '3'; // set receiver to accept buzzer
			send_msg('s','2','1'); // set P1 to polling for buzzer state
			send_msg('s','2','2'); // set P2 to polling for buzzer state
		}
		
		/*Waiting for buzzer press*/
		if(state == '3') {
			questiontimer_msec = 0;
			while((state == '3') && (questiontimer_msec < QUESTION_PERIOD));
			if(questiontimer_msec >= QUESTION_PERIOD) {  	//if question timeouts
				send_msg('s','0','1'); // set P1 to idle
				send_msg('s','0','2'); // set P2 to idle
				state = '2';		   // set state to displaying the next question
				uart_puts("No one answered this question.\n");
				uart_puts("The correct answer is: ");
  				uart_putc((char)(current_question->answer + 16));
  				uart_puts("  \n\n");
  				questions_left = questions_left - 1;
			}
		}
			
		/* Enter wager round */
		if ((state == '2') && (questions_left == 1)) { // 1 question left
			uart_puts("-------------FINAL WAGER ROUND----------------\n");
			uart_puts("This is the final question.\n");
			delay(60000);
			delay(60000);
			uart_puts("You can wager 0, 250, 500, 750, or 1000 points.\n");
			delay(60000);
			delay(60000);
			uart_puts("If you answer the question correctly, you will\n");
			uart_puts("get the points you wager. If you answer incorrectly,\n");
			uart_puts("you will lose the points you wager.\n");
			delay(60000);
			sleep(60000);
			delay(60000);
			getQuestion();
			state = '5'; // set receiver to accept buzzer
			send_msg('s','5','1'); // set P1 to polling for wager points
			send_msg('s','5','2'); // set P2 to polling for wager points
			uart_puts("Press once to wager 250 points. Twice for 500 points.\n");
			uart_puts("Thrice for 750 points. Four times for 1000 points.\n");
			uart_puts("Do not press to wager 0 points\n");
		}
	}
	
	while((state >= '5') && (state < '7')) {
		
		//Waiting for wager input state
		if((state == '5') && questions_left > 0) {
			uart_puts("Countdown: 10\n");
			delay(50000);
			delay(40000);
			uart_puts("Countdown: 9\n");
			delay(50000);
			delay(40000);
			uart_puts("Countdown: 8\n");
			delay(50000);
			delay(40000);
			uart_puts("Countdown: 7\n");
			delay(50000);
			delay(40000);
			uart_puts("Countdown: 6\n");
			delay(50000);
			delay(40000);
			uart_puts("Countdown: 5\n");
			delay(50000);
			delay(40000);
			uart_puts("Countdown: 4\n");
			delay(50000);
			delay(40000);
			uart_puts("Countdown: 3\n");
			delay(50000);
			delay(40000);
			uart_puts("Countdown: 2\n");
			delay(50000);
			delay(40000);
			uart_puts("Countdown: 1\n");
			delay(50000);
			delay(40000);
			uart_puts("Countdown: 0\n");
			uart_puts("Time's up!\n");
			while((P1wager == 'w') || (P2wager == 'w'));
			displayWager();
			send_msg('s','6','1'); // set P1 to polling for wager answer user input
			delay(30000);
			send_msg('s','6','2'); // set P2 to polling for wager answer user input
			uart_puts("Now, please enter your answer.\n");
			uart_puts("Press once for choice A, twice for B, thrice for C,\n");
  			uart_puts("four times for D!\n");
			uart_puts("Countdown: 5..\n");
			delay(50000);
			delay(50000);
			uart_puts("Countdown: 4..\n");
			delay(50000);
			delay(50000);
			uart_puts("Countdown: 3..\n");
			delay(50000);
			delay(50000);
			uart_puts("Countdown: 2..\n");
			delay(50000);
			delay(50000);
			uart_puts("Countdown: 1..\n");
			delay(50000);
			delay(50000);
			uart_puts("Countdown: 0..\n");
			state = '6';
		}
		
		if(state == '6') {
			while ((P1f_ans == 't') || (P2f_ans == 't'));
			answerQuestion('g','1');
		}
			
		/*End of the game, put processor in low power mode LPM4*/
		if (questions_left == 0) { // 0 question left
			uart_puts("-----------------GAME OVER--------------------\n");
			displayFinalScore();
			uart_puts("Thank you for playing! :) \n");
			uart_puts("To restart the game, please re-plug boards from power sources\n");
			state = '7';
			P1OUT = 0x00;		//Turn off all LEDs
			_BIS_SR(LPM4_bits + GIE);		//Enter level 4 low power mode to conserve power
		}
	}
}


/* Function to execute upon receiving a packet
 *   Called by the driver when new packet arrives */
void MRFI_RxCompleteISR(void) {
	
	/* Read the received data packet */
	mrfiPacket_t	packet;
	//__disable_interrupt();
	MRFI_Receive(&packet);
	
	/* Update content of packet */
	cmd = packet.frame[9];
	data = packet.frame[10];
	playerID = packet.frame[11];
	
	/* Check Ready signal from players */
	if ((cmd == 'r') && (playerID == '1') && (state == '1')) { // if P1 is ready
		uart_puts("Player 1 connected\n");
		p1_ready = 1;
		send_msg('s','0','1'); // set P1 to idle
	}
	if ((cmd == 'r') && (playerID == '2') && (state == '1')) { // if P2 is ready
		uart_puts("Player 2 connected\n");
		p2_ready = 1;
		send_msg('s','0','2'); // set P2 to idle
	}
	
	/* Check Buzzer signal from players */
	if ((cmd == 'z') && (playerID == '1') && (state == '3')) { // if P1 presses buzzer
		send_msg('s','4','2'); // set P2 to state "blocked"
		state = '4'; // set receiver state "accepting answer"
		active_player = '1';
		uart_puts("Player 1 BUZZ\n");
		delay(60000);
		delay(60000);
		send_msg('s','3','1'); // set P1 to state "accepting input"
	    uart_puts("Press once for choice A, twice for B, thrice for C,\n");
  		uart_puts("four times for D!\n");
		uart_puts("Countdown: 5..\n");
		delay(50000);
		delay(50000);
		uart_puts("Countdown: 4..\n");
		delay(50000);
		delay(50000);
		uart_puts("Countdown: 3..\n");
		delay(50000);
		delay(50000);
		uart_puts("Countdown: 2..\n");
		delay(50000);
		delay(50000);
		uart_puts("Countdown: 1..\n");
		delay(50000);
		delay(50000);
		uart_puts("Countdown: 0..\n");
		questiontimer_msec = 0;
	}
	
	if ((cmd == 'z') && (playerID == '2') && (state == '3')) { // if P2 presses buzzer
		send_msg('s','4','1'); // set P1 to state "blocked"
		state = '4'; // set receiver state "accepting answer"
		active_player = '2';
		uart_puts("Player 2 BUZZ\n");
		delay(60000);
		delay(60000);
		send_msg('s','3','2'); // set P2 to state "accepting input"
	    uart_puts("Press once for choice A, twice for B, thrice for C,\n");
  		uart_puts("four times for D!\n");
		uart_puts("Countdown: 5..\n");
		delay(50000);
		delay(50000);
		uart_puts("Countdown: 4..\n");
		delay(50000);
		delay(50000);
		uart_puts("Countdown: 3..\n");
		delay(50000);
		delay(50000);
		uart_puts("Countdown: 2..\n");
		delay(50000);
		delay(50000);
		uart_puts("Countdown: 1..\n");
		delay(50000);
		delay(50000);
		uart_puts("Countdown: 0..\n");
		questiontimer_msec = 0;
	}
	
	/* Accept answer input from active player */
	if ((cmd == 'a') && (playerID == active_player) && (state == '4')) {
		send_msg('s','4','1'); // set P1 to state "blocked"
		send_msg('s','4','2'); // set P2 to state "blocked"
		answerQuestion(data,'0'); // answer question
		active_player = '0'; // reset active player to none
		state = '2';
	}
	
	/* Accept the data from both players */
	if ((cmd == 'f') && (state == '6')) {
		if(playerID == '1') {
			P1f_ans = data;
			send_msg('s','4','1'); // set P1 to state "blocked"
		} else if(playerID == '2') {
			P2f_ans = data;
			send_msg('s','4','2'); // set P2 to state "blocked"
		}
	}
	
	/*Obtain wager input from both players*/
	if((cmd == 'w') && (state == '5')) {
		if(playerID == '1') {
			if(data > '4') {data = '0';}
			P1wager = data;
			send_msg('s','4','1'); // set P1 to state "blocked"
		} else if(playerID == '2') {
			if(data > '4') {data = '0';}
			P2wager = data;
			send_msg('s','4','2'); // set P2 to state "blocked"
		}
	}
	
	/* Toggle the red LED to signal that data has arrived */
	P1OUT ^= RED_LED;
	//__enable_interrupt();
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

/* Function to send char data in a wireless payload*/
void send_msg(volatile char cmd, volatile char data, const char playerID) {
		mrfiPacket_t 	packet;
		//__disable_interrupt();
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
			packet.frame[1] = 0x12;		/* Destination */
			packet.frame[2] = 0x34;
			packet.frame[3] = 0xab;
			packet.frame[4] = 0xcd;
			
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
			//__enable_interrupt();
}

/*----------------------------------------------
 * Initializes the timer
 * ---------------------------------------------*/
void init (void) {
	WDTCTL = WDTPW + WDTHOLD;	// turn off watchdog */
	
	/* Set up Timer B (1000 cycles = 1 millisecond) */
  	TBCCR0 = TIMERB_PERIOD;
 	TBCTL = TBCLR|MC_1|TBSSEL_2|ID_0;
  	TBCCTL0 = CCIE;
  	questiontimer_msec = 0;
}

/* -------------Interrupt vector for time increments------------ (Timer B) */
#pragma vector=TIMERB0_VECTOR
__interrupt void timerB (void) {
  	questiontimer_msec++;
  	if(questiontimer_msec == 60000) {
		questiontimer_msec = 0;
	}
}
