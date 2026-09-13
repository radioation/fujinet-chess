#ifndef _CLIENT_H_
#define _CLIENT_H_

#include <stdint.h>

/*
 * */


// FujiNet AppKey settings. These should not be changed
#define AK_LOBBY_CREATOR_ID 1     // FUJINET Lobby
#define AK_LOBBY_APP_ID 1         // Lobby Enabled Game
#define AK_LOBBY_KEY_USERNAME 0   // Lobby Username key
#define AK_LOBBY_KEY_SERVER 241   // IMPORTANT: THIS MUST MATCH THE ID YOU REGISTERD. DON'T USE 241 ON THE REAL LOBBY.  YOUR SERVER MUST ALSO USE THIS AS `LOBBY_CLIENT_APP_KEY`

// Fujitzee
#define AK_CREATOR_ID 0x5364      // Eric Carr's creator id
#define AK_APP_ID 1               // Fujzee App ID
#define AK_KEY_PREFS 0            // Preferences



uint8_t doGet( char* q );
uint8_t doPost( char* q );

#endif // _CLIENT_H_
