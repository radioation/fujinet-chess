
#include <atari.h>
#include <conio.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <peekpoke.h>
#include <stdbool.h>

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
#define  KNIGHT  5
#define  PAWN   6

#define NO_PLAYER  0
#define PLAYER_ONE  1
#define PLAYER_TWO  2

#define BOARD_NUM_COLS 8
#define BOARD_START 16
#define BOARD_STRIDE 32
#define BOARD_COL_SPACING 2

#define INPUT_WAIT_COUNT 10 

#define BUTTON_UP 14
#define BUTTON_DOWN 13
#define BUTTON_LEFT 11
#define BUTTON_RIGHT 7
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


typedef struct 
{
  // Sprite *sprite;
  int8_t col;     // board col
  int8_t row;     // board row
  //int8_t txt_x;   // sega used screen position because sprite. using text here.
  //int8_t txt_y;
  int8_t sel_col; // selected board column
  int8_t sel_row; // selected board row
  //int16_t sel_txt_x;
  //int16_t sel_txt_y;
  //Sprite *selected_spr;  
} CURSOR;

static const int8_t cursorStep = 2;
static const int8_t cursorColStart = 64; 
static const int8_t cursorRowStart = 16;

CURSOR chess_cursor;

static uint8_t current_player =NO_PLAYER;
static uint8_t who_am_i = NO_PLAYER;
static uint8_t inputWait = 0;

void init_dlist(void);

extern uint8_t screen_memory[];  
extern uint8_t message_memory[];  
extern uint8_t pmg_memory[380];
extern uint8_t font[];
extern uint8_t pmg_data[1024];
//
// static uint8_t board_data[BOARD_NUM_COLS];
CHESS_PIECE board[BOARD_NUM_COLS][BOARD_NUM_COLS]; // X, Y
int piecesTileIndex = -1;
const int8_t boardStartCol = 8;
const int8_t boardStartRow = 2;
const int8_t boardStep = 3;

void clear_board() {
  uint8_t x, y;
  for( x=0; x < BOARD_NUM_COLS; x++ ) {
    for(  y=0; y < BOARD_NUM_COLS; y++ ){
      board[x][y].type = EMPTY;
      board[x][y].player = NO_PLAYER;
    }
  }
}


void setup_pieces() {
  uint8_t i;
  // clear the board
  memset(board, 0, sizeof(CHESS_PIECE) * 8 * 8); // Set all to empty

  // set  pieces up
  board[0][7].type = ROOK;
  board[0][7].player = PLAYER_ONE;
  board[1][7].type = KNIGHT;
  board[1][7].player =  PLAYER_ONE; 
  board[2][7].type = BISHOP;
  board[2][7].player =  PLAYER_ONE; 
  board[3][7].type = QUEEN;
  board[3][7].player =  PLAYER_ONE;  
  board[4][7].type = KING;
  board[4][7].player =  PLAYER_ONE;
  board[5][7].type = BISHOP;
  board[5][7].player =  PLAYER_ONE;
  board[6][7].type = KNIGHT;
  board[6][7].player =  PLAYER_ONE;
  board[7][7].type = ROOK;
  board[7][7].player =  PLAYER_ONE;


  for ( i = 0; i < 8; i++) {
    board[i][6].type = PAWN;
    board[i][6].player = PLAYER_ONE;
    board[i][1].type = PAWN;
    board[i][1].player = PLAYER_TWO;
  }


  board[0][0].type = ROOK;
  board[0][0].player = PLAYER_TWO;
  board[1][0].type = KNIGHT;
  board[1][0].player =  PLAYER_TWO; 
  board[2][0].type = BISHOP;
  board[2][0].player =  PLAYER_TWO; 
  board[3][0].type = QUEEN;
  board[3][0].player =  PLAYER_TWO;  
  board[4][0].type = KING;
  board[4][0].player =  PLAYER_TWO;
  board[5][0].type = BISHOP;
  board[5][0].player =  PLAYER_TWO;
  board[6][0].type = KNIGHT;
  board[6][0].player =  PLAYER_TWO;
  board[7][0].type = ROOK;
  board[7][0].player =  PLAYER_TWO;
}


