# IMPORTANT
* this is *NOT* meant to run against the REAL FujiNet lobby. Just the 
  lobby I'm running on irata.greggallardo.com
  * `AK_LOBBY_KEY_SERVER` should be changed to the id registered. Again,
     I"m not running against the real lobby, 240 is just a value I'm using 
     on my test lobby.
  
  when you read the server location use `AK_LOBBY_*` values

```c
 read_appkey(AK_LOBBY_CREATOR_ID,  AK_LOBBY_APP_ID, AK_LOBBY_KEY_SERVER, tempBuffer);
```

This will give you the URL which can be broken down to the server and query (containing the table)


