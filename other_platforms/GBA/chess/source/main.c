
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

#define OAM_MEM ((volatile OBJATTR *)0x07000000)

//---------------------------------------------------------------------------------
// Program entry point
//---------------------------------------------------------------------------------
int main(void) {
//---------------------------------------------------------------------------------


	// the vblank interrupt must be enabled for VBlankIntrWait() to work
	// since the default dispatcher handles the bios flags no vblank handler
	// is required
	irqInit();
	irqEnable(IRQ_VBLANK);

//	consoleDemoInit();

    REG_DISPCNT = ( MODE_0 | BG0_ON | BG1_ON | OBJ_ENABLE | OBJ_1D_MAP );

    // setup palettes
    dmaCopy( chessboardPal, BG_PALETTE, chessboardPalLen );
    dmaCopy( chesspiecesPal, SPRITE_PALETTE, chesspiecesPalLen );


    // setup tiles
    dmaCopy( chessboardTiles, TILE_BASE_ADR(0), chessboardTilesLen );
    dmaCopy( chessboardMap, MAP_BASE_ADR(8), chessboardMapLen );
    dmaCopy( chesspiecesTiles, OBJ_BASE_ADR, chesspiecesTilesLen );

    REG_BG0CNT = ( BG_SIZE_0 | BG_16_COLOR | TILE_BASE(0) | MAP_BASE(8) );


//    // clear things out
//    for(int i = 0; i < 128; i++) {
//        OAM_MEM[i].attr0 = ATTR0_DISABLED;
//    }
	

	while (1) {
		VBlankIntrWait();
	}
}


