#ifndef BOARD_H
#define BOARD_H

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <ctype.h>
#include <time.h>
#include <string.h>
#include "SDL.h"

#define FALSE 0
#define TRUE 1
#define CELL_SIZE 10
#define CAMERA_MOVEMENT_SPEED 4
#define STARTING_POPULATION 4000

typedef struct
{
	int rows;
	int columns;
	uint8_t grid[ ];
} board;


/**
* Initializes a board with a given size and a given number of
* living cells.
*/
board* init_board( int rows, int columns, int living_cell_count );

/**
* Updates the board's state and return the number of living cells.
*/
int update_board( board* b );

/**
* Returns the state of the cell at location x, y in the given board
*/
int cell_state( int x, int y, board* b );

/**
* Returns the updated state of the cell at location x, y in the given board
*/
int updated_cell_state( int x, int y, board* b );

/**
* Write to the cell at the location x, y on the given board
* and return the writen value.
*/
int change_cell_state( int x, int y, int state, board* b );

/** 
* Draws the given board to the window.
*/
void draw_board( board* b, int camera_x, int camera_y, SDL_Renderer* renderer);

/**
* Kills all cells in the given board.
*/
void kill_all_cells( board* b );

#endif
