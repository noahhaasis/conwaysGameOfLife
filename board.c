#include "board.h"

#define MIN_CELL_SIZE 2 
#define MAX_CELL_SIZE 30
#define ZOOM_OUT(n) (n < 0)
#define ZOOM_IN(n) (n > 0)

inline bool change_cell_state( int x, int y, bool state, board *b );

/* Returns min or max if num is less then or greater then either of them. */
int clamp( int min, int max, int num)
{
    if ( num < min ) { return min; }
    if ( num > max ) { return max; }
    return num;
}

/* Restricts the x and y position of the camera or player view to fit into the board */
void clamp_view_pos( view *v, board *b )
{
    int max_camera_x = b->columns - v->width_in_cells;
    v->camera_x = clamp( 0, max_camera_x, v->camera_x );

    int max_camera_y = b->rows - v->height_in_cells;
    v->camera_y = clamp( 0, max_camera_y, v->camera_y );
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
    // This is not constant! RAND_MAX may vary on different systems!
    if ( RAND_MAX < INT_MAX )
    {
        rand_val |= rand( ) << 16;
    }
    return rand_val;
}

/*
 * Returns a uniformly distributed value in the range 0 to n-1.
 * Thanks @fbartho for pointing me to uniformly distributed values.
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
    Uint32 rand_x, rand_y;
    int bitmask;
    for ( int i = 0; i < living_cell_count; i++ )
    {
        rand_x = random_uniform( columns );
        rand_y = random_uniform( rows );
        // Skip cells that are already alive
        if ( cell_state( rand_x, rand_y, b ) )
        {
            i--;
            continue;
        }
        change_cell_state( rand_x, rand_y, TRUE, b );
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
    for ( int y = 0; y < b->rows; y++ )
    {
        for ( int x = 0; x < b->columns; x++ )
        {
            if ( change_cell_state( x, y, updated_cell_state( x, y, b ), temp_board ) )
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

inline bool pos_in_board( int x, int y, board *b )
{
    return x >= 0 && y >= 0 && x < b->columns && y < b->rows;
}

/* Return the bitmask for the cell at location (x, y). */
inline int cell_bitmask( int x, int y, board *b )
{
    return ( int ) powf( 2, ( y*b->columns + x ) % 8 );
}

inline bool cell_state( int x, int y, board* b )
{
    if ( !pos_in_board( x, y, b ) )
    {
        return FALSE;
    }
    int bitmask = cell_bitmask( x, y, b);
    int cell_states = b->grid[ ( ( y*b->columns + x ) / 8 ) ];
    return ( cell_states & bitmask ) != 0;
}

inline int living_neighbors( int x, int y, board *b )
{
    return cell_state( x - 1, y - 1, b ) +
           cell_state( x    , y - 1, b ) +
           cell_state( x + 1, y - 1, b ) +
           cell_state( x - 1, y    , b ) +
           cell_state( x + 1, y    , b ) +
           cell_state( x - 1, y + 1, b ) +
           cell_state( x    , y + 1, b ) +
           cell_state( x + 1, y + 1, b );
}

void toggle_cell_state( int x, int y, board *b )
{
    change_cell_state( x, y, !cell_state( x, y, b ), b );
}

inline bool updated_cell_state( int x, int y, board* b )
{
    // Count the living neighbors
    int living_neighbor_cells =  living_neighbors( x, y, b );

    // Return the new state of the cell at position board[x][y]
    return ( living_neighbor_cells == 3 || ( cell_state( x, y, b ) && ( living_neighbor_cells == 2 ) ) );
}


inline bool change_cell_state( int x, int y, bool state, board *b )
{
    if ( !pos_in_board( x, y, b ) )
    {
        return FALSE;
    }
    int bitmask = cell_bitmask( x, y, b );
    if ( state )
    {
        return b->grid[ ( y*b->columns + x ) / 8 ] |= bitmask;
    }
    return b->grid[ ( y*b->columns + x ) / 8 ] &= ~bitmask;
}


