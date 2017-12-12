#include "board.h"


board* init_board( int rows, int columns, int living_cell_count )
{
	srand( time( NULL ) );
	// Allocate the board
	board* b = malloc( sizeof( board ) + ( rows * columns ) * sizeof( uint8_t ) );
	b->rows = rows;
	b->columns = columns;
	// Populate the board
	int rand_coord;
	for ( int i = 0; i < living_cell_count; i++ )
	{
		rand_coord = rand( ) % ( rows*columns );
		if ( b->grid[ rand_coord ] == TRUE )
		{
			i--;
			continue;
		}
		b->grid[ rand_coord ] = TRUE;
	}
	for ( int i = 0; i < rows; i++ )
	{
		for ( int j = 0; j < columns; j++ )
		{
			if ( !(cell_state(j, i, b) == TRUE))
				change_cell_state(j, i, FALSE, b);
		}
	}
	return b;
}


int update_board( board* b )
{
	// Allocate a temporary board
	int board_byte_size = sizeof( board ) + ( b->rows*b->columns ) * sizeof( uint8_t );
	board* temp_board = malloc( board_byte_size );
	temp_board->rows = b->rows;
	temp_board->columns = b->columns;

	// Iterate over the given board
	int living_cells_count = 0;
	for ( int i = 0; i < b->rows; i++ )
	{
		for ( int j = 0; j < b->columns; j++ )
		{
			if ( change_cell_state( j, i, updated_cell_state( j, i, b ), temp_board ) )
				living_cells_count++;
		}
	}

	// Copy the temporary board into the given board
	memcpy( b, temp_board, board_byte_size );
	free( temp_board );
	return living_cells_count;
}


int cell_state( int x, int y, board* b )
{
	if ( x < 0 || y < 0 || x >= b->columns || y >= b->rows )
		return FALSE;
	return b->grid[ y*b->columns + x ];
}


int updated_cell_state( int x, int y, board* b )
{
	// Count the living neighbors
	int living_neighbor_cells = 0;
	if ( cell_state( x - 1, y - 1, b ) ) living_neighbor_cells++;
	if ( cell_state( x, y - 1, b ) ) living_neighbor_cells++;
	if ( cell_state( x + 1, y - 1, b ) ) living_neighbor_cells++;
	if ( cell_state( x - 1, y, b ) ) living_neighbor_cells++;
	if ( cell_state( x + 1, y, b ) ) living_neighbor_cells++;
	if ( cell_state( x - 1, y + 1, b ) ) living_neighbor_cells++;
	if ( cell_state( x, y + 1, b ) ) living_neighbor_cells++;
	if ( cell_state( x + 1, y + 1, b ) ) living_neighbor_cells++;

	// Return the new state of the cell at position board[x][y]
	if ( ( cell_state( x, y, b ) && ( living_neighbor_cells == 2 ) ) || living_neighbor_cells == 3 )
		return TRUE;
	return FALSE;
}


int change_cell_state( int x, int y, int state, board* b )
{
	if ( x < 0 || y < 0 || x >= b->columns || y >= b->rows )
		return 0;
	return b->grid[ y*b->columns + x ] = state;
}


void draw_board( board* b, SDL_Renderer* renderer )
{
	Uint8 cell_color;
	SDL_Rect rectangle;
	rectangle.w = rectangle.h = CELL_SIZE;

	// Iterate over all cells and draw them to the renderer
	for ( int row = 0; row < b->rows; row++ )
	{
		for ( int column = 0; column < b->columns; column++ )
		{
			// Draw black squares for dead cells and white squares for living cells
			cell_color = cell_state( column, row, b ) ? 255 : 0;
			SDL_SetRenderDrawColor( renderer, cell_color, cell_color, cell_color, 255);
			rectangle.x = column * CELL_SIZE;
			rectangle.y = row * CELL_SIZE;
			SDL_RenderDrawRect( renderer, &rectangle );
		}
	}
	// Draw the renderer to the screen
	SDL_RenderPresent( renderer );
}