void update_square(uint8_t col, uint8_t row){
  uint16_t pos = BOARD_START + (row * 32) + (col <<1);
  if( board[col][row].player > 0 ) {
    uint8_t color_bits = 128;
    if( board[col][row].player == PLAYER_TWO ) {
      color_bits = 0;
    }
    switch (board[col][row].type ) {
      case KING:
        screen_memory[ pos ] = 1 | color_bits;
        screen_memory[ pos+16 ] = 2 | color_bits;
        break;
      case QUEEN:
        screen_memory[ pos ] = 3 | color_bits;
        screen_memory[ pos+16 ] = 4 | color_bits;
        break;
      case BISHOP:
        screen_memory[ pos ] = 5 | color_bits;
        screen_memory[ pos+16 ] = 7 | color_bits;
        break;
      case KNIGHT:
        screen_memory[ pos ] = 8 | color_bits;
        screen_memory[ pos+16 ] = 9 | color_bits;
        break;
      case ROOK:
        screen_memory[ pos ] = 10 | color_bits;
        screen_memory[ pos+16 ] = 11 | color_bits;
        break;
      case PAWN:
        screen_memory[ pos ] = 0;
        screen_memory[ pos+16 ] = 12 | color_bits;
        break;
      default:
        break;
    };

  } else {
    screen_memory[ pos ] = 0;
    screen_memory[ pos+16 ]  = 0;
  }
}


void draw_pieces(){
  uint8_t row, col;
  //uint16_t pos = BOARD_START;
  for ( row = 0; row < 8; row++) {
    for ( col = 0; col < 8; col++) {
      update_square( col, row );

      // VDP_setTileMapEx( BG_A, pieces_img.tilemap, TILE_ATTR_FULL(PAL1, TRUE, FALSE, FALSE, piecesTileIndex),
      //        boardStartCol + col * boardStep,  // PLANE X Dest in tiles
      //       boardStartRow + row * boardStep,  // PLANE Y Dest in tiles
      //      0,  // REGION X start
      //     0,  // REGION Y start
      //    boardStep,  // Width
      //   boardStep,  // Height
      //  CPU);
      }

      //   pos +=2;
    }
    // pos +=16;
}

void clear_space( int8_t col, int8_t row ) {
  uint16_t pos = BOARD_START + row * 32 + col * 2;
  screen_memory[ pos ] = 0;
  screen_memory[ pos+16 ] = 0;
  //    VDP_setTileMapEx( BG_A, pieces_img.tilemap, TILE_ATTR_FULL(PAL1, TRUE, FALSE, FALSE, piecesTileIndex),
  //            boardStartCol + startCol * boardStep,  // PLANE X Dest in tiles
  //            boardStartRow + startRow * boardStep,  // PLANE Y Dest in tiles
  //            EMPTY,  // REGION X start
  //            0,  // REGION Y start
  //            boardStep,  // Width
  //            boardStep,  // Height
  //            CPU);

}


