
//{{BLOCK(chessboard)

//======================================================================
//
//	chessboard, 256x256@4, 
//	+ palette 16 entries, not compressed
//	+ 231 tiles (t|f reduced) not compressed
//	+ regular map (in SBBs), not compressed, 32x32 
//	Total size: 32 + 7392 + 2048 = 9472
//
//	Time-stamp: 2026-09-08, 17:19:10
//	Exported by Cearn's GBA Image Transmogrifier, v0.9.2
//	( http://www.coranac.com/projects/#grit )
//
//======================================================================

#ifndef GRIT_CHESSBOARD_H
#define GRIT_CHESSBOARD_H

#define chessboardTilesLen 7392
extern const unsigned int chessboardTiles[1848];

#define chessboardMapLen 2048
extern const unsigned short chessboardMap[1024];

#define chessboardPalLen 32
extern const unsigned short chessboardPal[16];

#endif // GRIT_CHESSBOARD_H

//}}BLOCK(chessboard)
