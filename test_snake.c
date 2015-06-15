/*
 * test_module.c
 *
 *  Created on: 7 áéðå 2015
 *      Author: alon
 */
#include <asm/errno.h>
extern int errno;
#include <stdio.h>
#include <fcntl.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>
#include <sys/mman.h>
#include "snake.h"
#include <sys/wait.h>

#define DOWN  '2'
#define LEFT  '4'
#define RIGHT '6'
#define UP    '8'

/*========================[instructions]========================*/
	// compile this test with:
	// gcc -Wall test_snake.c -o test_snake
	// then run with:
	// ./test_snake



	//here is an example of a driver install script:

	// #!/bin/bash
	// rm /dev/snake0
	// rm /dev/snake1
	// rm /dev/snake2
	// rm /dev/snake3
	//	.
	//	.
	//	.
	//	.
	// rm /dev/snake50
	// rmmod snake

	// make 
	// insmod ./snake.o max_games=50
	// mknod /dev/snake0 c 254 0
	// mknod /dev/snake1 c 254 1
	// mknod /dev/snake2 c 254 2
	// mknod /dev/snake3 c 254 3
	//	.
	//	.
	//	.
	//	.
	//	.
	// mknod /dev/snake50 c 254 50
/*========================[instructions]========================*/


/*========================[DEFINES]========================*/
	/**
	 * Evaluates b and continues if b is true.
	 * If b is false, ends the test by returning false and prints a detailed
	 * message about the failure.
	 */
	#define ASSERT(b) do { \
	    if (!(b)) { \
	            printf("\nAssertion failed at %s:%d %s ",__FILE__,__LINE__,#b); \
	            return false; \
	    } \
	} while (0)

	/**
	 * Macro used for running a test from the main function
	 */
	#define RUN_TEST(test) do { \
	    printf("Running "#test"... "); \
	    if (test()) { \
	        printf("[OK]\n");\
	    } else { \
	            printf("[Failed]\n"); \
	    } \
	} while(0)
/*========================[DEFINES]========================*/


