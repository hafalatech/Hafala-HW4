/*==================Includes==================*/

#include <linux/errno.h>
#include <linux/module.h>
#include <asm/semaphore.h>
#include <linux/fs.h> //for struct file_operations
#include <linux/kernel.h> //for printk()
#include <asm/uaccess.h> //for copy_from_user()
#include <linux/sched.h>
#include <linux/slab.h>         // for kmalloc
#include <linux/spinlock.h>
#include <linux/wait.h>
#include "hw3q1.h"
#include "snake.h"
MODULE_LICENSE("GPL");

/*==================Defines==================*/
#define SNAKE_MODULE "snake"
#define ZERO 0 
#define ONE 1 
#define TWO 2 
#define ERROR_SNAKE 10 

/*==================Structs==================*/
typedef struct my_private_data {
	Game *my_game;
	int my_color;
} Data;


/*==================Global Variables==================*/
static int major = -1; 			// will hold the major # of my device driver
struct file_operations fops; //forward declaration
int max_games = 0;
MODULE_PARM(max_games, "i" );

Game* games_array;
struct semaphore* semaphore_array;
struct semaphore* semaphore_array_open;
spinlock_t* spinlocks_array;
struct semaphore* semaphore_cond_white;
struct semaphore* semaphore_cond_black;
int* is_going_to_sleep_white;
int* is_going_to_sleep_black;


/*======================[Clean]========================*/
void cleanup_module(void)
{	
	printk("[HW4 cleanup_module] - start\n");
	int result = unregister_chrdev(major, SNAKE_MODULE);
    if(result < 0)
    {
        printk(KERN_ALERT "Could not unregister_chrdev: %d\n", result);
    }
	//todo - is there a destroy for semaphore kernel?
	kfree(games_array);
	kfree(semaphore_array);
	kfree(semaphore_array_open);
	kfree(semaphore_cond_white);
	kfree(semaphore_cond_black);
	kfree(is_going_to_sleep_white);
	kfree(is_going_to_sleep_black);
	kfree(spinlocks_array);
	printk("[HW4 cleanup_module] - end\n");
	return;
}

/*=======================[Open]=======================*/
int my_open( struct inode *inode, struct file *filep ) {
	int myMinor = MINOR(inode->i_rdev);
	printk("[HW4 OPEN] - NEW PLAYER IS TRYING TO JOIN MINOR %d\n",myMinor);

	struct semaphore *sem_for_open = &semaphore_array_open[myMinor];
	int myColor = 0;
	printk("[HW4 OPEN] - games_array[myMinor].is_game_cancled = %d , games_array[myMinor].is_game_finished = %d\n" , games_array[myMinor].is_game_cancled,games_array[myMinor].is_game_finished);
	if (games_array[myMinor].is_game_cancled != 0 || games_array[myMinor].is_game_finished != 0)
	{
		return -ERROR_SNAKE;
	}
	filep->f_op = &fops;
	
	//printk("[HW4 OPEN] - HERE1\n");
	spin_lock(spinlocks_array[myMinor]);
		if (games_array[myMinor].num_of_players == ZERO) {
			printk("[HW4 OPEN] - THE NEW PLAYER IS WHITE MINOR%d\n",myMinor);
			myColor = WHITE;
			games_array[myMinor].num_of_players++;	
		}
		else if (games_array[myMinor].num_of_players == ONE) {
			printk("[HW4 OPEN] - THE NEW PLAYER IS BLACK MINOR%d\n",myMinor);
			myColor = BLACK;
			games_array[myMinor].num_of_players++;
			printk("[HW4 OPEN] - WAKING UP WHITE IN CASE ITS SLEEPING MINOR%d\n",myMinor);
			up(sem_for_open);
		}
		else if (games_array[myMinor].num_of_players >= TWO) {
			printk("[HW4 OPEN] - THERE ARE ALREADY 2 PLAYERS, PREMITION DENIED MINOR%d\n",myMinor);
			spin_unlock(spinlocks_array[myMinor]);
			return -EPERM;
		}
	spin_unlock(spinlocks_array[myMinor]);

	printk("[HW4 OPEN] - ABOUT TO CALL INIT GAME MINOR%d\n",myMinor);
	if(games_array[myMinor].num_of_players == TWO) 
	{
		printk("[HW4 OPEN] - STARTING A NEW GAME! MINOR%d\n",myMinor);
		games_array[myMinor].last_color = BLACK; //IN ORDER TO ALLOW WHITE TO PLAY FIRST
		if (!Init(&games_array[myMinor].board))
		{
			//printk("Illegal M, N parameters.");
			return -1;
		}
	}
	//printk("[HW4 OPEN] - INIT GAME WAS CALLED \n");


	if (games_array[myMinor].num_of_players == ONE) {
		printk("[HW4 OPEN] - WHITE IS READY BUT BLACK IS NOT, GOING TO SLEEP MINOR%d\n",myMinor);
		down_interruptible(sem_for_open);
	}

	Data* newData = (Data*)kmalloc(sizeof(Data), GFP_KERNEL);
	newData->my_game = &(games_array[myMinor]);
	newData->my_color = myColor;
	filep->private_data = newData;
	printk("[HW4 OPEN] - MY_OPEN ENDED MINOR%d\n",myMinor);

	return 0;
}

