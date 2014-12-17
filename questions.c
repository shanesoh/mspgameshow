/* ---------------------------------
 * QUESTIONS CODE FILE
 * For 3140 Final Project
 * Alvin Adrian Wijaya (aaw39), Xue Rong Shane Soh (xs46)
 * Last updated 5/3/2013
 * --------------------------------- */

 #include "questions.h"
 #include "uart.h"
 #include "msp430x22x4.h"
 
 #include<stdlib.h>
 #include<time.h>
 #include<string.h>
 
 question* head = NULL;
 question* current_question = NULL;
 volatile unsigned int questions_left = 0;
 
 /*-------------------------------------------------------
  * Add question p to the tail of question queue
  * ------------------------------------------------------*/
void addToQueue(question** head_ref, question* q) {
	/* Get pointer to the current head node */
	question* current = *head_ref;
	
	/* If queue is currently empty, replace it with the new process */
	if (current == NULL) { *head_ref = q; }
	
	/* Otherwise, find the end of the list and append the new node */
	else {
		while (current->next != NULL) { current = current->next; }
		/* At this point, current points to the last node in the list */
		current->next = q;
	}
}

/* ---------------------------------------------------------
 * Get question from the queue
 * ---------------------------------------------------------*/
question* takeFromQueue(question** head_ref, int a) {
	/* Get pointer to the current head node in the queue*/
	question* current = *head_ref;
	question* before = NULL;
	question* after = current->next;
	int temp = a;
	
	if (head == NULL) { return NULL; } // if question queue is empty, return NULL
	
	while(temp > 0) {
		temp = temp - 1;	//Decrement the counter
		
		//if the current node is the last node
		if(current->next == NULL) {
			current = *head_ref;
			before = NULL;
			after = current->next;
			continue;
		}
		
		//if the current node is NOT the last node
		else {
			before = current;
			current = after;
			after = current->next;
			continue;
		}
	}
	
	//If the node picked is the head node
	if(current == head) {
		head = head->next;
		return current;
	} else if (after == NULL) {		//If the node picked is the last node
		before->next = NULL;
		return current;
	} else {						//If the node picked is in the middle
		before->next = after;
		return current;
	}
}	

/*-------------------------------------------
  * GET QUESTION
  * Obtains and displays the question on screen
  * The question is picked randomly
  * Ensures that all global variables are updated
  * ------------------------------------------*/
  
void getQuestion (void) {
	int random;				//The random number
	question* chosen;		//The chosen question
  	srand(time(0)); 		//Choose the current time as the seed for random number generator
  	random = rand() % questions_left;	//The integer remainder
  	chosen = takeFromQueue(&head, random);	//Obtain the question from queue
  	current_question = chosen;
  	delay(60000);
  	if((current_question->type == 1) && (questions_left > 1)) {
  		uart_puts("This is a DOUBLE POINTS QUESTION!\n");
  		delay(60000);
  		uart_puts("A correct answer will give you 200 points and\n");
  		uart_puts("a wrong answer will deduct 200 points!\n");
  		delay(60000);
  	}
  	delay(60000);
  	uart_puts("Get ready! The question will be displayed in:\n");
  	delay(60000);
  	uart_puts("3..\n");
  	delay(60000);
  	uart_puts("2..\n");
  	delay(60000);
  	uart_puts("1..\n\n");
  	delay(60000);
  	uart_puts(current_question->question);
  	delay(60000);
  	uart_puts("Choice A: ");
  	uart_puts(current_question->choiceA);
  	uart_puts("Choice B: ");
  	uart_puts(current_question->choiceB);
  	uart_puts("Choice C: ");
  	uart_puts(current_question->choiceC);
  	uart_puts("Choice D: ");
  	uart_puts(current_question->choiceD);
  	uart_puts("\n\n");
}

