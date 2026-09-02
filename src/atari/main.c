
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

uint8_t current_player;
uint8_t who_am_i;

void init_dlist(void);

extern uint8_t screen_memory[];  
extern uint8_t message_memory[];  
extern uint8_t pmg_memory[380];
extern uint8_t font[];
extern uint8_t pmg_data[1024];

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

