/* ---------------------------------
 * QUESTIONS TEST CODE HEADER FILE
 * For 3140 Final Project
 * Alvin Adrian Wijaya (aaw39), Xue Rong Shane Soh (xs46)
 * Last updated 4/24/2013
 * --------------------------------- */

#ifndef QUESTIONS_H_
#define QUESTIONS_H_
#endif

#include "msp430x22x4.h"
#include "uart.h"

#ifndef NULL
#define NULL 0
#endif



extern volatile char active_player;				//Current active player
extern volatile unsigned int player1_score;		//The current score of player 1
extern volatile unsigned int player2_score;		//The current score of player 2
extern volatile char P1wager;
extern volatile char P2wager;
extern volatile unsigned int P1wager_int;
extern volatile unsigned int P2wager_int;
extern volatile char P1f_ans;
extern volatile char P2f_ans;

typedef struct question_struct question;

/* Definition of the question struct. This keeps track of the questions */
struct question_struct{
	char* question;			//Question to be displayed
	char* choiceA;			//Choice A
	char* choiceB;			//Choice B
	char* choiceC;			//Choice C
	char* choiceD;			//Choice D
	char answer;			//The correct answer
	question* next;			//Pointer to the next question
	int type;				//Type of question (0 for normal, 1 for double points)
};

extern question* head;								//The first question in linkedlist
extern question* current_question;					//The current question
extern volatile unsigned int questions_left;					//Number of questions left

void getQuestion(void);								//Obtain a question from the database
void addToQueue(question** head_ref, question* q);	//Add a question to the queue
question* takeFromQueue(question** head_ref, int a);//Take from queue
int createQuestion(char*, char*, char*, char*, char*, char,int);	//create a question
void answerQuestion(char,char);						//Answer the question
void displayScore(void);							//Display the score
void displayFinalScore(void);						//Display the final score
void strreverse(char* begin, char* end);			//Helper method for the itoa function
void itoa(int value, char* str, int base);			//Integer to string function
void displayWager();								//Integer to display wager
