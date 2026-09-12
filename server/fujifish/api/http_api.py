import os
import signal
import threading
import copy


from lobby.lobby_client import GameClient, LobbyClient, GamePlayer, get_lobby

import json



from flask import Flask, request, Response

from fujifish.api.chess_game import GameTable, ChessGame, create_table

#######################################################
#
# Flask HTTP interface
#
app = Flask(__name__)




class TableMutex:
    def __init__(self) -> None:
        self._locks: Dict[str, threading.Lock] = {}
        self._guard = threading.Lock()

    def Lock(self, key: str) -> Callable[[], None]:
        # Return an unlock() closure (to mirror Go style)

        with self._guard:
            lock = self._locks.get(key)
            if lock is None:
                lock = threading.Lock()
                self._locks[key] = lock
        lock.acquire()

        unlocked = False

        def unlock() -> None:
            nonlocal unlocked # not part of inner function
            if not unlocked:
                lock.release()
                unlocked = True

        # return unlock() for unlocking.
        return unlock


STATE_MAP: Dict[ str, ChessGame ] = {}
TABLES : List[GameTable]  = []
table_mutex = TableMutex()


def initialize_tables():

    tables_json = os.getenv("GAME_SERVER_TABLES", "" )
    print(tables_json)
    raw_list = json.loads( tables_json )
    for table in raw_list:
        servername = table.get("servername")
        instance_url_suffix = table.get("instance_url_suffix").lower()
        bot_level = int(table.get("bot_level"))
        register_lobby = int(table.get("register_lobby"))
        table_obj, chess_game = create_table( servername, instance_url_suffix, bot_level, register_lobby )
        TABLES.append(table_obj)
        STATE_MAP[ instance_url_suffix ] =chess_game 
        chess_game.update_lobby()


def get_state( table:str ) -> Tuple[ Optional[ChessGame] ]:
    tbl = table.lower()
    #plyr = ""
    #if len(player) > 0:
    #    plyr = player.lower()

    unlock_fcn = table_mutex.Lock( tbl )
    state = None
    tmp_state = STATE_MAP.get( tbl )
    if tmp_state is not None:
        state = copy.deepcopy( tmp_state )
        #state.set_client_player_by_name( plyr )
    unlock_fcn()
    return state

def get_game( table:str ) -> Tuple[ Optional[ChessGame], Callable[ [], None]]:
    tbl = table.lower()
    #plyr = ""
    #if len(player) > 0:
    #    plyr = player.lower()

    unlock_fcn = table_mutex.Lock( tbl )
    state = STATE_MAP.get( tbl )
    #if tmp_state is not None:
    #    state = copy.deepcopy( tmp_state )
    #    #state.set_client_player_by_name( plyr )

    return state, unlock_fcn

def save_state( state: ChessGame ):
    STATE_MAP[ state.table] = state


def cleanup():
    for table, state in STATE_MAP.items():
        try:
            unlock_fcn = table_mutex.Lock( table )
            print("Try delete: " + state.servername )
            state.delete_from_lobby()
            unlock_fcn()
        except Exception as e:
            print(f"[cleanup] failed to delete lobby for table={getattr(gs, 'table', '?')}: {e}")


# Register the handlers
def shutdown_handler( signal_int, frame ):
    print(f"Caught signal: {signal_int}")
    cleanup()
    sys.exit(0)

signal.signal(signal.SIGINT, shutdown_handler)
signal.signal(signal.SIGTERM, shutdown_handler)





#@app.post("/newgame")
#def http_newgame():
#    body = request.get_data(as_text=True) or ""
#    print ("BODY: " + body)
#    mode = 'S'
#    player_1_side = 'W'
#    level = 3
#    lines = [ln.strip() for ln in body.splitlines() if ln.strip() != ""]
#    print(len(lines))
#    
#    if len(lines) > 0:
#        # first line shoudl be mode.
#        if lines[0] == 'D':
#            mode = 'D'
#        if len(lines) > 1:
#            if lines[1] == 'B':
#                player_1_side = 'B'
#            if len(lines) > 2:
#                level = int(lines[2] )
#    g = new_game(mode, player_1_side, level )
#    print( f"new game: {g.id} mode: {g.mode} side: {g.player_1_side}")
#    print( f"   p1: {g.player_1_id} p2: {g.player_2_id}")
#    return Response(g.id + ":" + g.player_1_id + "\n", mimetype="text/plain")