/*========================[UTILS]========================*/
	void doMediumTask()
	{
	   int j;
	   for(j=0; j<1000; j++)
	   {
	      short i;
	      for (i=1; i != 0; i++)
	      {
	         ;
	      }
	   }
	}


	void doLongTask()
	{
	   int j;
	   for(j=0; j<30000; j++)
	   {
	      short i;
	      for (i=1; i != 0; i++)
	      {
	         ;
	      }
	   }
	}

	char moduleName[13];
	char* moduleBase = "/dev/snake";
	int moduleNum = 0;

	static int isGameBoy = 0;
	static int *numOfPlayers;
	static int *gameHasEnded;
	static char *nextMove;
	static int *player;
	static int *Currentplayer;
	static int *a;
	static int *b;
	int doNotPrint = 0;
	char* getModule()
	{
		int i;
		for (i = 0; i < 13; ++i) { moduleName[i] = '\0'; }
				
		char moduleIndex[4]; 
		for (i = 0; i < 4; ++i) { moduleIndex[i] = '\0'; }
			
		sprintf(moduleIndex, "%d", moduleNum); //convert int to string

		strcat(moduleName , moduleBase );	
		strcat(moduleName , moduleIndex );
		moduleNum++;

		if (doNotPrint==0)
		{
			printf("Entering driver: %s\n",moduleName);
		}
		return moduleName;
	}


	void cleanScreen()
	{
		printf("\033[2J");
		printf("\r"); 
	}

	void printLogo()
	{
	  	char* gameboyLogo = "_n________________________\n|_|______________________|_|\n|  ,--------------------.  |\n|  |   _______ **** __  |  |\n|  |  /       \\ ** /  \\ |  |\n|  | /  /~~~\\  \\  (o  o)|  |\n|  | |  |   |  |   |  | |  |\n|  |(o  o)  \\  \\___/  / |  |\n|  | \\__/ ** \\       /  |  |\n|  |  |  **** ~~~~~~~   |  |\n|  |  ^                 |  |\n|  |       LOADING      |  |\n|  `--------------------'  |\n|      _  GAME BOY         |\n|    _| |_         ,-.     |\n|   |_ O _|   ,-. \"._,\"    |\n|     |_|    \"._,\"   A     |\n|       _  _    B          |\n|      // //               |\n|     // //     \\\\\\\\\\\\     |\n|     `  `       \\\\\\\\\\\\    ,\n|___________...__________,\"\n";
		cleanScreen(); 
		printf("\r");   	
		printf("%s\n",gameboyLogo);               
	}

	void printDeadSnake()
	{
	  	char* deadSnake = "|      _______ **** __     |\n|     /       \\ ** /  \\    |\n|    /  /~~~\\  \\  (X  X)   |\n|    |  |   |  |   |  |    |\n|   (X  X)  \\  \\___/  /    |\n|    \\__/ ** \\       /     |\n|     |  **** ~~~~~~~      |\n|     ^                    |\n|   * SNAKES  CRASHED! *   |\n";
		printf("%s\n",deadSnake);   		
	}


	void printGameOver()
	{
	  	char* gameboyLogo = "_n________________________\n|_|______________________|_|\n|  ,--------------------.  |\n|  |   _______ **** __  |  |\n|  |  /       \\ ** /  \\ |  |\n|  | /  /~~~\\  \\  (o  o)|  |\n|  | |  |   |  |   |  | |  |\n|  |(X  X)  \\  \\___/  / |  |\n|  | \\__/ ** \\       /  |  |\n|  |  |  **** ~~~~~~~   |  |\n|  |  ^                 |  |\n|  |***** GAME OVER ****|  |\n|  `--------------------'  |\n|      _  GAME BOY         |\n|    _| |_         ,-.     |\n|   |_ O _|   ,-. \"._,\"    |\n|     |_|    \"._,\"   A     |\n|       _  _    B          |\n|      // //               |\n|     // //     \\\\\\\\\\\\     |\n|     `  `       \\\\\\\\\\\\    ,\n|___________...__________,\"\n";
		cleanScreen(); 
		printf("\r");   	
		printf("%s\n",gameboyLogo);      
	}

	void printBoard(char* board)
	{
		if (isGameBoy == 0)
		{
			printf("-------[Board]-------\n");
			printf("%s\n", board);
			return;
		}

		cleanScreen(); 
		printf("\r"); 
		char* upper = "_n________________________\n|_|______________________|_|\n|  ,--------------------.  |\n|  |********************|  |\n|  |********************|  |";
		char* upperWhite = "_n________________________\n|_|______________________|_|\n|  ,--------------------.  |\n|  |********WHITE*******|  |\n|  |********************|  |";
		char* upperBlack = "_n________________________\n|_|______________________|_|\n|  ,--------------------.  |\n|  |********BLACK*******|  |\n|  |********************|  |";

		if (*Currentplayer == 1)
		{
			printf("%s\n",upperBlack);
		}
		else if (*Currentplayer == -1)
		{
			printf("%s\n",upperWhite);
		}
		else if (*Currentplayer == 0)
		{
			printf("%s\n",upper);
		}


		char middle[1024];
		int i;
		for (i = 0; i < 1024; ++i)
		{
			middle[i] = '\0';
		}
		//row1
		strcat(middle,"|  |**");
		strcat(middle,board);
		middle[21] = '\0';
		strcat(middle,"***|  |\n");

		//row2
		strcat(middle,"|  |**");
		strcat(middle,board+16);
		middle[50] = '\0';
		strcat(middle,"***|  |\n");

		//row3
		strcat(middle,"|  |**");
		strcat(middle,board+32);
		middle[79] = '\0';
		strcat(middle,"***|  |\n");

		//row4
		strcat(middle,"|  |**");
		strcat(middle,board+48);
		middle[108] = '\0';
		strcat(middle,"***|  |\n");

		//row5
		strcat(middle,"|  |**");
		strcat(middle,board+64);
		middle[137] = '\0';
		strcat(middle,"***|  |\n");

		//row6
		strcat(middle,"|  |**");
		strcat(middle,board);
		middle[166] = '\0';
		strcat(middle,"***|  |\0");

		printf("%s\n",middle );

		char* downer = "|  |********************|  |\n|  `--------------------'  |\n|      _  GAME BOY         |\n|    _| |_         ,-.     |\n|   |_ O _|   ,-. \"._,\"    |\n|     |_|    \"._,\"   A     |\n|       _  _    B          |\n|      // //               |\n|     // //     \\\\\\\\\\\\     |\n|     `  `       \\\\\\\\\\\\    ,\n|___________...__________,\"\n";
		printf("%s\n",downer);
	}

	void printButtonPress(char* board , char* dir)
	{
		char direction = dir[0];
		cleanScreen(); 
		char* upperWhite = "_n________________________\n|_|______________________|_|\n|  ,--------------------.  |\n|  |********WHITE*******|  |\n|  |********************|  |";
		char* upperBlack = "_n________________________\n|_|______________________|_|\n|  ,--------------------.  |\n|  |********BLACK*******|  |\n|  |********************|  |";

		if (*Currentplayer == 1)
		{
			printf("%s\n",upperBlack);
		}
		else if (*Currentplayer == -1)
		{
			printf("%s\n",upperWhite);
		}


		char middle[1024];
		int i;
		for (i = 0; i < 1024; ++i)
		{
			middle[i] = '\0';
		}
		//row1
		strcat(middle,"|  |**");
		strcat(middle,board);
		middle[21] = '\0';
		strcat(middle,"***|  |\n");

		//row2
		strcat(middle,"|  |**");
		strcat(middle,board+16);
		middle[50] = '\0';
		strcat(middle,"***|  |\n");

		//row3
		strcat(middle,"|  |**");
		strcat(middle,board+32);
		middle[79] = '\0';
		strcat(middle,"***|  |\n");

		//row4
		strcat(middle,"|  |**");
		strcat(middle,board+48);
		middle[108] = '\0';
		strcat(middle,"***|  |\n");

		//row5
		strcat(middle,"|  |**");
		strcat(middle,board+64);
		middle[137] = '\0';
		strcat(middle,"***|  |\n");

		//row6
		strcat(middle,"|  |**");
		strcat(middle,board);
		middle[166] = '\0';
		strcat(middle,"***|  |\0");

		printf("%s\n",middle );

		char* downerUp = "|  |********************|  |\n|  `--------------------'  |\n|      _  GAME BOY         |\n|    _|X|_         ,-.     |\n|   |_ O _|   ,-. \"._,\"    |\n|     |_|    \"._,\"   A     |\n|       _  _    B          |\n|      // //               |\n|     // //     \\\\\\\\\\\\     |\n|     `  `       \\\\\\\\\\\\    ,\n|___________...__________,\"";
		char* downerDown = "|  |********************|  |\n|  `--------------------'  |\n|      _  GAME BOY         |\n|    _| |_         ,-.     |\n|   |_ O _|   ,-. \"._,\"    |\n|     |X|    \"._,\"   A     |\n|       _  _    B          |\n|      // //               |\n|     // //     \\\\\\\\\\\\     |\n|     `  `       \\\\\\\\\\\\    ,\n|___________...__________,\"";
		char* downerRight = "|  |********************|  |\n|  `--------------------'  |\n|      _  GAME BOY         |\n|    _| |_         ,-.     |\n|   |_ O X|   ,-. \"._,\"    |\n|     |_|    \"._,\"   A     |\n|       _  _    B          |\n|      // //               |\n|     // //     \\\\\\\\\\\\     |\n|     `  `       \\\\\\\\\\\\    ,\n|___________...__________,\"";
		char* downerLeft = "|  |********************|  |\n|  `--------------------'  |\n|      _  GAME BOY         |\n|    _| |_         ,-.     |\n|   |X O _|   ,-. \"._,\"    |\n|     |_|    \"._,\"   A     |\n|       _  _    B          |\n|      // //               |\n|     // //     \\\\\\\\\\\\     |\n|     `  `       \\\\\\\\\\\\    ,\n|___________...__________,\"";
		
		if (direction == UP)
		{
			printf("%s\n",downerUp);
		}
		else if (direction == RIGHT)
		{
			printf("%s\n",downerRight);
		}
		else if (direction == LEFT)
		{
			printf("%s\n",downerLeft);	
		}
		else if (direction == DOWN)
		{
			printf("%s\n",downerDown);	
		}
		
		doMediumTask();
		doMediumTask();
		doMediumTask();
	}

	void printingProgressBar()
	{
		char* loading1 = "\n**********************\n**********************\n**********************\n**------------------**\n**-[GAME OF SNAKES]-**\n**------------------**\n*******[LOADING]******\n**********************\n[                    ]\n";
		char* loading2 = "\n**********************\n**********************\n**********************\n**------------------**\n**-[GAME OF SNAKES]-**\n**------------------**\n*******[LOADING]******\n**********************\n[+++++               ]\n";
		char* loading3 = "\n**********************\n**********************\n**********************\n**------------------**\n**-[GAME OF SNAKES]-**\n**------------------**\n*******[LOADING]******\n**********************\n[++++++++++          ]\n";
		char* loading4 = "\n**********************\n**********************\n**********************\n**------------------**\n**-[GAME OF SNAKES]-**\n**------------------**\n*******[LOADING]******\n**********************\n[+++++++++++++++     ]\n";
		char* loading5 = "\n**********************\n**********************\n**********************\n**------------------**\n**-[GAME OF SNAKES]-**\n**------------------**\n*******[LOADING]******\n**********************\n[++++++++++++++++++++]\n";

		char* loading[5] = {loading1,loading2,loading3,loading4,loading5};
		//print text
		int i;
		for (i = 0; i < 5; ++i)
		{
			int j;
			for (j = 0; j < 6; ++j)
			{
				doMediumTask();
			}
			printf("\033[11A");
			printf("\r");
			printf("%s\n",loading[i]);
		} 
	}

	void printWelcome()
	{
		char* gameboy1 = "_n________________________\n|_|______________________|_|\n|  ,--------------------.  |\n|  |                    |  |\n|  |                    |  |\n|  |                    |  |\n|  |                    |  |\n|  |                    |  |\n|  |                    |  |\n|  |                    |  |\n|  |                    |  |\n|  |                    |  |\n|  `--------------------'  |\n|      _  GAME BOY         |\n|    _| |_         ,-.     |\n|   |_ O _|   ,-. \"._,\"    |\n|     |_|    \"._,\"   A     |\n|       _  _    B          |\n|      // //               |\n|     // //     \\\\\\\\\\\\     |\n|     `  `       \\\\\\\\\\\\    ,\n|___________...__________,\"\n";
		char* gameboy2 = "_n________________________\n|_|______________________|_|\n|  ,--------------------.  |\n|  |********************|  |\n|  |********************|  |\n|  |********************|  |\n|  |********************|  |\n|  |********************|  |\n|  |********************|  |\n|  |********************|  |\n|  |********************|  |\n|  |********************|  |\n|  `--------------------'  |\n|      _  GAME BOY         |\n|    _| |_         ,-.     |\n|   |_ O _|   ,-. \"._,\"    |\n|     |_|    \"._,\"   A     |\n|       _  _    B          |\n|      // //               |\n|     // //     \\\\\\\\\\\\     |\n|     `  `       \\\\\\\\\\\\    ,\n|___________...__________,\"\n";
		char* gameboy3 = "_n________________________\n|_|______________________|_|\n|  ,--------------------.  |\n|  |********************|  |\n|  |********************|  |\n|  |********************|  |\n|  |********************|  |\n|  |******[WELCOME]*****|  |\n|  |********[TO]********|  |\n|  |********************|  |\n|  |********************|  |\n|  |********************|  |\n|  `--------------------'  |\n|      _  GAME BOY         |\n|    _| |_         ,-.     |\n|   |_ O _|   ,-. \"._,\"    |\n|     |_|    \"._,\"   A     |\n|       _  _    B          |\n|      // //               |\n|     // //     \\\\\\\\\\\\     |\n|     `  `       \\\\\\\\\\\\    ,\n|___________...__________,\"\n";
		char* gameboy4 = "_n________________________\n|_|______________________|_|\n|  ,--------------------.  |\n|  |********************|  |\n|  |********************|  |\n|  |********************|  |\n|  |********************|  |\n|  |* [GAME OF SNAKES] *|  |\n|  |********************|  |\n|  |********************|  |\n|  |********************|  |\n|  |********************|  |\n|  `--------------------'  |\n|      _  GAME BOY         |\n|    _| |_         ,-.     |\n|   |_ O _|   ,-. \"._,\"    |\n|     |_|    \"._,\"   A     |\n|       _  _    B          |\n|      // //               |\n|     // //     \\\\\\\\\\\\     |\n|     `  `       \\\\\\\\\\\\    ,\n|___________...__________,\"\n";
		char* gameboy5 = "_n________________________\n|_|______________________|_|\n|  ,--------------------.  |\n|  |********************|  |\n|  |********************|  |\n|  |********************|  |\n|  |* [GAME OF SNAKES] *|  |\n|  |********************|  |\n|  |********************|  |\n|  |********************|  |\n|  |*[PLAY]**[RUN TEST]*|  |\n|  |********************|  |\n|  `--------------------'  |\n|      _  GAME BOY         |\n|    _| |_         ,-.     |\n|   |_ O _|   ,-. \"._,\"    |\n|     |_|    \"._,\"   A     |\n|       _  _    B          |\n|      // //               |\n|     // //     \\\\\\\\\\\\     |\n|     `  `       \\\\\\\\\\\\    ,\n|___________...__________,\"\n";

		char* gameboy[5] = {gameboy1,gameboy2,gameboy3,gameboy4,gameboy5};

		int i;
		for (i = 0; i < 5; i++)
		{
			cleanScreen(); 
			printf("%s\n", gameboy[i]);
			if (i==4)
			{
				printf("Press A to [PLAY], Press B to [RUN TEST]\n");
			}
			int j;
			for (j = 0; j < 6; j++){
				doMediumTask();
			}
		}
	}
