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
*   Scroll wheel - Zoom
*
* TODO:
*     - Store the board as an array of 32 or 64 bit integers depending on the system's architecture
*     - Measure and improve the performance (There probably is a way to optimize the update_board function)
*     - Add support for the Run Length Encoded (RNE) file format
*     - Add a Makefile
*     - Test on linux
*     - Extract complicated boolean conditions in board.c into functions
*     - Remove everything regarding the RNE file format (YAGN - You ain't gonna need it)
*     - Don't access the board manually in init_board
*/
#include "board.h"
#define FILENAME_BUFFER_SIZE 256

typedef struct {
    bool wButtonDown;
    bool aButtonDown;
    bool sButtonDown;
    bool dButtonDown;
    bool kButtonDown;
    bool rButtonDown;
    bool upButtonDown;
    bool downButtonDown;
} buttons;

typedef struct
{
    bool leftButtonPressed;
    Uint32 last_cursor_x;
    Uint32 last_cursor_y;
} mouseState;

const Uint32 STARTING_POPULATION = 20000;


void update_button_states( buttons *bts, SDL_Event e, bool isKeydown )
{
    switch ( e.key.keysym.scancode )
    {
    case SDL_SCANCODE_W:
        bts->wButtonDown = isKeydown;
        break;
    case SDL_SCANCODE_A:
        bts->aButtonDown = isKeydown;
        break;
    case SDL_SCANCODE_S:
        bts->sButtonDown = isKeydown;
        break;
    case SDL_SCANCODE_D:
        bts->dButtonDown = isKeydown;
        break;
    case SDL_SCANCODE_UP:
        bts->upButtonDown = isKeydown;
        break;
    case SDL_SCANCODE_DOWN:
        bts->downButtonDown = isKeydown;
        break;
    case SDL_SCANCODE_K:
        bts->kButtonDown = isKeydown;
        break;
    case SDL_SCANCODE_R:
        bts->rButtonDown = isKeydown;
        break;
    }
}

int main(int argc, char** argv)
{
    // Setup SDL
    if ( SDL_Init( SDL_INIT_VIDEO ) )
    {
        fprintf( stderr, "error initializing SDL: %s\n", SDL_GetError( ) );
        goto SDLInitializationError;
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
        goto WindowCreationError;
    }
    // Create a renderer
    // NOTE: Handle multiple generations per frame if SDL_RENDERER_PRESENTVSYNC is set
    SDL_Renderer* renderer = SDL_CreateRenderer( window, -1, SDL_RENDERER_ACCELERATED|SDL_RENDERER_PRESENTVSYNC );
    if ( !renderer )
    {
        fprintf( stderr, "error creating renderer: %s\n", SDL_GetError( ) );
        goto RendererCreationError;
    }


    // Initialize the board and the players view on it
    view player_view;
    int window_height, window_width;
    SDL_GL_GetDrawableSize( window, &window_width, &window_height );
    player_view.cell_size = 10;
    player_view.height_in_cells = window_height / player_view.cell_size;
    player_view.width_in_cells = window_width / player_view.cell_size;
    player_view.window_height = window_height;
    player_view.window_width = window_width;
    player_view.movement_speed_in_cells = 3;
    player_view.min_movement_speed_in_pixels = player_view.movement_speed_in_cells * player_view.cell_size;
    const int BOARD_HEIGHT = player_view.window_height / 4;
    const int BOARD_WIDTH = player_view.window_width / 4;
    board* cell_board = init_board( BOARD_HEIGHT, BOARD_WIDTH, STARTING_POPULATION );
    // The starting position of the camera
    player_view.camera_x = ( BOARD_WIDTH - player_view.width_in_cells ) / 2;
    player_view.camera_y = ( BOARD_HEIGHT - player_view.height_in_cells ) / 2;

    int living_cells = 0;
    SDL_Event e;
    Uint32 last_update_time = 0;
    Uint32 ticks_per_lifecycle = 1000;

    uint8_t quit = FALSE;
    uint8_t paused = FALSE;

    // Draw the first state of the board
    draw_board( cell_board, &player_view, renderer );

    buttons keys = { FALSE };
    mouseState mouse = { FALSE, (Uint16)-1, (Uint16)-1 };

    // Game loop
    while ( !quit ) 
    {
        // Handle all input events
        while ( SDL_PollEvent( &e ) )
        {
            if ( e.type == SDL_QUIT )
            {
                quit = TRUE;
            }
            else if ( e.type == SDL_KEYDOWN || e.type == SDL_KEYUP )
            {
                bool isKeydown = e.type == SDL_KEYDOWN;
                update_button_states( &keys, e, isKeydown);
                switch ( e.key.keysym.scancode )
                {
                case SDL_SCANCODE_SPACE:
                    paused = isKeydown ^ paused;
                    break;
                case SDL_SCANCODE_Q:
                    quit = TRUE;
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
            else if ( e.wheel.y == 1 || e.wheel.y == -1)
            {
                // Zoom
                resize_board_view( -e.wheel.y, &player_view, cell_board );
            }
        }

        // React to presses of supported keys
        if ( keys.aButtonDown )
        {
            move_camera_by( -player_view.movement_speed_in_cells, 0, &player_view, cell_board, window );
        }
        if ( keys.wButtonDown )
        {
            move_camera_by( 0, -player_view.movement_speed_in_cells, &player_view, cell_board, window );
        }
        if ( keys.sButtonDown )
        {
            move_camera_by( 0, player_view.movement_speed_in_cells, &player_view, cell_board, window );
        }
        if ( keys.dButtonDown )
        {
            move_camera_by( player_view.movement_speed_in_cells, 0, &player_view, cell_board, window );
        }
        if ( keys.kButtonDown )
        {
            kill_all_cells( cell_board );
            keys.kButtonDown = FALSE;
        }
        if ( keys.rButtonDown )
        {
            free( cell_board );
            cell_board = init_board( BOARD_HEIGHT, BOARD_WIDTH, STARTING_POPULATION );
            keys.rButtonDown = FALSE;
        }
        if ( keys.upButtonDown )
        {
            if ( ticks_per_lifecycle > 100 )
            {
                ticks_per_lifecycle -= 100;
            }
        }
        if ( keys.downButtonDown )
        {
            if ( ticks_per_lifecycle < 10000 )
            {
                ticks_per_lifecycle += 100;
            }
        }
        if ( mouse.leftButtonPressed )
        {
            int cursor_x, cursor_y;
            SDL_GetGlobalMouseState( &cursor_x, &cursor_y );
            // change the cell state if the mouse points to a new cell
            if ( !( cursor_x / player_view.cell_size == mouse.last_cursor_x / player_view.cell_size && 
                    cursor_y / player_view.cell_size == mouse.last_cursor_y / player_view.cell_size ) )
            {
                Uint32 row = (Uint32) player_view.camera_y + cursor_y / player_view.cell_size;
                Uint32 column = (Uint32) player_view.camera_x + cursor_x / player_view.cell_size;
                toggle_cell_state( column, row, cell_board );
                mouse.last_cursor_x = cursor_x;
                mouse.last_cursor_y = cursor_y;
            }
        }
        // Only update the board at the interval stored in ticks_per_lifecycle
        if ( !( ( SDL_GetTicks( ) - last_update_time ) < ticks_per_lifecycle ) && !paused )
        {
            living_cells = update_board( cell_board );
            last_update_time = SDL_GetTicks( );
        }

        // Clear the entire screen and redraw it
        SDL_SetRenderDrawColor( renderer, 0, 0, 0, 255 );
        if ( SDL_RenderClear( renderer ) )
        {
            fprintf( stderr, "%s\n", SDL_GetError( ) );
        }
        draw_board( cell_board, &player_view, renderer );
    }

    // Clean up and exit
    free( cell_board );
    SDL_DestroyRenderer( renderer );
    RendererCreationError:
    SDL_DestroyWindow( window );
    SDL_QuitSubSystem( flags );
    WindowCreationError:
    SDL_Quit( );
    SDLInitializationError:
    return EXIT_SUCCESS;
}

