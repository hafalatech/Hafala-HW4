
#include <asm/errno.h>
extern int errno;
#include <stdio.h>
#include <fcntl.h>
#include <string.h>
#include <stdbool.h>




/*========================[DEFINES]========================*/
	/**
	 * Evaluates b and continues if b is true.
	 * If b is false, ends the test by returning false and prints a detailed
	 * message about the failure.
	 */
	#define ASSERT_TEST(b) do { \
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



int FirstTest()
{

	char* board;
	board = (char*)malloc(sizeof(char)*1024);
	
	int retval = 0;

	int a;
	int b;
	int pid1 = fork();
	int status;
	
	if(pid1 == 0) {
		//player 1 (white)
		a=open("/dev/snake0", O_RDWR);   
		retval = read(a, board, 1024);
		printf("Board: %s  \n", board);	
		_exit(0);
	} else {
		int pid2 = fork();
		if(pid2 == 0) {
			//player 2 (black)
			b=open("/dev/snake0", O_RDWR); 
			retval = read(a, board, 1024);
			printf("Board: %s  \n", board);
			_exit(0);
		} else {
			wait(&status);
		}
		wait(&status);
	}
	
	
    close(a);
    close(b);	

	free(board);
    return retval;
}


int main(){

        RUN_TEST(FirstTest);

        return 0;
}
