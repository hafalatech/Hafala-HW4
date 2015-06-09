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

MODULE_LICENSE("GPL");
MODULE_AUTHOR( "Henn, Roy and Guy" );

/*==================Structs==================*/
typedef struct game_t {
	int is_game_on;
	int num_of_players;
	
	/* game state variables */
	Matrix board;
	
	Point white_pos;
	int white_without_eat;
	
	Point black_pos;
	int black_without_eat;
} Game;

/*==================Global Variables==================*/
static int max_games = 0;
MODULE_PARM(max_games, "i" );

static Game games_array[max_games];
static int major = 0; 			// will hold the major # of my device driver
static int minor = 0;

struct file_operations fops = {         
        .open=         my_open,
        .release=     my_release,
        .read=          my_read,
        .write=         my_write,
        .llseek=        my_llseek,
        .ioctl=           my_ioctl,
        .owner=       THIS_MODULE
};

/*==================Init==================*/
int init_module(void)
{
	major = register_chrdev(ZERO, SNAKE_MODULE, &fops);
	if(major < 0)
	{
        return major;
	}
	int i;
	for(i = 0; i < max_games; i++) {
		games_array[i].is_game_on = 0; 
		games_array[i].num_of_players = 0;
	}
	return 0;
}

/*==================Clean==================*/
void cleanup_module(void)
{	
	unregister_chrdev(major, SNAKE_MODULE);
}

/*==================Open==================*/
int my_open( struct inode *inode, struct file *filp ) {

	if(flag == 0) {
		// U r the first player
	} else {
		//U r the second player
		//start the game
	}


	filp->private_data = allocate_private_data();

	if( filp->f_mode & FMODE_READ )
                //handle read opening
	if( filp->f_mode & FMODE_WRITE )
                //handle write opening
	if( filp->f_flags & O_NONBLOCK ) {
                //example of additional flag
	}
    
	if (MINOR( inode->i_rdev )==2){
        filp->f_op = &my_fops2;
    }

	return 0;
}

/*==================Release==================*/
int my_release( struct inode *inode, struct file *filp ) {
	if( filp->f_mode & FMODE_READ )
                //handle read close\ing
	if( filp->f_mode & FMODE_WRITE )
                //handle write closing

	MOD_DEC_USE_COUNT; /*no need in 2.4 or later*/

	return 0;
}

/*==================Read==================*/
ssize_t my_read( struct file *filp, char *buf, size_t count, loff_t *f_pos ) {
	//read the data
	//copy the data to user
    //return the ammount of read data
}

/*==================Write==================*/
ssize_t my_write(struct file *filp, const char *buf, size_t count, loff_t *f_pos) {
    //copy the data from user
	//write the data
    // return the ammount of written data
}

/*==================Llseek==================*/


/*==================Ioctl==================*/
int my_ioctl(struct inode *inode, struct file *filp, unsigned int cmd, unsigned long arg) {

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


