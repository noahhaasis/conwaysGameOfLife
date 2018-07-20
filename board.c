#include "board.h"

inline int power_of_two( int n )
{
    if ( n >= 0 && n <= 7 )
    {
        static int powers[ 32 ] = {
            1, 2, 4, 8, 16, 32, 64, 128,
            256, 512, 1024, 2048, 4096, 8192, 16384, 32768,
            65536, 131072, 262144, 524288, 1048576, 2097152, 4194304, 8388608,
            16777216, 33554432, 67108864, 134217728, 268435456, 536870912, 1073741824, 2147483648
        };
        return powers[ n ];
    }
    else
    {
        return ( int ) pow( ( double ) 2, n );
    }
}

int random( )
{
    static bool initialized;
    if ( !initialized )
    {
        srand( time( NULL ) );
        initialized = TRUE;
    }
    int rand_val = rand( );
    // This is not constant! RAND_MAX can vary on different systems!
    if ( RAND_MAX < INT_MAX )
    {
        rand_val |= rand( ) << 16;
    }
    return rand_val;
}

/*
 * Returns a uniformly distributed value in the range 0 to n-1.
 * Thanks @fbartho pointing me to uniformly distributed values.
 */
int random_uniform( int n )
{
    int rand_val;
    do
    {
        rand_val = random( );
    }
    while ( rand_val >= INT_MAX - ( INT_MAX % n ) );
    return rand_val % n ;
}


inline int board_byte_size( int rows, int columns )
{
    int size = sizeof( board ) + ( rows * columns ) / 8;
    // Add 1 if the number of cells is not divisible by 8 (8 bits = 1 bytes)
    size += ( rows * columns % 8 ) ? 1 : 0;
    return size;
}

board* init_board( int rows, int columns, int living_cell_count )
{
    board* b = calloc( 1, board_byte_size( rows, columns ) );
    b->rows = rows;
    b->columns = columns;
    // Populate the board
    Uint32 rand_coord;
    for ( int i = 0; i < living_cell_count; i++ )
    {
        // Generate a 32 bit random value
        Uint32 rand_range = ( rows * columns );
        rand_coord = random_uniform( rand_range );
        if ( ( b->grid[ rand_coord / 8] & power_of_two( rand_coord % 8 ) ) == TRUE )
        {
            i--;
            continue;
        }
        b->grid[ rand_coord / 8 ] |= power_of_two( rand_coord % 8 );
    }
    return b;
}


