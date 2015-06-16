#include <linux/string.h>
/*=========================================================================
Constants and definitions:
==========================================================================*/
#define N (4) /* the size of the board */
#define M (3)  /* the initial size of the snake */
#define K (5)  /* the number of turns a snake can survive without eating */

typedef char Player;
/* PAY ATTENTION! i will use the fact that white is positive one and black is negative
one to describe the segments of the snake. for example, if the white snake is 2 segments
long and the black snake is 3 segments long
white snake is  1   2
black snake is -1  -2  -3 */
#define WHITE ( 1) /* id to the white player */
#define BLACK (-1) /* id to the black player */
#define EMPTY ( 0) /* to describe an empty point */
/* to describe a point with food. having the value this big guarantees that there will be no
overlapping between the snake segments' numbers and the food id */
#define FOOD  (N*N)

#define FALSE (0)
#define TRUE  (1)

typedef int Direction;
#define DOWN  (2)
#define LEFT  (4)
#define RIGHT (6)
#define UP    (8)


#define STRCAT(buffer, strToAdd, size, counter) do { \
    if ( (!(counter) || (((counter)-(size))<(0))) ) { \
            return; \
    } \
    counter-=size; \
    strcat((buffer),(strToAdd)); \
} while (0)


/* a point in 2d space */
typedef struct
{
	int x, y;
} Point;
typedef int Matrix[N][N];

/* Game */
typedef struct game_t {
	int game_number;
	int is_game_cancled;
	int is_game_finished;
	int num_of_players;
	Player last_color; //0- initialize, 1- white, -1- black
	int winner; // (0) tie, (1) white, (-1) black
	int white_hunger;
	int black_hunger;
	Matrix board;
} Game;




typedef int ErrorCode;
#define ERR_OK      			((ErrorCode) 0)
#define ERR_BOARD_FULL			((ErrorCode)-1)
#define ERR_SNAKE_IS_TOO_HUNGRY ((ErrorCode)-2)
#define ERR_INVALID_TARGET 		((ErrorCode)-3)
#define ERR_INVALID_INPUT 		((ErrorCode)-4)

int Init(Matrix *matrix); /* initialize the board. return false if the board is illegal (should not occur, affected by N, M parameters) */
int Update(Matrix *matrix, Player player, char next_move, ErrorCode* e, int *white_hunger, int *black_hunger );/* handle all updating to this player. returns whether to continue or not. */
void Print(Matrix *matrix, char* buffer, int lenght);/* prints the state of the board */
Point GetInputLoc(Matrix *matrix, Player player, char next_move); /* calculates the location that the player wants to go to */
int CheckTarget(Matrix *matrix, Player player, Point p);/* checks if the player can move to the specified location */
Point GetSegment(Matrix *matrix, int segment);/* gets the location of a segment which is numbered by the value */
int IsAvailable(Matrix *matrix, Point p);/* returns if the point wanted is in bounds and not occupied by any snake */
ErrorCode CheckFoodAndMove(Matrix *matrix, Player player, Point p, int *white_hunger, int *black_hunger);/* handle food and advance the snake accordingly */
ErrorCode RandFoodLocation(Matrix *matrix);/* randomize a location for food. return ERR_BOARD_FULL if the board is full */
void AdvancePlayer(Matrix *matrix, Player player, Point p);/* advance the snake */
void IncSizePlayer(Matrix *matrix, Player player, Point p);/* advance the snake and increase it's size */
int IsMatrixFull(Matrix *matrix);/* check if the matrix is full */
int GetSize(Matrix *matrix, Player player);/* gets the size of the snake */


/*-------------------------------------------------------------------------
The main program. The program implements a snake game
-------------------------------------------------------------------------*/
int Init(Matrix *matrix)
{
	//initialize the matrix
	int i, j;
	for(i = 0; i < N; i++) {
		for(j = 0; j < N; j++) {
			(*matrix)[i][j] = 0;
		}
	}

	/* initialize the snakes location */
	for (i = 0; i < M; ++i)
	{
		(*matrix)[0][i] = WHITE * (i + 1);
		(*matrix)[N - 1][i] = BLACK * (i + 1);
	}
	/* initialize the food location */
	
	if (RandFoodLocation(matrix) != ERR_OK)
	{
		return FALSE;
	}
	printk("instructions: white player is represented by positive numbers, \nblack player is represented by negative numbers\n");

	return TRUE;
}