/*========================[UTILS]========================*/




/*========================[TESTS]========================*/
bool CreateNewGameTest()
{
	char* moduleName = getModule(); //e.g "/dev/snake0"
	int a;
	int b;
	int status;
	int pid1 = fork();
	if(pid1 == 0) {
		//player 1 (white)
		a=open(moduleName, O_RDWR);
		ASSERT(a>=0);
		doLongTask();
		close(a);
		_exit(0);
	} else {
		int pid2 = fork();
		if(pid2 == 0) {
			doLongTask(); // to make sure im the second one
			//player 2 (black)
			b=open(moduleName, O_RDWR);
			ASSERT(b>=0);
			close(b);
			_exit(0);
		} else {
			wait(&status);
		}
		wait(&status);
	}
    return true;
}

bool CreateGameSameModuleAfterCancelTest()
{
	char* moduleName = getModule();
	int a;
	int b;
	int status;
	int pid1 = fork();
	if(pid1 == 0) {
		//player 1 (white)
		a=open(moduleName, O_RDWR);
		ASSERT(a>=0);
		doLongTask();
		close(a);
		_exit(0);
	} else {
		int pid2 = fork();
		if(pid2 == 0) {
			doLongTask(); // to make sure im the second one
			//player 2 (black)
			b=open(moduleName, O_RDWR);
			ASSERT(b>=0);
			close(b); 
			_exit(0);
		} else {
			wait(&status);
		}
		wait(&status);
	}
    close(a);
    close(b);

    // game was canceled, now we gonna try again
    doLongTask();

	int pid3 = fork();
	if(pid3 == 0) {
		//player 1 (white)
		a=open(moduleName, O_RDWR); //should fail
		ASSERT(a<0);
		doLongTask();
		_exit(0);
	} else {
		int pid4 = fork();
		if(pid4 == 0) {
			doLongTask(); // to make sure im the second one
			//player 2 (black)
			b=open(moduleName, O_RDWR);  //should fail
			ASSERT(b<0); 
			_exit(0);
		} else {
			wait(&status);
		}
		wait(&status);
	}
    close(a);
    close(b);	
    return true;
}