/*-------------------------------------------------------------------------
 * createQuestion
 * 		This creates a new question with the question sentence q, choice A cA,
 * 		choice B cB, choice C cC, choice D cD, answer ans, and next question
 * 		nextQ.
 *------------------------------------------------------------------------*/
 int createQuestion(char* q, char* cA, char* cB, char* cC, char* cD, char ans, int qtype) {
	 question* new_q = (question*) malloc(sizeof(question));
	 if (new_q == NULL) {return -1; }	/* malloc failed */
	 new_q->question = q;
	 new_q->choiceA = cA;
	 new_q->choiceB = cB;
	 new_q->choiceC = cC;
	 new_q->choiceD = cD;
	 new_q->answer = ans;
	 new_q->type = qtype;
	 new_q->next = NULL;

	 /* Add to the question queue */
	 addToQueue(&head, new_q);
	 questions_left = questions_left + 1;
	 
	 return 0;	/* Successfully created question and bookkeeping */
 }
 
 /*--------------------------------------------------------------------------
  * answerQuestion
  * 	This method answers the question posed on UART and adjust the
  * 	active player's score accordingly
  * -------------------------------------------------------------------------*/
  void answerQuestion(char ans, char stat) {
  	
  	/* For wager round */
  	if(stat == '1') {
  		uart_puts("The correct answer is: ");
  		uart_putc((char)(current_question->answer + 16));
  		uart_puts("  \n\n");
  		
  		delay(60000);
  		
  		if((P1f_ans <= '0') || (P1f_ans > '4')) {	//if P1 gives invalid input
  			uart_puts("Player 1 gives invalid input\n");
  		} else {
  			uart_puts("Player 1 chooses: ");		//Display player 1 answer
  			uart_putc((char)(P1f_ans+16));
  			uart_puts("  \n");
  		}
  		if((P2f_ans <= '0') || (P2f_ans > '4')) {	//if P2 gives invalid input
  			uart_puts("Player 2 gives invalid input\n");
  		} else {
  			uart_puts("Player 2 chooses: ");		//Display player 2 answer
  			uart_putc((char)(P2f_ans+16));
  			uart_puts("  \n");
  		}
  		if(P1f_ans == current_question->answer) {
  			uart_puts("Player 1 answered correctly!\n");		//Check for P1 answer
  			player1_score = player1_score + P1wager_int;
  		} else {
  			uart_puts("Player 1 answered incorrectly!\n");
  			player1_score = player1_score - P1wager_int;
  		}
  		
  		if(P2f_ans == current_question->answer) {
  			uart_puts("Player 2 answered correctly!\n");		//Check for P2 answer
  			player2_score = player2_score + P2wager_int;
  		} else {
  			uart_puts("Player 2 answered incorrectly!\n");
  			player2_score = player2_score - P2wager_int;
  		}
  		
  	}
  	
  	/* For non-wager round */
  	if(stat == '0') {
  		if(active_player == '1') {					//If the active player is P1
  			if((ans <= '0') || (ans > '4')){		//Checking for invalid input
  				uart_puts("Invalid input.\n");
  			} else { 
  				uart_puts("Player 1 chooses: ");	//Display active player answer
  				uart_putc((char)(ans+16));
  				uart_puts("  \n");
  			}
  		} else { 
  			if((ans <= '0') || (ans > '4')) {		//If active player is P2
  				uart_puts("Invalid input.\n");	
  			} else {
  				uart_puts("Player 2 chooses: "); 
  				uart_putc((char)(ans+16));
  				uart_puts("  \n");
  			}
  		}
  		delay(60000);
  		delay(60000);
  		uart_puts("The correct answer is: ");				//Display correct answer
  		uart_putc((char)(current_question->answer + 16));
  		uart_puts("  \n");
  		delay(60000);
  		delay(60000);
  		if(ans == current_question->answer) {
  			if(active_player == '1') {						//Determine if P1 answers correctly
  				uart_puts("Player 1 answered correctly!\n");
  				if(current_question->type == 0) {
  					player1_score = player1_score + 100;	//Add points, 100 for normal, 200 for double points
  					uart_puts("100 points added!\n\n");
  				} else {
  					player1_score = player1_score + 200;
  					uart_puts("200 points added!\n\n");
  				}
  			} else {
  				uart_puts("Player 2 answered correctly!\n");//Determine if P2 answers correctly
  				if(current_question->type == 0) {
  					player2_score = player2_score + 100;	//Add points, 100 for normal, 200 for double points
  					uart_puts("100 points added!\n\n");
  				} else {
  					player2_score = player2_score + 200;
  					uart_puts("200 points added!\n\n");
  				}
  			}
  		} else {	//For wrong answer
  			if(active_player == '1') {						
  				uart_puts("Player 1 answered incorrectly!\n");
  				if(current_question->type == 0) {
  					player1_score = player1_score - 100;		//Deduct 100 for normal question, 200 for double points
  					uart_puts("100 points deducted :(\n");
  				} else {
  					player1_score = player1_score - 200;
  					uart_puts("200 points deducted :(\n");
  				}
  			} else {
  				uart_puts("Player 2 answered incorrectly!\n");
  				if(current_question->type == 0) {
  					player2_score = player2_score - 100;
  					uart_puts("100 points deducted :(\n");
  				} else {
  					player2_score = player2_score - 200;
  					uart_puts("200 points deducted :(\n");
  				}
  			}
  		}
  	}
  	displayScore();
  	uart_puts("----------------------------------------------\n\n");
  	questions_left = questions_left - 1;
  }
  
  /* -------------------------------------------------------
   * 	This method displays the score of the players.
   * ------------------------------------------------------*/
  void displayScore(void) {
		char buffer1[6];
		char buffer2[6];
		
		itoa(player1_score,buffer1,10);
		itoa(player2_score,buffer2,10);
		
		uart_puts("Current score for player 1 is: ");
		uart_puts(&buffer1[0]);
		uart_puts("  \n");
		uart_puts("Current score for player 2 is: ");
		uart_puts(&buffer2[0]);
		uart_puts("  \n\n");
  }
  
   /* -------------------------------------------------------
   * 	This method displays the final score of the players.
   * ------------------------------------------------------*/
  void displayFinalScore(void) {
		char buffer1[6];
		char buffer2[6];
		
		itoa(player1_score,buffer1,10);
		itoa(player2_score,buffer2,10);
		
		uart_puts("Final score for player 1 is: ");
		uart_puts(&buffer1[0]);
		uart_puts("  \n");
		uart_puts("Final score for player 2 is: ");
		uart_puts(&buffer2[0]);
		uart_puts("  \n\n");
  }

  /**
 * Ansi C "itoa" based on Kernighan & Ritchie's "Ansi C"
 * with slight modification to optimize for specific architecture:
 */
	