/*========================[Release]========================*/
int my_release( struct inode *inode, struct file *filep ) {
	Data* m_data = ((Data*)filep->private_data);
	int myMinor = MINOR(inode->i_rdev);
	printk("[HW4 RELEASE] CALLED MINOR %d pid %d\n",myMinor, current->pid );
	if(m_data->my_color == WHITE)
	{
		printk("[HW4 RELEASE] IM WHITE MINOR %d pid %d\n",myMinor, current->pid );
	}else{
		printk("[HW4 RELEASE] IM BLACK MINOR %d pid %d\n",myMinor, current->pid );	
	}

	games_array[myMinor].is_game_cancled = 1;

	struct semaphore *sem_cond_white = &semaphore_cond_white[myMinor];
	struct semaphore *sem_cond_black = &semaphore_cond_black[myMinor];
	int i;
	printk("[HW4 RELEASE] THERE ARE %d black sleeping\n", is_going_to_sleep_black[myMinor]);	
	printk("[HW4 RELEASE] THERE ARE %d white sleeping\n", is_going_to_sleep_white[myMinor]);	
	for (i = is_going_to_sleep_black[myMinor]; i > 0; --i)
	{
		printk("[HW4 RELEASE] WAKING UP BLACK\n");	
		up(sem_cond_black);
	}
	for (i = is_going_to_sleep_white[myMinor]; i > 0; --i)
	{
		printk("[HW4 RELEASE] WAKING UP WHITE\n");	
		up(sem_cond_white);
	}

	kfree(filep->private_data);
	printk("[HW4 RELEASE] ENDED MINOR %d pid %d\n",myMinor, current->pid );
	return 0;
}

/*========================[Read]========================*/
ssize_t my_read( struct file *filep, char *buf, size_t count, loff_t *f_pos ) {
	printk("[HW4 MY_READ] - STARTED\n");
	Data* m_data = ((Data*)filep->private_data);
	if ((m_data->my_game)->is_game_cancled != 0 || (m_data->my_game)->is_game_finished != 0)
	{
		printk("[HW4 MY_READ] - THIS GAME IS LONG GONE\n");
		return -ERROR_SNAKE;
	}
	
	int myMinor = (m_data->my_game)->game_number;
	char* board_buffer = (char*)kmalloc(sizeof(char)*count,GFP_KERNEL);
	struct semaphore *sem = &semaphore_array[myMinor];
	
	down_interruptible(sem);
		Print(&(m_data->my_game)->board, board_buffer, count);
		printk("%s\n",board_buffer);
	up(sem);
	
	int newCount = strlen(board_buffer);
	int result = copy_to_user(buf, board_buffer, newCount);

	kfree(board_buffer);
	printk("[HW4 MY_READ] - ENDED\n");
	return result;
}