bool CreateGameWithThreePlayersTest()
{
	char* moduleName = getModule();
	int a;
	int b;
	int c;
	int status;
	int pid1 = fork();
	if(pid1 == 0) {
		//player 1 (white)
		a=open(moduleName, O_RDWR);
		ASSERT(a>=0);
		close(a);
		_exit(0);
	} else {
		int pid2 = fork();
		if(pid2 == 0) {
			doLongTask(); // to make sure im the second one
			//player 2 (black)
			b=open(moduleName, O_RDWR);
			ASSERT(b>=0);
			doLongTask();
			doLongTask();
			doLongTask();
			doLongTask();
			close(b);
			_exit(0);
		} else {
			int pid3 = fork();
			if(pid3 == 0) {
				doLongTask();
				doLongTask();
				//player 3
				c=open(moduleName, O_RDWR);
				ASSERT(c<0); 
				_exit(0);
			} else {
				wait(&status);
			}
			wait(&status);
		}
		wait(&status);
	}	
    return true;
}

bool PrintBoardTest()
{
	char* moduleName = getModule();
	char board[1024];
	int a;
	int b;
	int status;
	int pid1 = fork();
	if(pid1 == 0) {
		//player 1 (white)
		a=open(moduleName, O_RDWR);
		ASSERT(a>=0);



		//LET PLAYER 2 THE CHANCE TO PRINT BEFORE I GO		
		int j;
		for (j = 0; j < 6; j++){
			doMediumTask();
		}

		close(a);
		_exit(0);
	} else {
		int pid2 = fork();
		if(pid2 == 0) {
			doLongTask();
			//player 2 (black)
			b=open(moduleName, O_RDWR); 
			ASSERT(b>=0);
			int readRes = read(b, board, 1024);
			printBoard(board);
			ASSERT(readRes == 0);
			close(b);
			_exit(0);
		} else {
			wait(&status);
		}
		wait(&status);
	}	
    return true;
}

bool MakeMoveTest()
{
	char* moduleName = getModule();
	char board[1024];
	char nextMove[2];
	nextMove[0] = DOWN; 
	nextMove[1] = '\0'; 
	int a;
	int b;
	int status;
	int pid1 = fork();
	if(pid1 == 0) {
		//player 1 (white)
		a=open(moduleName, O_RDWR);
		ASSERT(a>=0);

		//board before
		int readRes = read(a, board, 1024);	
		ASSERT(readRes == 0);
		printf("board before move is:\n");
		printBoard(board);

		int writeval = write(a, nextMove, 2);	
		ASSERT(writeval == 0);

		//board after move
		readRes = read(a, board, 1024);	
		ASSERT(readRes == 0);
		printf("board after move is:\n");
		printBoard(board);

		doLongTask();
		close(a);
		_exit(0);
	} else {
		int pid2 = fork();
		if(pid2 == 0) {
			doLongTask();
			//player 2 (black)
			b=open(moduleName, O_RDWR); 
			ASSERT(b>=0);

			doLongTask();
			close(b);
			_exit(0);
		} else {
			wait(&status);
		}
		wait(&status);
	}	
    return true;
}

bool MakeMoveCrashWallTest()
{
	char* moduleName = getModule();
	char board[1024];
	char nextMove[2];
	nextMove[0] = UP; 
	nextMove[1] = '\0'; 
	int size = 2;
	int a;
	int b;
	int status;
	int pid1 = fork();
	if(pid1 == 0) {
		//player 1 (white)
		a=open(moduleName, O_RDWR);
		ASSERT(a>=0);

		//board before
		int readRes = read(a, board, 1024);	
		ASSERT(readRes == 0);

		int writeval = write(a, nextMove, size);
		ASSERT(writeval == -1);

		//board after illigal move
		readRes = read(a, board, 1024);	
		ASSERT(readRes < 0);

		doLongTask();
		close(a);
		_exit(0);
	} else {
		int pid2 = fork();
		if(pid2 == 0) {
			doLongTask();
			//player 2 (black)
			b=open(moduleName, O_RDWR); 
			ASSERT(b>=0);

			doLongTask();
			doLongTask();
			doLongTask();
			close(b);
			_exit(0);
		} else {
			wait(&status);
		}
		wait(&status);
	}	
    return true;
}

