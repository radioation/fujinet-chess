
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
#define  KING  3
#define  QUEEN  6
#define  ROOK  9
#define  BISHOP  12
#define  KNIGHT  15
#define  PAWN  18

#define NO_PLAYER  0
#define PLAYER_ONE  1
#define PLAYER_TWO  2

#define FILE_X 97
#define RANK_Y 49

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
extern uint8_t pmg_memory[];
extern uint8_t font[];
extern uint8_t pmg_data[1024];

//static uint8_t board_data[BOARD_SIZE];

///* ------------------------------------------------------------------------- */
///* Memory-mapped Atari helpers                                                */
///* ------------------------------------------------------------------------- */
//
////#define PEEK8(addr)          (*(volatile uint8_t *)(addr))
////#define POKE8(addr, value)   (*(volatile uint8_t *)(addr) = (uint8_t)(value))
////#define PEEK16(addr)         (*(volatile uint16_t *)(addr))
//
//#define RAMTOP_ADDR          0x006A
//#define SAVMSC_ADDR          0x0058
//#define CHBAS_ADDR           0x02F4
//#define SDLSTL_ADDR          0x0230
//#define SDMCTL_ADDR          0x022F
//
//#define HPOSP0_ADDR          0xD000
//#define HPOSP1_ADDR          0xD001
//#define COLPM0_ADDR          0x02C0
//#define COLPM1_ADDR          0x02C1
//#define GRACTL_ADDR          0xD01D
//#define PMBASE_ADDR          0xD407
//
//#define STICK0_ADDR          0x0278
//#define STRIG0_ADDR          0x0284
//
//#define BOARD_WIDTH          8
//#define BOARD_SIZE           64
//
//#define FILE_X               ((uint8_t)'a')
//#define RANK_Y               ((uint8_t)'1')
//
//#define DEFAULT_HOST         "10.25.50.61"
//#define SERVER_PORT          ":55557/"
//
//
//
//#define FJNET_GET_MODE          12
//#define FJNET_POST_MODE         13
//#define FJNET_TRANSL            2
//
//#define FJNET_IN_SIZE           128
//#define FJNET_OUT_SIZE          128
//#define URL_SIZE             128
//
//#define WHITE_BISHOP         ((uint8_t)'B')   /* 66 */
//#define WHITE_KING           ((uint8_t)'K')   /* 75 */
//#define WHITE_KNIGHT         ((uint8_t)'N')   /* 78 */  
//#define WHITE_PAWN           ((uint8_t)'P')   /* 80  */
//#define WHITE_QUEEN          ((uint8_t)'Q')   /* 81 */
//#define WHITE_ROOK           ((uint8_t)'R')   /* 82 */
//
//#define BLACK_BISHOP         ((uint8_t)'b')   /* 98 */
//#define BLACK_KING           ((uint8_t)'k')   /* 107 */
//#define BLACK_KNIGHT         ( (uint8_t)'n')   /* 110 */
//#define BLACK_PAWN           ((uint8_t)'p')   /* 112 */
//#define BLACK_QUEEN          ((uint8_t)'q')   /* 113 */
//#define BLACK_ROOK           ((uint8_t)'r')   /* 114 */
//
//// LUTs for cursor/tile positions.
//static const uint8_t cursor_x[8] = {82,94,106,118,130,142,154,166};
//static const uint8_t cursor_y[8] = {89,81,73,65,57,49,41,33};
//static const uint8_t tile_x[8] = {8,11,14,17,20,23,26,29};
//static const uint8_t tile_y[8] = {17,15,13,11,9,7,5,3};
//
//static const uint8_t pm_cursor_data[6] = {
//    0xFF,0x81,0x81,0x81,0x81,0xFF
//};
//
//static const uint8_t pm_select_data[6] = {
//    0xC3,0x81,0x00,0x00,0x81,0xC3
//};
//
//// size: 1024 bytes  (fontmaker)
//static const unsigned char pieces_charset[1024] = {
//    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
//    0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55,  // light square
//    0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA,  // dark square
//    0xFF, 0xFF, 0x7D, 0x7D, 0x7D, 0xFF, 0xFF, 0x55,  // pawn bottom light
//    0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x7D, 0xFF,  
//    0xFF, 0xFF, 0xBE, 0xBE, 0xBE, 0xFF, 0xFF, 0xAA,
//    0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xBE, 0xFF, 0x55, 0x55, 0x55, 0x55, 0x55, 0x57, 0x57, 0x55,
//    0xFF, 0x7F, 0x7F, 0x7F, 0xFF, 0xFF, 0xFF, 0x55, 0xD5, 0x55, 0x55, 0x55, 0xD5, 0xF5, 0xF5, 0x55,
//    0x55, 0x55, 0x5D, 0x57, 0x57, 0x55, 0x55, 0x55, 0x55, 0x5D, 0xDD, 0xFF, 0xFF, 0xFF, 0xFF, 0x7F,
//    0x55, 0x55, 0xDD, 0xF5, 0xF5, 0xD5, 0xD5, 0x55, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAB, 0xAB, 0xAA,
//    0xFF, 0xBF, 0xBF, 0xBF, 0xFF, 0xFF, 0xFF, 0xAA, 0xEA, 0xAA, 0xAA, 0xAA, 0xEA, 0xFA, 0xFA, 0xAA,
//    0x00, 0x3C, 0x66, 0x6E, 0x76, 0x66, 0x3C, 0x00, 0x00, 0x18, 0x38, 0x18, 0x18, 0x18, 0x7E, 0x00,
//    0x00, 0x3C, 0x66, 0x0C, 0x18, 0x30, 0x7E, 0x00, 0x00, 0x7E, 0x0C, 0x18, 0x0C, 0x66, 0x3C, 0x00,
//    0x00, 0x0C, 0x1C, 0x3C, 0x6C, 0x7E, 0x0C, 0x00, 0x00, 0x7E, 0x60, 0x7C, 0x06, 0x66, 0x3C, 0x00,
//    0x00, 0x3C, 0x60, 0x7C, 0x66, 0x66, 0x3C, 0x00, 0x00, 0x7E, 0x06, 0x0C, 0x18, 0x30, 0x30, 0x00,
//    0x00, 0x3C, 0x66, 0x3C, 0x66, 0x66, 0x3C, 0x00, 0x00, 0x3C, 0x66, 0x3E, 0x06, 0x0C, 0x38, 0x00,
//    0x00, 0x00, 0x18, 0x18, 0x00, 0x18, 0x18, 0x00, 0x00, 0x00, 0x18, 0x18, 0x00, 0x18, 0x18, 0x30,
//    0x06, 0x0C, 0x18, 0x30, 0x18, 0x0C, 0x06, 0x00, 0x00, 0x00, 0x7E, 0x00, 0x00, 0x7E, 0x00, 0x00,
//    0x60, 0x30, 0x18, 0x0C, 0x18, 0x30, 0x60, 0x00, 0x00, 0x3C, 0x66, 0x0C, 0x18, 0x00, 0x18, 0x00,
//    0x00, 0x3C, 0x66, 0x6E, 0x6E, 0x60, 0x3E, 0x00, 0x00, 0x18, 0x3C, 0x66, 0x66, 0x7E, 0x66, 0x00,
//    0x00, 0x7C, 0x66, 0x7C, 0x66, 0x66, 0x7C, 0x00, 0x00, 0x3C, 0x66, 0x60, 0x60, 0x66, 0x3C, 0x00,
//    0x00, 0x78, 0x6C, 0x66, 0x66, 0x6C, 0x78, 0x00, 0x00, 0x7E, 0x60, 0x7C, 0x60, 0x60, 0x7E, 0x00,
//    0x00, 0x7E, 0x60, 0x7C, 0x60, 0x60, 0x60, 0x00, 0x00, 0x3E, 0x60, 0x60, 0x6E, 0x66, 0x3E, 0x00,
//    0x00, 0x66, 0x66, 0x7E, 0x66, 0x66, 0x66, 0x00, 0x00, 0x7E, 0x18, 0x18, 0x18, 0x18, 0x7E, 0x00,
//    0x00, 0x06, 0x06, 0x06, 0x06, 0x66, 0x3C, 0x00, 0x00, 0x66, 0x6C, 0x78, 0x78, 0x6C, 0x66, 0x00,
//    0x00, 0x60, 0x60, 0x60, 0x60, 0x60, 0x7E, 0x00, 0x00, 0x63, 0x77, 0x7F, 0x6B, 0x63, 0x63, 0x00,
//    0x00, 0x66, 0x76, 0x7E, 0x7E, 0x6E, 0x66, 0x00, 0x00, 0x3C, 0x66, 0x66, 0x66, 0x66, 0x3C, 0x00,
//    0x00, 0x7C, 0x66, 0x66, 0x7C, 0x60, 0x60, 0x00, 0x00, 0x3C, 0x66, 0x66, 0x66, 0x6C, 0x36, 0x00,
//    0x00, 0x7C, 0x66, 0x66, 0x7C, 0x6C, 0x66, 0x00, 0x00, 0x3C, 0x60, 0x3C, 0x06, 0x06, 0x3C, 0x00,
//    0x00, 0x7E, 0x18, 0x18, 0x18, 0x18, 0x18, 0x00, 0x00, 0x66, 0x66, 0x66, 0x66, 0x66, 0x7E, 0x00,
//    0x00, 0x66, 0x66, 0x66, 0x66, 0x3C, 0x18, 0x00, 0x00, 0x63, 0x63, 0x6B, 0x7F, 0x77, 0x63, 0x00,
//    0x00, 0x66, 0x66, 0x3C, 0x3C, 0x66, 0x66, 0x00, 0x00, 0x66, 0x66, 0x3C, 0x18, 0x18, 0x18, 0x00,
//    0x00, 0x7E, 0x0C, 0x18, 0x30, 0x60, 0x7E, 0x00, 0xAA, 0xAA, 0xAE, 0xAB, 0xAB, 0xAA, 0xAA, 0xAA,
//    0xAA, 0xAE, 0xEE, 0xFF, 0xFF, 0xFF, 0xFF, 0xBF, 0xAA, 0xAA, 0xEE, 0xFA, 0xFA, 0xEA, 0xEA, 0xAA,
//    0x55, 0x55, 0x55, 0x55, 0x55, 0x57, 0x57, 0x55, 0x55, 0x5D, 0x7F, 0x5D, 0xFF, 0xFF, 0xFF, 0x7F,
//    0x55, 0x55, 0x55, 0x55, 0xD5, 0xF5, 0xF5, 0x55, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAB, 0xAB, 0xAA,
//    0xAA, 0xAE, 0xBF, 0xAE, 0xFF, 0xFF, 0xFF, 0xBF, 0xAA, 0xAA, 0xAA, 0xAA, 0xEA, 0xFA, 0xFA, 0xAA,
//    0x55, 0x55, 0xDD, 0xDD, 0xFF, 0xFF, 0xFF, 0xFF, 0x55, 0x55, 0xD5, 0xD5, 0xD5, 0xD5, 0xD5, 0xD5,
//    0xAA, 0xAA, 0xEE, 0xEE, 0xFF, 0xFF, 0xFF, 0xFF, 0xAA, 0xAA, 0xEA, 0xEA, 0xEA, 0xEA, 0xEA, 0xEA,
//    0x57, 0x55, 0x55, 0x55, 0x55, 0x55, 0x57, 0x55, 0xFF, 0xFF, 0x7F, 0x7F, 0xFF, 0xFF, 0xFF, 0x55,
//    0xF5, 0xD5, 0x55, 0x55, 0xD5, 0xD5, 0xF5, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x57,
//    0x55, 0x55, 0x5D, 0x5D, 0x7F, 0x5F, 0xD7, 0xF7, 0x55, 0x55, 0x55, 0x55, 0x55, 0xD5, 0xD5, 0xF5,
//    0xAB, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAB, 0xAA, 0xFF, 0xFF, 0xBF, 0xBF, 0xFF, 0xFF, 0xFF, 0xAA,
//    0xFA, 0xEA, 0xAA, 0xAA, 0xEA, 0xEA, 0xFA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAB,
//    0xAA, 0xAA, 0xAE, 0xAE, 0xBF, 0xAF, 0xEB, 0xFB, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xEA, 0xEA, 0xFA,
//    0x5F, 0x5F, 0x57, 0x55, 0x55, 0x57, 0x57, 0x55, 0xF5, 0xF5, 0xF5, 0xD5, 0xD5, 0xF5, 0xF5, 0x55,
//    0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x57, 0x5F, 0x55, 0x55, 0x7D, 0xFF, 0xDF, 0xFF, 0xFF, 0xFF,
//    0x55, 0x55, 0x55, 0x55, 0xD5, 0xD5, 0xF5, 0xF5, 0xAF, 0xAF, 0xAB, 0xAA, 0xAA, 0xAB, 0xAB, 0xAA,
//    0xFA, 0xFA, 0xFA, 0xEA, 0xEA, 0xFA, 0xFA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAB, 0xAF,
//    0xAA, 0xAA, 0xBE, 0xFF, 0xEF, 0xFF, 0xFF, 0xFF, 0xAA, 0xAA, 0xAA, 0xAA, 0xEA, 0xEA, 0xFA, 0xFA,
//    0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAB, 0xAF, 0x00, 0x18, 0x0C, 0x7E, 0x0C, 0x18, 0x00, 0x00,
//    0x00, 0x18, 0x3C, 0x7E, 0x7E, 0x3C, 0x18, 0x00, 0x00, 0x00, 0x3C, 0x06, 0x3E, 0x66, 0x3E, 0x00,
//    0x00, 0x60, 0x60, 0x7C, 0x66, 0x66, 0x7C, 0x00, 0x00, 0x00, 0x3C, 0x60, 0x60, 0x60, 0x3C, 0x00,
//    0x00, 0x06, 0x06, 0x3E, 0x66, 0x66, 0x3E, 0x00, 0x00, 0x00, 0x3C, 0x66, 0x7E, 0x60, 0x3C, 0x00,
//    0x00, 0x0E, 0x18, 0x3E, 0x18, 0x18, 0x18, 0x00, 0x00, 0x00, 0x3E, 0x66, 0x66, 0x3E, 0x06, 0x7C,
//    0x00, 0x60, 0x60, 0x7C, 0x66, 0x66, 0x66, 0x00, 0x00, 0x18, 0x00, 0x38, 0x18, 0x18, 0x3C, 0x00,
//    0x00, 0x06, 0x00, 0x06, 0x06, 0x06, 0x06, 0x3C, 0x00, 0x60, 0x60, 0x6C, 0x78, 0x6C, 0x66, 0x00,
//    0x00, 0x38, 0x18, 0x18, 0x18, 0x18, 0x3C, 0x00, 0x00, 0x00, 0x66, 0x7F, 0x7F, 0x6B, 0x63, 0x00,
//    0x00, 0x00, 0x7C, 0x66, 0x66, 0x66, 0x66, 0x00, 0x00, 0x00, 0x3C, 0x66, 0x66, 0x66, 0x3C, 0x00,
//    0x00, 0x00, 0x7C, 0x66, 0x66, 0x7C, 0x60, 0x60, 0x00, 0x00, 0x3E, 0x66, 0x66, 0x3E, 0x06, 0x06,
//    0x00, 0x00, 0x7C, 0x66, 0x60, 0x60, 0x60, 0x00, 0x00, 0x00, 0x3E, 0x60, 0x3C, 0x06, 0x7C, 0x00,
//    0x00, 0x18, 0x7E, 0x18, 0x18, 0x18, 0x0E, 0x00, 0x00, 0x00, 0x66, 0x66, 0x66, 0x66, 0x3E, 0x00,
//    0x00, 0x00, 0x66, 0x66, 0x66, 0x3C, 0x18, 0x00, 0x00, 0x00, 0x63, 0x6B, 0x7F, 0x3E, 0x36, 0x00,
//    0x00, 0x00, 0x66, 0x3C, 0x18, 0x3C, 0x66, 0x00, 0x00, 0x00, 0x66, 0x66, 0x66, 0x3E, 0x0C, 0x78,
//    0x00, 0x00, 0x7E, 0x0C, 0x18, 0x30, 0x7E, 0x00, 0x00, 0x18, 0x3C, 0x7E, 0x7E, 0x18, 0x3C, 0x00,
//    0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x00, 0x7E, 0x78, 0x7C, 0x6E, 0x66, 0x06, 0x00,
//    0x08, 0x18, 0x38, 0x78, 0x38, 0x18, 0x08, 0x00, 0x10, 0x18, 0x1C, 0x1E, 0x1C, 0x18, 0x10, 0x00
//};
//
///* ------------------------------------------------------------------------- */
///* Program state                                                              */
///* ------------------------------------------------------------------------- */
//
//static uint8_t board_data[BOARD_SIZE];
//
//static uint8_t fjnet_in_buff[FJNET_IN_SIZE];
//static uint8_t fjnet_out_buff[FJNET_OUT_SIZE];
//
//static uint8_t who_am_i;
//static uint8_t current_player;
//static uint8_t current_piece;
//static uint8_t cursor_col = 4;
//static uint8_t cursor_row = 3;
//
//static int8_t select_col = -1; // -1 means "nothing selected"
//static int8_t select_row = -1;
//
//
//static uint8_t game_id[9];
//
//
//static uint8_t player_id[9];
//
//static char mode =  'S';
//static char side  = 'W';
//static char level = '5';
//
//static char game_host[40];
//static char game_id_input[16];
//static char base_url[80];
//
//static uint16_t screen_addr;
//static uint16_t pm_base;
//static uint16_t p0_addr;
//static uint16_t p1_addr;
//static uint16_t old_y;
//static uint16_t old_sel_y;
//
///* ------------------------------------------------------------------------- */
///* Forward declarations                                                       */
///* ------------------------------------------------------------------------- */
//
//static void setup_video(void);
//static void setup_pm_graphics(void);
//static void setup_board(void);
//static void draw_board(void);
//static void draw_tile(uint8_t bx, uint8_t by);
//static void draw_piece(uint8_t bx, uint8_t by, uint8_t piece);
//static void place_tiles(uint8_t x, uint8_t y,
//                        uint8_t t1, uint8_t t2, uint8_t t3,
//                        uint8_t t4, uint8_t t5, uint8_t t6);
//static void move_cursor_pm(void);
//static void uci_move(uint8_t x1, uint8_t y1,
//                     uint8_t x2, uint8_t y2,
//                     uint8_t promotion);
//
//static uint8_t do_post(const char *endpoint,
//                       const uint8_t *buffer,
//                       uint8_t buffer_len);
//static uint8_t do_get(const char *endpoint, const char *param);
//
//static void pause_frames(uint8_t frames);
//static void screen_clear_row(uint8_t row);
//static void screen_put_at(uint8_t x, uint8_t y, uint8_t atascii);
//static void screen_print_at(uint8_t x, uint8_t y, const char *text);
//static void screen_input_at(uint8_t x, uint8_t y,
//                            const char *prompt,
//                            char *buffer,
//                            uint8_t max_len);
//
///* ------------------------------------------------------------------------- */
///* Basic Atari screen helpers                                                 */
///* ------------------------------------------------------------------------- */
//
//static uint8_t atascii_to_screen(uint8_t c)
//{
//    uint8_t inverse = c & 0x80;
//    uint8_t low = c & 0x7F;
//
//    /*
//     * Atari internal screen-code conversion.
//     * Important here because the FastBasic source PRINTed CHR$($21), while
//     * this C version writes directly to screen RAM.
//     */
//    if (low < 32) {
//        low = (uint8_t)(low + 64);
//    } else if (low < 96) {
//        low = (uint8_t)(low - 32);
//    }
//
//    return (uint8_t)(inverse | low);
//}
//
//
//static void screen_put_at(uint8_t x, uint8_t y, uint8_t atascii)
//{
//    volatile uint8_t *screen = (volatile uint8_t *)screen_addr;
//
//    screen[(uint16_t)y * 40u + x] = atascii_to_screen(atascii);
//}
//
//
//static void screen_print_at(uint8_t x, uint8_t y, const char *text)
//{
//    while (*text != '\0' && x < 40) {
//        screen_put_at(x++, y, (uint8_t)*text++);
//    }
//}
//
//
//static void screen_clear_row(uint8_t row)
//{
//    uint8_t x;
//
//    for (x = 0; x < 40; ++x) {
//        screen_put_at(x, row, (uint8_t)' ');
//    }
//}
//
//
///*
// * Minimal line editor that works without relying on a text window.
// * This is intentionally simple because it only services the startup prompts.
// */
//static void screen_input_at(uint8_t x, uint8_t y,
//                            const char *prompt,
//                            char *buffer,
//                            uint8_t max_len)
//{
//    uint8_t n = 0;
//    uint8_t prompt_len = (uint8_t)strlen(prompt);
//    unsigned char c;
//
//    screen_clear_row(y);
//    screen_print_at(x, y, prompt);
//
//    if (max_len == 0) {
//        return;
//    }
//
//    for (;;) {
//        c = cgetc();
//
//        if (c == '\r' || c == '\n' || c == 0x9B) {
//            break;
//        }
//
//        /* ATASCII delete/backspace and ASCII BS. */
//        if ((c == 0x7E || c == 0x08) && n > 0) {
//            --n;
//            screen_put_at((uint8_t)(x + prompt_len + n), y, ' ');
//            continue;
//        }
//
//        if (c >= 32 && c < 127 && n < (uint8_t)(max_len - 1)) {
//            buffer[n++] = (char)c;
//            screen_put_at((uint8_t)(x + prompt_len + n - 1), y, c);
//        }
//    }
//
//    buffer[n] = '\0';
//}
//
//
//static void pause_frames(uint8_t frames)
//{
//    while (frames--) {
//        waitvsync();
//    }
//}
//
///* ------------------------------------------------------------------------- */
///* Video / character set                                                     */
///* ------------------------------------------------------------------------- */
//
//static void setup_video(void)
//{
//    uint8_t original_ramtop;
//    uint8_t font_page;
//    uint8_t pm_page;
//    uint16_t dl;
//    uint8_t i;
//
//    /*
//     * FastBasic reserved four pages for the custom character set.
//     *
//     * We reserve eight pages here before calling _graphics():
//     *     4 pages = 1 KB double-line PMG area
//     *     4 pages = 1 KB character set
//     *
//     * Keeping both regions above RAMTOP prevents the CIO graphics setup from
//     * placing its screen/display-list memory on top of them.
//     */
//    original_ramtop = PEEK(RAMTOP_ADDR);
//
//    font_page = (uint8_t)(original_ramtop - 4);
//    pm_page   = (uint8_t)(font_page - 4);
//
//    /* A double-line PM area needs a 1 KB boundary. */
//    pm_page &= 0xFC;
//
//    POKE(RAMTOP_ADDR, pm_page);
//
//    // antic 4 all over
//    _graphics(12 + 16);
//
//    screen_addr = PEEKW(SAVMSC_ADDR);
//
//    pm_base = (uint16_t)pm_page << 8;
//    font_page = (uint8_t)(pm_page + 4);
//
//    memcpy((void *)((uint16_t)font_page << 8), pieces_charset, sizeof(pieces_charset));
//
//    POKE(CHBAS_ADDR, font_page);
//
//    /* OG FastBasic:
//       dl = DPEEK(560) + 4
//       POKE dl-1,$46
//       POKE dl+2,$06
//       MSET dl+3,19,$04
//       ...
//     */
//    dl = (uint16_t)(PEEKW(SDLSTL_ADDR) + 4);
//
//    POKE(dl - 1, 0x46);
//    POKE(dl + 2, 0x06);
//
//    for (i = 0; i < 19; ++i) {
//        POKE(dl + 3 + i, 0x04);
//    }
//
//    POKE(dl + 22, 0x84);
//    POKE(dl + 23, 0x02);
//    POKE(dl + 24, 0x02);
//    POKE(dl + 25, 0x41);
//    POKE(dl + 26, (uint8_t)(PEEKW(SDLSTL_ADDR) & 0xFF));
//    POKE(dl + 27, (uint8_t)(PEEKW(SDLSTL_ADDR) >> 8));
//
//        _setcolor(0, 1, 8);
//        _setcolor(1, 12, 2);
//        _setcolor(2, 0, 0);
//        _setcolor(3, 0, 14);
//        _setcolor(4, 9, 0);
//
//      // TODO DLI color change after I figure out if this works with FujiNet SIO 
//      //
//}
//
///* ------------------------------------------------------------------------- */
///* Player/Missile cursor                                                      */
///* ------------------------------------------------------------------------- */
//
//static void setup_pm_graphics(void)
//{
//    /*Double-line PM memory map:
//        +$0180 missiles
//        +$0200 player 0
//        +$0280 player 1
//     */
//    memset((void *)pm_base, 0, 1024);
//
//    p0_addr = pm_base + 0x0200;
//    p1_addr = pm_base + 0x0280;
//
//    POKE(PMBASE_ADDR, (uint8_t)(pm_base >> 8));
//
//    /*
//     * Existing playfield is normal width ($22).
//     * Add player DMA ($08); bit 4 remains clear for double-line resolution.
//     */
//    POKE(SDMCTL_ADDR, (uint8_t)(PEEK(SDMCTL_ADDR) | 0x08));
//
//    /* Enable player DMA/display. Missiles are not needed here. */
//    POKE(GRACTL_ADDR, 0x02);
//
//    /* FastBasic SETCOLOR -4,3,14 and SETCOLOR -3,4,12. */
//    POKE(COLPM0_ADDR, 0x3E);
//    POKE(COLPM1_ADDR, 0x4C);
//
//    old_y = p0_addr + cursor_y[cursor_row];
//    old_sel_y = p1_addr;
//
//    move_cursor_pm();
//}
//
//
//static void move_cursor_pm(void)
//{
//    waitvsync();
//
//    memset((void *)old_y, 0, 6);
//
//    POKE(HPOSP0_ADDR, cursor_x[cursor_col]);
//    old_y = p0_addr + cursor_y[cursor_row];
//    memcpy((void *)old_y, pm_cursor_data, 6);
//
//    if (select_col >= 0) {
//        POKE(HPOSP1_ADDR, cursor_x[(uint8_t)select_col]);
//        old_sel_y = p1_addr + cursor_y[(uint8_t)select_row];
//        memcpy((void *)old_sel_y, pm_select_data, 6);
//    } else {
//        memset((void *)old_sel_y, 0, 6);
//    }
//}
//
///* ------------------------------------------------------------------------- */
///* Board setup / rendering                                                    */
///* ------------------------------------------------------------------------- */
//
//static void setup_board(void)
//{
//    memset(board_data, 0, sizeof(board_data));
//
//    board_data[0] = WHITE_ROOK;
//    board_data[1] = WHITE_KNIGHT;
//    board_data[2] = WHITE_BISHOP;
//    board_data[3] = WHITE_QUEEN;
//    board_data[4] = WHITE_KING;
//    board_data[5] = WHITE_BISHOP;
//    board_data[6] = WHITE_KNIGHT;
//    board_data[7] = WHITE_ROOK;
//
//    memset(&board_data[8], WHITE_PAWN, 8);
//
//    memset(&board_data[48], BLACK_PAWN, 8);
//
//    board_data[56] = BLACK_ROOK;
//    board_data[57] = BLACK_KNIGHT;
//    board_data[58] = BLACK_BISHOP;
//    board_data[59] = BLACK_QUEEN;
//    board_data[60] = BLACK_KING;
//    board_data[61] = BLACK_BISHOP;
//    board_data[62] = BLACK_KNIGHT;
//    board_data[63] = BLACK_ROOK;
//}
//
//
//static void place_tiles(uint8_t x, uint8_t y,
//                        uint8_t t1, uint8_t t2, uint8_t t3,
//                        uint8_t t4, uint8_t t5, uint8_t t6)
//{
//    /*
//     * The FastBasic version prints the third character twice because of
//     * inverse-video behavior. Keep that quirk here.
//     */
//    screen_put_at(x,     y,     t1);
//    screen_put_at(x + 1, y,     t2);
//    screen_put_at(x + 2, y,     t3);
//    screen_put_at(x + 2, y,     t3);
//
//    screen_put_at(x,     y + 1, t4);
//    screen_put_at(x + 1, y + 1, t5);
//    screen_put_at(x + 2, y + 1, t6);
//    screen_put_at(x + 2, y + 1, t6);
//}
//
//
//static void draw_tile(uint8_t bx, uint8_t by)
//{
//    uint8_t x = tile_x[bx];
//    uint8_t y = tile_y[by];
//    uint8_t tile = (((bx & 1) ^ (by & 1)) != 0) ? 0x21 : 0x22;
//
//    place_tiles(x, y, tile, tile, tile, tile, tile, tile);
//}
//
//
//static void draw_piece(uint8_t bx, uint8_t by, uint8_t piece)
//{
//    uint8_t x = tile_x[bx];
//    uint8_t y = tile_y[by];
//    uint8_t color_adjust = (piece < 'a') ? 0x80 : 0x00;
//    uint8_t light = (uint8_t)((bx & 1) ^ (by & 1));
//
//    uint8_t t1 = 0;
//    uint8_t t2 = 0;
//    uint8_t t3 = 0;
//    uint8_t t4 = 0;
//    uint8_t t5 = 0;
//    uint8_t t6 = 0;
//
//    if (piece == WHITE_PAWN || piece == BLACK_PAWN) {
//        if (light) {
//            t1=0x21; t2=0x24; t3=0x21;
//            t4=0x21; t5=0x23; t6=0x21;
//        } else {
//            t1=0x22; t2=0x26; t3=0x22;
//            t4=0x22; t5=0x25; t6=0x22;
//        }
//    } else if (piece == WHITE_KNIGHT || piece == BLACK_KNIGHT) {
//        if (light) {
//            t1=0x16; t2=0x17; t3=0x18;
//            t4=0x14; t5=0x09; t6=0x15;
//        } else {
//            t1=0x1E; t2=0x1C; t3=0x1D;
//            t4=0x19; t5=0x0F; t6=0x1A;
//        }
//    } else if (piece == WHITE_BISHOP || piece == BLACK_BISHOP) {
//        if (light) {
//            t1=0x0B; t2=0x0C; t3=0x0D;
//            t4=0x08; t5=0x09; t6=0x0A;
//        } else {
//            t1=0x11; t2=0x12; t3=0x13;
//            t4=0x0E; t5=0x0F; t6=0x10;
//        }
//    } else if (piece == WHITE_ROOK || piece == BLACK_ROOK) {
//        if (light) {
//            t1=0x21; t2=0x04; t3=0x05;
//            t4=0x27; t5=0x28; t6=0x29;
//        } else {
//            t1=0x22; t2=0x06; t3=0x07;
//            t4=0x2D; t5=0x2E; t6=0x2F;
//        }
//    } else if (piece == WHITE_QUEEN || piece == BLACK_QUEEN) {
//        if (light) {
//            t1=0x2A; t2=0x2B; t3=0x2C;
//            t4=0x27; t5=0x28; t6=0x29;
//        } else {
//            t1=0x5B; t2=0x5C; t3=0x5D;
//            t4=0x2D; t5=0x2E; t6=0x2F;
//        }
//    } else if (piece == WHITE_KING || piece == BLACK_KING) {
//        if (light) {
//            t1=0x5E; t2=0x5F; t3=0x00;
//            t4=0x27; t5=0x28; t6=0x29;
//        } else {
//            t1=0x01; t2=0x02; t3=0x03;
//            t4=0x2D; t5=0x2E; t6=0x2F;
//        }
//    } else {
//        draw_tile(bx, by);
//        return;
//    }
//
//    t1 = (uint8_t)(t1 + color_adjust);
//    t2 = (uint8_t)(t2 + color_adjust);
//    t3 = (uint8_t)(t3 + color_adjust);
//    t4 = (uint8_t)(t4 + color_adjust);
//    t5 = (uint8_t)(t5 + color_adjust);
//    t6 = (uint8_t)(t6 + color_adjust);
//
//    place_tiles(x, y, t1,t2,t3,t4,t5,t6);
//}
//
//
//static void draw_board(void)
//{
//    uint8_t row;
//    uint8_t col;
//    uint8_t index = 0;
//
//    for (row = 0; row < 8; ++row) {
//        for (col = 0; col < 8; ++col) {
//            if (board_data[index] == 0) {
//                draw_tile(col, row);
//            } else {
//                draw_piece(col, row, board_data[index]);
//            }
//
//            ++index;
//        }
//    }
//}
//
///* ------------------------------------------------------------------------- */
///* Local execution of a server-approved UCI move                              */
///* ------------------------------------------------------------------------- */
//
//static void uci_move(uint8_t x1, uint8_t y1,
//                     uint8_t x2, uint8_t y2,
//                     uint8_t promotion)
//{
//    uint8_t piece;
//
//    (void)promotion; /* TODO: promotion support, matching the FastBasic TODO. */
//
//    piece = board_data[x1 + (uint8_t)(y1 * 8)];
//
//    board_data[x1 + (uint8_t)(y1 * 8)] = 0;
//    board_data[x2 + (uint8_t)(y2 * 8)] = piece;
//
//    draw_tile(x1, y1);
//    draw_piece(x2, y2, piece);
//
//    /*
//     * Castling side effects copied directly from the FastBasic client.
//     * The server remains authoritative about whether the king move is legal.
//     */
//
//    if (piece == WHITE_KING &&
//        x1 == 4 && y1 == 0 && x2 == 6 && y2 == 0) {
//
//        board_data[7] = 0;
//        board_data[5] = WHITE_ROOK;
//        draw_tile(7, 0);
//        draw_piece(5, 0, WHITE_ROOK);
//
//    } else if (piece == WHITE_KING &&
//               x1 == 4 && y1 == 0 && x2 == 2 && y2 == 0) {
//
//        board_data[0] = 0;
//        board_data[3] = WHITE_ROOK;
//        draw_tile(0, 0);
//        draw_piece(3, 0, WHITE_ROOK);
//
//    } else if (piece == BLACK_KING &&
//               x1 == 4 && y1 == 7 && x2 == 6 && y2 == 7) {
//
//        board_data[63] = 0;
//        board_data[61] = BLACK_ROOK;
//        draw_tile(7, 7);
//        draw_piece(5, 7, BLACK_ROOK);
//
//    } else if (piece == BLACK_KING &&
//               x1 == 4 && y1 == 7 && x2 == 2 && y2 == 7) {
//
//        board_data[56] = 0;
//        board_data[59] = BLACK_ROOK;
//        draw_tile(0, 7);
//        draw_piece(3, 7, BLACK_ROOK);
//    }
//}
//
///* ------------------------------------------------------------------------- */
///* FujiNet                                                                    */
///* ------------------------------------------------------------------------- */
//
//static void make_url(char *dst, const char *endpoint)
//{
//    strcpy(dst, "N2:HTTP://");
//    strcat(dst, base_url);
//    strcat(dst, endpoint);
//}
//
//
//static uint8_t read_response(const char *url)
//{
//    int16_t result;
//
//    memset(fjnet_in_buff, 0, sizeof(fjnet_in_buff));
//
//    /*
//     * network_read() blocks until the requested amount is read or EOF.
//     * fn_bytes_read reports the actual count at EOF.
//     */
//    result = network_read(url, fjnet_in_buff, sizeof(fjnet_in_buff) - 1);
//
//    if (result < 0) {
//        fjnet_in_buff[0] = 0;
//        return 0;
//    }
//
//    if (fn_bytes_read < sizeof(fjnet_in_buff)) {
//        fjnet_in_buff[fn_bytes_read] = 0;
//    } else {
//        fjnet_in_buff[sizeof(fjnet_in_buff) - 1] = 0;
//    }
//
//    return (fn_bytes_read != 0);
//}
//
//
//static uint8_t do_post(const char *endpoint,
//                       const uint8_t *buffer,
//                       uint8_t buffer_len)
//{
//    char url[URL_SIZE];
//    uint8_t ok = 0;
//
//    make_url(url, endpoint);
//
//    if (network_open(url, FJNET_POST_MODE, FJNET_TRANSL) != FN_ERR_OK) {
//        return 0;
//    }
//
//    if (network_http_post_bin(url, buffer, buffer_len) == FN_ERR_OK) {
//        ok = read_response(url);
//    }
//
//    network_close(url);
//    return ok;
//}
//
//
//static uint8_t do_get(const char *endpoint, const char *param)
//{
//    char url[URL_SIZE];
//
//    make_url(url, endpoint);
//    strcat(url, "?");
//    strcat(url, param);
//
//    if (network_open(url, FJNET_GET_MODE, FJNET_TRANSL) != FN_ERR_OK) {
//        return 0;
//    }
//
//    if (!read_response(url)) {
//        network_close(url);
//        return 0;
//    }
//
//    network_close(url);
//    return 1;
//}
//
///* ------------------------------------------------------------------------- */
///* Startup game negotiation                                                   */
///* ------------------------------------------------------------------------- */
//
//static uint8_t is_legal_response(void)
//{
//    return fjnet_in_buff[0] == 'l' &&
//           fjnet_in_buff[1] == 'e' &&
//           fjnet_in_buff[2] == 'g' &&
//           fjnet_in_buff[3] == 'a' &&
//           fjnet_in_buff[4] == 'l';
//}
//
//
//static void show_player(void)
//{
//    screen_clear_row(1);
//
//    if (current_player == 1) {
//        screen_print_at(0, 1, "PLAYER: ONE");
//    } else if (current_player == 2) {
//        screen_print_at(0, 1, "PLAYER: TWO");
//    } else {
//        screen_print_at(0, 1, "PLAYER:");
//    }
//}
//
//
//static void setup_network_game(void)
//{
//    char answer[16];
//
//    screen_input_at(0, 21, "Enter Host:", game_host, sizeof(game_host));
//
//    mode = 'S';
//    side = 'W';
//    level = '5';
//    game_id_input[0] = '\0';
//
//    screen_input_at(0, 21, "Start or Join Game?:", answer, sizeof(answer));
//
//    if (answer[0] == 'S' || answer[0] == 's') {
//        screen_input_at(0, 21, "1 or 2 Players?:", answer, sizeof(answer));
//
//        if (answer[0] == '2') {
//            mode = 'D';
//        } else {
//            screen_input_at(0, 21, "Skill Level 1-10:",
//                            answer, sizeof(answer));
//
//            if (answer[0] != '\0') {
//                level = answer[0];
//            }
//        }
//    } else {
//        screen_input_at(0, 21, "Enter GameID:",
//                        game_id_input, sizeof(game_id_input));
//    }
//
//    if (strlen(game_host) > 8) {
//        strcpy(base_url, game_host);
//        strcat(base_url, SERVER_PORT);
//    } else {
//        strcpy(base_url, DEFAULT_HOST SERVER_PORT);
//    }
//
//    memset(fjnet_in_buff, 0, sizeof(fjnet_in_buff));
//    memset(fjnet_out_buff, 0, sizeof(fjnet_out_buff));
//    memset(game_id, 0, sizeof(game_id));
//    memset(player_id, 0, sizeof(player_id));
//
//    if (strlen(game_id_input) < 8) {
//        /* Start a game. */
//        fjnet_out_buff[0] = (uint8_t)mode;
//        fjnet_out_buff[1] = 10;
//        fjnet_out_buff[2] = (uint8_t)side;
//        fjnet_out_buff[3] = 10;
//        fjnet_out_buff[4] = (uint8_t)level;
//        fjnet_out_buff[5] = 10;
//
//        if (do_post("newgame", fjnet_out_buff, 6)) {
//            /*
//             * Expected old protocol response:
//             *   8-byte game id
//             *   separator
//             *   8-byte player id
//             */
//            memcpy(fjnet_out_buff, fjnet_in_buff, 17);
//
//            fjnet_out_buff[8] = 10;
//            fjnet_out_buff[17] = 10;
//
//            memcpy(game_id, fjnet_in_buff, 8);
//            game_id[8] = '\0';
//
//            memcpy(player_id, fjnet_in_buff + 9, 8);
//            player_id[8] = '\0';
//
//            screen_print_at(4, 0, (char *)game_id);
//        } else {
//            screen_clear_row(21);
//            screen_print_at(0, 21, "Unable to connect");
//        }
//
//        who_am_i = 1;
//    } else {
//        /* Join an existing game. */
//        mode = 'D';
//        side = 'W';
//
//        memcpy(fjnet_out_buff, game_id_input, 8);
//        fjnet_out_buff[8] = 10;
//
//        if (do_post("joingame", fjnet_out_buff, 9)) {
//            memcpy(fjnet_out_buff + 9, fjnet_in_buff, 8);
//            fjnet_out_buff[17] = 10;
//
//            memcpy(game_id, game_id_input, 8);
//            game_id[8] = '\0';
//
//            memcpy(player_id, fjnet_in_buff, 8);
//            player_id[8] = '\0';
//        } else {
//            screen_clear_row(21);
//            screen_print_at(0, 21, "Unable to connect");
//        }
//
//        who_am_i = 2;
//    }
//
//    current_player = 0;
//    show_player();
//}
//
///* ------------------------------------------------------------------------- */
///* Turn handling                                                              */
///* ------------------------------------------------------------------------- */
//
//static void submit_selected_move(void)
//{
//    /*
//     * Bytes 0..17 already contain:
//     *
//     *      GAMEID\nPLAYERID\n
//     *
//     * Add the four-character UCI move and newline at 18..22.
//     */
//    fjnet_out_buff[18] = (uint8_t)(FILE_X + (uint8_t)select_col);
//    fjnet_out_buff[19] = (uint8_t)(RANK_Y + (uint8_t)select_row);
//    fjnet_out_buff[20] = (uint8_t)(FILE_X + cursor_col);
//    fjnet_out_buff[21] = (uint8_t)(RANK_Y + cursor_row);
//    fjnet_out_buff[22] = 10;
//
//    if (do_post("move", fjnet_out_buff, 23)) {
//        if (is_legal_response()) {
//            uci_move((uint8_t)select_col,
//                     (uint8_t)select_row,
//                     cursor_col,
//                     cursor_row,
//                     0);
//
//            if (mode == 'S') {
//                current_player = 2;
//            } else if (mode == 'D') {
//                current_player = (current_player == 1) ? 2 : 1;
//                show_player();
//            }
//        } else {
//            screen_clear_row(21);
//            screen_print_at(0, 21, "Invalid move");
//        }
//    } else {
//        screen_clear_row(21);
//        screen_print_at(0, 21, "Lost connection");
//    }
//
//    select_col = -1;
//    select_row = -1;
//    memset((void *)old_sel_y, 0, 6);
//
//    pause_frames(10);
//}
//
//
//static void handle_my_turn(void)
//{
//    uint8_t stick;
//
//    if (PEEK(STRIG0_ADDR) == 0) {
//        if (select_col < 0) {
//            current_piece =
//                board_data[cursor_col + (uint8_t)(cursor_row * 8)];
//
//            if ((who_am_i == 1 &&
//                 current_piece >= WHITE_BISHOP &&
//                 current_piece <= WHITE_ROOK) ||
//                (who_am_i == 2 &&
//                 current_piece >= BLACK_BISHOP &&
//                 current_piece <= BLACK_ROOK)) {
//
//                select_col = (int8_t)cursor_col;
//                select_row = (int8_t)cursor_row;
//
//                _sound(0, 104, 10, 4);
//                pause_frames(1);
//                _sound(0, 0, 0, 0);
//            } else {
//                _sound(0, 32, 2, 4);
//                pause_frames(1);
//                _sound(0, 0, 0, 0);
//            }
//
//            pause_frames(9);
//        } else {
//            submit_selected_move();
//        }
//    }
//
//    stick = PEEK(STICK0_ADDR);
//
//    if (stick == 15) {
//        _sound(0, 0, 0, 0);
//        return;
//    }
//
//    /*
//     * Preserve the original exact-direction joystick behavior:
//     *   7  = right
//     *   11 = left
//     *   14 = up
//     *   13 = down
//     */
//    if (stick == 7) {
//        cursor_col = (cursor_col == 7) ? 0 : (uint8_t)(cursor_col + 1);
//    } else if (stick == 11) {
//        cursor_col = (cursor_col == 0) ? 7 : (uint8_t)(cursor_col - 1);
//    }
//
//    if (stick == 14) {
//        cursor_row = (cursor_row == 7) ? 0 : (uint8_t)(cursor_row + 1);
//    } else if (stick == 13) {
//        cursor_row = (cursor_row == 0) ? 7 : (uint8_t)(cursor_row - 1);
//    }
//
//    _sound(0, 180, 2, 4);
//    pause_frames(1);
//    _sound(0, 0, 0, 0);
//    pause_frames(9);
//
//    move_cursor_pm();
//}
//
//
//static void poll_other_player(void)
//{
//    char query[20];
//
//    strcpy(query, "gid=");
//    strcat(query, (char *)game_id);
//
//    fjnet_in_buff[0] = 0;
//
//    if (!do_get("status", query)) {
//        return;
//    }
//
//    /*
//     * Old fixed-position status protocol:
//     *
//     * TURN w:LAST e7e6-:MVNO 2
//     * 0123456789012345678901234
//     */
//    if (fjnet_in_buff[0] == 'T') {
//        if (fjnet_in_buff[5] == 'w') {
//            current_player = 1;
//        } else if (fjnet_in_buff[5] == 'b') {
//            current_player = 2;
//        } else {
//            current_player = 0;
//        }
//
//        if (current_player == who_am_i && fjnet_in_buff[12] != '-') {
//            uci_move((uint8_t)(fjnet_in_buff[12] - FILE_X),
//                     (uint8_t)(fjnet_in_buff[13] - RANK_Y),
//                     (uint8_t)(fjnet_in_buff[14] - FILE_X),
//                     (uint8_t)(fjnet_in_buff[15] - RANK_Y),
//                     fjnet_in_buff[16]);
//        }
//
//        show_player();
//
//    } else if (fjnet_in_buff[0] == 'O') {
//        /*
//         * Old response example:
//         *
//         * OVER 0-1 1:TURN w:LAST d8h4:MVNO 4
//         */
//        screen_clear_row(1);
//
//        if (fjnet_in_buff[7] == '0') {
//            screen_print_at(0, 1, "PLAYER ONE WINS");
//        } else if (fjnet_in_buff[7] == '1') {
//            screen_print_at(0, 1, "PLAYER TWO WINS");
//        } else if (fjnet_in_buff[7] == '2') {
//            screen_print_at(0, 1, "DRAW");
//        }
//
//        if (fjnet_in_buff[23] >= 'a' && fjnet_in_buff[23] <= 'h') {
//            uci_move((uint8_t)(fjnet_in_buff[23] - FILE_X),
//                     (uint8_t)(fjnet_in_buff[24] - RANK_Y),
//                     (uint8_t)(fjnet_in_buff[25] - FILE_X),
//                     (uint8_t)(fjnet_in_buff[26] - RANK_Y),
//                     0);
//        }
//    }
//
//    if (current_player != who_am_i) {
//        memset((void *)old_y, 0, 6);
//        memset((void *)old_sel_y, 0, 6);
//        pause_frames(120);
//    } else {
//        move_cursor_pm();
//    }
//}
//
/* ------------------------------------------------------------------------- */
/* main                                                                       */
/* ------------------------------------------------------------------------- */

static void setup_charset() {
    uint16_t i = 0;
    ///////////////////////////////////////////////////
    // SETUP CHARSET //////////////////////////////////
    uint16_t addr = (uint16_t)( font);
    OS.chbas = addr >> 8;
    memset( screen_memory, 0, 1024 );
    for( i=0; i < 64; ++i ) { 
    screen_memory[20+i] = i;
      }
}

static void setup_pm_graphics() {
     ANTIC.pmbase = (uint16_t) pmg_memory  >> 8;
    memset( pmg_memory, 0, 1024 );
   memcpy( pmg_memory, pmg_data, sizeof( pmg_data ) );
      GTIA_WRITE.gractl = 0x1B; 
      OS.sdmctl = 0x2D;
   //GTIA_WRITE.gractl = 0x03; // POKE 53277,e players and missile
//  OS.gprior = 4;
//
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
  GTIA_WRITE.hposp3=0x9C;                                                GTIA_WRITE.hposm0 = 0xBC;
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
    init_dlist();
    setup_charset();
    

    //////////////////////////////////////////////////////////////
    // setup sprites
//    setup_pm_graphics();

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