void move_piece( int8_t startCol, int8_t startRow, int8_t endCol, int8_t endRow, int8_t promotype ){
  //if( do_move( startCol, startRow, endCol, endRow ) ) {
  uint8_t p = board[startCol][startRow].player;
  uint8_t cp = board[startCol][startRow].type;
  board[endCol][endRow] = board[startCol][startRow];
  board[startCol][startRow].type = EMPTY;
  board[startCol][startRow].player = NO_PLAYER;

  // check for special cases
  //
  //   ' castles to check
  //   ' white
  //   ' e1g1 -  4,7,6,7
  //   ' e1c1 -  4,7,2,7
  //   ' black
  //   ' e8g8 -  4,0,6,0
  //   ' e8c8 -  4,0,2,0
  // 

  if ( cp == KING && p == PLAYER_TWO &&  startCol == 4 && startRow ==0 && endCol == 6 && endRow == 0 ) {
    // move black rook from right
    board[7][0].type = EMPTY;
    board[7][0].player = NO_PLAYER;
    update_square( 7, 0 );
    board[5][0].type = ROOK;
    board[5][0].player = p;

  } else if ( cp == KING && p == PLAYER_TWO && startCol == 4 && startRow ==0 && endCol == 2 && endRow == 0 ) {
    // move rook from left
    board[0][0].type = EMPTY;
    board[0][0].player = NO_PLAYER;
    update_square( 0, 0 );
    board[3][0].type = ROOK;
    board[3][0].player = p;
  } else if ( cp == KING && p == PLAYER_ONE && startCol == 4 && startRow ==7 && endCol == 6 && endRow == 7 ) {
    // move rook from right
    board[7][7].type = EMPTY;
    board[7][7].player = NO_PLAYER;
    update_square( 7, 7 );
    board[5][7].type = ROOK;
    board[5][7].player = p;
  } else if ( cp == KING && p == PLAYER_ONE && startCol == 4 && startRow ==7 && endCol == 2 && endRow == 7 ) {
    // move rook from left
    board[0][7].type = EMPTY;
    board[0][7].player = NO_PLAYER;
    update_square( 0, 7 );
    board[3][7].type = ROOK;
    board[3][7].player = p;
  }
  // if pawn,

  draw_pieces();
  update_square( startCol, startRow );

  //}
}




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
  /*
     screen_memory [ 16 ] =  10;  // black is actual offset
     screen_memory [ 32 ] =  11;
     screen_memory [ 48 ] =  0;  
     screen_memory [ 64 ] =  12;

     screen_memory [ 240 ] =  10+128; // white is + 128
     screen_memory [ 256 ] =  11+128;
     screen_memory [ 208 ] =  0;  
     screen_memory [ 224] =  12+128;
     */

  // fake cursor
  /*
  screen_memory [ 112 + 2 ] =  28 + 64;  //  cursor color is +64
  screen_memory [ 128 + 2] =  29 + 64;
*/

  //for( i=0; i < 64; ++i ) { 
  //  screen_memory[20+i] = i;
  //}
  sprintf((char *)message_memory, "fujinet"); 
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



void cursor_init( ) {
  chess_cursor.col = 4;  // board position
  chess_cursor.row = 4;
//  chess_cursor.txt_x = chess_cursor.col * cursorStep + cursorColStart;
//  chess_cursor.txt_y = chess_cursor.row * cursorStep + cursorRowStart;
  //cursor.sprite =  sprite;

  chess_cursor.sel_col = -1;  // not on board
  chess_cursor.sel_row = -1;
//  chess_cursor.sel_txt_x = -1;
//  chess_cursor.sel_txt_y = -1;

  //cursor->selected_spr = selected_sprite;
  //SPR_setAnim( cursor->selected_spr, 1 );
  // SPR_setVisibility( cursor->selected_spr, HIDDEN );
}

bool cursor_move( uint8_t stick ) {
  uint8_t old_col, old_row;
  bool didMove = false;
  uint16_t pos = BOARD_START;
  old_col = chess_cursor.col;
  old_row = chess_cursor.row;
  if( stick == BUTTON_LEFT ) {
    chess_cursor.col--;
    if( chess_cursor.col < 0 ) {
      chess_cursor.col = 7;
    }
    //chess_cursor.txt_x = chess_cursor.col * cursorStep + cursorColStart;
    didMove = true;
  } 
  if( stick == BUTTON_RIGHT ) {
    chess_cursor.col++;
    if( chess_cursor.col > 7 ) {
      chess_cursor.col = 0;
    }
    //chess_cursor.txt_x = chess_cursor.col * cursorStep + cursorColStart;
    didMove = true;
  } 
  if( stick == BUTTON_UP ) {
    chess_cursor.row--;
    if( chess_cursor.row < 0 ) {
      chess_cursor.row = 7;
    }
    //chess_cursor.txt_y = chess_cursor.row * cursorStep + cursorRowStart;
    didMove = true;
  }
  if( stick == BUTTON_DOWN ) {
    chess_cursor.row++;
    if( chess_cursor.row > 7 ) {
      chess_cursor.row = 0;
    }
    //chess_cursor.txt_y = chess_cursor.row * cursorStep + cursorRowStart;
    didMove = true;
  }
  if ( didMove ) {
    update_square( old_col, old_row );
    pos += (chess_cursor.row * 32 ) + chess_cursor.col * 2;
    screen_memory[ pos ] = 28 + 64;
    screen_memory[ pos+16 ] = 29 + 64;
  }
  return didMove;
}