void draw_board( board* b, view *player_view, SDL_Renderer* renderer )
{
    Uint8 red_channel, green_channel, blue_channel;
    bool current_cell_alive;
    SDL_Rect rectangle;
    rectangle.w = rectangle.h = player_view->cell_size;

    // Iterate over all cells in the view and draw them to the renderer
    int screenHeight, screenWidth;
    SDL_GetRendererOutputSize( renderer, &screenWidth, &screenHeight );
    for ( int row = 0; row < player_view->height_in_cells; row++ )
    {
        for ( int column = 0; column < player_view->width_in_cells; column++ )
        {
            current_cell_alive = cell_state( column + player_view->camera_x, row + player_view->camera_y, b );

            red_channel   = current_cell_alive ? LIVING_CELL_R : DEAD_CELL_R;
            green_channel = current_cell_alive ? LIVING_CELL_G : DEAD_CELL_G;
            blue_channel  = current_cell_alive ? LIVING_CELL_B : DEAD_CELL_B;

            SDL_SetRenderDrawColor( renderer, red_channel, green_channel, blue_channel, 255 );
            rectangle.x = column*player_view->cell_size;
            rectangle.y = row*player_view->cell_size;
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

inline bool camera_in_bounds( view *v, board* b )
{
    bool x_in_boundaries = v->camera_x >= 0 && ( v->camera_x <= b->columns - v->width_in_cells );
    bool y_in_boundaries = v->camera_y >= 0 && ( v->camera_y <= b->rows - v->height_in_cells );
    return x_in_boundaries && y_in_boundaries;
}

inline void get_view_center( view *v, int *x, int *y )
{
    *x = v->camera_x + v->width_in_cells / 2;
    *y = v->camera_y + v->height_in_cells / 2;
}

/* Sets the pos of the view so that the view's center is at the given position */
inline void set_view_pos_to_center( int x, int y, view *v )
{
    v->camera_x = x - ( v->width_in_cells / 2 );
    v->camera_y = y - ( v->height_in_cells / 2 );
}

/**
* Resizes the view. Adds the zoom factor the cell_size.
* The center keeps the same.
*/
void resize_board_view( int zoom, view* player_view, board* world )
{
    if ( !( player_view->cell_size > MIN_CELL_SIZE && ZOOM_OUT( zoom ) ||
          ( player_view->cell_size < MAX_CELL_SIZE && ZOOM_IN( zoom ) ) ) )
    {
        return;
    }

    // Update the view
    int old_center_x, old_center_y;
    get_view_center( player_view, &old_center_x, &old_center_y );

    player_view->cell_size += zoom;
    player_view->height_in_cells = player_view->window_height / player_view->cell_size;
    player_view->width_in_cells = player_view->window_width / player_view->cell_size;
    // Center the camera if the view is bigger then the board
    int new_center_y = player_view->height_in_cells > world->rows ? world->rows / 2 : old_center_y;
    int new_center_x = player_view->width_in_cells > world->columns ? world->columns / 2 : old_center_x;

    set_view_pos_to_center( new_center_x, new_center_y, player_view );

    // Change the movement speed in cells (Don't allow it to be zero)
    player_view->movement_speed_in_cells = player_view->min_movement_speed_in_pixels / player_view->cell_size;
    player_view->movement_speed_in_cells = player_view->movement_speed_in_cells ? player_view->movement_speed_in_cells : 1;

    // Change the camera position to fit into the board
    // clamp_view_pos( player_view, world );
    if ( !( player_view->height_in_cells > world->rows || player_view->width_in_cells > world->columns ) )
    {
        clamp_view_pos( player_view, world );
    }
}

void move_camera_by( int x, int y, view* player_view, board* game_board, SDL_Window* window )
{
    if ( camera_in_bounds( player_view, game_board ) )
    {
        player_view->camera_x += x;
        player_view->camera_y += y;
        clamp_view_pos( player_view, game_board );
    }
}