int Update(Matrix *matrix, Player player, char next_move, ErrorCode* e, int *white_hunger, int *black_hunger )
{
	//first check the input!!!
	Direction dir = next_move - '0';
	if (dir != UP && dir != DOWN && dir != LEFT && dir != RIGHT)
	{
		*e = ERR_INVALID_INPUT;
		return FALSE;
	}

	printk("[HW4 UPDATE] - STARTED - char is %c\n",next_move);
	Point p = GetInputLoc(matrix, player, next_move);

	if (!CheckTarget(matrix, player, p))
	{
		printk("[HW4 UPDATE]% d lost.\n", player);
		*e = ERR_INVALID_TARGET;
		return FALSE;
	}
	*e = CheckFoodAndMove(matrix, player, p, white_hunger, black_hunger);
	if (*e == ERR_BOARD_FULL)
	{
		printk("[HW4 UPDATE]the board is full, tie\n"); // return value 5
		return FALSE;
	}
	if (*e == ERR_SNAKE_IS_TOO_HUNGRY)
	{
		printk("[HW4 UPDATE]% d lost. the snake is too hungry\n", player);
		return FALSE;
	}
	// only option is that e == ERR_OK
	if (IsMatrixFull(matrix))
	{
		printk("[HW4 UPDATE]the board is full, tie\n");
		return FALSE;
	}
	printk("[HW4 UPDATE] - ENDED\n");
	return TRUE;
}

Point GetInputLoc(Matrix *matrix, Player player, char next_move)
{
	Direction dir = next_move - '0';
	printk("[HW4 GetInputLoc] - STARTED - dir is %d\n",dir);
	Point p;
	p.x = -1;
	p.y = -1;
	do
	{

		if (dir != UP   && dir != DOWN && dir != LEFT && dir != RIGHT)
		{
			return p;
		}
		else
		{
			break;
		}
	} while (TRUE);

	p = GetSegment(matrix, player);

	switch (dir)
	{
	case UP:    --p.y; break;
	case DOWN:  ++p.y; break;
	case LEFT:  --p.x; break;
	case RIGHT: ++p.x; break;
	}
	printk("[HW4 GetInputLoc] - ENDED\n");
	return p;
}

Point GetSegment(Matrix *matrix, int segment)
{
	/* just run through all the matrix */
	Point p;
	for (p.x = 0; p.x < N; ++p.x)
	{
		for (p.y = 0; p.y < N; ++p.y)
		{
			if ((*matrix)[p.y][p.x] == segment)
				return p;
		}
	}
	p.x = p.y = -1;
	return p;
}

int GetSize(Matrix *matrix, Player player)
{
	/* check one by one the size */
	Point p, next_p;
	int segment = 0;
	while (TRUE)
	{
		next_p = GetSegment(matrix, segment += player);
		if (next_p.x == -1)
			break;

		p = next_p;
	}

	return (*matrix)[p.y][p.x] * player;
}

int CheckTarget(Matrix *matrix, Player player, Point p)
{
	/* is empty or is the tail of the snake (so it will move the next
	to make place) */
	return IsAvailable(matrix, p) || ((*matrix)[p.y][p.x] == player * GetSize(matrix, player));
}

int IsAvailable(Matrix *matrix, Point p)
{
	return
		/* is out of bounds */
		!(p.x < 0 || p.x >(N - 1) ||
		p.y < 0 || p.y >(N - 1) ||
		/* is empty */
		((*matrix)[p.y][p.x] != EMPTY && (*matrix)[p.y][p.x] != FOOD));
}

ErrorCode CheckFoodAndMove(Matrix *matrix, Player player, Point p, int *white_hunger, int *black_hunger)
{
	if (player == BLACK)
		printk("[HW4 CheckFoodAndMove] BLACK - ITS HUNGER IS %d\n",*black_hunger); 
	if (player == WHITE)
		printk("[HW4 CheckFoodAndMove] WHITE - ITS HUNGER IS %d\n",*white_hunger); 


	/* if the player did come to the place where there is food */
	if ((*matrix)[p.y][p.x] == FOOD)
	{
		if (player == BLACK) *black_hunger = K;
		if (player == WHITE) *white_hunger = K;

		IncSizePlayer(matrix, player, p);

		if (RandFoodLocation(matrix) != ERR_OK)
			return ERR_BOARD_FULL;
	}
	else /* check hunger */
	{
		printk("[HW4 CheckFoodAndMove] player is %d\n",player); 
		if (player == BLACK && --(*black_hunger) == 0)
			return ERR_SNAKE_IS_TOO_HUNGRY;
		if (player == WHITE && --(*white_hunger) == 0)
			return ERR_SNAKE_IS_TOO_HUNGRY;

		AdvancePlayer(matrix, player, p);
	}
	return ERR_OK;
}

