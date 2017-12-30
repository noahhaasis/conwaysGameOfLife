/**
* A implementation of Conway's game of life writen in
* the C programming language using the SDL library.
*
* Keybindings:
*	Q			 - Quit
*	K			 - Kill all cells
*   R            - Repopulate the board
*   W			 - Up
*   A			 - Left 
*	S			 - Down
*	D			 - Right
*	Space		 - Pause
*	Up-Arrow	 - Speed the simulation up
*	Down-Arrow	 - Slow the simulation down
*	Mouse button - Change the clicked cell's state
*/
#include "board.h"

unsigned char valid_camera_position( int x, int y, board* game_board, SDL_Window* window );


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
					if(valid_camera_position(camera_x, camera_y - CAMERA_MOVEMENT_SPEED, cell_board, window))
						camera_y -= CAMERA_MOVEMENT_SPEED;
					break;
				case SDL_SCANCODE_A:
					if ( valid_camera_position( camera_x - CAMERA_MOVEMENT_SPEED, camera_y, cell_board, window ) )
						camera_x -= CAMERA_MOVEMENT_SPEED;
					break;
				case SDL_SCANCODE_S:
					if ( valid_camera_position( camera_x, camera_y + CAMERA_MOVEMENT_SPEED, cell_board, window ) )
						camera_y += CAMERA_MOVEMENT_SPEED;
					break;
				case SDL_SCANCODE_D:
					if ( valid_camera_position( camera_x + CAMERA_MOVEMENT_SPEED, camera_y, cell_board, window ) )
						camera_x += CAMERA_MOVEMENT_SPEED;
					break;
				case SDL_SCANCODE_SPACE:
					paused = !paused;
					break;
				case SDL_SCANCODE_Q:
					quit = TRUE;
					break;
				case SDL_SCANCODE_UP:
					if ( refresh_rate > 100 )
						refresh_rate -= 100;
					break;
				case SDL_SCANCODE_DOWN:
					if ( refresh_rate < 10000 )
						refresh_rate += 100;
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

			else if ( e.type == SDL_MOUSEBUTTONUP )
			{
				int board_x = e.button.x / CELL_SIZE + camera_x;
				int board_y = e.button.y / CELL_SIZE + camera_y;
				change_cell_state( board_x, board_y, !cell_state( board_x, board_y, cell_board ), cell_board );
			}
		}

		if ( !(( SDL_GetTicks( ) - last_update_time ) < refresh_rate) && ! paused)
		{
			living_cells = update_board( cell_board );
			last_update_time = SDL_GetTicks( );
		}

		draw_board( cell_board, camera_x, camera_y, renderer );

		// TODO: Fix the resfreshrate to 60 franes per second
	}

	// Clean up and exit
	free( cell_board );
	SDL_DestroyRenderer( renderer );
	SDL_DestroyWindow( window );
	SDL_Quit( );
	return EXIT_SUCCESS;
}


unsigned char valid_camera_position( int x, int y, board* game_board, SDL_Window* window )
{
	int windowHeight, windowWidth;
	SDL_GL_GetDrawableSize( window, &windowWidth, &windowHeight );
	return ( x + windowWidth / CELL_SIZE <= game_board->columns ) && ( y + windowHeight / CELL_SIZE <= game_board->rows ) && x >= 0 && y >= 0;
}