int update_board( board* b )
{
    // Allocate a temporary board
    board* temp_board = malloc( board_byte_size( b->rows, b->columns ) );
    temp_board->rows = b->rows;
    temp_board->columns = b->columns;

    // Iterate over the given board
    int living_cells_count = 0;
    for ( int i = 0; i < b->rows; i++ )
    {
        for ( int j = 0; j < b->columns; j++ )
        {
            if ( change_cell_state( j, i, updated_cell_state( j, i, b ), temp_board ) )
            {
                living_cells_count++;
            }
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
    {
        return FALSE;
    }
    return ( b->grid[ ( ( y*b->columns + x ) / 8 ) ] & power_of_two( ( y*b->columns + x ) % 8 ) ) != 0;
}


bool updated_cell_state( int x, int y, board* b )
{
    // Count the living neighbors
    int living_neighbor_cells = 
        cell_state( x - 1, y - 1, b ) +
        cell_state( x, y - 1, b ) +
        cell_state( x + 1, y - 1, b ) +
        cell_state( x - 1, y, b ) +
        cell_state( x + 1, y, b ) +
        cell_state( x - 1, y + 1, b ) +
        cell_state( x, y + 1, b ) +
        cell_state( x + 1, y + 1, b ) ;

    // Return the new state of the cell at position board[x][y]
    return ( living_neighbor_cells == 3 || ( cell_state( x, y, b ) && ( living_neighbor_cells == 2 ) ) );
}


bool change_cell_state( int x, int y, bool state, board* b )
{
    if ( x < 0 || y < 0 || x >= b->columns || y >= b->rows )
    {
        return FALSE;
    }
    else if ( state )
    {
        return b->grid[ ( y*b->columns + x ) / 8 ] |= power_of_two( ( y*b->columns + x ) % 8 );
    }
    else
    {
        return b->grid[ ( y*b->columns + x ) / 8 ] &= ~power_of_two( ( y*b->columns + x ) % 8 );
    }
}


void draw_board( board* b, view player_view, SDL_Renderer* renderer )
{
    Uint8 red_channel, green_channel, blue_channel;
    bool current_cell_alive;
    SDL_Rect rectangle;
    rectangle.w = rectangle.h = player_view.cell_size;

    // Iterate over all cells and draw them to the renderer
    int screenHeight, screenWidth;
    SDL_GetRendererOutputSize( renderer, &screenWidth, &screenHeight );
    for ( int row = 0; row < player_view.height_in_cells; row++ )
    {
        for ( int column = 0; column < player_view.width_in_cells; column++ )
        {
            // Draw black squares for dead cells and white squares for living cells
            current_cell_alive = cell_state( column + player_view.camera_x, row + player_view.camera_y, b );

            red_channel = current_cell_alive ? LIVING_CELL_R : DEAD_CELL_R;
            green_channel = current_cell_alive ? LIVING_CELL_G : DEAD_CELL_G;
            blue_channel = current_cell_alive ? LIVING_CELL_B : DEAD_CELL_B;

            SDL_SetRenderDrawColor( renderer, red_channel, green_channel, blue_channel, 255 );
            rectangle.x = column*player_view.cell_size;
            rectangle.y = row*player_view.cell_size;
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


/**
* Resizes the view. Adds the zoom factor the cell_size.
*/
void resize_board_view( int zoom, view* player_view, board* world )
{
    if ( !( ( player_view->cell_size > 2 && zoom < 0 ) || ( player_view->cell_size < 30 && zoom > 0 ) ) )
    {
        return;
    }

    // Update the view
    int center_y = player_view->camera_y + player_view->height_in_cells / 2;
    int center_x = player_view->camera_x + player_view->width_in_cells / 2;
    player_view->cell_size += zoom;
    player_view->height_in_cells = player_view->window_height / player_view->cell_size;
    player_view->width_in_cells = player_view->window_width / player_view->cell_size;
    // Center the camera if the view is bigger then the board
    center_y = player_view->height_in_cells > world->rows ? world->rows / 2 : center_y;
    center_x = player_view->width_in_cells > world->columns ? world->columns / 2 : center_x;

    player_view->camera_x = center_x - ( player_view->width_in_cells / 2 );
    player_view->camera_y = center_y - ( player_view->height_in_cells / 2 );

    // Change the movement speed in cells (Don't allow it to be zero)
    player_view->movement_speed_in_cells = player_view->min_movement_speed_in_pixels / player_view->cell_size;
    player_view->movement_speed_in_cells = player_view->movement_speed_in_cells ? player_view->movement_speed_in_cells : 1;

    // Change the camera position in case it is out of bounds and the view is not bigger then the board
    if ( !( player_view->height_in_cells > world->rows || player_view->width_in_cells > world->columns ) )
    {
        player_view->camera_x = player_view->camera_x < 0 ? 0 : player_view->camera_x;
        player_view->camera_y = player_view->camera_y < 0 ? 0 : player_view->camera_y;
        player_view->camera_x = ( player_view->camera_x + player_view->width_in_cells ) > world->columns ? world->columns - player_view->width_in_cells : player_view->camera_x;
        player_view->camera_y = ( player_view->camera_y + player_view->height_in_cells ) > world->rows ? world->rows - player_view->height_in_cells : player_view->camera_y;
    }
}

void move_camera_by( int x, int y, view* player_view, board* game_board, SDL_Window* window )
{
    bool x_in_boundaries = player_view->camera_x >= 0 && ( player_view->camera_x <= game_board->columns - player_view->width_in_cells );
    bool y_in_boundaries = player_view->camera_y >= 0 && ( player_view->camera_y <= game_board->rows - player_view->height_in_cells );
    // Return if the camera is out of bounds
    if ( !( x_in_boundaries && y_in_boundaries ) )
    {
        return;
    }
    int windowHeight, windowWidth;
    SDL_GL_GetDrawableSize( window, &windowWidth, &windowHeight );
    player_view->camera_x += x;
    player_view->camera_x = player_view->camera_x < 0 ? 0 : player_view->camera_x;
    player_view->camera_x = !( player_view->camera_x + windowWidth / player_view->cell_size <= game_board->columns ) ? game_board->columns - windowWidth / player_view->cell_size : player_view->camera_x;

    player_view->camera_y += y;
    player_view->camera_y = player_view->camera_y < 0 ? 0 : player_view->camera_y;
    player_view->camera_y = !( player_view->camera_y + windowHeight / player_view->cell_size <= game_board->rows ) ? game_board->rows - windowHeight / player_view->cell_size : player_view->camera_y;
}