bool MakeMoveIlligalCharacterTest()
{
	char* moduleName = getModule();
	int size = 2;
	int a;
	int b;
	int status;
	int pid1 = fork();
	if(pid1 == 0) {
		//player 1 (white)
		char nextMove[2];
		nextMove[0] = DOWN; 
		nextMove[1] = '\0'; 
		a=open(moduleName, O_RDWR);
		ASSERT(a>=0);

		int writeval = write(a, nextMove, size);	
		ASSERT(writeval == 0);

		doLongTask();
		doLongTask();
		doLongTask();
		doLongTask();
		close(a);
		_exit(0);
	} else {
		int pid2 = fork();
		if(pid2 == 0) {
			doLongTask();
			char nextMove[2];
			nextMove[0] = '9';  //  <---- illigal move
			nextMove[1] = '\0'; 
			//player 2 (black)
			b=open(moduleName, O_RDWR); 
			ASSERT(b>=0);

			int writeval = write(b, nextMove, size);
			ASSERT(writeval == -1);

			doLongTask();
			close(b);
			_exit(0);
		} else {
			wait(&status);
		}
		wait(&status);
	}	
    return true;
}

bool MakeMultipleMovesTest()
{
	//you should see somthing like:
	// 3  2  1   (might also be with 4)
	//-3 -2 -1   (might also be with -4)
	char* moduleName = getModule();
	char board[1024];
	int a;
	int b;
	int status;
	int pid1 = fork();
	if(pid1 == 0) {
		char nextMove[5];
		nextMove[0] = DOWN; 
		nextMove[1] = RIGHT; 
		nextMove[2] = RIGHT; 
		nextMove[3] = RIGHT; 
		nextMove[4] = '\0'; 
		//player 1 (white)
		a=open(moduleName, O_RDWR);
		ASSERT(a>=0);

		int writeval = write(a, nextMove, 5);	
		ASSERT(writeval == 0);

		doLongTask();
		close(a);
		_exit(0);
	} else {
		int pid2 = fork();
		if(pid2 == 0) {
			char nextMove[5];
			nextMove[0] = UP;
			nextMove[1] = RIGHT; 
			nextMove[2] = RIGHT; 
			nextMove[3] = RIGHT; 
			nextMove[4] = '\0'; 

			doLongTask();
			//player 2 (black)
			b=open(moduleName, O_RDWR); 
			ASSERT(b>=0);

			int writeval = write(b, nextMove, 5);	
			ASSERT(writeval == 0);


			//board after move
			int readRes = read(b, board, 1024);	
			ASSERT(readRes == 0);
			printBoard(board);

			doLongTask();
			close(b);
			_exit(0);
		} else {
			wait(&status);
		}
		wait(&status);
	}	
    return true;
}

bool MakeMoveCrashSnakeTest()
{
	char* moduleName = getModule();
	int a;
	int b;
	int status;
	int pid1 = fork();
	if(pid1 == 0) {
		char nextMove[4];
		nextMove[0] = DOWN; 
		nextMove[1] = DOWN; 
		nextMove[2] = DOWN; 
		nextMove[3] = '\0'; 
		//player 1 (white)
		a=open(moduleName, O_RDWR);
		ASSERT(a>=0);

		int writeval = write(a, nextMove, 4);
		ASSERT(writeval == -1);

		doLongTask();
		doLongTask();
		doLongTask();
		doLongTask();
		close(a);
		_exit(0);
	} else {
		int pid2 = fork();
		if(pid2 == 0) {
			char nextMove[4];
			nextMove[0] = UP; 
			nextMove[1] = UP; 
			nextMove[2] = UP; 
			nextMove[3] = '\0'; 
			doLongTask();
			//player 2 (black)
			b=open(moduleName, O_RDWR); 
			ASSERT(b>=0);

			int writeval = write(b, nextMove, 4);
			ASSERT(writeval == -1);

			printDeadSnake();

			doLongTask();
			doLongTask();
			doLongTask();
			close(b);
			_exit(0);
		} else {
			wait(&status);
		}
		wait(&status);
	}	
    return true;
}

bool MakeMultipleMovesWithIlligalMoveTest()
{
	char* moduleName = getModule();
	int a;
	int b;
	int status;
	int pid1 = fork();
	if(pid1 == 0) {
		char nextMove[6];
		nextMove[0] = DOWN; 
		nextMove[1] = RIGHT; 
		nextMove[2] = '9';    //   <-------illigal!
		nextMove[3] = RIGHT; 
		nextMove[4] = UP; 
		nextMove[5] = '\0'; 
		//player 1 (white)
		a=open(moduleName, O_RDWR);
		ASSERT(a>=0);

		int writeval = write(a, nextMove, 6);
		ASSERT(writeval < 0);

		doLongTask();
		close(a);
		_exit(0);
	} else {
		int pid2 = fork();
		if(pid2 == 0) {
			char nextMove[4];
			nextMove[0] = UP;
			nextMove[1] = RIGHT; 
			nextMove[2] = RIGHT; 
			nextMove[3] = '\0'; 

			doLongTask();
			//player 2 (black)
			b=open(moduleName, O_RDWR); 
			ASSERT(b>=0);

			int writeval = write(b, nextMove, 4);
			ASSERT(writeval < 0);

			doLongTask();
			close(b);
			_exit(0);
		} else {
			//waitpid(pid2, &status,WNOHANG);
			wait(&status);
		}

		//waitpid(pid1, &status,WNOHANG);
		wait(&status);
	}	
    return true;
}

bool SnakeGetColorTest()
{
	char* moduleName = getModule();
	int a;
	int b;
	int status;
	int pid1 = fork();
	if(pid1 == 0) {
		//player 1 (white)
		a=open(moduleName, O_RDWR);
		ASSERT(a>=0);

		int ioctl_retval = ioctl(a, SNAKE_GET_COLOR);
        ASSERT(ioctl_retval == 4);

		doLongTask();
		close(a);
		_exit(0);
	} else {
		int pid2 = fork();
		if(pid2 == 0) {
			doLongTask();
			//player 2 (black)
			b=open(moduleName, O_RDWR); 
			ASSERT(b>=0);

			int ioctl_retval = ioctl(b, SNAKE_GET_COLOR);
        	ASSERT(ioctl_retval == 2);

			doLongTask();
			close(b);
			_exit(0);
		} else {
			wait(&status);
		}
		wait(&status);
	}
    return true;
}

