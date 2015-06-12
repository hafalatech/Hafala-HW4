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
#include "../hw4.h"



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




int PrintBuffer(char* buffer, int size)
{
    int i;
    for (i=0 ; i < size; i++)
    {
            printf("%c", buffer[i]);
    }
    printf("\n");
    return 0;
}

/*
 * returns 0 iff equal
 */
int CompareBuffers(char* buffer1, char* buffer2, int size)
{
    int i;
    for (i=0 ; i<size ; i++)
    {
            if (buffer1[i] - buffer2[i] != 0)
            {
                    return buffer1[i] - buffer2[i];
            }
    }
    return 0;
}

void InitBufferAbc(char* buffer, int size)
{
    char k = 'a';
    int i;
    for(i=0 ; i < size ; i++ )
    {
            buffer[i]= k;
            k++;
    }
}

int FirstTest()
{

	char board[1024];
	int retval = 0;

	int a;
	int b;
	int pid1 = fork();
	int status;
	
	if(pid1 == 0) {
		//player 1 (white)
		a=open("/dev/snake0", O_RDWR);   
		retval = read(a, board, 1000);
		printf("Board: \n %s", board);	
		_exit(0);
	} else {
		int pid2 = fork();
		if(pid2 == 0) {
			//player 2 (black)
			b=open("/dev/snake0", O_RDWR); 
			retval = read(a, board, 1000);
			printf("Board: \n %s", board);
			_exit(0);
		} else {
			wait(&status);
		}
		wait(&status);
	}
	
	
    close(a);
    close(b);	

    return retval;
}

int TestBasicReadWrite()
{
        const int BUF_SIZE = 24;
        char buffer[BUF_SIZE];
        char buffer2[BUF_SIZE];
        char buffer3[BUF_SIZE];

        int d=open("/dev/decryptor", O_RDWR);           //available also:
        int e=open("/dev/encryptor", O_RDWR);           //O_RDONLY , O_WRONLY
        
        InitBufferAbc(buffer, BUF_SIZE);

/**********************testing simple read write**************************/
        int retval = write(e,buffer, 16);
        ASSERT_TEST(retval == 16);

        retval = read(e, buffer2, 24);
        ASSERT_TEST(retval == 16);
        ASSERT_TEST(CompareBuffers(buffer, buffer2, 16) != 0);

        retval = write(d, buffer2, 16);
        ASSERT_TEST(retval == 16);

        retval = read(d, buffer3, 24);
        ASSERT_TEST(retval == 16);
        ASSERT_TEST(CompareBuffers(buffer, buffer3, 16) == 0);

        close(d);
        close(e);

        return 1;
}

int TestContinuity()
{
        const int BUF_SIZE = 80;
        int chunk_size = 80;
        char buffer[BUF_SIZE];
        char buffer2[BUF_SIZE];
        char buffer3[BUF_SIZE];

        int d=open("/dev/decryptor", O_RDWR);           //available also:
        int e=open("/dev/encryptor", O_RDWR);           //O_RDONLY , O_WRONLY

        InitBufferAbc(buffer, BUF_SIZE);

        int i, retval;

        for(i=0; i<100; i++)
        {
                retval = write(e, buffer, chunk_size);
                ASSERT_TEST(retval == chunk_size);

                retval = read(e, buffer2, chunk_size);
                ASSERT_TEST(retval == chunk_size);
                ASSERT_TEST(CompareBuffers(buffer, buffer2, chunk_size) != 0);

                retval = write(d, buffer2, chunk_size);
                ASSERT_TEST(retval == chunk_size);

                retval = read(d, buffer3, chunk_size);
                ASSERT_TEST(retval == chunk_size);
                ASSERT_TEST(CompareBuffers(buffer, buffer3, chunk_size) == 0);
        }

        close(d);
        close(e);

        return 1;
}

int TestCyclic()
{
        int d=open("/dev/decryptor", O_RDWR);           //available also:
        int e=open("/dev/encryptor", O_RDWR);           //O_RDONLY , O_WRONLY
        const int BUF_SIZE = 1024*3;
        int chunk_size = 1024*3;
        char buffer[BUF_SIZE];
        char buffer2[BUF_SIZE];
        char buffer3[BUF_SIZE];

        int i, retval;
        InitBufferAbc(buffer, BUF_SIZE);

        for (i=0; i<2; i++)
        {
                retval = write(e, buffer, chunk_size);
                ASSERT_TEST(retval == chunk_size);

                retval = read(e, buffer2, chunk_size);
                ASSERT_TEST(retval == chunk_size);
                ASSERT_TEST(CompareBuffers(buffer, buffer2, chunk_size) != 0);

                retval = write(d, buffer2, chunk_size);
                ASSERT_TEST(retval == chunk_size);

                retval = read(d, buffer3, chunk_size);
                ASSERT_TEST(retval == chunk_size);
                ASSERT_TEST(CompareBuffers(buffer, buffer3, chunk_size) == 0);
        }

        close(d);
        close(e);
        return 1;
}

