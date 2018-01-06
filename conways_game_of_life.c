/**
* A implementation of Conway's game of life writen in
* the C programming language using the SDL library.
*
* Keybindings:
*   Q            - Quit
*   K            - Kill all cells
*   R            - Repopulate the board
*   W            - Up
*   A            - Left 
*   S            - Down
*   D            - Right
*   Space        - Pause
*   Up-Arrow     - Speed the simulation up
*   Down-Arrow   - Slow the simulation down
*   Mouse button - Change the clicked cell's state
*
* TODO:
*     - Fix the framerate to 60 fps
*     - Zoom with the scroll wheel
*/
#include "board.h"

typedef struct {
	Uint8 wButtonDown;
	Uint8 aButtonDown;
	Uint8 sButtonDown;
	Uint8 dButtonDown;
	Uint8 upButtonDown;
	Uint8 downButtonDown;
} buttons;

typedef struct
{
    Uint8 leftButtonPressed;
    Uint16 last_cursor_x;
    Uint16 last_cursor_y;
} mouseState;

bool valid_camera_position( int x, int y, board* game_board, SDL_Window* window );


int main(int argc, char** argv)
{
	// Setup SDL
	if ( SDL_Init( SDL_INIT_VIDEO ) )
	{
		fprintf( stderr, "error initializing SDL: %s\n", SDL_GetError( ) );
		return 1;
	}

	// Create a window
	SDL_WindowFlags flags =
		SDL_WINDOW_SHOWN | SDL_WINDOW_MOUSE_FOCUS | SDL_WINDOW_ALLOW_HIGHDPI | SDL_WINDOW_FULLSCREEN;
	SDL_Window* window = SDL_CreateWindow(
		"Conway's game of life",
		SDL_WINDOWPOS_CENTERED,
		SDL_WINDOWPOS_CENTERED,
		0,
		0,
		flags );
	if ( !window )
	{
		fprintf( stderr, "error creating window: %s\n", SDL_GetError( ) );
		SDL_Quit( );
		return 1;
	}
	// Create a renderer
	SDL_Renderer* renderer = SDL_CreateRenderer( window, -1, SDL_RENDERER_ACCELERATED );
	if ( !renderer )
	{
		fprintf( stderr, "error creating renderer: %s\n", SDL_GetError( ) );
		SDL_DestroyWindow( window );
		SDL_Quit( );
		return 1;
	}

	// Initialize the board
	int window_height, window_width;
	SDL_GL_GetDrawableSize( window, &window_width, &window_height );
	const int BOARD_HEIGHT = (window_height / CELL_SIZE)*2;
	const int BOARD_WIDTH = (window_width / CELL_SIZE)*2;
	board* cell_board = init_board( BOARD_HEIGHT, BOARD_WIDTH, STARTING_POPULATION );

	int living_cells = 0;
	SDL_Event e;
	Uint32 last_update_time = 1000;
	unsigned int refresh_rate = 1000;

	uint8_t quit = FALSE;
	uint8_t paused = FALSE;

	// Initialize the camera
	int camera_x = ( BOARD_WIDTH - window_width/CELL_SIZE ) / 2;
	int camera_y = ( BOARD_HEIGHT - window_height/CELL_SIZE ) / 2;

	// Draw the first state of the board
	draw_board( cell_board, camera_x, camera_y, renderer );

	buttons keys = { FALSE };
    mouseState mouse = { FALSE, (Uint16)-1, (Uint16)-1 };

	// Game loop
	while ( !quit ) 
	{
		// Handle all input events
		while ( SDL_PollEvent( &e ) )
		{
			if ( e.type == SDL_QUIT )
				quit = TRUE;
			else if ( e.type == SDL_KEYDOWN )
			{
				switch ( e.key.keysym.scancode )
				{
				case SDL_SCANCODE_W:
					keys.wButtonDown = TRUE;
					break;
				case SDL_SCANCODE_A:
					keys.aButtonDown = TRUE;
					break;
				case SDL_SCANCODE_S:
					keys.sButtonDown = TRUE;
					break;
				case SDL_SCANCODE_D:
					keys.dButtonDown = TRUE;
					break;
				case SDL_SCANCODE_SPACE:
					paused = !paused;
					break;
				case SDL_SCANCODE_Q:
					quit = TRUE;
					break;
				case SDL_SCANCODE_UP:
					keys.upButtonDown = TRUE;
					break;
				case SDL_SCANCODE_DOWN:
					keys.downButtonDown = TRUE;
					break;
				case SDL_SCANCODE_K:
					kill_all_cells( cell_board );
					break;
				case SDL_SCANCODE_R:
				{
					free( cell_board );
					cell_board = init_board( BOARD_HEIGHT, BOARD_WIDTH, STARTING_POPULATION );
				} break;
				default:
					break;
				}

			}
			else if ( e.type == SDL_KEYUP )
			{
				switch ( e.key.keysym.scancode )
				{
				case SDL_SCANCODE_W:
					keys.wButtonDown = FALSE;
					break;
				case SDL_SCANCODE_A:
					keys.aButtonDown = FALSE;
					break;
				case SDL_SCANCODE_S:
					keys.sButtonDown = FALSE;
					break;
				case SDL_SCANCODE_D:
					keys.dButtonDown = FALSE;
					break;
				case SDL_SCANCODE_UP:
					keys.upButtonDown = FALSE;
					break;
				case SDL_SCANCODE_DOWN:
					keys.downButtonDown = FALSE;
					break;
				}
			}
			else if ( e.type == SDL_MOUSEBUTTONUP )
			{
                mouse.leftButtonPressed = FALSE;
                mouse.last_cursor_x = mouse.last_cursor_y = (Uint16)-1;
			}
            else if ( e.type == SDL_MOUSEBUTTONDOWN )
            {
                mouse.leftButtonPressed = TRUE;
            }
		}

		// React to the W, A, S, D, up and down keys
		if ( keys.aButtonDown )
		{
			if ( valid_camera_position( camera_x - CAMERA_MOVEMENT_SPEED, camera_y, cell_board, window ) )
				camera_x -= CAMERA_MOVEMENT_SPEED;
		}
		if ( keys.wButtonDown )
		{
			if ( valid_camera_position( camera_x, camera_y - CAMERA_MOVEMENT_SPEED, cell_board, window ) )
				camera_y -= CAMERA_MOVEMENT_SPEED;
		}
		if ( keys.sButtonDown )
		{
			if ( valid_camera_position( camera_x, camera_y + CAMERA_MOVEMENT_SPEED, cell_board, window ) )
				camera_y += CAMERA_MOVEMENT_SPEED;
		}
		if ( keys.dButtonDown )
		{
			if ( valid_camera_position( camera_x + CAMERA_MOVEMENT_SPEED, camera_y, cell_board, window ) )
				camera_x += CAMERA_MOVEMENT_SPEED;
		}
		if ( keys.upButtonDown )
		{
			if ( refresh_rate > 100 )
				refresh_rate -= 100;

		}
		if ( keys.downButtonDown )
		{
			if ( refresh_rate < 10000 )
				refresh_rate += 100;
		}
        if ( mouse.leftButtonPressed )
        {
            Uint16 cursor_x, cursor_y;
            SDL_GetGlobalMouseState( (int *)&cursor_x, (int *)&cursor_y );
            // change the cell state if the mouse position changed
            if ( !( cursor_x / CELL_SIZE == mouse.last_cursor_x / CELL_SIZE && cursor_y / CELL_SIZE == mouse.last_cursor_y / CELL_SIZE ) )
            {
                Uint16 row = camera_y + cursor_y / CELL_SIZE;
                Uint16 column = camera_x + cursor_x / CELL_SIZE;
                change_cell_state( column, row, !cell_state(column, row, cell_board), cell_board );
                mouse.last_cursor_x = cursor_x;
                mouse.last_cursor_y = cursor_y;
            }
        }


		if ( !(( SDL_GetTicks( ) - last_update_time ) < refresh_rate) && ! paused)
		{
			living_cells = update_board( cell_board );
			last_update_time = SDL_GetTicks( );
		}

		draw_board( cell_board, camera_x, camera_y, renderer );
	}

	// Clean up and exit
	free( cell_board );
	SDL_DestroyRenderer( renderer );
	SDL_DestroyWindow( window );
	SDL_Quit( );
	return EXIT_SUCCESS;
}


bool valid_camera_position( int x, int y, board* game_board, SDL_Window* window )
{
	int windowHeight, windowWidth;
	SDL_GL_GetDrawableSize( window, &windowWidth, &windowHeight );
	return ( x + windowWidth / CELL_SIZE <= game_board->columns ) && ( y + windowHeight / CELL_SIZE <= game_board->rows ) && x >= 0 && y >= 0;
}