/*========================[Write]========================*/
ssize_t my_write(struct file *filep, const char *buf, size_t count, loff_t *f_pos) {
    printk("[HW4 WRITE] - STARTED pid %d\n",current->pid );
    Data* m_data = ((Data*)filep->private_data);
    if ((m_data->my_game)->is_game_cancled != 0 || (m_data->my_game)->is_game_finished != 0)
	{
		printk("[HW4 WRITE] - THIS GAME IS CANCELED pid %d\n",current->pid );
		return -ERROR_SNAKE;
	}

	int myMinor = (m_data->my_game)->game_number;
	char* buffer = (char*)kmalloc(sizeof(char)*count, GFP_KERNEL);
	int result = copy_from_user(buffer, buf, count);

	struct semaphore *sem = &semaphore_array[myMinor];
	struct semaphore *sem_cond_white = &semaphore_cond_white[myMinor];
	struct semaphore *sem_cond_black = &semaphore_cond_black[myMinor];

	Game* current_game = m_data->my_game;
	int i = 0;
	while(buffer[i] != '\0') {
		if(m_data->my_color == WHITE)
		{
			printk("[HW4 WRITE] IM WHITE - NEW ITERATION pid %d\n",current->pid );
		}else{
			printk("[HW4 WRITE] IM BLACK - NEW ITERATION pid %d\n",current->pid );
		}

		//before making a move, make sure the game is still on
		if (current_game->is_game_cancled != 0 || current_game->is_game_finished != 0)
		{	
			//check if the other guy is sleeping, if so, wake him up
			if(m_data->my_color == WHITE && is_going_to_sleep_black[myMinor] > 0)
			{
				printk("[HW4 WRITE] IM WHITE - WAKING UP BLACK PLAYER AND FINISH pid %d\n",current->pid );
				is_going_to_sleep_black[myMinor]--;
				up(sem_cond_black);
			}else if (m_data->my_color == BLACK && is_going_to_sleep_white[myMinor] > 0 ){
				printk("[HW4 WRITE] IM BLACK - WAKING UP WHITE PLAYER AND FINISH pid %d\n",current->pid );
				is_going_to_sleep_white[myMinor]--;
				up(sem_cond_white);
			}

			if(m_data->my_color == WHITE)
			{
				printk("[HW4 WRITE] IM WHITE - I WANTED TO MAKE A MOVE BUT GAME OVER pid %d\n",current->pid );
				printk("[HW4 WRITE] IM WHITE - RETURN pid %d\n",current->pid );
			}else{
				printk("[HW4 WRITE] IM BLACK - I WANTED TO MAKE A MOVE BUT GAME OVER pid %d\n",current->pid );
				printk("[HW4 WRITE] IM BLACK - RETURN pid %d\n",current->pid );
			}
			return -ERROR_SNAKE;
		}


		if(m_data->my_color == WHITE)
		{
			printk("[HW4 WRITE] IM WHITE - READING NEXT CHAR pid %d\n",current->pid );
		}else{
			printk("[HW4 WRITE] IM BLACK - READING NEXT CHAR pid %d\n",current->pid );	
		}
		ErrorCode update_error;	
		int hasLocked = 0;

		down_interruptible(sem);
			if (current_game->last_color == m_data->my_color)
			{		
				//cant do this move until other player finished
				hasLocked = 1;
				if(m_data->my_color == WHITE)
				{
					is_going_to_sleep_white[myMinor]++;
				}else {
					is_going_to_sleep_black[myMinor]++;
				}

				up(sem);
				if(m_data->my_color == WHITE)
				{
					printk("[HW4 WRITE] IM WHITE - NOT MY TURN - GOING TO SLEEP pid %d\n",current->pid );
					printk("[HW4 WRITE] IM WHITE - is_going_to_sleep_white[myMinor] =  %d\n",is_going_to_sleep_white[myMinor]);
					down_interruptible(sem_cond_white);
				}else {
					printk("[HW4 WRITE] IM BLACK - NOT MY TURN - GOING TO SLEEP pid %d\n",current->pid );
					printk("[HW4 WRITE] IM BLACK - is_going_to_sleep_black[myMinor] %d\n",is_going_to_sleep_black[myMinor] );
					down_interruptible(sem_cond_black);
				}	
			}
		if(hasLocked == 0)
		{
			up(sem);
		}

		down_interruptible(sem);			
			
			//before making a move, make sure the game is still on, and that the other player did no made an 
			//invalid move, or that the game has already finished
			if (current_game->is_game_cancled != 0 || current_game->is_game_finished != 0)
			{	
				if(m_data->my_color == WHITE)
				{
					printk("[HW4 WRITE] IM WHITE - I WANTED TO MAKE A MOVE BUT THE OTHER GUY SUCKS pid %d\n",current->pid );
				}else{
					printk("[HW4 WRITE] IM BLACK - I WANTED TO MAKE A MOVE BUT THE OTHER GUY SUCKS pid %d\n",current->pid );	
				}

				//check if the other guy is sleeping, if so, wake him up
				if(m_data->my_color == WHITE && is_going_to_sleep_black[myMinor] > 0)
				{
					printk("[HW4 WRITE] IM WHITE - WAKING UP BLACK PLAYER AND FINISH pid %d\n",current->pid );				
					is_going_to_sleep_black[myMinor]--;
					up(sem_cond_black);
				}else if (m_data->my_color == BLACK && is_going_to_sleep_white[myMinor] > 0){
					printk("[HW4 WRITE] IM BLACK - WAKING UP WHITE PLAYER AND FINISH pid %d\n",current->pid );	
					is_going_to_sleep_white[myMinor]--;
					up(sem_cond_white);
				}


				if(m_data->my_color == WHITE)
				{
					printk("[HW4 WRITE] IM WHITE - RETURN pid %d\n",current->pid );
				}else{
					printk("[HW4 WRITE] IM BLACK - RETURN pid %d\n",current->pid );
				}
				up(sem);
				return -ERROR_SNAKE;
			}

			if(m_data->my_color == WHITE)
			{
				printk("[HW4 WRITE] IM WHITE - CALLING TO UPDATE pid %d\n",current->pid );
			}else{
				printk("[HW4 WRITE] IM BLACK - CALLING TO UPDATE pid %d\n",current->pid );	
			}	
			//make move
			Update(&(current_game->board), m_data->my_color , buffer[i], &update_error , &(current_game->white_hunger), &(current_game->black_hunger));
			current_game->last_color = m_data->my_color;

			if(update_error == ERR_INVALID_TARGET || update_error == ERR_SNAKE_IS_TOO_HUNGRY) {
				
				if(m_data->my_color == WHITE)
				{
					printk("[HW4 WRITE] IM WHITE - LOSER - SETTING is_game_finished to 1 pid %d\n",current->pid );
				}else{
					printk("[HW4 WRITE] IM BLACK - LOSER - SETTING is_game_finished to 1 pid %d\n",current->pid );	
				}
				result = -ERROR_SNAKE;
				current_game->is_game_finished = 1;
				current_game->winner = -(m_data->my_color);
			} else if (update_error == ERR_BOARD_FULL) {
				
				if(m_data->my_color == WHITE)
				{
					printk("[HW4 WRITE] IM WHITE - no winner - SETTING is_game_finished to 1 pid %d\n",current->pid );
				}else{
					printk("[HW4 WRITE] IM BLACK - no winner - SETTING is_game_finished to 1 pid %d\n",current->pid );	
				}
				current_game->is_game_finished = 1;
				current_game->winner = 0; //tie
			} 

			if(m_data->my_color == WHITE && is_going_to_sleep_black[myMinor] > 0)
			{
				printk("[HW4 WRITE] IM WHITE - WAKING UP BLACK PLAYER TO MAKE HIS TURN pid %d\n",current->pid );
				is_going_to_sleep_black[myMinor]--;
				up(sem_cond_black);
			}else if (m_data->my_color == BLACK && is_going_to_sleep_white[myMinor] > 0){
				printk("[HW4 WRITE] IM BLACK - WAKING UP WHITE PLAYER TO MAKE HIS TURN pid %d\n",current->pid );
				is_going_to_sleep_white[myMinor]--;
				up(sem_cond_white);
			}
		up(sem);
		i++;
	}
	
	kfree(buffer);
	printk("[HW4 WRITE] - ENDED result = %d\n",result);
	return result;
}

