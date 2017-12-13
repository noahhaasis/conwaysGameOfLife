/**
* A implementation of Conway's game of life writen in
* the C programming language using the SDL library.
*
* Keybindings:
*	Q			 - Quit
*	K			 - Kill all cells
*   R            - Repopulate the board
*	Space		 - Pause
*	Up-Arrow	 - Speed the simulation up
*	Down-Arrow	 - Slow the simulation down
*	Mouse button - Change the clicked cell's state
*/
#include "board.h"


int main( int argc, char* argv[ ] )
{
	// Check for proper input
	if ( argc != 2 || !isdigit( argv[ 1 ][ 0 ] ))
	{
		printf( "Usage: ./conways_game_of_life <living cells>\n" );
		return 1;
	}

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
	SDL_GetWindowSize( window, &window_width, &window_height );
	board* cell_board = init_board( window_height / CELL_SIZE, window_width / CELL_SIZE, atoi( argv[ 1 ] ) );

	int living_cells = 0;
	SDL_Event e;
	Uint32 last_update_time = 1000;
	unsigned int refresh_rate = 1000;

	uint8_t quit = FALSE;
	uint8_t paused = FALSE;

	// Draw the first state of the board
	draw_board( cell_board, renderer );

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
					int rows = cell_board->rows;
					int columns = cell_board->columns;
					free( cell_board );
					cell_board = init_board(rows, columns, atoi(argv[1]) );
				} break;
				default:
					break;
				}
			}

			else if ( e.type == SDL_MOUSEBUTTONUP )
			{
				int board_x = e.button.x / CELL_SIZE;
				int board_y = e.button.y / CELL_SIZE;
				change_cell_state( board_x, board_y, !cell_state( board_x, board_y, cell_board ), cell_board );
			}
		}

		if ( !(( SDL_GetTicks( ) - last_update_time ) < refresh_rate) && ! paused)
		{
			living_cells = update_board( cell_board );
			last_update_time = SDL_GetTicks( );
		}

		draw_board( cell_board, renderer );

		// If all cells are dead print the board and exit the game loop
		if ( living_cells <= 0 )
		{
			SDL_Delay( refresh_rate );
			draw_board( cell_board, renderer );
			break;
		}
	}

	// Clean up and exit
	free( cell_board );
	SDL_DestroyRenderer( renderer );
	SDL_DestroyWindow( window );
	SDL_Quit( );
	return EXIT_SUCCESS;
}
