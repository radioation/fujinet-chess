
#include <atari.h>
#include <conio.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <peekpoke.h>

#include <fujinet-network.h>


// Enum to represent piece types (using offsets for lookup into image)
/*
typedef enum {
    EMPTY = 0,
    KING = 3,
    QUEEN = 6,
    ROOK = 9,
    BISHOP = 12,
    KNIGHT = 15,
    PAWN = 18
} PIECE_TYPE;
typedef enum {
    NO_PLAYER = 0,
    PLAYER_ONE = 1,
    PLAYER_TWO = 2
} PLAYER;
*/

#define  EMPTY  0
#define  KING  1
#define  QUEEN  2
#define  ROOK  3
#define  BISHOP  4
#define  KNIGHT  15
#define  PAWN  18

#define NO_PLAYER  0
#define PLAYER_ONE  1
#define PLAYER_TWO  2

#define BOARD_START 16
#define BOARD_STRIDE 32
#define BOARD_COL_SPACING 2


// Structure to represent a chess piece
typedef struct {
    uint8_t type;   // Type of the piece
    uint8_t player;     // which player
} CHESS_PIECE;

// structure for loggin two checkers
typedef struct {
    uint8_t count;
    uint8_t x[2];
    uint8_t y[2];
    uint8_t is_biroqu[2]; // is a bishop, rook, or queen
} CHECKERS;

uint8_t current_player =NO_PLAYER;
uint8_t who_am_i = NO_PLAYER;
uint8_t inputWait = 0;

void init_dlist(void);

extern uint8_t screen_memory[];  
extern uint8_t message_memory[];  
extern uint8_t pmg_memory[380];
extern uint8_t font[];
extern uint8_t pmg_data[1024];
//
//static uint8_t board_data[BOARD_SIZE];

static void setup_charset() {
  uint16_t i = 0;
  ///////////////////////////////////////////////////
  // SETUP CHARSET //////////////////////////////////
  uint16_t addr = (uint16_t)( font);
  OS.chbas = addr >> 8;
  memset( screen_memory, 0, 480 );
  // top row
  memset( screen_memory      , 29 + 64 + 128, 16 );
  // bottom row. 
  memset( screen_memory + 272, 29 + 64 + 128, 16 );


  // piece test 
  screen_memory [ 16 ] =  10;  // black is actual offset
  screen_memory [ 32 ] =  11;
  screen_memory [ 48 ] =  0;  
  screen_memory [ 64 ] =  12;
 
  screen_memory [ 240 ] =  10+128; // white is + 128
  screen_memory [ 256 ] =  11+128;
  screen_memory [ 208 ] =  0;  
  screen_memory [ 224] =  12+128;


  // fake cursor
  screen_memory [ 112 + 2 ] =  28 + 64;  //  cursor color is +64
  screen_memory [ 128 + 2] =  29 + 64;
 
  //for( i=0; i < 64; ++i ) { 
  //  screen_memory[20+i] = i;
  //}
  sprintf(message_memory, "fujinet"); 
}

static void setup_pm_graphics() {
  ANTIC.pmbase = (uint16_t) pmg_memory  >> 8;
  memset( pmg_memory, 0, 1024 );
  memcpy( pmg_memory, pmg_data, sizeof( pmg_data ) );
  GTIA_WRITE.gractl = 0x1B; 

  OS.sdmctl = 0x2D;

  // Chess board background
  OS.pcolr0=
    OS.pcolr1=
    OS.pcolr2=
    OS.pcolr3=0x28;

  // Playfield colors
  OS.color0 = 0x00;
  OS.color1 = 0x7A;
  OS.color2 = 0x0e;
  OS.color3 = 0x34;
  OS.color4 = 0xC6;

  // Set P/M's behind playfield (pieces)
  OS.gprior = 0x14;

  // Position players and missiles (board bits)
  GTIA_WRITE.hposp0=0x3C;
  GTIA_WRITE.hposp1=0x5C;
  GTIA_WRITE.hposp2=0x7C;
  GTIA_WRITE.hposp3=0x9C;
  GTIA_WRITE.hposm0 = 0xBC;
  GTIA_WRITE.hposm1 =
    GTIA_WRITE.hposm2 =
    GTIA_WRITE.hposm3 = 0x38;
  GTIA_WRITE.sizep0 =
    GTIA_WRITE.sizep1 =
    GTIA_WRITE.sizep2 =
    GTIA_WRITE.sizep3 = 0x03;
  GTIA_WRITE.sizem = 0xFF; // All four missiles size 3



}



void cursor_init( CURSOR *cursor, Sprite *sprite, Sprite *selected_sprite ) {
    cursor->col = 4;  // board position
    cursor->row = 4;
    cursor->pos_x = cursor->col * cursorStep + cursorColStart;
    cursor->pos_y = cursor->row * cursorStep + cursorRowStart;
    cursor->sprite =  sprite;

    cursor->sel_col = -1;  // not on board
    cursor->sel_row = -1;
    cursor->sel_pos_x = -32;
    cursor->sel_pos_y = -32;
    cursor->selected_spr = selected_sprite;
    SPR_setAnim( cursor->selected_spr, 1 );
    SPR_setVisibility( cursor->selected_spr, HIDDEN );
}

