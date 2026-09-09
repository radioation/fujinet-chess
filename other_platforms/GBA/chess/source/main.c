
#include <gba_console.h>
#include <gba_video.h>
#include <gba_interrupt.h>
#include <gba_systemcalls.h>
#include <gba_input.h>
#include <gba_dma.h>
#include <gba_sprites.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include "chessboard.h"
#include "chesspieces.h"
#include "cursor.h"

#define OAM_MEM ((volatile OBJATTR *)0x07000000)

#define INPUT_WAIT_COUNT 10



// Enum to represent piece types (using offsets for lookup into image)
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


#define FILE_X 97
#define RANK_Y 49



// Sprite data structures
typedef struct 
{
    //Sprite *sprite;
    int32_t col;     // board col
    int32_t row;     // board row
    int32_t pos_x;  
    int32_t pos_y;
    int32_t sel_col; // selected board column
    int32_t sel_row; // selected board row
    int32_t sel_pos_x;
    int32_t sel_pos_y;
    int32_t tile_index; 
    int32_t selected_tile_index; 

    int32_t tile_step;
    int32_t frame_count;
    int32_t frame;
    int32_t frame_delay;
    int ticks;
    int32_t attr_size;
    //Sprite *selected_spr;
} CURSOR;

const int32_t cursorStep = 16;
const int32_t cursorColStart = 32;
const int32_t cursorRowStart = 16;

const int32_t cursorOffset = 0;
const int32_t piecesOffset = cursorTilesLen;
CURSOR cursor;



void cursor_init(  int32_t cursor_tile_start, int32_t selected_tile_start) {
    cursor.col = 4;  // board position
    cursor.row = 4;
    cursor.pos_x = cursor.col * cursorStep + cursorColStart;
    cursor.pos_y = cursor.row * cursorStep + cursorRowStart;
    //   cursor.sprite =  sprite;

    cursor.sel_col = -1;  // not on board
    cursor.sel_row = -1;
    cursor.sel_pos_x = -16;
    cursor.sel_pos_y = -16;


    cursor.tile_step = 4;
    cursor.frame_count = 4;
    cursor.frame = 0;
    
    cursor.frame_delay = 2;
    cursor.attr_size = ATTR1_SIZE_16;

    //    cursor.selected_spr = selected_sprite;

    OAM_MEM[0].attr0 = ATTR0_NORMAL | ATTR0_COLOR_16 | ATTR0_SQUARE | OBJ_Y( cursor.pos_y );
    OAM_MEM[0].attr1 = cursor.attr_size | OBJ_X(cursor.pos_x);
    OAM_MEM[0].attr2 = ATTR2_PALETTE(0) | OBJ_CHAR(0) | OBJ_PRIORITY(0);

    OAM_MEM[1].attr0 = ATTR0_NORMAL | ATTR0_COLOR_16 | ATTR0_SQUARE | OBJ_Y( cursor.sel_pos_y );
    OAM_MEM[1].attr1 = cursor.attr_size | OBJ_X(cursor.sel_pos_x);
    OAM_MEM[1].attr2 = ATTR2_PALETTE(0) | OBJ_CHAR( cursor.tile_step * cursor.frame_count) | OBJ_PRIORITY(0);



    //    SPR_setAnim( cursor.selected_spr, 1 );
    //    SPR_setVisibility( cursor.selected_spr, HIDDEN );
}


bool cursor_move(  int32_t keys ) {
    bool didMove = false;
    if( keys & KEY_LEFT ) {
        cursor.col--;
        if( cursor.col < 0 ) {
            cursor.col = 7;
        }
        cursor.pos_x = cursor.col * cursorStep + cursorColStart;
        didMove = true;
    } 
    if( keys & KEY_RIGHT ) {
        cursor.col++;
        if( cursor.col > 7 ) {
            cursor.col = 0;
        }
        cursor.pos_x = cursor.col * cursorStep + cursorColStart;
        didMove = true;
    } 
    if( keys & KEY_UP ) {
        cursor.row--;
        if( cursor.row < 0 ) {
            cursor.row = 7;
        }
        cursor.pos_y = cursor.row * cursorStep + cursorRowStart;
        didMove = true;
    }
    if( keys & KEY_DOWN ) {
        cursor.row++;
        if( cursor.row > 7 ) {
            cursor.row = 0;
        }
        cursor.pos_y = cursor.row * cursorStep + cursorRowStart;
        didMove = true;
    }
    return didMove;
}



