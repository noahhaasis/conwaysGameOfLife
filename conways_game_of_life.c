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
	int window_size = atoi( argv[ 1 ] ) * CELL_SIZE;
	SDL_WindowFlags flags = 
		SDL_WINDOW_SHOWN | SDL_WINDOW_MOUSE_FOCUS | SDL_WINDOW_ALLOW_HIGHDPI;
	SDL_Window* window = SDL_CreateWindow( 
		"Conway's game of life", 
		SDL_WINDOWPOS_CENTERED, 
		SDL_WINDOWPOS_CENTERED, 
		window_size, 
		window_size, 
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
	board* cell_board = init_board( atoi( argv[ 1 ] ), atoi( argv[ 2 ] ) );

	int living_cells = 0;
	SDL_Event e;
	int quit = FALSE;
	Uint32 last_update_time = 1000;

	// Game loop
	while ( !quit ) {
		// Handle all input events
		while ( SDL_PollEvent( &e ) != 0)
		{
			if ( e.type == SDL_QUIT )
				quit = TRUE;
		}
		
		if ( (SDL_GetTicks( ) - last_update_time) < REFRESH_RATE )
			continue;

		draw_board( cell_board , renderer);
		living_cells = update_board( cell_board );
		last_update_time = SDL_GetTicks( );
		
		// If all cells are dead print the board and exit the game loop
		if ( living_cells <= 0 )
		{
			SDL_Delay( 30 );
			draw_board( cell_board , renderer);
			break;
		}
		SDL_Delay( 30 );
	}

	// Clean up and exit
	free( cell_board );
	SDL_DestroyRenderer( renderer );
	SDL_DestroyWindow( window );
	SDL_Quit( );
	return EXIT_SUCCESS;
}
