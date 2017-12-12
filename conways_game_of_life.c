#include "board.h"


void print_board( board* b ); // Print the board for the sake of debugging


int main( int argc, char* argv[ ] )
{
	// Check for proper input
	if ( argc != 3 || !isdigit( argv[ 1 ][ 0 ] ) || !isdigit( argv[ 2 ][ 0 ] )
		|| ( atoi( argv[ 1 ] ) * atoi( argv[ 1 ] ) ) < atoi( argv[ 2 ] ) )
	{
		printf( "Usage: ./conways_game_of_life <board size> <living cells>\n" );
		return 1;
	}

	// Setup SDL
	if ( SDL_Init( SDL_INIT_VIDEO ) != 0 )
	{
		fprintf( stderr, "error initializing SDL: %s", SDL_GetError( ) );
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
	if ( window == NULL)
	{
		fprintf( stderr, "error creating window: %s", SDL_GetError( ) );
		SDL_Quit( );
		return 1;
	}
	// Create a renderer
	SDL_Renderer* renderer = SDL_CreateRenderer( window, -1, SDL_RENDERER_ACCELERATED );

	// Initialize the board
	int window_height, window_width;
	SDL_GetWindowSize( window, &window_width, &window_height );
	int board_height = window_height / CELL_SIZE;
	int board_width = window_width / CELL_SIZE;
	board* cell_board = init_board( board_height, board_width, atoi( argv[ 2 ] ) );

	int living_cells = 0;
	SDL_Event e;
	Uint32 last_update_time = 1000;

	uint8_t quit = FALSE;
	uint8_t paused = FALSE;

	// Draw the first state of the board
	draw_board( cell_board, renderer );

	// Game loop
	while ( !quit ) {
		// Handle all input events
		while ( SDL_PollEvent( &e ) != 0)
		{
			if ( e.type == SDL_QUIT || e.type == SDL_KEYDOWN && e.key.keysym.scancode == SDL_SCANCODE_Q )
				quit = TRUE;
			else if ( e.type == SDL_KEYDOWN && e.key.keysym.scancode == SDL_SCANCODE_SPACE )
				paused = !paused;
			else if ( e.type == SDL_MOUSEBUTTONUP )
			{
				int board_x = e.button.x / CELL_SIZE;
				int board_y = e.button.y / CELL_SIZE;
				change_cell_state( board_x, board_y, !cell_state( board_x, board_y, cell_board ), cell_board );
			}
		}
		
		if ( (SDL_GetTicks( ) - last_update_time) < REFRESH_RATE )
			continue;

		if ( !paused )
		{
			living_cells = update_board( cell_board );
			last_update_time = SDL_GetTicks( );
		}

		draw_board( cell_board, renderer );
		
		// If all cells are dead print the board and exit the game loop
		if ( living_cells <= 0 )
		{
			SDL_Delay( REFRESH_RATE );
			draw_board( cell_board , renderer);
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
