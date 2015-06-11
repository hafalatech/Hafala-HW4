/*==================Includes==================*/
#include <linux/module.h>
#include <linux/kernel.h> // for using printk 
#include <linux/errno.h>
#include <asm/semaphore.h>
#include <linux/fs.h> //for struct file_operations
#include <asm/uaccess.h> //for copy_from_user()
//#include <linux/sched.h>
//#include <linux/slab.h>         // for kmalloc
//#include <linux/spinlock.h>
//#include <linux/wait.h>
#include "snake.h"
#include "hw3q1.h"

/*==================Defines==================*/
#define SNAKE_MODULE "snake"
#define ZERO 0 
#define ONE 1 
#define TWO 2 
#define ERROR_SNAKE -10 

MODULE_LICENSE("GPL");
MODULE_AUTHOR( "Henn, Roy and Guy" );

/*==================Structs==================*/
typedef struct my_private_data {
	Game* my_game;
	int my_color;
} Data;


/*==================Global Variables==================*/
static int max_games = 0;
MODULE_PARM(max_games, "i" );

static Game games_array[max_games];
struct semaphore semaphore_array[max_games],
static int major = 0; 			// will hold the major # of my device driver


struct file_operations fops = {         
    .open=        my_open,
    .release=     my_release,
    .read=        my_read,
    .write=       my_write,
    .llseek=      my_llseek,
    .ioctl=       my_ioctl,
    .owner=       THIS_MODULE
};

/*==================[static functions]==================*/
static int get_minor_from_inode(struct inode* inode)
{
        return MINOR(inode->i_rdev);
}


/*========================[Init]========================*/
int init_module(void)
{
	major = register_chrdev(ZERO, SNAKE_MODULE, &fops);
	if(major < 0)
	{
        return major;
	}
	int i;
	for(i = 0; i < max_games; i++) {
		games_array[i].game_number = i;
		games_array[i].is_game_cancled = 0;
		games_array[i].is_game_finished = 0;
		games_array[i].num_of_players = 0;
		games_array[i].last_color = 0;	
		games_array[i].winner = 0;
		
		sem_init(&semaphore_array[i], 1 , 2);
	}
	return 0;
}

/*======================[Clean]========================*/
void cleanup_module(void)
{	
	unregister_chrdev(major, SNAKE_MODULE);
	int i;
	for(i = 0; i < max_games; i++) {
		sem_destroy(&semaphore_array[i]);
	}
}

/*=======================[Open]=======================*/
int my_open( struct inode *inode, struct file *filep ) {

	int myMinor = get_minor_from_inode(inode);
	(struct semaphore)* sem = &semaphore_array[myMinor];
	int myColor = 0;
	if (games_array[myMinor].is_game_cancled != 0 || games_array[myMinor].is_game_finished != 0)
	{
		return ERROR_SNAKE;
	}

	filep->f_op = &fops;
	
	sem_wait(sem);
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
	sem_post(sem);
	
	if(games_array[myMinor].num_of_players == TWO) {
		StartNewGame(&games_array[myMinor]));
	}

	Data* newData = (Data*)kmalloc(sizeof(Data));
	newData->my_game = &(games_array[myMinor]);
	newData->my_color = myColor;
	filep->private_data = newData;

	return 0;
}

/*========================[Release]========================*/
int my_release( struct inode *inode, struct file *filep ) {
	int myMinor = get_minor_from_inode(inode);
	games_array[myMinor].is_game_cancled = 1;
	kfree(filep->private_data);
	
	return 0;
}

/*========================[Read]========================*/
ssize_t my_read( struct file *filep, char *buf, size_t count, loff_t *f_pos ) {
	if ((filep->private_data->my_game)->is_game_cancled != 0)
	{
		return ERROR_SNAKE;
	}
	
	int myMinor = (filep->private_data->my_game)->game_number;
	char* board_buffer = (char*)kmalloc(sizeof(char)*count);
	(struct semaphore)* sem = &semaphore_array[myMinor];
	
	sem_wait(sem);
	Print(((filep->private_data)->my_game)->board, board_buffer, count);
	sem_post(sem);
	
	int result = copy_to_user(buf, board_buffer, count);

	kfree(board_buffer);
	return result;
}

/*========================[Write]========================*/
ssize_t my_write(struct file *filep, const char *buf, size_t count, loff_t *f_pos) {
    if ((filep->private_data->my_game)->is_game_cancled != 0)
	{
		return ERROR_SNAKE;
	}

	int myMinor = (filep->private_data->my_game)->game_number;
	char* buffer = (char*)kmalloc(sizeof(char)*count);
	int result = copy_from_user(buffer, buf, count);
	(struct semaphore)* sem = &semaphore_array[myMinor];

	if ((filep->private_data->my_game)->last_color) == (filep->private_data)->my_color)
	{
		//cant do this move until other player finished
	}

	while(*buffer != '\0') {
		(filep->private_data->my_game)->next_move = *buffer;
		(filep->private_data->my_game)->last_color = (filep->private_data)->my_color;
		buffer++;
		//wake up LOCK_Update
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

	switch( cmd ) {
		case SNAKE_GET_WINNER:
            //handle SNAKE_GET_WINNER;
			if(filep->private_data->is_winner != 0 && filep->private_data->my_color == WHITE) {
				return 4;
			} else if(filep->private_data->is_winner != 0 && filep->private_data->my_color == BLACK) {
				return 2;
			} else if(filep->private_data->my_game->is_game_finished == 0) {
				return -1;
			}
			return -ENOTTY;
			break;

		case SNAKE_GET_COLOR:
			//handle SNAKE_GET_COLOR;
			if(filep->private_data->my_color == WHITE) {
				return 4;
			} else {
				return 2;
			}
			break;

		default: return -ENOTTY;
	}
	return 0;
}