bool cursor_move( CURSOR *cursor, u16 joypad ) {
    bool didMove = FALSE;
    if( joypad & BUTTON_LEFT ) {
        cursor->col--;
        if( cursor->col < 0 ) {
            cursor->col = 7;
        }
        cursor->pos_x = cursor->col * cursorStep + cursorColStart;
        didMove = TRUE;
    } 
    if( joypad & BUTTON_RIGHT ) {
        cursor->col++;
        if( cursor->col > 7 ) {
            cursor->col = 0;
        }
        cursor->pos_x = cursor->col * cursorStep + cursorColStart;
        didMove = TRUE;
    } 
    if( joypad & BUTTON_UP ) {
        cursor->row--;
        if( cursor->row < 0 ) {
            cursor->row = 7;
        }
        cursor->pos_y = cursor->row * cursorStep + cursorRowStart;
        didMove = TRUE;
    }
    if( joypad & BUTTON_DOWN ) {
        cursor->row++;
        if( cursor->row > 7 ) {
            cursor->row = 0;
        }
        cursor->pos_y = cursor->row * cursorStep + cursorRowStart;
        didMove = TRUE;
    }
    return didMove;
}

void cursor_update_from_pos( CURSOR *cursor, s8 col, s8 row, s8 sel_col, s8 sel_row ) {
    cursor->col = col;
    cursor->pos_x = cursor->col * cursorStep + cursorColStart;
    cursor->row = row;
    cursor->pos_y = cursor->row * cursorStep + cursorRowStart;

    cursor->sel_col = sel_col;
    cursor->sel_pos_x = cursor->sel_col * cursorStep + cursorColStart;
    cursor->sel_row = sel_row;
    cursor->sel_pos_y = cursor->sel_row * cursorStep + cursorRowStart;
    if( cursor->sel_col >= 0 ) {
        SPR_setVisibility( cursor->selected_spr, VISIBLE );
    } else {
        SPR_setVisibility( cursor->selected_spr, HIDDEN );
    }
}

void cursor_clear_selected( CURSOR* cursor ) {
    char message[40];
    cursor->sel_col = -1;
    cursor->sel_row = -1;
    cursor->sel_pos_x = -32;
    cursor->sel_pos_y = -32;
    SPR_setVisibility( cursor->selected_spr, HIDDEN );
    //char message[40];
    strclr(message);
    sprintf( message, "X: %d y: %d sx: %d sy %d    ", cursor->col, cursor->row, cursor->sel_col, cursor->sel_row);
}



bool cursor_action( CURSOR* cursor, CHESS_PIECE brd[8][8], u8 player ) {
    if( cursor->sel_col < 0 ) {
        // no piece selected yet, check if player owns the current piece.
        if( brd[(u8)cursor->col][(u8)cursor->row].player == player ) { 
            cursor->sel_col = cursor->col;
            cursor->sel_row = cursor->row;
            cursor->sel_pos_x = cursor->sel_col * cursorStep + cursorColStart;
            cursor->sel_pos_y = cursor->sel_row * cursorStep + cursorRowStart;
            SPR_setVisibility( cursor->selected_spr, VISIBLE );
        }
    } else {
        //char message[40];
        //strclr(message);
        //sprintf( message, "X: %d y: %d sx: %d sy %d    ", cursor->col, cursor->row, cursor->sel_col, cursor->sel_row);
        //VDP_drawText( message, 0, 1 );
        // return true if destination is clear or a different player, BUT DON"T UPDATE BOARD 
        return ( brd[(u8)cursor->col][(u8)cursor->row].player != player );

    }
    return false;
}




static void handle_my_turn() {
  // read joystick to move cursor
  if( inputWait == 0 ) {
    /* SEGA!
                u16 joypad  = JOY_readJoypad( JOY_1 );
                // update local position
                if( cursor_move( &cursor, joypad ) == TRUE ) {
                    XGM_startPlayPCM(SND_MOVE,1,SOUND_PCM_CH2);
                    inputWait = INPUT_WAIT_COUNT;
                }
                // if A, 
                if( joypad & BUTTON_A ) {
                    bool trySend =  cursor_action( &cursor, board, currentPlayer );
                    inputWait = INPUT_WAIT_COUNT;
                    if( trySend ) {
                        // send possible move
                        if( send_move( &cursor, 1 ) ) {
                           currentPlayer = currentPlayer == PLAYER_ONE ? PLAYER_TWO : PLAYER_ONE; 
                        }
                    }
                } else if( joypad & BUTTON_C ) {
                    cursor_clear_selected( &cursor );
                    inputWait = INPUT_WAIT_COUNT;
                } 
     */

  } else {
      if ( input_wait > 0 ) {
          --inputWait;
      }
  }

 

}

int main(void)
{

  //////////////////////////////////////////////////////////////
  // setup screen and palettes
  setup_charset();


  //////////////////////////////////////////////////////////////
  // setup sprites
  setup_pm_graphics();

  init_dlist();
  //////////////////////////////////////////////////////////////
  // Networking setup



  // screen_print_at(0, 1, "PLAYER:");

  //    setup_network_game();


  for (;;) {
    if (current_player == who_am_i) {
      //            handle_my_turn();
    } else {
      //           poll_other_player();
    }
  }

  return 0;
}