bool GetWinnerWhiteWinTest()
{
	char* moduleName = getModule();
	int a;
	int b;
	int status;
	int pid1 = fork();
	if(pid1 == 0) {
		char nextMove[5];
		nextMove[0] = DOWN; 
		nextMove[1] = RIGHT; 
		nextMove[2] = RIGHT; 
		nextMove[3] = RIGHT; 
		nextMove[4] = '\0'; 
		//player 1 (white)
		a=open(moduleName, O_RDWR);
		ASSERT(a>=0);

		int writeval = write(a, nextMove, 5);	
		ASSERT(writeval < 0);

		int ioctl_retval = ioctl(a, SNAKE_GET_WINNER);
        ASSERT(ioctl_retval == 4);

		doLongTask();
		close(a);
		_exit(0);
	} else {
		int pid2 = fork();
		if(pid2 == 0) {
			char nextMove[5];
			nextMove[0] = UP;
			nextMove[1] = UP; 
			nextMove[2] = UP; 
			nextMove[3] = UP; 
			nextMove[4] = '\0'; 

			doLongTask();
			//player 2 (black)
			b=open(moduleName, O_RDWR); 
			ASSERT(b>=0);

			int writeval = write(b, nextMove, 5);	
			ASSERT(writeval < 0);

			int ioctl_retval = ioctl(b, SNAKE_GET_WINNER);
	        ASSERT(ioctl_retval == 4); //even if im black i should know that the white won!

			doLongTask();
			close(b);
			_exit(0);
		} else {
			wait(&status);
		}
		wait(&status);
	}
	return true;		
}

bool GetWinnerBlackWinTest()
{
	char* moduleName = getModule();
	int a;
	int b;
	int status;
	int pid1 = fork();
	if(pid1 == 0) {
		char nextMove[5];
		nextMove[0] = DOWN; 
		nextMove[1] = DOWN; 
		nextMove[2] = DOWN; 
		nextMove[3] = DOWN; 
		nextMove[4] = '\0'; 
		//player 1 (white)
		a=open(moduleName, O_RDWR);
		ASSERT(a>=0);

		int writeval = write(a, nextMove, 5);	
		ASSERT(writeval < 0);

		int ioctl_retval = ioctl(a, SNAKE_GET_WINNER);
        ASSERT(ioctl_retval == 2); //even if im white i should know that the black won!

		doLongTask();
		close(a);
		_exit(0);
	} else {
		int pid2 = fork();
		if(pid2 == 0) {
			char nextMove[5];
			nextMove[0] = UP;
			nextMove[1] = RIGHT; 
			nextMove[2] = RIGHT; 
			nextMove[3] = RIGHT; 
			nextMove[4] = '\0'; 

			doLongTask();
			//player 2 (black)
			b=open(moduleName, O_RDWR); 
			ASSERT(b>=0);

			int writeval = write(b, nextMove, 5);	
			ASSERT(writeval < 0);

			int ioctl_retval = ioctl(b, SNAKE_GET_WINNER);
	        ASSERT(ioctl_retval == 2); 

			doLongTask();
			close(b);
			_exit(0);
		} else {
			wait(&status);
		}
		wait(&status);
	}
	return true;	
}

bool GetWinnerGameInProgressTest()
{
	char* moduleName = getModule();
	int a;
	int b;
	int status;
	int pid1 = fork();
	if(pid1 == 0) {
		char nextMove[2];
		nextMove[0] = DOWN; 
		nextMove[1] = '\0'; 
		//player 1 (white)
		a=open(moduleName, O_RDWR);
		ASSERT(a>=0);

		int writeval = write(a, nextMove, 2);	
		ASSERT(writeval == 0);

		int ioctl_retval = ioctl(a, SNAKE_GET_WINNER);
        ASSERT(ioctl_retval == -1); //even if im white i should know that the black won!

		nextMove[0] = RIGHT; 

		writeval = write(a, nextMove, 2);	
		ASSERT(writeval == 0);


		doLongTask();
		close(a);
		_exit(0);
	} else {
		int pid2 = fork();
		if(pid2 == 0) {
			char nextMove[2];
			nextMove[0] = UP;
			nextMove[1] = '\0'; 

			doLongTask();
			//player 2 (black)
			b=open(moduleName, O_RDWR); 
			ASSERT(b>=0);

			int writeval = write(b, nextMove, 2);	
			ASSERT(writeval == 0);

			int ioctl_retval = ioctl(b, SNAKE_GET_WINNER);
	        ASSERT(ioctl_retval == -1); 

			nextMove[0] = RIGHT;

	        writeval = write(b, nextMove, 2);	
			ASSERT(writeval == 0);

			doLongTask();
			close(b);
			_exit(0);
		} else {
			wait(&status);
		}
		wait(&status);
	}
	return true;	
}

