/* ---------------------------------
 * DEBOUNCING TEST CODE HEADER FILE
 * For 3140 Final Project
 * Alvin Adrian Wijaya (aaw39), Xue Rong Shane Soh (xs46)
 * Last updated 4/17/2013
 * --------------------------------- */

#ifndef 3140FINAL_H_
#define 3140FINAL_H_
#endif

#include "msp430x22x4.h"
#include "questions.h"

#ifndef NULL
#define NULL 0
#endif

const unsigned int DEBOUNCE_DELAY = 20;			//Delay of 5ms for proper debouncing
const unsigned int TIMER_PRESS = 5000;			//Time given for player to enter answer
const unsigned int TIMERA_PERIOD = 30000;		//Period of timer A
const unsigned int TIMERB_PERIOD = 1000;		//Period of timer B
const unsigned int QUESTION_PERIOD = 10000;		//Period of each question (deadline to buzz)
const unsigned int WAGER_PERIOD = 10000;		//Period of wager round
volatile unsigned int global_msec;				//Global counter of milliseconds
volatile unsigned int timerhbt_msec;			//Heartbeat counter in milliseconds
volatile unsigned int timerdeb_msec;			//Debouncing counter in milliseconds
volatile unsigned int timerpress_msec;			//Timer for pressing in milliseconds
volatile unsigned int questiontimer_msec;		//The timer for question deadline in milliseconds
volatile unsigned int sw_stat;					//Switch status, 0 for unpressed, 1 for checking

void init(void);
void debounce_numpress(void);
void debounce_buzzer(void);
void send_msg(volatile char cmd, volatile char data, const char playerID);
