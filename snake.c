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
typedef struct game_t {
	int is_game_cancled;
	int num_of_players;
	
	/* game state variables */
	Matrix board;
	
	Point white_pos;
	int white_without_eat;
	
	Point black_pos;
	int black_without_eat;

	int last_color;
	char next_move;
} Game;

typedef struct my_private_data {
	Game* my_game;
	int my_color;
	int my_is_winner;
} Data;


/*==================Global Variables==================*/
static int max_games = 0;
MODULE_PARM(max_games, "i" );

static Game games_array[max_games];
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
		games_array[i].num_of_players = 0;
		games_array[i].last_color = 0;	
	}
	return 0;
}

/*======================[Clean]========================*/
void cleanup_module(void)
{	
	unregister_chrdev(major, SNAKE_MODULE);
}


/*=======================[Open]=======================*/
int my_open( struct inode *inode, struct file *filep ) {

	int myMinor = get_minor_from_inode(inode);
	int myColor = 0;
	if (games_array[myMinor].is_game_cancled != 0)
	{
		return ERROR_SNAKE;
	}

	filep->f_op = &fops;

	if (games_array[myMinor].num_of_players == ZERO) {
		myColor = WHITE;
		games_array[myMinor].num_of_players++;	
		//todo - wait (semaphore?)
	}
	else if (games_array[myMinor].num_of_players == ONE) {
		myColor = BLACK;
		games_array[myMinor].num_of_players++;
		//free player 1 from wait
		StartNewGame(games_array[myMinor].board , &(games_array[myMinor].next_move));
	}
	else if (games_array[myMinor].num_of_players >= TWO) {
		return -1;
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
	return 0;
}




/*========================[Read]========================*/
ssize_t my_read( struct file *filep, char *buf, size_t count, loff_t *f_pos ) {
	if ((filep->private_data->my_game)->is_game_cancled != 0)
	{
		return ERROR_SNAKE;
	}

	char* board_buffer = (char*)kmalloc(sizeof(char)*count);

	//should we semaphore this shit?
	Print(filep->private_data).board, board_buffer, count);
	
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

	char* buffer = (char*)kmalloc(sizeof(char)*count);
	int result = copy_from_user(buffer, buf, count);


	if ((filep->private_data->my_game)->last_color) == (filep->private_data)->my_color)
	{
		//cant do this move until other player finished
	}

	(filep->private_data->my_game)->next_move = *buffer;
	(filep->private_data->my_game)->last_color = (filep->private_data)->my_color;
	//wake up LOCK_Update


	return result;
}




/*========================[Llseek]========================*/
int my_llseek(struct file *filep, loff_t a, int num) {
	return -ENOSYS;
}


/*========================[Ioctl]========================*/
int my_ioctl(struct inode *inode, struct file *filep, unsigned int cmd, unsigned long arg) {

	switch( cmd ) {
		case ZERO:
            //handle SNAKE_GET_WINNER;
			break;

		case ONE:
			//handle SNAKE_GET_COLOR;
			break;

		default: return -ENOTTY;
	}
	return 0;
}