@app.post("/joingame")
def http_joingame():
    body = request.get_data(as_text=True) or ""
    tbl = request.args.get("table")
    if tbl is None:
        return Response("invalid gid\n", mimetype="text/plain", status=400)
    print ("BODY: " + body)
    print ("table: " + tbl)
    lines = [ln.strip() for ln in body.splitlines() if ln.strip() != ""]
    if len(lines) > 0 :
        #game = get_game(gid)
        game, unlock = get_game(tbl)
        try:
            if game is not None:
                # try to join
                playerid,side = game.join_game(lines[0], lines[1] )
                return Response( playerid + "\n" + side +"\n", mimetype="text/plain")
        finally:
            unlock()
        return Response("table not found\n", mimetype="text/plain", status=404)

            
    else:
        return Response("invalid\n", mimetype="text/plain", status=400)


@app.post("/move")
def http_move():
    """
    Body format (plain text, LF line endings):
        <uci_move>\n
        [<movetime_ms>]\n        # optional, default 300
    Returns:
        - "invalid move\n"           if POST body is invalid.
        - "illegal move\n"           if move is not legal.
        - "<bestmove>\n"             if valid, give best move from stockfish (single player will move)
    """
    body = request.get_data(as_text=True) or ""
    tbl = request.args.get("table")
    if tbl is None:
        return Response("invalid table\n", mimetype="text/plain", status=400)

    game, unlock = get_game(tbl)
    if game is None:
        return Response("table not found\n", mimetype="text/plain", status=404)

    try:
        print ("BODY: " + body)
        print ("table: " + tbl)
        lines = [ln.strip() for ln in body.splitlines() if ln.strip() != ""]
 
        # need both player and the UCI move )
        if len(lines) < 2:
            return Response("invalid format\n", mimetype="text/plain", status=400)
       
        pid,  uci_move = lines[0], lines[1].lower()
 
        if not pid.isalnum() or ( pid.isalnum() and  len(pid) != 8 ):
            return Response("invalid format - p\n", mimetype="text/plain", status=400)
        if not uci_move.isalnum() or len(uci_move) > 5 or len(uci_move) < 4:
            return Response("invalid format - m\n", mimetype="text/plain", status=400)
 
        movetime_ms = 300
        if len(lines) >= 3 :
            if lines[2].isdigit():
                movetime_ms = int(lines[3])
            else:
                return Response("invalid format - t\n", mimetype="text/plain", status=400)
 
 
        move_result = game.do_move( pid, uci_move, movetime_ms)
        if move_result['valid'] == True:
            #if 'engine_move' in move_result:
            #    return Response( move_result['engine_move'], mimetype="text/plain", status = 200)
            #else:
            return Response( move_result['message'], mimetype="text/plain", status = 200)
        else:
            return Response(move_result['message'], mimetype="text/plain", status=400)
    finally:
        unlock()


@app.get("/board")
def http_board():
    table = request.args.get('table')
    if not table: return Response("ERR missing table\n", mimetype="text/plain", status = 400 )
    game = get_state(table)
    if not game: return Response("ERR no game\n", mimetype="text/plain", status = 404 )

    return Response(str(game.board) + "\n", mimetype="text/plain")


@app.get("/status")
def http_status():
    table = request.args.get('table')
    print(" GOT TABLE " + table )
    if not table: return Response("ERR missing table\n", mimetype="text/plain", status = 400 )
    game = get_state(table)
    print(" GOT GAME " )
    if not game: return Response("ERR no game\n", mimetype="text/plain", status = 404 )

    print( game.state_line() )

    return Response(str(game.state_line()) + "\n", mimetype="text/plain")