int TestChangeKey()
{
        int oldKey = 3;
        int newKey = 17;
        const int BUF_SIZE = 24;
        char buffer[BUF_SIZE];
        char buffer2[BUF_SIZE];
        char buffer3[BUF_SIZE];

        int d=open("/dev/decryptor", O_RDWR);           //available also:
        int e=open("/dev/encryptor", O_RDWR);           //O_RDONLY , O_WRONLY

        InitBufferAbc(buffer, BUF_SIZE);

        int retval;
        retval = ioctl(e, HW4_SET_KEY, oldKey);
        ASSERT_TEST(retval == 0);
        retval = ioctl(d, HW4_SET_KEY, oldKey);
        ASSERT_TEST(retval == 0);

        retval = write(e,buffer, BUF_SIZE);
        ASSERT_TEST(retval == BUF_SIZE);

        retval = read(e, buffer2, BUF_SIZE);
        ASSERT_TEST(retval == BUF_SIZE);
        ASSERT_TEST(CompareBuffers(buffer, buffer2, BUF_SIZE) != 0);

        //Changing to new key
        retval = ioctl(d, HW4_SET_KEY, newKey);
        ASSERT_TEST(retval == 0);

        retval = write(d, buffer2, BUF_SIZE);
        ASSERT_TEST(retval == BUF_SIZE);

        retval = read(d, buffer3, BUF_SIZE);
        ASSERT_TEST(retval == BUF_SIZE);
        //We are not supposed to be able to decrypt, because we used the old key for encryption new key for decryption:
        ASSERT_TEST(CompareBuffers(buffer, buffer3, BUF_SIZE) != 0);

        //Changing back to the old key
        retval = ioctl(d, HW4_SET_KEY, oldKey);
        ASSERT_TEST(retval == 0);
        retval = write(d, buffer2, BUF_SIZE);
        ASSERT_TEST(retval == BUF_SIZE);
        retval = read(d, buffer3, BUF_SIZE);
        ASSERT_TEST(retval == BUF_SIZE);
        //Now supposed to be able to read:
        ASSERT_TEST(CompareBuffers(buffer, buffer3, BUF_SIZE) == 0);

        close(d);
        close(e);

        return 1;
}

int TestReaderWait()
{

        const int BUF_SIZE = 1024;
        int chunk_size = 1024;
        char message[] = "Luke, I am your father  ";
        int messageLength = strlen(message);
        char buffer2[BUF_SIZE];
        int i, retval, status;

        int id = fork();
        if (id == 0)
        {
                int d=open("/dev/decryptor", O_RDWR);
                sleep(1);
                retval = read(d, buffer2, BUF_SIZE);
                ASSERT_TEST(retval == messageLength);
                ASSERT_TEST(CompareBuffers(message, buffer2, messageLength) == 0);
                close(d);
                _exit(0);
        }
        // Parent process here:
        int d=open("/dev/decryptor", O_RDWR);           //available also:
        int e=open("/dev/encryptor", O_RDWR);           //O_RDONLY , O_WRONLY
        sleep(2);
        retval = write(e, message, messageLength);
        ASSERT_TEST(retval == messageLength);
        retval = read(e, buffer2, BUF_SIZE);
        ASSERT_TEST(retval == messageLength);
        retval = write(d, buffer2, messageLength);
        ASSERT_TEST(retval == messageLength);

        wait(&status);
        close(d);
        close(e);
        return 1;
}

