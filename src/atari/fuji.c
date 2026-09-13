#include "fuji.h"

#include <string.h>

char server[64];
char qtable[64];
char playerid[10];
static char url[128];


uint8_t doGet( char* path, char* buffer, int16_t buffer_len ) {
  static int16_t bytes_read;

  // create URL
  strcpy(url, "n:");
  strcat(url, server);
  strcat(url, path);
  strcat(url, qtable);

  // open network conn
  if (network_open(url, OPEN_MODE_HTTP_GET, OPEN_TRANS_NONE)) {
    return -1;
  }

  // read 
  bytes_read = network_read(url, buffer, buffer_len );
  network_close(url);
  return bytes_read;
}


uint8_t doPost( char* path, char* data, char* buffer, int16_t buffer_len ) {
  static int16_t bytes_read;

  // create URL
  strcpy(url, "n:");
  strcat(url, server);
  strcat(url, path);
  strcat(url, qtable);


  // open POST
  if (network_open(url, OPEN_MODE_HTTP_POST, OPEN_TRANS_NONE)) {
    return -1;
  }

  network_http_post(url, data );


  bytes_read = network_read(url, buffer, buffer_len );
  network_close(url);
  return bytes_read;


}
