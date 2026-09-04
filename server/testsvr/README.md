# TL;DR
0. run a tnfd server with custom lobby client that points to 
  your own lobby server.
1. start lobby
2. start testsvr pointed at lobby
3. Use fujinet to select 


## What's going on here?
FujiNet-Wifi has two pieces
* fujinet-lobby client : talks to server to find active Game Servers
* fujinet-lobby server : Central registry for FujiNet Game system.

You have to provide 
* Game server : Your Game Server.  THis needs to know how 
  to talk to the `Fujinet Lobby Server` to 
* Game client : 

## Lobby client
0. download source from [github](https://github.com/FujiNetWIFI/fujinet-lobby)
1. modify `clients/src/main.c` to point to your server.

```diff
  -#define LOBBY_ENDPOINT "N:https://lobby.fujinet.online/view"
  -#define LOBBY_QA_ENDPOINT "N:https://qalobby.fujinet.online/view"
  -//#define LOBBY_QA_ENDPOINT "N:http://localhost:8080/view"
  +#define LOBBY_ENDPOINT "N:http://irata.greggallardo.com:8080/view"
  +#define LOBBY_QA_ENDPOINT "N:http://irata.greggallardo.com:8080/view"
```
2. build it 
```bash
    cd clients
   make atari
```
3. copy to your tnfs server folder

## Lobby server 
0. download from 
1. use make to run??
```bash
    cd server
    make
```

## Game Server



# Testing


```bash
 # get state without joining
 curl -X GET "http://10.25.50.61:5364/view?player=gus&table=lounge"

 # add two players to server
 curl -X POST "http://10.25.50.61:5364/state?player=henry&table=lounge"
 curl -X POST "http://10.25.50.61:5364/state?player=shawn&table=lounge"

 # leave server
 curl -X POST "http://10.25.50.61:5364/leave?player=henry&table=lounge"

 # rejoin server
 curl -X POST "http://10.25.50.61:5364/state?player=henry&table=lounge"

 # get state without joining
 curl -X GET "http://10.25.50.61:5364/view?player=gus&table=lounge"

 # post moves 
 curl -X POST "http://10.25.50.61:5364/move?player=henry&table=lounge&move=e2e4"
 curl -X POST "http://10.25.50.61:5364/move?player=shawn&table=lounge&move=e2e4"
 curl -X POST "http://10.25.50.61:5364/move?player=henry&table=lounge&move=e2e4"
```