/*
   void cursor_update_from_pos( CURSOR *cursor, int32_t col, int32_t row, int32_t sel_col, int32_t sel_row ) {
   cursor.col = col;
   cursor.pos_x = cursor.col * cursorStep + cursorColStart;
   cursor.row = row;
   cursor.pos_y = cursor.row * cursorStep + cursorRowStart;

   cursor.sel_col = sel_col;
   cursor.sel_pos_x = cursor.sel_col * cursorStep + cursorColStart;
   cursor.sel_row = sel_row;
   cursor.sel_pos_y = cursor.sel_row * cursorStep + cursorRowStart;
//   if( cursor.sel_col >= 0 ) {
//       SPR_setVisibility( cursor.selected_spr, VISIBLE );
//   } else {
//       SPR_setVisibility( cursor.selected_spr, HIDDEN );
//   }
}
 *?

 void cursor_clear_selected( CURSOR* cursor ) {
 char message[40];
 cursor.sel_col = -1;
 cursor.sel_row = -1;
 cursor.sel_pos_x = -32;
 cursor.sel_pos_y = -32;
//SPR_setVisibility( cursor.selected_spr, HIDDEN );
//char message[40];
//strclr(message);
//sprintf( message, "X: %d y: %d sx: %d sy %d    ", cursor.col, cursor.row, cursor.sel_col, cursor.sel_row);
}



bool cursor_action( CURSOR* cursor, CHESS_PIECE brd[8][8], int32_t player ) {
if( cursor.sel_col < 0 ) {
// no piece selected yet, check if player owns the current piece.
if( brd[(int32_t)cursor.col][(int32_t)cursor.row].player == player ) { 
cursor.sel_col = cursor.col;
cursor.sel_row = cursor.row;
cursor.sel_pos_x = cursor.sel_col * cursorStep + cursorColStart;
cursor.sel_pos_y = cursor.sel_row * cursorStep + cursorRowStart;
SPR_setVisibility( cursor.selected_spr, VISIBLE );
}
} else {
//char message[40];
//strclr(message);
//sprintf( message, "X: %d y: %d sx: %d sy %d    ", cursor.col, cursor.row, cursor.sel_col, cursor.sel_row);
//VDP_drawText( message, 0, 1 );
// return true if destination is clear or a different player, BUT DON"T UPDATE BOARD 
return ( brd[(int32_t)cursor.col][(int32_t)cursor.row].player != player );

}
return false;
}
 */


void sprite_update() {
    cursor.ticks++;
    if ( cursor.ticks > cursor.frame_delay ) {
        cursor.ticks = 0;
        cursor.frame++;
        if( cursor.frame >= cursor.frame_count ) {
            cursor.frame = 0;
        }
        OAM_MEM[0].attr0 = ATTR0_NORMAL | ATTR0_COLOR_16 | ATTR0_SQUARE | OBJ_Y( cursor.pos_y );
        OAM_MEM[0].attr1 = cursor.attr_size | OBJ_X(cursor.pos_x);
        OAM_MEM[0].attr2 = ATTR2_PALETTE(0) | OBJ_CHAR( cursor.frame * cursor.frame_count) | OBJ_PRIORITY(0);

        OAM_MEM[1].attr0 = ATTR0_NORMAL | ATTR0_COLOR_16 | ATTR0_SQUARE | OBJ_Y( cursor.sel_pos_y );
        OAM_MEM[1].attr1 = cursor.attr_size | OBJ_X(cursor.sel_pos_x);
        OAM_MEM[1].attr2 = ATTR2_PALETTE(0) | OBJ_CHAR( ( cursor.frame +  cursor.tile_step ) * cursor.frame_count) | OBJ_PRIORITY(0);


    }

}



//---------------------------------------------------------------------------------
// Program entry point
//---------------------------------------------------------------------------------
int main(void) {
    //---------------------------------------------------------------------------------

    int32_t input_wait = INPUT_WAIT_COUNT;
    // the vblank interrupt must be enabled for VBlankIntrWait() to work
    // since the default dispatcher handles the bios flags no vblank handler
    // is required
    irqInit();
    irqEnable(IRQ_VBLANK);

    //	consoleDemoInit();

    REG_DISPCNT = ( MODE_0 | BG0_ON | BG1_ON | OBJ_ENABLE | OBJ_1D_MAP );

    // setup palettes
    dmaCopy( chessboardPal, BG_PALETTE, chessboardPalLen );
    dmaCopy( cursorPal, SPRITE_PALETTE, cursorPalLen );
    dmaCopy( chesspiecesPal, SPRITE_PALETTE + 16, chesspiecesPalLen );


    // setup tiles
    dmaCopy( chessboardTiles, TILE_BASE_ADR(0), chessboardTilesLen );
    dmaCopy( chessboardMap, MAP_BASE_ADR(8), chessboardMapLen );
    dmaCopy( cursorTiles, OBJ_BASE_ADR, cursorTilesLen );
    dmaCopy( chesspiecesTiles, OBJ_BASE_ADR+piecesOffset, chesspiecesTilesLen );

    REG_BG0CNT = ( BG_SIZE_0 | BG_16_COLOR | TILE_BASE(0) | MAP_BASE(8) );


    // clear things out
    for(int i = 0; i < 128; i++) {
        OAM_MEM[i].attr0 = ATTR0_DISABLED;
    }
    cursor_init( 0, 4 );	
    while (1) {
        VBlankIntrWait();
        // readKeys();
        scanKeys();
        if( input_wait == 0 ) {
            int32_t keys = ~(REG_KEYINPUT);
            cursor_move( keys );
            input_wait =INPUT_WAIT_COUNT;
        } else {
            --input_wait;
        }
        sprite_update();


    }

}