/*========================[Llseek]========================*/
loff_t my_llseek(struct file *filep, loff_t a, int num) {
	return -ENOSYS;
}

/*========================[Ioctl]========================*/
int my_ioctl(struct inode *inode, struct file *filep, unsigned int cmd, unsigned long arg) {
    printk("[HW4 IOCTL] - CALLED\n");
    Data* m_data = ((Data*)filep->private_data);
 //    if ((m_data->my_game)->is_game_cancled != 0 || (m_data->my_game)->is_game_finished != 0)
	// {
	// 	return -ERROR_SNAKE;
	// }

	switch( cmd ) {
		case SNAKE_GET_WINNER:
            //handle SNAKE_GET_WINNER;
			if(m_data->my_game->winner > 0) {
				printk("[HW4 IOCTL] - SNAKE_GET_WINNER - WHITE IS WINNER\n");
				return 4;
			} else if(m_data->my_game->winner < 0) {
				printk("[HW4 IOCTL] - SNAKE_GET_WINNER - BLACK IS WINNER\n");
				return 2;
			} else if(m_data->my_game->is_game_finished == 0) {
				printk("[HW4 IOCTL] - SNAKE_GET_WINNER - GAME IS NOT FINISHED\n");
				return -1;
			}
			return -ENOTTY;
			break;

		case SNAKE_GET_COLOR:
			//handle SNAKE_GET_COLOR;
			if(m_data->my_color == WHITE) {
				printk("[HW4 IOCTL] -SNAKE_GET_COLOR- WHITE ENDED\n");
				return 4;
			} else if (m_data->my_color == BLACK) {
				printk("[HW4 IOCTL] -SNAKE_GET_COLOR- BLACK ENDED\n");
				return 2;
			}
			break;

		default: 
			printk("[HW4 IOCTL] -UNRECOGNIZED COMMAND\n");
			return -ENOTTY;

	}
	printk("[HW4 IOCTL] - ENDED\n");
	return 0;
}