void AdvancePlayer(Matrix *matrix, Player player, Point p)
{
	/* go from last to first so the identifier is always unique */
	Point p_tmp, p_tail = GetSegment(matrix, GetSize(matrix, player) * player);
	int segment = GetSize(matrix, player) * player;
	while (TRUE)
	{
		p_tmp = GetSegment(matrix, segment);
		(*matrix)[p_tmp.y][p_tmp.x] += player;
		segment -= player;
		if (segment == 0)
			break;
	}
	(*matrix)[p_tail.y][p_tail.x] = EMPTY;
	(*matrix)[p.y][p.x] = player;
}

void IncSizePlayer(Matrix *matrix, Player player, Point p)
{
	/* go from last to first so the identifier is always unique */
	Point p_tmp;
	int segment = GetSize(matrix, player)*player;
	while (TRUE)
	{
		p_tmp = GetSegment(matrix, segment);
		(*matrix)[p_tmp.y][p_tmp.x] += player;
		segment -= player;
		if (segment == 0)
			break;
	}
	(*matrix)[p.y][p.x] = player;
}

ErrorCode RandFoodLocation(Matrix *matrix)
{
	Point p;
	do
	{
		p.x = jiffies % N;
		p.y = jiffies % N;

	} while (!(IsAvailable(matrix, p) || IsMatrixFull(matrix)));

	if (IsMatrixFull(matrix))
		return ERR_BOARD_FULL;

	(*matrix)[p.y][p.x] = FOOD;
	return ERR_OK;
}

int IsMatrixFull(Matrix *matrix)
{
	Point p;
	for (p.x = 0; p.x < N; ++p.x)
	{
		for (p.y = 0; p.y < N; ++p.y)
		{
			if ((*matrix)[p.y][p.x] == EMPTY || (*matrix)[p.y][p.x] == FOOD)
				return FALSE;
		}
	}
	return TRUE;
}


void Print(Matrix *matrix, char* buffer, int lenght)
{
	int counter = lenght;
	printk("[HW1 Print] - PRINTING BOARD");
	//ripud
	int i;
	for (i = 0; i < lenght; ++i)
	{
		buffer[i] = '\0';
	}

	Point p;
	for (i = 0; i < N + 1; ++i)
	{
		STRCAT(buffer, "---", 3, counter);
	}
	STRCAT(buffer,"\n",1,counter);
	for (p.y = 0; p.y < N; ++p.y)
	{
		STRCAT(buffer,"|",1,counter);
		for (p.x = 0; p.x < N; ++p.x)
		{
			int currentVal = (int)((*matrix)[p.y][p.x]);
			if(currentVal == FOOD)
			{
				STRCAT(buffer,"  *",3 , counter);
			} 
			else if (currentVal == EMPTY)
			{
				STRCAT(buffer,"  .", 3, counter);
			}
			else if (currentVal > 0)
			{
				STRCAT(buffer,"  ", 2, counter);
				char valToChar[2];
				valToChar[0] = currentVal + '0';
				valToChar[1] = '\0';
				STRCAT(buffer,valToChar, 1, counter);
			}
			else if (currentVal < 0)
			{
				STRCAT(buffer," -", 2, counter);
				char valToChar[2];
				valToChar[0] = (-currentVal) + '0';
				valToChar[1] = '\0';
				STRCAT(buffer,valToChar, 1, counter);	
			}
		}
		STRCAT(buffer," |\n", 3, counter);
	}
	for (i = 0; i < N + 1; ++i)
	{
		STRCAT(buffer,"---", 3, counter);
	}
	STRCAT(buffer,"\n", 1, counter);
	printk("[HW1 Print] - PRINT FINISHED");
}

