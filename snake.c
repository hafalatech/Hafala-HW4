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
#include "snake.h"
#include "hw3q1.h"
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
struct semaphore* semaphore_cond;
int* is_going_to_sleep;


/*======================[Clean]========================*/
void cleanup_module(void)
{	
	int result = unregister_chrdev(major, SNAKE_MODULE);
    if(result < 0)
    {
        printk(KERN_ALERT "Could not unregister_chrdev: %d\n", result);
    }
	//todo - is there a destroy for semaphore kernel?
	kfree(games_array);
	kfree(semaphore_array);
	kfree(semaphore_cond);
	kfree(is_going_to_sleep);
	return;
}

/*=======================[Open]=======================*/
int my_open( struct inode *inode, struct file *filep ) {

	int myMinor = MINOR(inode->i_rdev);
	struct semaphore *sem = &semaphore_array[myMinor];
	int myColor = 0;
	if (games_array[myMinor].is_game_cancled != 0 || games_array[myMinor].is_game_finished != 0)
	{
		return -ERROR_SNAKE;
	}

	filep->f_op = &fops;
	
	down_interruptible(sem);
		if (games_array[myMinor].num_of_players == ZERO) {
			myColor = WHITE;
			games_array[myMinor].num_of_players++;	
		}
		else if (games_array[myMinor].num_of_players == ONE) {
			myColor = BLACK;
			games_array[myMinor].num_of_players++;
		}
		else if (games_array[myMinor].num_of_players >= TWO) {
			return -1;
		}
	
		if(games_array[myMinor].num_of_players == TWO) 
		{
			games_array[myMinor].last_color = BLACK; //IN ORDER TO ALLOW WHITE TO PLAY FIRST
			//games_array[myMinor].board = { { EMPTY } };
			int i, j;
			for(i = 0; i < N; i++) {
				for(j = 0; j < N; j++) {
					games_array[myMinor].board[i][j] = 0;
				}
			}
			if (!Init(&games_array[myMinor].board))
			{
				printk("Illegal M, N parameters.");
				return -1;
			}
		}
	up(sem);

	Data* newData = (Data*)kmalloc(sizeof(Data), GFP_KERNEL);
	newData->my_game = &(games_array[myMinor]);
	newData->my_color = myColor;
	filep->private_data = newData;

	return 0;
}

/*========================[Release]========================*/
int my_release( struct inode *inode, struct file *filep ) {
	int myMinor = MINOR(inode->i_rdev);
	struct semaphore *sem = &semaphore_array[myMinor];
	down_interruptible(sem);
	games_array[myMinor].is_game_cancled = 1;
	up(sem);
	kfree(filep->private_data);
	
	return 0;
}

/*========================[Read]========================*/
ssize_t my_read( struct file *filep, char *buf, size_t count, loff_t *f_pos ) {
	Data* m_data = ((Data*)filep->private_data);
	if ((m_data->my_game)->is_game_cancled != 0 || (m_data->my_game)->is_game_finished != 0)
	{
		return -ERROR_SNAKE;
	}
	
	int myMinor = (m_data->my_game)->game_number;
	char* board_buffer = (char*)kmalloc(sizeof(char)*count,GFP_KERNEL);
	struct semaphore *sem = &semaphore_array[myMinor];
	
	down_interruptible(sem);
		Print(&(m_data->my_game)->board, board_buffer, count);
	up(sem);
	
	int result = copy_to_user(buf, board_buffer, count);

	kfree(board_buffer);
	return result;
}

/*========================[Write]========================*/
ssize_t my_write(struct file *filep, const char *buf, size_t count, loff_t *f_pos) {
    Data* m_data = ((Data*)filep->private_data);
    if ((m_data->my_game)->is_game_cancled != 0 || (m_data->my_game)->is_game_finished != 0)
	{
		return -ERROR_SNAKE;
	}

	int myMinor = (m_data->my_game)->game_number;
	char* buffer = (char*)kmalloc(sizeof(char)*count, GFP_KERNEL);
	int result = copy_from_user(buffer, buf, count);

	struct semaphore *sem = &semaphore_array[myMinor];
	struct semaphore *sem_cond = &semaphore_cond[myMinor];

	Game* current_game = m_data->my_game;
	int i = 0;
	while(buffer[i] != '\0') {
		ErrorCode update_error;	
		int hasLocked = 0;

		down_interruptible(sem);
			if (current_game->last_color == m_data->my_color)
			{	
				//cant do this move until other player finished
				hasLocked = 1;
				is_going_to_sleep[myMinor] = 1;
				up(sem);
				down_interruptible(sem_cond);
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
				return -ERROR_SNAKE;
			}

			//make move
			Update(&(current_game->board), m_data->my_color , buffer[i], &update_error);
			current_game->last_color = m_data->my_color;

			if(update_error == ERR_INVALID_TARGET || update_error == ERR_SNAKE_IS_TOO_HUNGRY) {
				current_game->is_game_finished = 1;
				current_game->winner = -(m_data->my_color);
			} else if (update_error == ERR_BOARD_FULL || update_error == ERR_OK) {
				current_game->is_game_finished = 1;
				current_game->winner = 0; //tie
			} 
			if(is_going_to_sleep[myMinor] == 1)
			{
				up(sem_cond);
			}
		up(sem);
		i++;
	}
	
	kfree(buffer);
	return result;
}

/*========================[Llseek]========================*/
int my_llseek(struct file *filep, loff_t a, int num) {
	return -ENOSYS;
}

/*========================[Ioctl]========================*/
int my_ioctl(struct inode *inode, struct file *filep, unsigned int cmd, unsigned long arg) {
    Data* m_data = ((Data*)filep->private_data);
    if ((m_data->my_game)->is_game_cancled != 0 || (m_data->my_game)->is_game_finished != 0)
	{
		return -ERROR_SNAKE;
	}

	switch( cmd ) {
		case SNAKE_GET_WINNER:
            //handle SNAKE_GET_WINNER;
			if(m_data->my_game->winner > 0 && m_data->my_color == WHITE) {
				return 4;
			} else if(m_data->my_game->winner < 0 && m_data->my_color == BLACK) {
				return 2;
			} else if(m_data->my_game->is_game_finished == 0) {
				return -1;
			}
			return -ENOTTY;
			break;

		case SNAKE_GET_COLOR:
			//handle SNAKE_GET_COLOR;
			if(m_data->my_color == WHITE) {
				return 4;
			} else {
				return 2;
			}
			break;

		default: return -ENOTTY;
	}
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
	major = register_chrdev(ZERO, SNAKE_MODULE, &fops);
	if(major < 0)
	{
        return major;
	}

	games_array = (Game*)kmalloc(sizeof(Game)*max_games, GFP_KERNEL);
	semaphore_array = (struct semaphore*)kmalloc(sizeof(struct semaphore)*max_games, GFP_KERNEL);
	semaphore_cond = (struct semaphore*)kmalloc(sizeof(struct semaphore)*max_games, GFP_KERNEL);
	is_going_to_sleep = (int*)kmalloc(sizeof(int)*max_games, GFP_KERNEL);
	
	int i;
	for(i = 0; i < max_games; i++) {
		games_array[i].game_number = i;
		games_array[i].is_game_cancled = 0;
		games_array[i].is_game_finished = 0;
		games_array[i].num_of_players = 0;
		games_array[i].last_color = 0;	
		games_array[i].winner = 0;
		is_going_to_sleep[i]=0;

		sema_init(&semaphore_array[i], 1);
		sema_init(&semaphore_cond[i], 0);
	}
	return 0;
}