bool MakeSimpleGameGameAux(char* myModuleName)
{
	char* moduleName = myModuleName;
	char nextMove[2];
	nextMove[0] = DOWN; 
	nextMove[1] = '\0'; 
	int a;
	int b;
	int status;
	int pid1 = fork();
	if(pid1 == 0) {
		//player 1 (white)
		a=open(moduleName, O_RDWR);
		ASSERT(a>=0);

		int writeval = write(a, nextMove, 2);	
		ASSERT(writeval == 0);

		doLongTask();
		close(a);
		_exit(0);
	} else {
		int pid2 = fork();
		if(pid2 == 0) {
			doLongTask();
			//player 2 (black)
			b=open(moduleName, O_RDWR); 
			ASSERT(b>=0);

			doLongTask();
			close(b);
			_exit(0);
		} else {
			wait(&status);
		}
		wait(&status);
	}	
    return true;
}
bool TreeGamesAtTheSameTime()
{
	doNotPrint = 1;
	printf("This might take a while...\n");
	//some birocrat shit you can ignore
	char moduleName1[13]; moduleName1[0] = '\0';
	char moduleName2[13]; moduleName2[0] = '\0';
	char moduleName3[13]; moduleName3[0] = '\0';
	char* tempModule = getModule();
	strcpy(moduleName1, tempModule);
	tempModule = getModule();
	strcpy(moduleName2, tempModule);
	tempModule = getModule();
	strcpy(moduleName3, tempModule);
	doNotPrint = 0;

	//test start here
	static int *StartGames;
	StartGames = mmap(NULL, sizeof(*StartGames), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
	*StartGames = 0;

	int status;
	int game1 = fork();
	if(game1 == 0) 
	{
		//this will be game 1
		while(*StartGames==0)
		{
			//halt until everyone is ready
			;
		}
		printf("Im Entering driver: %s\n",moduleName1);
		MakeSimpleGameGameAux(moduleName1);
		_exit(0);
	}
	else
	{
			int game2 = fork();
			if(game2 == 0) 
			{
				//this will be game 2
				while(*StartGames==0)
				{
					//halt until everyone is ready
					;
				}
				printf("Im Entering driver: %s\n",moduleName2);
				MakeSimpleGameGameAux(moduleName2);
				_exit(0);
			}
			else
			{
					int game3 = fork();
					if(game3 == 0) 
					{
						//this will be game 3
						while(*StartGames==0)
						{
							//halt until everyone is ready
							;
						}
						printf("Im Entering driver: %s\n",moduleName3);
						MakeSimpleGameGameAux(moduleName3);
						_exit(0);
					}
					else
					{
							doLongTask();
							doLongTask();
							doLongTask();
							//Launch games!
							*StartGames=1;
						wait(&status);
					}
				wait(&status);
			}
		wait(&status);
	}
	munmap(StartGames, sizeof (*StartGames));
	return true;
}

bool TwoWhitesAgainstOneBlackTest()
{
	printf("\nno offend to black people, you guys are awesome!\n");
	char* moduleName = getModule();
	int a;
	int b;
	int status;
	int pid1 = fork();
	if(pid1 == 0) {
		//player 1 (white)
		a=open(moduleName, O_RDWR);
		ASSERT(a>=0);

		int myBetterHalf = fork();
		if(myBetterHalf == 0)
		{
			char nextMove[5];
			nextMove[0] = DOWN; 
			nextMove[1] = RIGHT; 
			nextMove[2] = RIGHT; 
			nextMove[3] = RIGHT; 
			nextMove[4] = '\0'; 

			int writeval = write(a, nextMove, 5);	
			ASSERT(writeval < 0);
			_exit(0);
		}
		else
		{
			doLongTask();
			char nextMove[5];
			nextMove[0] = DOWN; 
			nextMove[1] = RIGHT; 
			nextMove[2] = RIGHT; 
			nextMove[3] = RIGHT; 
			nextMove[4] = '\0'; 
			int writeval = write(a, nextMove, 5);
				
			ASSERT(writeval < 0);
			doLongTask();
			close(a);
			wait(&status);
		}
		_exit(0);
	} else {
		int pid2 = fork();
		if(pid2 == 0) {
			char nextMove[2];
			nextMove[0] = UP;
			nextMove[1] = '\0'; 

			doLongTask();
			doLongTask();
			//player 2 (black)
			b=open(moduleName, O_RDWR); 
			ASSERT(b>=0);
			int writeval = write(b, nextMove, 2);	
			ASSERT(writeval == 0);

			doLongTask();
			close(b);
			_exit(0);
		} else {
			wait(&status);
		}
		wait(&status);
	}	
    return true;
}

bool ThreeAgainstThreeTest()
{
	char* moduleName = getModule();
	int a;
	int b;
	int status;
	int pid1 = fork();
	if(pid1 == 0) {
		//player 1 (white)
		a=open(moduleName, O_RDWR);
		ASSERT(a>=0);

		int myFirstSon = fork();
		if(myFirstSon == 0)
		{
			char nextMove[5];
			nextMove[0] = DOWN; 
			nextMove[1] = RIGHT; 
			nextMove[2] = RIGHT; 
			nextMove[3] = RIGHT; 
			nextMove[4] = '\0'; 

			int writeval = write(a, nextMove, 5);	
			ASSERT(writeval == 0);
			_exit(0);
		}
		else
		{
			doLongTask();
			int mySecondSon = fork();
			if(mySecondSon == 0)
			{
				char nextMove[5];
				nextMove[0] = DOWN; 
				nextMove[1] = RIGHT; 
				nextMove[2] = RIGHT; 
				nextMove[3] = RIGHT; 
				nextMove[4] = '\0'; 

				int writeval = write(a, nextMove, 5);	
				ASSERT(writeval < 0);
				_exit(0);
			}
			else
			{
				doLongTask();
				doLongTask();
				char nextMove[5];
				nextMove[0] = DOWN; 
				nextMove[1] = RIGHT; 
				nextMove[2] = RIGHT; 
				nextMove[3] = RIGHT; 
				nextMove[4] = '\0'; 
				int writeval = write(a, nextMove, 5);
					
				ASSERT(writeval < 0);
				doLongTask();
				close(a);
				wait(&status);
			}
			wait(&status);
		}
		_exit(0);
	} else {
		int pid2 = fork();
		if(pid2 == 0) {
			//player 2 (black)
			b=open(moduleName, O_RDWR);
			ASSERT(b>=0);

			int myFirstSon = fork();
			if(myFirstSon == 0)
			{
				char nextMove[2];
				nextMove[0] = UP; 
				nextMove[1] = '\0'; 

				int writeval = write(b, nextMove, 2);	
				ASSERT(writeval == 0);
				_exit(0);
			}
			else
			{
				int mySecondSon = fork();
				if(mySecondSon == 0)
				{
					doLongTask();
					char nextMove[2];
					nextMove[0] = RIGHT; 
					nextMove[1] = '\0'; 

					int writeval = write(b, nextMove, 2);	
					ASSERT(writeval == 0);
					_exit(0);
				}
				else
				{
					//father code
					doLongTask();
					char nextMove[2];
					nextMove[0] = RIGHT; 
					nextMove[1] = '\0'; 
					int writeval = write(b, nextMove, 2);
						
					ASSERT(writeval == 0);
					doLongTask();
					close(b);
					wait(&status);
				}
				wait(&status);
			}
			_exit(0);
		} else {
			wait(&status);
		}
		wait(&status);
	}	
    return true;
}



//todo - get string instead of char!
void PlayFullGame()
{
	isGameBoy = 1;
	char nextMoveAux;
	int lastPlayer = 0;
	
	numOfPlayers = mmap(NULL, sizeof (*numOfPlayers), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
	*numOfPlayers = 0;

	gameHasEnded = mmap(NULL, sizeof (*gameHasEnded), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
	*gameHasEnded = -1; // game will be over if gameHasEnded is != -1
	
	nextMove = mmap(NULL, sizeof(*nextMove) * 2, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
	nextMove[1] = '\0';

	player = mmap(NULL, sizeof (*player), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
	*player = 100;

	Currentplayer = mmap(NULL, sizeof (*Currentplayer), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
	*Currentplayer = 0;

	a = mmap(NULL, sizeof (*a), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
	b = mmap(NULL, sizeof (*b), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);

	char* moduleName = "/dev/snake17";
	int status;
	int pid1 = fork();
	if(pid1 == 0) {
		char board[1024];
		//player 1 (white)
		*a=open(moduleName, O_RDWR);
		*numOfPlayers = *numOfPlayers +1;
		while(*gameHasEnded == -1)
		{
			if(*player == 1)
			{
				*player = 0;
				*Currentplayer = 1;
				read(*a, board, 1024); //board before
				printButtonPress(board,nextMove);

				int res = write(*a,nextMove, 2);
				if(res < 0 ) {*gameHasEnded =1;}

				read(*a, board, 1024);
				printBoard(board);
				*player = 100;
			}
		}
		_exit(0);
	} else {
		int pid2 = fork();
		if(pid2 == 0) {
			char board[1024];
			doLongTask();
			//player 2 (black)
			*b=open(moduleName, O_RDWR);
			*Currentplayer = -1;
			read(*b, board, 1024);
			printBoard(board);
			*numOfPlayers = *numOfPlayers +1;
			while(*gameHasEnded == -1)
			{
				if(*player == -1)
				{
					*player = 0;
					*Currentplayer = -1;
					read(*b, board, 1024);
					printButtonPress(board,nextMove);
					int res = write(*b,nextMove, 2);
					if(res < 0 ) {*gameHasEnded =1;}

					read(*b, board, 1024);
					printBoard(board);
					*player = 100;
					*gameHasEnded = ioctl(*b, SNAKE_GET_WINNER);
				}
			}
			_exit(0);
		} else {
			printLogo();
			doLongTask();
			lastPlayer = -1;
			//manager
			while(*gameHasEnded == -1)
			{
				if(*player == 100 && *numOfPlayers==2)
				{
					char nm;
					scanf(" %c", &nm);
					if(nm != 'a' && nm != 'A' 
						&& nm != 's' && nm != 'S'
						&& nm != 'w' && nm != 'W'
						&& nm != 'd' && nm != 'D'

						&& nm != '2' && nm != '4'
						&& nm != '6' && nm != '8')
					{
						continue;
					}

					if (nm == 'w' || nm == 'W' || nm == '8')
					{     
						nextMoveAux = UP;
					}
					else if (nm == 's' || nm == 'S' || nm == '2')
					{
				    	nextMoveAux = DOWN;
					}
					else if (nm == 'a' || nm == 'A' || nm == '4')
					{
				    	nextMoveAux = LEFT;
					}
					else if (nm == 'd' || nm == 'D' || nm == '6')
					{
				    	nextMoveAux = RIGHT;
					}

					nextMove[0] = nextMoveAux; 
					nextMove[1] = '\0';
					*player = lastPlayer * (-1);
					lastPlayer = *player;
				}
			}
			printGameOver();
		    close(*a);
    		close(*b);	
			wait(&status);
		}
		wait(&status);
	}
	munmap(numOfPlayers, sizeof (*numOfPlayers));
	munmap(gameHasEnded, sizeof (*gameHasEnded));
	munmap(nextMove, sizeof (*nextMove) * 2);
	munmap(player, sizeof (*player));
	munmap(Currentplayer, sizeof (*Currentplayer));
	munmap(a, sizeof (*a));
	munmap(b, sizeof (*b));
}


//bool multiple moves at once
//bool try to open more drivers then possible ??!?!
/*========================[TESTS]========================*/


int main(){
	setvbuf(stdout, NULL, _IONBF, 0);
    setvbuf(stderr, NULL, _IONBF, 0);
    printf("\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n");
    printf("\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n"); 

	printWelcome();

	char answer;
	while (scanf(" %c", &answer) == 1 && (answer != 'A' && answer != 'a' && answer != 'b' && answer != 'B')){
		printf("\033[2A");
		printf("Wrong input - Press A to play, Press B to run test\n");
	}
	printf("\033[2A");
	printf("                                                   \n                                                   \n");
	printf("\033[2A");

	if(answer == 'B' || answer == 'b')
	{
		cleanScreen();
		char* running = "_n________________________\n|_|______________________|_|\n|  ,--------------------.  |\n|  |********************|  |\n|  |********************|  |\n|  |********************|  |\n|  |***--------------***|  |\n|  |**-[RUNNING TEST]-**|  |\n|  |***--------------***|  |\n|  |********************|  |\n|  |********************|  |\n|  |********************|  |\n|  `--------------------'  |\n|      _  GAME BOY         |\n|    _| |_         ,-.     |\n|   |_ O _|   ,-. \"._,\"    |\n|     |_|    \"._,\"   A     |\n|       _  _    B          |\n|      // //               |\n|     // //     \\\\\\\\\\\\     |\n|     `  `       \\\\\\\\\\\\    ,\n|___________...__________,\"\n";
		printf("%s\n", running);
		printf("\n\n");
        RUN_TEST(CreateNewGameTest);
        RUN_TEST(CreateGameSameModuleAfterCancelTest);
        RUN_TEST(CreateGameWithThreePlayersTest);
        RUN_TEST(PrintBoardTest);
        RUN_TEST(MakeMoveTest);
        RUN_TEST(MakeMoveCrashWallTest);
        RUN_TEST(MakeMoveIlligalCharacterTest);
        RUN_TEST(MakeMultipleMovesTest);
        RUN_TEST(MakeMoveCrashSnakeTest);
        RUN_TEST(MakeMultipleMovesWithIlligalMoveTest);
        RUN_TEST(SnakeGetColorTest);
        RUN_TEST(GetWinnerWhiteWinTest);
        RUN_TEST(GetWinnerBlackWinTest);
        RUN_TEST(GetWinnerGameInProgressTest);
        RUN_TEST(TreeGamesAtTheSameTime);
        RUN_TEST(TwoWhitesAgainstOneBlackTest);
        RUN_TEST(ThreeAgainstThreeTest);

        
	}
	else if (answer == 'a' || answer == 'A')
	{
		PlayFullGame();
	}
	
    return 0;
}
