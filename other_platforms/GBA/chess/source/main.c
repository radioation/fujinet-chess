
#include <gba_console.h>
#include <gba_video.h>
#include <gba_interrupt.h>
#include <gba_systemcalls.h>
#include <gba_input.h>
#include <gba_dma.h>
#include <gba_sprites.h>
#include <stdio.h>
#include <stdlib.h>

#include "chessboard.h"
#include "chesspieces.h"
#include "cursor.h"

#define OAM_MEM ((volatile OBJATTR *)0x07000000)




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



void cursor_init( CURSOR *cursor, int32_t cursor_tile_start, int32_t selected_tile_start) {
    cursor->col = 4;  // board position
    cursor->row = 4;
    cursor->pos_x = cursor->col * cursorStep + cursorColStart;
    cursor->pos_y = cursor->row * cursorStep + cursorRowStart;
 //   cursor->sprite =  sprite;

    cursor->sel_col = -1;  // not on board
    cursor->sel_row = -1;
    cursor->sel_pos_x = -16;
    cursor->sel_pos_y = -16;
//    cursor->selected_spr = selected_sprite;

    OAM_MEM[0].attr0 = ATTR0_NORMAL | ATTR0_COLOR_16 | ATTR0_SQUARE | OBJ_Y( cursor->pos_y );
    OAM_MEM[0].attr1 = ATTR1_SIZE_16 | OBJ_X(cursor->pos_x);
    OAM_MEM[0].attr2 = ATTR2_PALETTE(0) | OBJ_CHAR(0) | OBJ_PRIORITY(0);



//    SPR_setAnim( cursor->selected_spr, 1 );
//    SPR_setVisibility( cursor->selected_spr, HIDDEN );
}





//---------------------------------------------------------------------------------
// Program entry point
//---------------------------------------------------------------------------------
int main(void) {
//---------------------------------------------------------------------------------

    CURSOR cursor;

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
    cursor_init( &cursor, 0, 4 );	

	while (1) {
		VBlankIntrWait();
	}
}