struct file_operations fops = {         
    .open=        my_open,
    .release=     my_release,
    .read=        my_read,
    .write=       my_write,
    .llseek=      my_llseek,
    .ioctl=       my_ioctl,
    .owner=       THIS_MODULE
};


/*========================[Init]========================*/
int init_module(void)
{
	printk("[ HW4 ] - init_module CALLED\n");
	major = register_chrdev(ZERO, SNAKE_MODULE, &fops);
	if(major < 0)
	{
        return major;
	}

	games_array = (Game*)kmalloc(sizeof(Game)*max_games, GFP_KERNEL);
	semaphore_array_open = (struct semaphore*)kmalloc(sizeof(struct semaphore)*max_games, GFP_KERNEL);
	semaphore_array = (struct semaphore*)kmalloc(sizeof(struct semaphore)*max_games, GFP_KERNEL);
	semaphore_cond_white = (struct semaphore*)kmalloc(sizeof(struct semaphore)*max_games, GFP_KERNEL);
	semaphore_cond_black = (struct semaphore*)kmalloc(sizeof(struct semaphore)*max_games, GFP_KERNEL);

	is_going_to_sleep_white = (int*)kmalloc(sizeof(int)*max_games, GFP_KERNEL);
	is_going_to_sleep_black = (int*)kmalloc(sizeof(int)*max_games, GFP_KERNEL);


	spinlocks_array = (spinlock_t*)kmalloc(sizeof(spinlock_t)*max_games, GFP_KERNEL);
	
	
	int i;
	for(i = 0; i < max_games; i++) {
		games_array[i].game_number = i;
		games_array[i].is_game_cancled = 0;
		games_array[i].is_game_finished = 0;
		games_array[i].num_of_players = 0;
		games_array[i].last_color = 0;	
		games_array[i].winner = 0;
		games_array[i].white_hunger = K;
		games_array[i].black_hunger = K;
		is_going_to_sleep_white[i]=0;
		is_going_to_sleep_black[i]=0;

		spin_lock_init(&spinlocks_array[i]);

		sema_init(&semaphore_array_open[i], 0);
		sema_init(&semaphore_array[i], 1);
		sema_init(&semaphore_cond_white[i], 0);
		sema_init(&semaphore_cond_black[i], 0);


	}
	return 0;
}
