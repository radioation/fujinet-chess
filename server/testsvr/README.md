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

0. Run from the parent directory not fujifish
```bah
cd {YOUR_PATH}/fujinet-chess/server
```

1. setup python the way you would on your distro

```bash
sudo dnf install python3 python3-pip pytest

python3 -m venv env
. env/bin/activate

pip3 install dotenv requests
```

2. setup an env

```bash
cp .env_example .env
vim .env
```
you'll need to 
* change `LOBBY_CLIENT_APP_KEY=<YOU NEED A REGISTRED APP KEY HERE>  ` by giving it an APP key 
* set the `GAME_CLIENT_PLATFORMS` path to the game client 


3. run it

```bash
 python -m testsvr.run
```

## Game client

go to `fujinet-chess/server/testclient` folder

```bash
fb minclient.bas
```
copy the resulting xex to your tnfs folder


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