int TestWriterWait()
{
        const int BUF_SIZE = 1024*4;

        char message[] = "Luke, I am your father  ";
        int messageLength = strlen(message);

        char buffer[BUF_SIZE];
        char buffer2[BUF_SIZE];
        int i, retval, status;

        InitBufferAbc(buffer, BUF_SIZE);
        //Fill the decrypor's buffer:
        int d=open("/dev/decryptor", O_RDWR);
        retval = write(d, buffer, BUF_SIZE);
        ASSERT_TEST(retval == BUF_SIZE);
        close(d);

        int id = fork();
        if (id == 0)
        {
                int d=open("/dev/decryptor", O_RDWR);           //available also:
                sleep(2);
                // Empty the decryptor's buffer:
                retval = read(d, buffer, BUF_SIZE);
                ASSERT_TEST(retval == BUF_SIZE);
                sleep(1);
                retval = read(d, buffer2, BUF_SIZE);
                ASSERT_TEST(retval == messageLength);
                ASSERT_TEST(CompareBuffers(message, buffer2, messageLength) == 0);
                close(d);
                _exit(0);
        }
        // Parent process here:
        d=open("/dev/decryptor", O_RDWR);               //available also:
        int e=open("/dev/encryptor", O_RDWR);           //O_RDONLY , O_WRONLY
        sleep(1);
        retval = write(e, message, messageLength);
        ASSERT_TEST(retval == messageLength);
        retval = read(e, buffer2, BUF_SIZE);
        ASSERT_TEST(retval == messageLength);
        retval = write(d, buffer2, messageLength);
        ASSERT_TEST(retval == messageLength);
        wait(&status);
        close(d);
        close(e);

        return 1;
}

static int WriteSingleChunk(int mode, char* buffer, int chunk_size)
{
        int device;
        switch(mode)
        {
        case 0:
                device=open("/dev/decryptor", O_RDWR);
                break;
        case 1:
                device=open("/dev/encryptor", O_RDWR);
                break;
        default:
                return 0;
        }

        int retval = write(device, buffer, chunk_size);
        ASSERT_TEST(retval == chunk_size);
        close(device);
        return 1;
}

static int ReadSingleChunk(int mode, char* expectedBuffer, int chunk_size)
{
        int device;
        switch(mode)
        {
        case 0:
                device=open("/dev/decryptor", O_RDWR);
                break;
        case 1:
                device=open("/dev/encryptor", O_RDWR);
                break;
        default:
                return 0;
        }

        char buffer[chunk_size];

        int retval = read(device, buffer, chunk_size);
        ASSERT_TEST(retval == chunk_size);
        ASSERT_TEST(CompareBuffers(expectedBuffer, buffer, chunk_size) == 0);
        close(device);
        return 1;
}

int PerformMultipleWrites(int concurrent_users, int mode, char* buffer, int size)
{
        int i, id, status;
        for (i=0; i<concurrent_users; i++)
        {
                id = fork();
                if (id == 0)
                {
                        ASSERT_TEST(WriteSingleChunk(mode, buffer, size));
                        _exit(0);
                }
        }
        while (wait(&status) != -1);
        return 1;
}

int PerformMultipleReads(int concurrent_users, int mode, char* expectedBuffer, int size)
{
        int i, id, status;
        for (i=0; i<concurrent_users; i++)
        {
                id = fork();
                if (id == 0)
                {
                        ASSERT_TEST(ReadSingleChunk(mode, expectedBuffer, size));
                        _exit(0);
                }
        }
        while (wait(&status) != -1);
        return 1;
}

int TestConcurrency()
{
        const int concurrent_users = 100;
        const int chunk_size = 24;
        char buffer[chunk_size];
        char expectedEncrypted[chunk_size];
        InitBufferAbc(buffer, chunk_size);

        // Encrypt the message, to initialize the expectedEncrypted that will be sent to the child processes
        int e=open("/dev/encryptor", O_RDWR);
        write(e, buffer, chunk_size);
        read(e, expectedEncrypted, chunk_size);
        close(e);

        ASSERT_TEST(PerformMultipleWrites(concurrent_users, 1, buffer, chunk_size));            // encrypt messages
        ASSERT_TEST(PerformMultipleReads(concurrent_users, 1, expectedEncrypted, chunk_size));

        ASSERT_TEST(PerformMultipleWrites(concurrent_users, 0, expectedEncrypted, chunk_size)); // decrypt messages
        ASSERT_TEST(PerformMultipleReads(concurrent_users, 0, buffer, chunk_size));
        return 1;
}

int main(){
        //setvbuf(stdout, NULL, _IONBF, 0);
        //setvbuf(stderr, NULL, _IONBF, 0);
        RUN_TEST(FirstTest);
        /*
		RUN_TEST(TestBasicReadWrite);
        RUN_TEST(TestContinuity);
        RUN_TEST(TestCyclic);
        RUN_TEST(TestChangeKey);

        // Waiting tests:
        RUN_TEST(TestReaderWait);
        RUN_TEST(TestWriterWait);
        // Concurrency test:
        RUN_TEST(TestConcurrency);
		*/
        return 0;
}