void cursor_update_from_pos( int8_t  col, int8_t  row, int8_t  sel_col, int8_t  sel_row ) {
  chess_cursor.col = col;
  //chess_cursor.txt_x = chess_cursor.col * cursorStep + cursorColStart;
  chess_cursor.row = row;
  //chess_cursor.txt_y = chess_cursor.row * cursorStep + cursorRowStart;

  chess_cursor.sel_col = sel_col;
  //chess_cursor.sel_txt_x = chess_cursor.sel_col * cursorStep + cursorColStart;
  chess_cursor.sel_row = sel_row;
  //chess_cursor.sel_txt_y = chess_cursor.sel_row * cursorStep + cursorRowStart;
  if( chess_cursor.sel_col >= 0 ) {
    //SPR_setVisibility( chess_cursor.selected_spr, VISIBLE );
  } else {
    // SPR_setVisibility( chess_cursor.selected_spr, HIDDEN );
  }
}

void cursor_clear_selected( ) {
  //    char message[40];
  chess_cursor.sel_col = -1;
  chess_cursor.sel_row = -1;
  //chess_cursor.sel_txt_x = -1;
  //chess_cursor.sel_txt_y = -1;
  ////SPR_setVisibility( chess_cursor.selected_spr, HIDDEN );
  //char message[40];
  //strclr(message);
  //sprintf( message, "X: %d y: %d sx: %d sy %d    ", chess_cursor.col, chess_cursor.row, chess_cursor.sel_col, chess_cursor.sel_row);
}



bool cursor_action( CHESS_PIECE brd[8][8], uint8_t player ) {
  if( chess_cursor.sel_col < 0 ) {
    // no piece selected yet, check if player owns the current piece.
    if( brd[(uint8_t)chess_cursor.col][(uint8_t)chess_cursor.row].player == player ) { 
      chess_cursor.sel_col = chess_cursor.col;
      chess_cursor.sel_row = chess_cursor.row;
      //chess_cursor.sel_txt_x = chess_cursor.sel_col * cursorStep + cursorColStart;
      //chess_cursor.sel_txt_y = chess_cursor.sel_row * cursorStep + cursorRowStart;
      ////SPR_setVisibility( chess_cursor.selected_spr, VISIBLE );
    }
  } else {
    //char message[40];
    //strclr(message);
    //sprintf( message, "X: %d y: %d sx: %d sy %d    ", chess_cursor.col, chess_cursor.row, chess_cursor.sel_col, chess_cursor.sel_row);
    //VDP_drawText( message, 0, 1 );
    // return true if destination is clear or a different player, BUT DON"T UPDATE BOARD 
    return ( brd[(uint8_t)chess_cursor.col][(uint8_t)chess_cursor.row].player != player );

  }
  return false;
}



static void handle_my_turn() {
  // read joystick to move cursor
  if( inputWait == 0 ) {
    uint8_t stick = OS.stick0;
    inputWait = INPUT_WAIT_COUNT;
    if( cursor_move( stick ) == true ) {
    }
    if( !OS.strig0 ) {
      // need 
      bool trySend =  cursor_action( board, current_player );
      if( trySend ) {
      }
    }
    /* SEGA!
       u16 stick  = JOY_readJoypad( JOY_1 );
    // update local position
    if( cursor_move( &cursor, stick ) == true ) {
    XGM_startPlayPCM(SND_MOVE,1,SOUND_PCM_CH2);
    inputWait = INPUT_WAIT_COUNT;
    }
    // if A, 
    if( stick & BUTTON_A ) {
    bool trySend =  cursor_action( &cursor, board, currentPlayer );
    inputWait = INPUT_WAIT_COUNT;
    if( trySend ) {
    // send possible move
    if( send_move( &cursor, 1 ) ) {
    currentPlayer = currentPlayer == PLAYER_ONE ? PLAYER_TWO : PLAYER_ONE; 
    }
    }
    } else if( stick & BUTTON_C ) {
    cursor_clear_selected( &cursor );
    inputWait = INPUT_WAIT_COUNT;
    } 
    */

  } else {
    if ( inputWait > 0 ) {
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
  cursor_init();


  clear_board();
  setup_pieces();
  draw_pieces();

      inputWait = INPUT_WAIT_COUNT;
  current_player = PLAYER_ONE;
  who_am_i = PLAYER_ONE;

  // main loop
  for (;;) {
    waitvsync();
    if (current_player == who_am_i) {
      handle_my_turn();
    } else {
      //           poll_other_player();
    }
  }

  return 0;
}