void strreverse(char* begin, char* end) {
	char aux;
	while(end>begin)
		aux=*end, *end--=*begin, *begin++=aux;
}

	
void itoa(int value, char* str, int base) {
	static char num[] = "0123456789abcdefghijklmnopqrstuvwxyz";
	char* wstr=str;
	int sign;
	div_t res;
	
	// Validate base
	if (base<2 || base>35){ *wstr='\0'; return; }
	
	// Take care of sign
	if ((sign=value) < 0) value = -value;
	
	// Conversion. Number is reversed.
	do {
		res = div(value,base);
		*wstr++ = num[res.rem];
	}while(value=res.quot);
	if(sign<0) *wstr++='-';
	*wstr='\0';
	
	// Reverse string
	strreverse(str,wstr-1);
}

/*------------------------------------------------------------------------
 * displayWager - this method displays the wager points for both players
 * It also convers the player's wager in char to the corresponding integer
 * -----------------------------------------------------------------------*/
 void displayWager(void) {
 	char buffer1[6];
	char buffer2[6];
		
	//Convert P1wager to the corresponding integer
 	switch(P1wager) {
 		case '0':
 		P1wager_int = 0;
 		break;
 		case '1':
 		P1wager_int = 250;
 		break;
 		case '2':
 		P1wager_int = 500;
 		break;
 		case '3':
 		P1wager_int = 750;
 		break;
 		case '4':
 		P1wager_int = 1000;
 		break;
 		default:
 		P1wager_int = 0;
 		break;
 	}
 	
  	switch(P2wager) {
 		case '0':
 		P2wager_int = 0;
 		break;
 		case '1':
 		P2wager_int = 250;
 		break;
 		case '2':
 		P2wager_int = 500;
 		break;
 		case '3':
 		P2wager_int = 750;
 		break;
 		case '4':
 		P2wager_int = 1000;
 		break;
 		default:
 		P2wager_int = 0;
 		break;
 	}
 	
 	//Convert the player's scores to string
	itoa(P1wager_int,buffer1,10);
	itoa(P2wager_int,buffer2,10);
		
	//Display players' scores
	uart_puts("Player 1 wagers: ");
	uart_puts(&buffer1[0]);
	uart_puts("  \n");
	uart_puts("Player 2 wagers: ");
	uart_puts(&buffer2[0]);
	uart_puts("  \n\n");
 	
 }
