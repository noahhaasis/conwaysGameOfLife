#ifndef BOARD_H
#define BOARD_H

#include <stdio.h>
#include <stdint.h>
#include <ctype.h>
#include <time.h>
#include <string.h>
#include <math.h>
#include <assert.h>
#include "SDL.h"
#if DEBUG
#define _CRTDBG_MAP_ALLOC
#endif
#include <stdlib.h>  
#if DEBUG
#include <crtdbg.h>  
#endif

#define FALSE 0
#define TRUE 1

typedef Uint8 bool;

typedef struct
{
	int rows;
	int columns;
	bool grid[ ];
} board;

typedef struct
{
    int camera_x;
    int camera_y;
    int cell_size;
    int height_in_cells;
    int width_in_cells;
    int window_height;
    int window_width;
} view;


/**
* Initialize a board with a given size and a given number of
* living cells.
*/
board* init_board( int rows, int columns, int living_cell_count );

/**
* Update the board's state and return the number of living cells.
*/
int update_board( board* b );

/**
* Return the state of the cell at location x, y in the given board
*/
bool cell_state( int x, int y, board* b );

/**
* Return the updated state of the cell at location x, y in the given board
*/
bool updated_cell_state( int x, int y, board* b );

/**
* Write to the cell at the location x, y on the given board
* and return the written value.
*/
bool change_cell_state( int x, int y, bool state, board* b );

/** 
* Draw the given board to the window.
*/
void draw_board( board* b, view player_view, SDL_Renderer* renderer );

/**
* Kill all cells in the given board.
*/
void kill_all_cells( board* b );

/**
* Resizes the view. Adds the zoom factor the cell_size.
*/
void resize_board_view( int zoom, view* player_view, board* world );


/**
* Moves the camera in the given direction by a given amount.
* If it's not possbile to move by x and y it moves as far in the direction as possible.
* And if the camera position is out of bounds it doesn't move the camera at all.
*/
void move_camera_by( int x, int y, view* player_view, board* game_board, SDL_Window* window );

#endif
