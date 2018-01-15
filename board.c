#include "board.h"

inline int power_of_two( int n )
{
    if ( n >= 0 && n <= 7 )
    {
        int powers[ 32 ] = {
            1, 2, 4, 8, 16, 32, 64, 128,
            256, 512, 1024, 2048, 4096, 8192, 16384, 32768,
            65536, 131072, 262144, 524288, 1048576, 2097152, 4194304, 8388608,
            16777216, 33554432, 67108864, 134217728, 268435456, 536870912, 1073741824, 2147483648
        };
        return powers[ n ];
    }
    else
        return ( int ) pow( ( double ) 2, n );
}

board* init_board( int rows, int columns, int living_cell_count )
{
	srand( (unsigned int)time( (time_t*)NULL ) );
	board* b = calloc( 1, board_byte_size( rows, columns ) );
	b->rows = rows;
	b->columns = columns;
	// Populate the board
	Uint32 rand_coord;
	for ( int i = 0; i < living_cell_count; i++ )
	{
		// Generate a 32 bit random value
		rand_coord = (rand( ) << 16 | rand()) % ( rows*columns );
		if ( (b->grid[rand_coord / 8]  & power_of_two(rand_coord % 8)) == TRUE )
		{
			i--;
			continue;
		}
		b->grid[ rand_coord / 8 ] |= power_of_two(rand_coord % 8);
	}
	return b;
}


int update_board( board* b )
{
	// Allocate a temporary board
	board* temp_board = malloc( board_byte_size( b->rows, b->columns) );
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
	memcpy( b, temp_board, board_byte_size( b->rows, b->columns) );
	free( temp_board );
	return living_cells_count;
}


bool cell_state( int x, int y, board* b )
{
	if ( x < 0 || y < 0 || x >= b->columns || y >= b->rows )
		return FALSE;
	return (b->grid[ ( ( y*b->columns + x ) / 8 ) ] & power_of_two( ( y*b->columns + x ) % 8 )) != 0;
}


bool updated_cell_state( int x, int y, board* b )
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
    // printf( "Row: %d of %d; Column: %d of %d\n", x, b->rows, y, b->columns );
	if ( ( cell_state( x, y, b ) && ( living_neighbor_cells == 2 ) ) || living_neighbor_cells == 3 )
		return TRUE;
	return FALSE;
}


bool change_cell_state( int x, int y, bool state, board* b )
{
	if ( x < 0 || y < 0 || x >= b->columns || y >= b->rows )
		return 0;
    if ( state )
	    return b->grid[ (y*b->columns + x)/8 ] |= power_of_two( ( y*b->columns + x ) % 8 );
    else 
        return b->grid[ (y*b->columns + x)/8 ] &= ~power_of_two( ( y*b->columns + x ) % 8 );
}


void draw_board( board* b, view player_view, SDL_Renderer* renderer )
{
	Uint8 cell_color;
	SDL_Rect rectangle;
	rectangle.w = rectangle.h = player_view.cell_size;

	// Iterate over all cells and draw them to the renderer
	int screenHeight, screenWidth;
	SDL_GetRendererOutputSize( renderer, &screenWidth, &screenHeight );
	for ( int row = 0, screenRows = screenHeight/ player_view.cell_size; row < screenRows; row++ )
	{
		for ( int column = 0, screenColumns = screenWidth/ player_view.cell_size; column < screenColumns; column++ )
		{
			// Draw black squares for dead cells and white squares for living cells
			cell_color = cell_state( column + player_view.camera_x, row + player_view.camera_y, b ) ? 255 : 0;
			SDL_SetRenderDrawColor( renderer, cell_color, cell_color, cell_color, 255);
			rectangle.x = column*player_view.cell_size;
			rectangle.y = row* player_view.cell_size;
			SDL_RenderDrawRect( renderer, &rectangle );
		}
	}
	// Draw the renderer to the screen
	SDL_RenderPresent( renderer );
}

void kill_all_cells( board * b )
{
    memset( ( char* ) b + sizeof( board ), 0, board_byte_size( b->rows, b->columns ) - sizeof( board ) );
}

inline int board_byte_size( int rows, int columns )
{
    int size = sizeof( board ) + ( rows * columns ) / 8;
    // Add 1 if the number of cells is not divisible by 8 (8 bits = 1 bytes)
    size += ( rows * columns % 8 ) ? 1 : 0;
    return size;
}